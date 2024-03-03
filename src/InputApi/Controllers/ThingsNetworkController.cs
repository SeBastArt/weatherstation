using System.Globalization;
using System.Text.Json;
using InfluxDB.Client;
using InfluxDB.Client.Api.Domain;
using InfluxDB.Client.Core.Flux.Domain;
using InfluxDB.Client.Writes;
using InputApi.DataTransferObjects;
using InputApi.Helper;
using InputApi.Services;
using Microsoft.AspNetCore.Mvc;
using Newtonsoft.Json.Linq;

namespace InputApi.Controllers;

[Route("api/[controller]")]
[ApiController]
public class ThingsNetworkController : ControllerBase
{
    private readonly string _bucket;
    private readonly string _organisation;
    private readonly InfluxDbService _service;
    private readonly string _host;
    private readonly string _token;

    public ThingsNetworkController(InfluxDbService service, IConfiguration configuration)
    {
        _service = service;
        _bucket = configuration.GetValue<string>("INFLUXDB_INIT_BUCKET");
        _organisation = configuration.GetValue<string>("INFLUXDB_INIT_ORG");
        _token = configuration.GetValue<string>("INFLUXDB_INIT_ADMIN_TOKEN");
        _host = configuration.GetValue<string>("InfluxDB:Host");
    }

    [HttpGet("GetHumidity", Name = "GetHumidity")]
    public async Task<ActionResult<double>> GetHumidity([FromQuery] string eui)
    {

        if (string.IsNullOrEmpty(eui)) return NoContent();
        return Ok(await _service.QueryAsync(async query =>
        {
            var flux = $"from(bucket: \"{_bucket}\")" +
                       "|> range(start: 0)" +
                       "|> filter(fn: (r) => r[\"_measurement\"] == \"weather\")" +
                       "|> filter(fn: (r) => r[\"_field\"] == \"Humidity\")" +
                       $"|> filter(fn: (r) => r[\"dev_eui\"] == \"{eui}\")" +
                       "|> group()" +
                       "|> last()";

            var tables = await query.QueryAsync(flux, _organisation);
            return float.Parse(tables.First().Records.First().GetValue().ToString() ?? string.Empty,
                CultureInfo.CurrentCulture);
        }));
    }

    [HttpGet("GetBattery", Name = "GetBattery")]
    public async Task<ActionResult<double>> GetBattery([FromQuery] string eui)
    {
        if (string.IsNullOrEmpty(eui)) return NoContent();
        return Ok(await _service.QueryAsync(async query =>
        {
            var flux = $"from(bucket: \"{_bucket}\")" +
                       "|> range(start: 0)" +
                       "|> filter(fn: (r) => r[\"_measurement\"] == \"weather\")" +
                       "|> filter(fn: (r) => r[\"_field\"] == \"Battery\")" +
                       $"|> filter(fn: (r) => r[\"dev_eui\"] == \"{eui}\")" +
                       "|> group()" +
                       "|> last()";

            var tables = await query.QueryAsync(flux, _organisation);
            return float.Parse(tables.First().Records.First().GetValue().ToString() ?? string.Empty,
                CultureInfo.CurrentCulture);
        }));
    }

    [HttpGet("GetTemperature", Name = "GetTemperature")]
    public async Task<ActionResult<double>> GetTemperature([FromQuery] string eui)
    {
        if (string.IsNullOrEmpty(eui)) return NoContent();
        return Ok(await _service.QueryAsync(async query =>
        {
            var flux = $"from(bucket: \"{_bucket}\")" +
                       "|> range(start: -1d)" +
                       "|> filter(fn: (r) => r[\"_measurement\"] == \"weather\")" +
                       "|> filter(fn: (r) => r[\"_field\"] == \"Temperature\")" +
                       $"|> filter(fn: (r) => r[\"dev_eui\"] == \"{eui}\")" +
                       "|> group()" +
                       "|> last()";

            var tables = await query.QueryAsync(flux, _organisation);
            return float.Parse(tables.First().Records.First().GetValue().ToString() ?? string.Empty,
                CultureInfo.CurrentCulture);
        }));
    }

