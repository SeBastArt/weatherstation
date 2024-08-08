using System.Text.Json;
using InfluxDB.Client;
using InfluxDB.Client.Core.Flux.Domain;

namespace Application.Interfaces;

public interface IInfluxDbService
{
    void Write(Action<WriteApi> action);
    Task<List<FluxTable>> QueryFluxAsync(string fluxQuery, string organization);
    
    Task<double> GetHumidity(string eui);
    Task<double> GetBattery(string eui);
    Task<double> GetTemperature(string eui);
    Task<List<double>> GetTemperaturesOfToday(string eui);
    Task<List<double>> GetHumidityOfToday(string eui);
    void PostDataPoint(JsonElement data);
}