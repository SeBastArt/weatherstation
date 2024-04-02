using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;

namespace Weatherstation.Pages;

public class IndexModel : PageModel
{
    private readonly ILogger<IndexModel> _logger;

    public IndexModel(ILogger<IndexModel> logger)
    {
        _logger = logger;
    }

    public void OnGet()
    {
        // httpclient to http://localhost:5002/api/ThingsNetwork/GetTemperature with eui=0025CA0A00000476
        HttpClient client = new HttpClient();
        client.BaseAddress = new Uri("http://backend/");
        HttpResponseMessage response = client.GetAsync("api/ThingsNetwork/GetTemperature?eui=0025CA0A00000476").Result;
        if (response.IsSuccessStatusCode)
        {
            string data = response.Content.ReadAsStringAsync().Result;
            ViewData["Temperature"] = data;
        }
        else
        {
            ViewData["Temperature"] = "Error";
        }
    }
}