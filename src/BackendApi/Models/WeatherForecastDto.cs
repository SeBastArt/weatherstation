namespace BackendApi.Models;

public class WeatherForecastDto
{
    public string City { get; set; }
    public List<WeatherForecastEntryDto> Forecast { get; set; }
}

public class WeatherForecastEntryDto
{
    public DateTime DateTime { get; set; }
    public double Temperature { get; set; }
    public int Humidity { get; set; }
    public int Pressure { get; set; }
    public string Description { get; set; }
    public string Icon { get; set; }
    public double WindSpeed { get; set; }
    public int WindDegree { get; set; }
}
