using System.Text.Json;
using Applikation.DataTransferObjects;

namespace Applikation.Interfaces;

public interface IPayloadProcessor
{
    bool CanProcess(string deviceId);
    void ProcessPayload(JsonElement payload, WeatherDataPoint weatherDataPoint);
}