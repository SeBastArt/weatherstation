using BackendApi.Models.WeatherCache;
using Microsoft.EntityFrameworkCore;

namespace BackendApi.Data;

public class WeatherCacheDbContext : DbContext
{
    public WeatherCacheDbContext(DbContextOptions<WeatherCacheDbContext> options) : base(options)
    {
    }

    public DbSet<CachedCurrentWeather> CurrentWeather { get; set; }
    public DbSet<CachedWeatherForecast> WeatherForecasts { get; set; }
    public DbSet<CachedWeatherForecastEntry> WeatherForecastEntries { get; set; }

    protected override void OnModelCreating(ModelBuilder modelBuilder)
    {
        base.OnModelCreating(modelBuilder);

        // Configure CachedWeatherForecast and its entries
        modelBuilder.Entity<CachedWeatherForecast>(entity =>
        {
            entity.HasKey(e => e.Id);
            entity.Property(e => e.City).IsRequired().HasMaxLength(100);
            entity.Property(e => e.CachedAt).IsRequired();
        });

        modelBuilder.Entity<CachedWeatherForecastEntry>(entity =>
        {
            entity.HasKey(e => e.Id);
            entity.Property(e => e.Description).HasMaxLength(200);
            entity.Property(e => e.Icon).HasMaxLength(10);
            
            // Configure relationship
            entity.HasOne(e => e.Forecast)
                  .WithMany(f => f.Forecast)
                  .HasForeignKey(e => e.ForecastId)
                  .OnDelete(DeleteBehavior.Cascade);
        });

        // Configure CachedCurrentWeather
        modelBuilder.Entity<CachedCurrentWeather>(entity =>
        {
            entity.HasKey(e => e.Id);
            entity.Property(e => e.City).IsRequired().HasMaxLength(100);
            entity.Property(e => e.Description).HasMaxLength(200);
            entity.Property(e => e.Icon).HasMaxLength(10);
        });
    }
}