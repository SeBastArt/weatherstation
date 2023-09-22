using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using HoneyPal.Dtos;
using HoneyPal.Services;

namespace HoneyPal.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class PaypalController : ControllerBase
    {
        private readonly IMqttClientService _mqttClientService;
        private readonly IConfiguration _config;

        public PaypalController(IMqttClientService mqttClientService, IConfiguration config)
        {
            _mqttClientService = mqttClientService;
            _config = config;
        }

        [HttpPost]
        public async Task<IActionResult> SendMessage(Root data)
        {
            string topic = _config.GetValue<string>("Mqtt:Topic");
            await _mqttClientService.SendMessage(topic, data.event_type);
            return Ok();
        }
    }
}
