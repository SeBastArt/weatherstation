using System.Security.Cryptography;
using System.Text;
using Microsoft.Extensions.Configuration;
using MQTTnet;
using MQTTnet.Client;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using HoneyPal.Dtos;

namespace HoneyPal.Services;

public interface IMqttClientService
{
    Task SendMessage(string topic, string data);
}

public class MqttClientService : IMqttClientService, IDisposable
{
    private readonly string _hmacKey;
    private readonly IMqttClient _mqttclient;

    public MqttClientService(IConfiguration config, IWebHostEnvironment env)
    {
        try
        {
            _hmacKey = config.GetValue<string>("HMAC_SECRET_KEY");
            var broker = config.GetValue<string>("Mqtt:Broker");
            int? port = config.GetValue<int>("Mqtt:Port");
            var mqttClientOptions = new MqttClientOptionsBuilder()
                .WithTcpServer(broker, port)
                .Build();

            var mqttFactory = new MqttFactory();
            _mqttclient = mqttFactory.CreateMqttClient();
            _mqttclient.ConnectAsync(mqttClientOptions, CancellationToken.None).Wait();
        }
        catch (Exception exception)
        {
            Console.WriteLine(exception);
            throw;
        }
    }

    public void Dispose()
    {
        _mqttclient.DisconnectAsync().Wait();
        _mqttclient.Dispose();
    }

    public async Task SendMessage(string topic, string data)
    {
        var payload = data;

        // Generieren Sie einen HMAC-Schlüssel aus dem geheimen Schlüssel und dem Salt
        var keyBytes = Encoding.UTF8.GetBytes(_hmacKey);

        // Generieren Sie einen Zeitstempel
        var timestamp = DateTimeOffset.UtcNow.ToUnixTimeSeconds();

        // Kombinieren Sie Zeitstempel und Payload
        var dataToSign = $"{timestamp}:{payload}";

        // Berechnen Sie die Checksumme für die Daten
        var checksum = CalculateChecksum(dataToSign);

        // Kombinieren Sie Zeitstempel, Payload und Checksumme
        var signedData = $"{timestamp}:{payload}:{checksum}";

        // Berechnen Sie den HMAC-Wert für die Payload
        var payloadBytes = Encoding.UTF8.GetBytes(signedData);
        var hmacValue = CalculateHmac(payloadBytes, keyBytes);
        var secureMessage = new SecureMessage
        {
            Payload = signedData,
            Hmac = hmacValue,
            Timestamp = timestamp
        };
        var jsonString = JsonConvert.SerializeObject(secureMessage);
        var applicationMessage = new MqttApplicationMessageBuilder()
            .WithTopic(topic)
            .WithPayload(jsonString)
            .Build();

        await _mqttclient.PublishAsync(applicationMessage, CancellationToken.None);
    }

    private static string CalculateHmac(byte[] data, byte[] key)
    {
        using (var hmac = new HMACSHA256(key))
        {
            var hash = hmac.ComputeHash(data);
            return BitConverter.ToString(hash).Replace("-", "");
        }
    }

    private static string CalculateChecksum(string data)
    {
        // Hier können Sie eine Checksummenberechnung implementieren (z. B. CRC32 oder SHA-1)
        using (var sha1 = SHA256.Create())
        {
            var dataBytes = Encoding.Default.GetBytes(data);
            var checksumBytes = sha1.ComputeHash(dataBytes);
            return BitConverter.ToString(checksumBytes).Replace("-", "");
        }
    }
}