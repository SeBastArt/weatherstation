using System.Text.Json;
using Applikation.DataTransferObjects;
using Applikation.Interfaces;

namespace Applikation.Services.PayloadProcessors;

public class DraginoPayloadProcessor: IPayloadProcessor
{
    public bool CanProcess(string deviceId) => deviceId.Contains("dragino");

    public void ProcessPayload(JsonElement payload, WeatherDataPoint weatherDataPoint)
    {
        var draginoPayload = payload.Deserialize<DecodedPayloadDragino>();
        if (draginoPayload != null)
        {
            weatherDataPoint.Temperature = Math.Round(draginoPayload.Temperatur, 2);
            weatherDataPoint.Battery = Math.Round(draginoPayload.Battery, 2);
        }
    }
}