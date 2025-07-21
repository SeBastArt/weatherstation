namespace BackendApi.Services;

public class WeatherCacheBackgroundService : BackgroundService
{
    private readonly IServiceProvider _serviceProvider;
    private readonly ILogger<WeatherCacheBackgroundService> _logger;
    private readonly IConfiguration _configuration;

    public WeatherCacheBackgroundService(
        IServiceProvider serviceProvider,
        ILogger<WeatherCacheBackgroundService> logger,
        IConfiguration configuration)
    {
        _serviceProvider = serviceProvider;
        _logger = logger;
        _configuration = configuration;
    }

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        _logger.LogInformation("Weather Cache Background Service started");

        // Initial delay to let the application start properly
        await Task.Delay(TimeSpan.FromSeconds(30), stoppingToken);

        while (!stoppingToken.IsCancellationRequested)
        {
            try
            {
                using var scope = _serviceProvider.CreateScope();
                var weatherCacheService = scope.ServiceProvider.GetRequiredService<IWeatherCacheService>();

                if (await weatherCacheService.IsDataStaleAsync())
                {
                    _logger.LogInformation("Data is stale, refreshing...");
                    await weatherCacheService.RefreshWeatherDataAsync();
                }
                else
                {
                    _logger.LogDebug("Data is still fresh");
                }
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error in weather cache background service");
            }

            // Wait for the configured interval
            var refreshIntervalMinutes = _configuration.GetValue<int>("WeatherCache:RefreshIntervalMinutes", 60);
            var delay = TimeSpan.FromMinutes(refreshIntervalMinutes);
            _logger.LogDebug("Next refresh in {Minutes} minutes", refreshIntervalMinutes);
            
            await Task.Delay(delay, stoppingToken);
        }

        _logger.LogInformation("Weather Cache Background Service stopped");
    }
}