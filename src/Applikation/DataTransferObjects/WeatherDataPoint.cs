namespace Applikation.DataTransferObjects;

public class WeatherDataPoint
{
    public DateTime Time { get; set; }
    public double Temperature { get; set; } = 0.0;
    public double Humidity { get; set; } = 0.0;
    public double Battery { get; set; } = 0.0;
    public string DevEui { get; set; }
    public string DeviceId { get; set; }
}