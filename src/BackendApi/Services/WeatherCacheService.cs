using BackendApi.Data;
using BackendApi.Models;
using BackendApi.Models.WeatherCache;
using Microsoft.EntityFrameworkCore;
using System.Text.Json;

namespace BackendApi.Services;

public class WeatherCacheService : IWeatherCacheService
{
    private readonly WeatherCacheDbContext _context;
    private readonly IConfiguration _configuration;
    private readonly HttpClient _httpClient;
    private readonly ILogger<WeatherCacheService> _logger;

    public WeatherCacheService(
        WeatherCacheDbContext context,
        IConfiguration configuration,
        HttpClient httpClient,
        ILogger<WeatherCacheService> logger)
    {
        _context = context;
        _configuration = configuration;
        _httpClient = httpClient;
        _logger = logger;
    }

    public async Task<CurrentWeatherDto?> GetCurrentWeatherAsync()
    {
        // Try to get from cache first
        var cachedWeather = await _context.CurrentWeather
            .OrderByDescending(w => w.CachedAt)
            .FirstOrDefaultAsync();

        if (cachedWeather != null && !IsStale(cachedWeather.CachedAt))
        {
            _logger.LogDebug("Returning cached current weather data");
            return cachedWeather.ToDto();
        }

        // If no cache or stale, refresh and return
        await RefreshCurrentWeatherAsync();
        cachedWeather = await _context.CurrentWeather
            .OrderByDescending(w => w.CachedAt)
            .FirstOrDefaultAsync();

        return cachedWeather?.ToDto();
    }

    public async Task<WeatherForecastDto?> GetWeatherForecastAsync()
    {
        // Try to get from cache first
        var cachedForecast = await _context.WeatherForecasts
            .Include(f => f.Forecast)
            .OrderByDescending(f => f.CachedAt)
            .FirstOrDefaultAsync();

        if (cachedForecast != null && !IsStale(cachedForecast.CachedAt))
        {
            _logger.LogDebug("Returning cached forecast data");
            return cachedForecast.ToDto();
        }

        // If no cache or stale, refresh and return
        await RefreshWeatherForecastAsync();
        cachedForecast = await _context.WeatherForecasts
            .Include(f => f.Forecast)
            .OrderByDescending(f => f.CachedAt)
            .FirstOrDefaultAsync();

        return cachedForecast?.ToDto();
    }

    public async Task<List<WeatherForecastEntryDto>> GetNextHourlyForecastAsync(int hours = 15)
    {
        var forecast = await GetWeatherForecastAsync();
        if (forecast?.Forecast == null) return new List<WeatherForecastEntryDto>();

        return forecast.Forecast.Take(hours).ToList();
    }

    public async Task RefreshWeatherDataAsync()
    {
        _logger.LogInformation("Refreshing weather data...");
        
        try
        {
            await RefreshCurrentWeatherAsync();
            await RefreshWeatherForecastAsync();
            _logger.LogInformation("Weather data refreshed successfully");
        }
        catch (Exception ex)
        {
            _logger.LogError(ex, "Error refreshing weather data");
            throw;
        }
    }

    public async Task<bool> IsDataStaleAsync()
    {
        var currentWeather = await _context.CurrentWeather
            .OrderByDescending(w => w.CachedAt)
            .FirstOrDefaultAsync();

        var forecast = await _context.WeatherForecasts
            .OrderByDescending(f => f.CachedAt)
            .FirstOrDefaultAsync();

        return currentWeather == null || forecast == null ||
               IsStale(currentWeather.CachedAt) || IsStale(forecast.CachedAt);
    }

