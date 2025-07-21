using Application.Interfaces;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
using BackendApi.Models;
using BackendApi.Services;
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
    private readonly IWeatherCacheService _weatherCacheService;
    
    public ThingsNetworkController(IInfluxDbService service, IConfiguration configuration, IWeatherCacheService weatherCacheService)
    {
        _service = service;
        _configuration = configuration;
        _httpClient = new HttpClient();
        _weatherCacheService = weatherCacheService;
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
        var weather = await _weatherCacheService.GetCurrentWeatherAsync();
        if (weather == null) return NotFound("Weather data not available");
        return Ok(weather);
    }

    [HttpGet("GetWeatherForecast")]
    public async Task<ActionResult<WeatherForecastDto>> GetWeatherForecast()
    {
        var forecast = await _weatherCacheService.GetWeatherForecastAsync();
        if (forecast == null) return NotFound("Weather forecast not available");
        return Ok(forecast);
    }

    [HttpGet("GetNext5HourlyForecast")]
    public async Task<ActionResult<List<WeatherForecastEntryDto>>> GetNext5HourlyForecast()
    {
        return await GetNextHourlyForecast(5);
    }

    [HttpGet("GetNextHourlyForecast")]
    public async Task<ActionResult<List<WeatherForecastEntryDto>>> GetNextHourlyForecast([FromQuery] int hours = 15)
    {
        var forecast = await _weatherCacheService.GetNextHourlyForecastAsync(hours);
        return Ok(forecast);
    }
}