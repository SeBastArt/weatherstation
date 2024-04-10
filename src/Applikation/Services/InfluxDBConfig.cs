using Applikation.Interfaces;
using Microsoft.Extensions.Configuration;

namespace Applikation.Services;

public class InfluxDbConfig(IConfiguration configuration) : IInfluxDbConfig
{
    public string Host { get; } = configuration.GetValue<string>("InfluxDB:Host") ?? throw new InvalidOperationException("InfluxDB host configuration is missing.");
    public string Token { get; } = configuration.GetValue<string>("InfluxDB:AdminToken") ?? throw new InvalidOperationException("InfluxDB token configuration is missing.");
    public string Bucket { get; } = configuration.GetValue<string>("InfluxDB:Bucket") ?? throw new InvalidOperationException("InfluxDB bucket configuration is missing.");
    public string Organization { get; } = configuration.GetValue<string>("InfluxDb:Organization") ?? throw new InvalidOperationException("InfluxDB organization configuration is missing.");
}