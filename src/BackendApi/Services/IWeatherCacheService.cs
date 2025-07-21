using BackendApi.Models;

namespace BackendApi.Services;

public interface IWeatherCacheService
{
    Task<CurrentWeatherDto?> GetCurrentWeatherAsync();
    Task<WeatherForecastDto?> GetWeatherForecastAsync();
    Task<List<WeatherForecastEntryDto>> GetNextHourlyForecastAsync(int hours = 15);
    Task RefreshWeatherDataAsync();
    Task<bool> IsDataStaleAsync();
}