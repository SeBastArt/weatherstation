﻿using System.Text.Json;
using Applikation.DataTransferObjects;
using Applikation.Interfaces;

namespace Applikation.Services.PayloadProcessors;

public class DecentlabPayloadProcessor: IPayloadProcessor
{
    public bool CanProcess(string deviceId) => deviceId.Contains("decentlab");

    public void ProcessPayload(JsonElement payload, WeatherDataPoint weatherDataPoint)
    {
        var decentlabPayload = payload.Deserialize<DecodedPayloadDecentlab>();
        if (decentlabPayload != null)
        {
            weatherDataPoint.Temperature = Math.Round(decentlabPayload.Temperature!.Value, 2);
            weatherDataPoint.Battery = Math.Round(decentlabPayload.BatteryVoltage.Value, 2);
        }
    }
}