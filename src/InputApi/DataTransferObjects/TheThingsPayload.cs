using System.Text.Json.Serialization;
using Newtonsoft.Json;

namespace InputApi.DataTransferObjects;

public class TheThingsPayload<T>
{
    [JsonProperty("end_device_ids")]
    [JsonPropertyName("end_device_ids")]
    public EndDeviceIds EndDeviceIds { get; set; }

    [JsonProperty("correlation_ids")]
    [JsonPropertyName("correlation_ids")]
    public List<string> CorrelationIds { get; set; }

    [JsonProperty("received_at")]
    [JsonPropertyName("received_at")]
    public string ReceivedAt { get; set; }

    [JsonProperty("uplink_message")]
    [JsonPropertyName("uplink_message")]
    public UplinkMessage<T> UplinkMessage { get; set; }
}

public class DecodedPayloadLiard
{
    [JsonProperty("analog_in_3")]
    [JsonPropertyName("analog_in_3")]
    public double AnalogIn3 { get; set; }

    [JsonProperty("relative_humidity_2")]
    [JsonPropertyName("relative_humidity_2")]
    public double RelativeHumidity2 { get; set; }

    [JsonProperty("temperature_1")]
    [JsonPropertyName("temperature_1")]
    public double Temperature1 { get; set; }
}

public class DecodedPayloadDecentlab
{
    [JsonProperty("battery_voltage")]
    [JsonPropertyName("battery_voltage")]
    public BatteryVoltage BatteryVoltage { get; set; }

    [JsonProperty("device_id")]
    [JsonPropertyName("device_id")]
    public int DeviceId { get; set; }

    [JsonProperty("protocol_version")]
    [JsonPropertyName("protocol_version")]
    public int ProtocolVersion { get; set; }

    [JsonProperty("temperature")]
    [JsonPropertyName("temperature")]
    public Temperature? Temperature { get; set; }
}

public class DecodedPayloadDragino
{
    [JsonProperty("Battery")]
    [JsonPropertyName("Battery")]
    public double Battery { get; set; }

    [JsonProperty("Temperatur")]
    [JsonPropertyName("Temperatur")]
    public double Temperatur { get; set; }
}

public class DataRate
{
    [JsonProperty("lora")]
    [JsonPropertyName("lora")]
    public Lora Lora { get; set; }
}

public class EndDeviceIds
{
    [JsonProperty("device_id")]
    [JsonPropertyName("device_id")]
    public string DeviceId { get; set; }

    [JsonProperty("application_ids")]
    [JsonPropertyName("application_ids")]
    public ApplicationIds ApplicationIds { get; set; }

    [JsonProperty("dev_eui")]
    [JsonPropertyName("dev_eui")]
    public string DevEui { get; set; }

    [JsonProperty("join_eui")]
    [JsonPropertyName("join_eui")]
    public string JoinEui { get; set; }

    [JsonProperty("dev_addr")]
    [JsonPropertyName("dev_addr")]
    public string DevAddr { get; set; }
}

public class GatewayIds
{
    [JsonProperty("gateway_id")]
    [JsonPropertyName("gateway_id")]
    public string GatewayId { get; set; }

    [JsonProperty("eui")]
    [JsonPropertyName("eui")]
    public string Eui { get; set; }
}

public class Location
{
    [JsonProperty("latitude")]
    [JsonPropertyName("latitude")]
    public double Latitude { get; set; }

    [JsonProperty("longitude")]
    [JsonPropertyName("longitude")]
    public double Longitude { get; set; }

    [JsonProperty("altitude")]
    [JsonPropertyName("altitude")]
    public int Altitude { get; set; }

    [JsonProperty("source")]
    [JsonPropertyName("source")]
    public string Source { get; set; }
}

public class Lora
{
    [JsonProperty("bandwidth")]
    [JsonPropertyName("bandwidth")]
    public int Bandwidth { get; set; }

    [JsonProperty("spreading_factor")]
    [JsonPropertyName("spreading_factor")]
    public int SpreadingFactor { get; set; }

    [JsonProperty("coding_rate")]
    [JsonPropertyName("coding_rate")]
    public string CodingRate { get; set; }
}

public class NetworkIds
{
    [JsonProperty("net_id")]
    [JsonPropertyName("net_id")]
    public string NetId { get; set; }