    private async Task RefreshCurrentWeatherAsync()
    {
        var apiKey = _configuration["OpenWeatherMap:ApiKey"];
        var url = $"https://api.openweathermap.org/data/2.5/weather?lat=51.050407&lon=13.737262&appid={apiKey}&units=metric";
        
        var response = await _httpClient.GetAsync(url);
        if (!response.IsSuccessStatusCode) 
        {
            _logger.LogError("Failed to fetch current weather: {StatusCode}", response.StatusCode);
            return;
        }

        var json = await response.Content.ReadAsStringAsync();
        using var doc = JsonDocument.Parse(json);
        var root = doc.RootElement;
        
        var weather = root.GetProperty("weather")[0];
        var main = root.GetProperty("main");
        var wind = root.GetProperty("wind");

        var cachedWeather = new CachedCurrentWeather
        {
            City = root.GetProperty("name").GetString() ?? "",
            Temperature = main.GetProperty("temp").GetDouble(),
            Humidity = main.GetProperty("humidity").GetInt32(),
            Pressure = main.GetProperty("pressure").GetInt32(),
            Description = weather.GetProperty("description").GetString() ?? "",
            Icon = weather.GetProperty("icon").GetString() ?? "",
            WindSpeed = wind.GetProperty("speed").GetDouble(),
            WindDegree = wind.GetProperty("deg").GetInt32(),
            WeatherDateTime = DateTime.SpecifyKind(
                DateTimeOffset.FromUnixTimeSeconds(root.GetProperty("dt").GetInt64()).DateTime,
                DateTimeKind.Utc
            ),
            CachedAt = DateTime.UtcNow
        };

        // Clear old data and add new
        var oldData = await _context.CurrentWeather.ToListAsync();
        _context.CurrentWeather.RemoveRange(oldData);
        _context.CurrentWeather.Add(cachedWeather);
        await _context.SaveChangesAsync();

        _logger.LogDebug("Current weather refreshed");
    }

    private async Task RefreshWeatherForecastAsync()
    {
        var apiKey = _configuration["OpenWeatherMap:ApiKey"];
        var url = $"https://api.openweathermap.org/data/2.5/forecast?lat=51.050407&lon=13.737262&appid={apiKey}&units=metric";
        
        var response = await _httpClient.GetAsync(url);
        if (!response.IsSuccessStatusCode) 
        {
            _logger.LogError("Failed to fetch weather forecast: {StatusCode}", response.StatusCode);
            return;
        }

        var json = await response.Content.ReadAsStringAsync();
        using var doc = JsonDocument.Parse(json);
        var root = doc.RootElement;
        
        var city = root.GetProperty("city").GetProperty("name").GetString() ?? "";
        var list = root.GetProperty("list");

        var cachedForecast = new CachedWeatherForecast
        {
            City = city,
            CachedAt = DateTime.UtcNow,
            Forecast = new List<CachedWeatherForecastEntry>()
        };

        foreach (var entry in list.EnumerateArray())
        {
            var weather = entry.GetProperty("weather")[0];
            var main = entry.GetProperty("main");
            var wind = entry.GetProperty("wind");
        
            cachedForecast.Forecast.Add(new CachedWeatherForecastEntry
            {
                DateTime = DateTime.SpecifyKind(
                    DateTime.Parse(entry.GetProperty("dt_txt").GetString() ?? ""),
                    DateTimeKind.Utc
                ),
                Temperature = main.GetProperty("temp").GetDouble(),
                Humidity = main.GetProperty("humidity").GetInt32(),
                Pressure = main.GetProperty("pressure").GetInt32(),
                Description = weather.GetProperty("description").GetString() ?? "",
                Icon = weather.GetProperty("icon").GetString() ?? "",
                WindSpeed = wind.GetProperty("speed").GetDouble(),
                WindDegree = wind.GetProperty("deg").GetInt32()
            });
        }

        // Clear old data and add new
        var oldData = await _context.WeatherForecasts.Include(f => f.Forecast).ToListAsync();
        _context.WeatherForecasts.RemoveRange(oldData);
        _context.WeatherForecasts.Add(cachedForecast);
        await _context.SaveChangesAsync();

        _logger.LogDebug("Weather forecast refreshed");
    }

    private bool IsStale(DateTime cachedAt)
    {
        var refreshIntervalMinutes = _configuration.GetValue<int>("WeatherCache:RefreshIntervalMinutes", 60);
        return DateTime.UtcNow - cachedAt > TimeSpan.FromMinutes(refreshIntervalMinutes);
    }
}