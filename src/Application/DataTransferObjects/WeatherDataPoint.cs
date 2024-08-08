namespace Application.DataTransferObjects;

public class WeatherDataPoint
{
    public DateTime Time { get; init; }
    public double Temperature { get; set; }
    public double Humidity { get; set; }
    public double Battery { get; set; }
    public string? DevEui { get; init; }
    public string? DeviceId { get; init; }
}