    [JsonProperty("tenant_id")]
    [JsonPropertyName("tenant_id")]
    public string TenantId { get; set; }

    [JsonProperty("cluster_id")]
    [JsonPropertyName("cluster_id")]
    public string ClusterId { get; set; }

    [JsonProperty("cluster_address")]
    [JsonPropertyName("cluster_address")]
    public string ClusterAddress { get; set; }
}

public class RxMetadatum
{
    [JsonProperty("gateway_ids")]
    [JsonPropertyName("gateway_ids")]
    public GatewayIds GatewayIds { get; set; }

    [JsonProperty("time")]
    [JsonPropertyName("time")]
    public DateTime Time { get; set; }

    [JsonProperty("timestamp")]
    [JsonPropertyName("timestamp")]
    public object Timestamp { get; set; }

    [JsonProperty("rssi")]
    [JsonPropertyName("rssi")]
    public int Rssi { get; set; }

    [JsonProperty("channel_rssi")]
    [JsonPropertyName("channel_rssi")]
    public int ChannelRssi { get; set; }

    [JsonProperty("snr")]
    [JsonPropertyName("snr")]
    public double Snr { get; set; }

    [JsonProperty("location")]
    [JsonPropertyName("location")]
    public Location Location { get; set; }

    [JsonProperty("uplink_token")]
    [JsonPropertyName("uplink_token")]
    public string UplinkToken { get; set; }

    [JsonProperty("channel_index")]
    [JsonPropertyName("channel_index")]
    public int ChannelIndex { get; set; }

    [JsonProperty("received_at")]
    [JsonPropertyName("received_at")]
    public string ReceivedAt { get; set; }
}

public class Settings
{
    [JsonProperty("data_rate")]
    [JsonPropertyName("data_rate")]
    public DataRate DataRate { get; set; }

    [JsonProperty("frequency")]
    [JsonPropertyName("frequency")]
    public string Frequency { get; set; }

    [JsonProperty("timestamp")]
    [JsonPropertyName("timestamp")]
    public long Timestamp { get; set; }

    [JsonProperty("time")]
    [JsonPropertyName("time")]
    public DateTime Time { get; set; }
}

public class Temperature
{
    [JsonProperty("displayName")]
    [JsonPropertyName("displayName")]
    public string DisplayName { get; set; }

    [JsonProperty("unit")]
    [JsonPropertyName("unit")]
    public string Unit { get; set; }

    [JsonProperty("value")]
    [JsonPropertyName("value")]
    public double Value { get; set; }
}

public class UplinkMessage<T>
{
    [JsonProperty("session_key_id")]
    [JsonPropertyName("session_key_id")]
    public string SessionKeyId { get; set; }

    [JsonProperty("f_port")]
    [JsonPropertyName("f_port")]
    public int FPort { get; set; }

    [JsonProperty("f_cnt")]
    [JsonPropertyName("f_cnt")]
    public int FCnt { get; set; }

    [JsonProperty("frm_payload")]
    [JsonPropertyName("frm_payload")]
    public string FrmPayload { get; set; }

    [JsonProperty("decoded_payload")]
    [JsonPropertyName("decoded_payload")]
    public T DecodedPayload { get; set; }

    [JsonProperty("rx_metadata")]
    [JsonPropertyName("rx_metadata")]
    public List<RxMetadatum> RxMetadata { get; set; }

    [JsonProperty("settings")]
    [JsonPropertyName("settings")]
    public Settings Settings { get; set; }

    [JsonProperty("received_at")]
    [JsonPropertyName("received_at")]
    public string ReceivedAt { get; set; }

    [JsonProperty("consumed_airtime")]
    [JsonPropertyName("consumed_airtime")]
    public string ConsumedAirtime { get; set; }

    [JsonProperty("network_ids")]
    [JsonPropertyName("network_ids")]
    public NetworkIds NetworkIds { get; set; }
}

public class ApplicationIds
{
    [JsonProperty("application_id")]
    [JsonPropertyName("application_id")]
    public string ApplicationId { get; set; }
}

public class BatteryVoltage
{
    [JsonProperty("displayName")]
    [JsonPropertyName("displayName")]
    public string DisplayName { get; set; }

    [JsonProperty("unit")]
    [JsonPropertyName("unit")]
    public string Unit { get; set; }

    [JsonProperty("value")]
    [JsonPropertyName("value")]
    public double Value { get; set; }
}