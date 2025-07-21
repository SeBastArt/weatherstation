using System.ComponentModel.DataAnnotations;

namespace BackendApi.Models.WeatherCache;

public class CachedCurrentWeather
{
    [Key]
    public int Id { get; set; }
    
    public string City { get; set; } = string.Empty;
    public double Temperature { get; set; }
    public int Humidity { get; set; }
    public int Pressure { get; set; }
    public string Description { get; set; } = string.Empty;
    public string Icon { get; set; } = string.Empty;
    public double WindSpeed { get; set; }
    public int WindDegree { get; set; }
    public DateTime WeatherDateTime { get; set; }
    public DateTime CachedAt { get; set; }
    
    // Convert to DTO
    public CurrentWeatherDto ToDto()
    {
        return new CurrentWeatherDto
        {
            City = City,
            Temperature = Temperature,
            Humidity = Humidity,
            Pressure = Pressure,
            Description = Description,
            Icon = Icon,
            WindSpeed = WindSpeed,
            WindDegree = WindDegree,
            DateTime = WeatherDateTime
        };
    }
}