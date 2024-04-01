using System.Text.Json;
using Application.DataTransferObjects;
using Application.Interfaces;

namespace Application.Services.PayloadProcessors;

public class LiardPayloadProcessor : IPayloadProcessor
{
    public bool CanProcess(string deviceId) => deviceId.Contains("liard");

    public void ProcessPayload(JsonElement payload, WeatherDataPoint weatherDataPoint)
    {
        var liardPayload = payload.Deserialize<DecodedPayloadLiard>();
        if (liardPayload != null)
        {
            weatherDataPoint.Temperature = Math.Round(liardPayload.Temperature1, 2);
            weatherDataPoint.Humidity = liardPayload.RelativeHumidity2;
            weatherDataPoint.Battery = liardPayload.AnalogIn3;
        }
    }
}