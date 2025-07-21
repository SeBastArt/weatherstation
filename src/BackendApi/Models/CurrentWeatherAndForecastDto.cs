namespace BackendApi.Models;

public class CurrentWeatherAndForecastDto
{
    public CurrentWeatherDto CurrentWeather { get; set; }
    public List<WeatherForecastEntryDto> Forecast { get; set; }
}
