using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;

namespace Weatherstation.Pages;

public class IndexModel : PageModel
{
    private readonly ILogger<IndexModel> _logger;
    private readonly IConfiguration _configuration;

    public IndexModel(ILogger<IndexModel> logger, IConfiguration configuration)
    {
        _logger = logger;
        _configuration = configuration;
    }

    public void OnGet()
    {
        // httpClient to https://backend/api/Temperature
        var backendApi = new Uri(_configuration["BackendApi:Url"] ?? throw new InvalidOperationException());
        var httpClient = new HttpClient();
        httpClient.BaseAddress = backendApi;
        var response = httpClient.GetAsync("/api/ThingsNetwork/GetTemperature?eui=dfgdfg'").Result;
        var content = response.Content.ReadAsStringAsync().Result;
        ViewData["Temperature"] = content;
    }
}