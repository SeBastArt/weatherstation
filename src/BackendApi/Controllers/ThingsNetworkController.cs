using Application.Interfaces;
using Microsoft.AspNetCore.Mvc;

namespace BackendApi.Controllers;

[Route("api/[controller]")]
[ApiController]
public class ThingsNetworkController : ControllerBase
{
    private readonly IInfluxDbService _service;
    
    public ThingsNetworkController(IInfluxDbService service)
    {
        _service = service;
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
}