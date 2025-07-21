using System.ComponentModel.DataAnnotations;

namespace BackendApi.Models.WeatherCache;

public class CachedWeatherForecast
{
    [Key]
    public int Id { get; set; }
    
    public string City { get; set; } = string.Empty;
    public DateTime CachedAt { get; set; }
    
    // Navigation property
    public List<CachedWeatherForecastEntry> Forecast { get; set; } = new();
    
    // Convert to DTO
    public WeatherForecastDto ToDto()
    {
        return new WeatherForecastDto
        {
            City = City,
            Forecast = Forecast.OrderBy(f => f.DateTime).Select(f => f.ToDto()).ToList()
        };
    }
}

public class CachedWeatherForecastEntry
{
    [Key]
    public int Id { get; set; }
    
    public int ForecastId { get; set; }
    public CachedWeatherForecast Forecast { get; set; } = null!;
    
    public DateTime DateTime { get; set; }
    public double Temperature { get; set; }
    public int Humidity { get; set; }
    public int Pressure { get; set; }
    public string Description { get; set; } = string.Empty;
    public string Icon { get; set; } = string.Empty;
    public double WindSpeed { get; set; }
    public int WindDegree { get; set; }
    
    // Convert to DTO
    public WeatherForecastEntryDto ToDto()
    {
        return new WeatherForecastEntryDto
        {
            DateTime = DateTime,
            Temperature = Temperature,
            Humidity = Humidity,
            Pressure = Pressure,
            Description = Description,
            Icon = Icon,
            WindSpeed = WindSpeed,
            WindDegree = WindDegree
        };
    }
}