    [HttpGet("GetTemperaturesOfToday", Name = "GetTemperaturesOfToday")]
    public async Task<ActionResult<List<double>>> GetTemperaturesOfToday([FromQuery] string eui)
    {
        if (string.IsNullOrEmpty(eui)) return NoContent();
        return Ok(await _service.QueryAsync(async query =>
        {
            var flux = $"from(bucket: \"{_bucket}\")" +
                       $"|> range(start: -23h) " +
                       "|> filter(fn: (r) => r[\"_measurement\"] == \"weather\")" +
                       "|> filter(fn: (r) => r[\"_field\"] == \"Temperature\")" +
                       $"|> filter(fn: (r) => r[\"dev_eui\"] == \"{eui}\")" +
                       $"|> aggregateWindow(every: 1h, fn: mean, createEmpty: false) " +
                       $"|> yield(name: \"interpolated\")";

            var tables = await query.QueryAsync(flux, _organisation);

            var interpolatedDataPoints = new List<double>();
            foreach (var fluxRow in tables.First().Records)
            {
                var temperature = float.Parse(fluxRow.GetValueByKey("_value").ToString() ?? string.Empty,
                    CultureInfo.CurrentCulture);
                var roundedValue = Math.Round(temperature, 1);
                interpolatedDataPoints.Add(roundedValue);
            }

            return interpolatedDataPoints;
        }));
    }

    [HttpPost("PostDataPoint", Name = "PostDataPoint")]
    public ActionResult PostDataPoint([FromBody] JsonElement data)
    {
        var theThingsPayload = data.Deserialize<TheThingsPayload<JsonElement>>();
        if (theThingsPayload == null) return NoContent();

        var weatherDataPoint = new WeatherDataPoint
        {
            DeviceId = theThingsPayload.EndDeviceIds.DeviceId,
            DevEui = theThingsPayload.EndDeviceIds.DevEui,
            Time = DateTime.UtcNow
        };

        var decodedPayload = theThingsPayload.UplinkMessage.DecodedPayload;

        if (weatherDataPoint.DeviceId.Contains("dragino"))
        {
            var draginoPayload = decodedPayload.Deserialize<DecodedPayloadDragino>();
            if (draginoPayload != null)
            {
                weatherDataPoint.Temperature = Math.Round(draginoPayload.Temperatur, 2);
                weatherDataPoint.Battery = Math.Round(draginoPayload.Battery, 2);
            }
        }

        if (weatherDataPoint.DeviceId.Contains("decentlab"))
        {
            var decentlabPayload = decodedPayload.Deserialize<DecodedPayloadDecentlab>();
            if (decentlabPayload != null)
            {
                weatherDataPoint.Temperature = Math.Round(decentlabPayload.Temperature.Value, 2);
                weatherDataPoint.Battery = Math.Round(decentlabPayload.BatteryVoltage.Value, 2);
            }
        }

        if (weatherDataPoint.DeviceId.Contains("liard"))
        {
            var liardPayload = decodedPayload.Deserialize<DecodedPayloadLiard>();
            if (liardPayload != null)
            {
                weatherDataPoint.Temperature = Math.Round(liardPayload.Temperature1, 2);
                weatherDataPoint.Humidity = liardPayload.RelativeHumidity2;
                weatherDataPoint.Battery = liardPayload.AnalogIn3;
            }
        }

        _service.Write(write =>
        {
            var point = PointData.Measurement("weather")
                .Tag("dev_eui", weatherDataPoint.DevEui)
                .Field("Temperature", weatherDataPoint.Temperature)
                .Field("Humidity", weatherDataPoint.Humidity)
                .Field("Battery", weatherDataPoint.Battery)
                .Timestamp(weatherDataPoint.Time, WritePrecision.Ns);

            write.WritePoint(point, _bucket, _organisation);
        });
        return Ok();
    }
}