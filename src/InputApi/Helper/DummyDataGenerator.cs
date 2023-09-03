using InfluxDB.Client;
using InfluxDB.Client.Api.Domain;
using InfluxDB.Client.Writes;
using InputApi.DataTransferObjects;
using InputApi.Services;

namespace InputApi.Helper
{
    public class DummyDataGenerator
    {
        public List<WeatherDataPoint> GenerateDummyData()
        {
            var dummyData = new List<WeatherDataPoint>();
            var devEui = "dummy_sensor";
            const string deviceId = "dummy_device";

            var currentTime = DateTime.UtcNow.AddHours(-24);

            while (currentTime <= DateTime.UtcNow)
            {
                var temperature = GetRandomValue(-10.0, 30.0, 1);
                var humidity = GetRandomValue(20.0, 90.0, 1);
                var battery = GetRandomValue(2.8, 3.1, 1);
                dummyData.Add(new WeatherDataPoint
                {
                    DevEui = devEui,
                    DeviceId = deviceId,
                    Time = currentTime,
                    Temperature = temperature,
                    Humidity = humidity,
                    Battery = battery,
                });

                currentTime = currentTime.AddMinutes(15);
            }

            return dummyData;
        }

        private static double GetRandomValue(double minValue, double maxValue, int decimalPlaces)
        {
            var random = new Random();
            var randomValue = random.NextDouble() * (maxValue * 10 - minValue * 10) + minValue * 10;
            randomValue /= 10; // Shift the range to -10 to 30
            var roundedValue = Math.Round(randomValue, decimalPlaces);

            return roundedValue;
        }
    }
}