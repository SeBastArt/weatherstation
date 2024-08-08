using System.Globalization;
using System.Text.Json;
using Application.DataTransferObjects;
using Application.Interfaces;
using InfluxDB.Client;
using InfluxDB.Client.Api.Domain;
using InfluxDB.Client.Core.Flux.Domain;
using InfluxDB.Client.Writes;

namespace Application.Services;

public class InfluxDbService(IInfluxDbConfig config) : IInfluxDbService
{
    private readonly string _bucket = config.Bucket;
    private readonly string _host = config.Host;
    private readonly string _organization = config.Organization;
    private readonly string _token = config.Token;

    private string BuildTemperaturesOfTodayQuery(string eui)
    {
        return $"from(bucket: \"{_bucket}\")" +
               $"|> range(start: -23h) " +
               "|> filter(fn: (r) => r[\"_measurement\"] == \"weather\")" +
               "|> filter(fn: (r) => r[\"_field\"] == \"Temperature\")" +
               $"|> filter(fn: (r) => r[\"dev_eui\"] == \"{eui}\")" +
               "|> aggregateWindow(every: 1h, fn: mean, createEmpty: false) " +
               "|> yield(name: \"interpolated\")";
    }
    
    private string BuildHumidityOfTodayQuery(string eui)
    {
        return $"from(bucket: \"{_bucket}\")" +
               "|> range(start: 0)" +
               "|> filter(fn: (r) => r[\"_measurement\"] == \"weather\")" +
               "|> filter(fn: (r) => r[\"_field\"] == \"Humidity\")" +
               $"|> filter(fn: (r) => r[\"dev_eui\"] == \"{eui}\")" +
               "|> aggregateWindow(every: 1h, fn: mean, createEmpty: false) " +
               "|> yield(name: \"interpolated\")";
    }

    private string BuildHumidityQuery(string eui)
    {
        return $"from(bucket: \"{_bucket}\")" +
               "|> range(start: 0)" +
               "|> filter(fn: (r) => r[\"_measurement\"] == \"weather\")" +
               "|> filter(fn: (r) => r[\"_field\"] == \"Humidity\")" +
               $"|> filter(fn: (r) => r[\"dev_eui\"] == \"{eui}\")" +
               "|> group()" +
               "|> last()";
    }
    
    private string BuildBatteryQuery(string eui)
    {
        return $"from(bucket: \"{_bucket}\")" +
               "|> range(start: 0)" +
               "|> filter(fn: (r) => r[\"_measurement\"] == \"weather\")" +
               "|> filter(fn: (r) => r[\"_field\"] == \"Battery\")" +
               $"|> filter(fn: (r) => r[\"dev_eui\"] == \"{eui}\")" +
               "|> group()" +
               "|> last()";
    }
    
    private string BuildTemperatureQuery(string eui)
    {
        return $"from(bucket: \"{_bucket}\")" +
               "|> range(start: -1d)" +
               "|> filter(fn: (r) => r[\"_measurement\"] == \"weather\")" +
               "|> filter(fn: (r) => r[\"_field\"] == \"Temperature\")" +
               $"|> filter(fn: (r) => r[\"dev_eui\"] == \"{eui}\")" +
               "|> group()" +
               "|> last()";
    }
    
    public async Task<List<double>> GetTemperaturesOfToday(string eui)
    {
        var fluxQuery = BuildTemperaturesOfTodayQuery(eui);
        return await ExecuteListQuery(fluxQuery);
    }

    public Task<List<double>> GetHumidityOfToday(string eui)
    {
        var fluxQuery = BuildHumidityOfTodayQuery(eui);
        return ExecuteListQuery(fluxQuery);
    }


    public async Task<List<FluxTable>> QueryFluxAsync(string fluxQuery, string organization)
    {
        using var client = new InfluxDBClient(_host, _token);
        var queryApi = client.GetQueryApi();
        return await queryApi.QueryAsync(fluxQuery, organization);
    }

    public async Task<double> GetHumidity(string eui)
    {
        var fluxQuery = BuildHumidityQuery(eui);
        return await ExecuteQuery(fluxQuery);
    }

    public async Task<double> GetBattery(string eui)
    {
        var fluxQuery = BuildBatteryQuery(eui);
        return await ExecuteQuery(fluxQuery);
    }

    public async Task<double> GetTemperature(string eui)
    {
        var fluxQuery = BuildTemperatureQuery(eui);
        return await ExecuteQuery(fluxQuery);
    }
    
    private async Task<List<double>> ExecuteListQuery(string fluxQuery)
    {
        var tables = await QueryFluxAsync(fluxQuery, _organization);
        var interpolatedDataPoints = new List<double>();
        foreach (var fluxRow in tables.First().Records)
        {
            var value = float.Parse(fluxRow.GetValueByKey("_value").ToString() ?? string.Empty,
                CultureInfo.CurrentCulture);
            var roundedValue = Math.Round(value, 1);
            interpolatedDataPoints.Add(roundedValue);
        }

        return interpolatedDataPoints;
    }
    
    private async Task<double> ExecuteQuery(string fluxQuery)
    {
        var tables = await QueryFluxAsync(fluxQuery, _organization);
        return float.Parse(tables.First().Records.First().GetValue().ToString() ?? string.Empty,
            CultureInfo.CurrentCulture);
    }
    
    public void Write(Action<WriteApi> action)
    {
        using var client = new InfluxDBClient(_host, _token);
        using var write = client.GetWriteApi();
        action(write);
    }

    public void PostDataPoint(JsonElement data)
    {
        var theThingsPayload = data.Deserialize<TheThingsPayload<JsonElement>>();
        if (theThingsPayload == null) return;

        var weatherDataPoint = new WeatherDataPoint
        {
            DeviceId = theThingsPayload.EndDeviceIds.DeviceId,
            DevEui = theThingsPayload.EndDeviceIds.DevEui,
            Time = DateTime.UtcNow
        };

        var processorFactory = new PayloadProcessorFactory();
        var processor = processorFactory.GetProcessor(weatherDataPoint.DeviceId);
        processor.ProcessPayload(theThingsPayload.UplinkMessage.DecodedPayload, weatherDataPoint);

        Write(write =>
        {
            var point = PointData.Measurement("weather")
                .Tag("dev_eui", weatherDataPoint.DevEui)
                .Field("Temperature", weatherDataPoint.Temperature)
                .Field("Humidity", weatherDataPoint.Humidity)
                .Field("Battery", weatherDataPoint.Battery)
                .Timestamp(weatherDataPoint.Time, WritePrecision.Ns);

            write.WritePoint(point, _bucket, _organization);
        });
    }
    
}