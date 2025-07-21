using Application.Interfaces;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
using BackendApi.Models;
using System.Net.Http;
using System.Text.Json;

namespace BackendApi.Controllers;

[Route("api/[controller]")]
[ApiController]
public class ThingsNetworkController : ControllerBase
{
    private readonly IInfluxDbService _service;
    private readonly IConfiguration _configuration;
    private readonly HttpClient _httpClient;
    
    public ThingsNetworkController(IInfluxDbService service, IConfiguration configuration)
    {
        _service = service;
        _configuration = configuration;
        _httpClient = new HttpClient();
    }
    
    [HttpGet("GetHumidity", Name = "GetHumidity")]
    public async Task<ActionResult<double>> GetHumidity([FromQuery] string eui)
    {
        if (string.IsNullOrEmpty(eui)) return NoContent();
        var humidity = await _service.GetHumidity(eui);
        return Ok(humidity);
    }
    
    [HttpGet("GetBattery", Name = "GetBattery")]
    public async Task<ActionResult<double>> GetBattery([FromQuery] string eui)
    {
        if (string.IsNullOrEmpty(eui)) return NoContent();
        var battery = await _service.GetBattery(eui);
        return Ok(battery);
    }
    
    [HttpGet("GetTemperature", Name = "GetTemperature")]
    public async Task<ActionResult<double>> GetTemperature([FromQuery] string eui)
    {
        if (string.IsNullOrEmpty(eui)) return NoContent();
        var temperature = await _service.GetTemperature(eui);
        return Ok(temperature);
    }

    [HttpGet("GetTemperaturesOfToday", Name = "GetTemperaturesOfToday")]
    public async Task<ActionResult<List<double>>> GetTemperaturesOfToday([FromQuery] string eui)
    {
        if (string.IsNullOrEmpty(eui)) return NoContent();
        var temperatures = await _service.GetTemperaturesOfToday(eui);
        return Ok(temperatures);
    }
    
    [HttpGet("GetHumidityOfToday", Name = "GetHumidityOfToday")]
    public async Task<ActionResult<List<double>>> GetHumidityOfToday([FromQuery] string eui)
    {
        if (string.IsNullOrEmpty(eui)) return NoContent();
        var humidity = await _service.GetHumidityOfToday(eui);
        return Ok(humidity);
    }

    [HttpGet("GetCurrentWeather")]
    public async Task<ActionResult<CurrentWeatherDto>> GetCurrentWeather()
    {
        var apiKey = _configuration["OpenWeatherMap:ApiKey"];
        var url = $"https://api.openweathermap.org/data/2.5/weather?lat=51.050407&lon=13.737262&appid={apiKey}&units=metric";
        var response = await _httpClient.GetAsync(url);
        if (!response.IsSuccessStatusCode) return StatusCode((int)response.StatusCode);
        var json = await response.Content.ReadAsStringAsync();
        using var doc = JsonDocument.Parse(json);
        var root = doc.RootElement;
        var weather = root.GetProperty("weather")[0];
        var main = root.GetProperty("main");
        var wind = root.GetProperty("wind");
        var dto = new CurrentWeatherDto
        {
            City = root.GetProperty("name").GetString(),
            Temperature = main.GetProperty("temp").GetDouble(),
            Humidity = main.GetProperty("humidity").GetInt32(),
            Pressure = main.GetProperty("pressure").GetInt32(),
            Description = weather.GetProperty("description").GetString(),
            Icon = weather.GetProperty("icon").GetString(),
            WindSpeed = wind.GetProperty("speed").GetDouble(),
            WindDegree = wind.GetProperty("deg").GetInt32(),
            DateTime = DateTimeOffset.FromUnixTimeSeconds(root.GetProperty("dt").GetInt64()).DateTime
        };
        return Ok(dto);
    }

    [HttpGet("GetWeatherForecast")]
    public async Task<ActionResult<WeatherForecastDto>> GetWeatherForecast()
    {
        var apiKey = _configuration["OpenWeatherMap:ApiKey"];
        var url = $"https://api.openweathermap.org/data/2.5/forecast?lat=51.050407&lon=13.737262&appid={apiKey}&units=metric";
        var response = await _httpClient.GetAsync(url);
        if (!response.IsSuccessStatusCode) return StatusCode((int)response.StatusCode);
        var json = await response.Content.ReadAsStringAsync();
        using var doc = JsonDocument.Parse(json);
        var root = doc.RootElement;
        var city = root.GetProperty("city").GetProperty("name").GetString();
        var list = root.GetProperty("list");
        var forecast = new List<WeatherForecastEntryDto>();
        foreach (var entry in list.EnumerateArray())
        {
            var weather = entry.GetProperty("weather")[0];
            var main = entry.GetProperty("main");
            var wind = entry.GetProperty("wind");
            forecast.Add(new WeatherForecastEntryDto
            {
                DateTime = DateTime.Parse(entry.GetProperty("dt_txt").GetString()),
                Temperature = main.GetProperty("temp").GetDouble(),
                Humidity = main.GetProperty("humidity").GetInt32(),
                Pressure = main.GetProperty("pressure").GetInt32(),
                Description = weather.GetProperty("description").GetString(),
                Icon = weather.GetProperty("icon").GetString(),
                WindSpeed = wind.GetProperty("speed").GetDouble(),
                WindDegree = wind.GetProperty("deg").GetInt32(),
            });
        }
        var dto = new WeatherForecastDto
        {
            City = city,
            Forecast = forecast
        };
        return Ok(dto);
    }

    [HttpGet("GetNext5HourlyForecast")]
    public async Task<ActionResult<List<WeatherForecastEntryDto>>> GetNext5HourlyForecast()
    {
        return await GetNextHourlyForecast(5);
    }

    [HttpGet("GetNextHourlyForecast")]
    public async Task<ActionResult<List<WeatherForecastEntryDto>>> GetNextHourlyForecast([FromQuery] int hours = 15)
    {
        var apiKey = _configuration["OpenWeatherMap:ApiKey"];
        var url = $"https://api.openweathermap.org/data/2.5/forecast?lat=51.050407&lon=13.737262&appid={apiKey}&units=metric";
        var response = await _httpClient.GetAsync(url);
        if (!response.IsSuccessStatusCode) return StatusCode((int)response.StatusCode);
        var json = await response.Content.ReadAsStringAsync();
        using var doc = JsonDocument.Parse(json);
        var root = doc.RootElement;
        var list = root.GetProperty("list");
        var forecast = new List<WeatherForecastEntryDto>();
        
        int count = 0;
        foreach (var entry in list.EnumerateArray())
        {
            if (count >= hours) break;
            
            var weather = entry.GetProperty("weather")[0];
            var main = entry.GetProperty("main");
            var wind = entry.GetProperty("wind");
            forecast.Add(new WeatherForecastEntryDto
            {
                DateTime = DateTime.Parse(entry.GetProperty("dt_txt").GetString()),
                Temperature = main.GetProperty("temp").GetDouble(),
                Humidity = main.GetProperty("humidity").GetInt32(),
                Pressure = main.GetProperty("pressure").GetInt32(),
                Description = weather.GetProperty("description").GetString(),
                Icon = weather.GetProperty("icon").GetString(),
                WindSpeed = wind.GetProperty("speed").GetDouble(),
                WindDegree = wind.GetProperty("deg").GetInt32(),
            });
            count++;
        }
        
        return Ok(forecast);
    }
}