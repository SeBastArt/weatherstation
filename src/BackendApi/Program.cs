using Application.Interfaces;
using Application.Services;
using BackendApi.Data;
using BackendApi.Services;
using Microsoft.EntityFrameworkCore;

var builder = WebApplication.CreateBuilder(args);

builder.Services.AddControllers();

// Add CORS policy to allow any origin, header, and method
builder.Services.AddCors(options =>
{
    options.AddDefaultPolicy(policy =>
    {
        policy.AllowAnyOrigin()
              .AllowAnyHeader()
              .AllowAnyMethod();
    });
});

// Add services to the container.
// Learn more about configuring Swagger/OpenAPI at https://aka.ms/aspnetcore/swashbuckle
builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();

builder.Services.AddSingleton<IInfluxDbConfig>(new InfluxDbConfig(builder.Configuration));
builder.Services.AddSingleton<IInfluxDbService, InfluxDbService>();

// Add PostgreSQL DbContext
builder.Services.AddDbContext<WeatherCacheDbContext>(options =>
    options.UseNpgsql(builder.Configuration.GetConnectionString("DefaultConnection")));

// Add Weather Cache Services
builder.Services.AddHttpClient<IWeatherCacheService, WeatherCacheService>();
builder.Services.AddScoped<IWeatherCacheService, WeatherCacheService>();
builder.Services.AddHostedService<WeatherCacheBackgroundService>();

var app = builder.Build();

// Apply database migrations automatically
using (var scope = app.Services.CreateScope())
{
    var context = scope.ServiceProvider.GetRequiredService<WeatherCacheDbContext>();
    context.Database.Migrate();
}

// Configure the HTTP request pipeline.
//if (app.Environment.IsDevelopment())
//{
    app.UseSwagger();
    app.UseSwaggerUI();
//}

app.UseCors(); // Enable CORS middleware
app.UseAuthorization();
app.MapControllers();

app.Run();