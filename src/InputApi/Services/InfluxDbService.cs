﻿using InfluxDB.Client;

namespace InputApi.Services;

public class InfluxDbService
{
    private readonly string _host;
    private readonly string _token;

    public InfluxDbService(IConfiguration configuration)
    {
        _token = configuration.GetValue<string>("INFLUXDB_INIT_ADMIN_TOKEN");
        _host = configuration.GetValue<string>("InfluxDB:Host");
    }

    public void Write(Action<WriteApi> action)
    {
        using var client = new InfluxDBClient(_host, _token);
        using var write = client.GetWriteApi();
        action(write);
    }

    public async Task<T> QueryAsync<T>(Func<QueryApi, Task<T>> action)
    {
        using var client = new InfluxDBClient(_host, _token);
        var query = client.GetQueryApi();
        return await action(query);
    }
}