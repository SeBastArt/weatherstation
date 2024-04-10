using System.Text.Json;
using Application.Interfaces;
using Microsoft.AspNetCore.Mvc;

namespace OldThingsApi.Controllers;

[Route("api/[controller]")]
[ApiController]
public class ThingsNetworkController : ControllerBase
{
    private readonly IInfluxDbService _service;
    
    public ThingsNetworkController(IInfluxDbService service)
    {
        _service = service;
    }

    [HttpPost("PostDataPoint", Name = "PostDataPoint")]
    public ActionResult PostDataPoint([FromBody] JsonElement data)
    {
        _service.PostDataPoint(data);
        return Ok();
    }
}