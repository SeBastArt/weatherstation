#include "mbedtls/md.h"
#include <ArduinoJson.h>
#include <locale>
#include <WiFi.h>
#include <PubSubClient.h>

#define MQTT_MAX_PACKET_SIZE 1024  // Erh?he die Puffergr??e auf 1 KB

const char* ssid = "wlansecure";
const char* password = "schwarzweiss";
String secretKey = "zYQuEpCGpnbMauktZsUEMm9iKE9DQqoWyeyemvqJ";

/* Server and IP address ------------------------------------------------------*/
IPAddress my_ip;
const char* mqtt_broker = "mqtt.wondering-developer.de";
const char* topic = "payment/receive";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

/* hmac algorithm ---------------------------------------------------------------*/
byte hmac_result[32];
mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

/* Ntp server time -------------------------------------------------------------------*/
const char* ntp_server_ = "pool.ntp.org";
const long  gmt_offset_sec_ = 3600;
const int   daylight_offset_sec_ = 3600;
struct tm time_info;

void connect_to_wifi()
{
    delay(10);

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    // Applying SSID and password
    WiFi.begin(ssid, password);

    // Waiting the connection to a router
    while (WiFiClass::status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // Connection is complete
    Serial.println("");
    Serial.println("WiFi connected");

    // Show obtained IP address in local Wifi net
    Serial.println(my_ip = WiFi.localIP());
}

auto update_date_time() -> void
{
    configTime(gmt_offset_sec_, daylight_offset_sec_, ntp_server_);
    if (!getLocalTime(&time_info)) {
        Serial.println("Failed to obtain time -2 ");
    }
}


bool IsCrcValid(const String& payload, String crc)
{
    mbedtls_md_context_t ctx;

    const size_t payloadLength = payload.length();
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, reinterpret_cast<const unsigned char*>(payload.c_str()), payloadLength);
    mbedtls_md_finish(&ctx, hmac_result);
    mbedtls_md_free(&ctx);

    String result = "";
    for (int i = 0; i < sizeof(hmac_result); i++) {
        if (hmac_result[i] < 0x10) {
            result += '0';
        }
        result += String(hmac_result[i], HEX);
    }
    result.toLowerCase();
    crc.toLowerCase();

    return result == crc;
}

bool IsHMACValid(String payload, String key, String hmac) {
    byte hmac_result[32];
    mbedtls_md_context_t ctx;

    const int payloadLength = payload.length();
    const int keyLength = key.length();

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, reinterpret_cast<const unsigned char*>(key.c_str()), keyLength);
    mbedtls_md_update(&ctx, reinterpret_cast<const unsigned char*>(payload.c_str()), payloadLength);

    mbedtls_md_hmac_finish(&ctx, hmac_result);
    mbedtls_md_free(&ctx);

    String result = "";
    for (int i = 0; i < sizeof(hmac_result); i++) {
        if (hmac_result[i] < 0x10) {
            result += '0';
        }
        result += String(hmac_result[i], HEX);
    }
    result.toLowerCase();
    hmac.toLowerCase();

    result.toLowerCase();
    hmac.toLowerCase();

    return result == hmac;
}

void subscribe_to_topic(const char* topic) {
    Serial.print("Subscribing to topic: ");
    Serial.println(topic);
    client.subscribe(topic);
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Received message [");
    Serial.print(topic);
    Serial.print("]: ");

    // get time first, for correct timestamp
    time_t now;
    time(&now);

    // format payload to String
    String payloadJsonStr;
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        payloadJsonStr += (char)payload[i];
    }
    Serial.println();
    // Deserialize to json format
    StaticJsonDocument<300> doc;
    if (deserializeJson(doc, payloadJsonStr.c_str())) {
        Serial.println("Failed to deserialize the payload");
        return;
    }

    //Payload - "timestamp:type:crc"
    String payloadStr = doc["Payload"].as<String>();
    int firstColon = payloadStr.indexOf(':');
    int secondColon = payloadStr.indexOf(':', firstColon + 1);

    String timestamp = payloadStr.substring(0, firstColon);
    String type = payloadStr.substring(firstColon + 1, secondColon);
    const String crc = payloadStr.substring(secondColon + 1);

    //Hmac
    String hmac = doc["Hmac"].as<String>();

    long unixTimestamp = doc["Timestamp"].as<long>();
    int time_diff = abs(now - unixTimestamp);

    // Test Timestamp, difference can be under 5sec - prevent replay attacks
    if (time_diff >= 5)
    {
        Serial.println("Timestamp not match - request discarded");
        return;
    }

    // Test CRC - prevent manipulations
    if (!IsCrcValid(timestamp + ":" + type, crc))
    {
        Serial.println("CRC is not correct - request discarded");
        return;
    }

    // Test HMAC - Authentication
    if (!IsHMACValid(payloadStr, secretKey, hmac))
    {
        Serial.println("HMAC is not correct - request discarded");
        return;
    }

    Serial.println("request is valid! - " + type);
}

// Function to convert Unix timestamp to local time
void unix_time_to_local_time(long int unix_timestamp) {
    // Create struct representing Unix timestamp
    struct tm* local_time = localtime(&unix_timestamp);

    // Convert to human-readable local time string
    char time_string[80];
    strftime(time_string, 80, "%Y-%m-%d %H:%M:%S", local_time);

    // Print the local time string
    printf("Unix timestamp %ld corresponds to local time %s\n", unix_timestamp, time_string);
}

void test_timestamp_diff(long int unix_timestamp) {
    // Get current time in Unix timestamp
    long int current_time = time(NULL);

    // Get the difference between the timestamps in seconds
    long int time_diff = current_time - unix_timestamp;

    // Check if the time difference is less than 30 seconds
    if (time_diff > -30 && time_diff < 30) {
        printf("The difference between the local time and the timestamp %ld is less than 30 seconds\n", unix_timestamp);
    }
    else {
        printf("The difference between the local time and the timestamp %ld is more than 30 seconds\n", unix_timestamp);
    }
}

void setup() {

    Serial.begin(115200);
    delay(1000);
    connect_to_wifi();
    update_date_time();
}

void reconnect() {
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str())) {
            Serial.println("connected to wondering-developer broker");
        }
        else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    subscribe_to_topic(topic);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}
