using System.Text.Json;
using Application.DataTransferObjects;

namespace Application.Interfaces;

public interface IPayloadProcessor
{
    bool CanProcess(string deviceId);
    void ProcessPayload(JsonElement payload, WeatherDataPoint weatherDataPoint);
}