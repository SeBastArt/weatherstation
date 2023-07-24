/* Includes ------------------------------------------------------------------*/
#include <dummy.h>
#include "EPD.h"
#include "DEV_Config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "src/Display.h"

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const String host = "http://example.com";
const String http_port = "403";
const String resource = "/GetTemperature";

constexpr int date_start_x = 20;
constexpr int date_start_y = 90;

constexpr int time_start_x = 20;
constexpr int time_start_y = 40;

int time_to_sleep = 1;

/* Server and IP address ------------------------------------------------------*/
IPAddress my_ip;        // IP address in your local wifi net

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

float fetch_data_from_api(const char* eui)
{
    HTTPClient http;
    const String server_path = host + ":" + http_port + resource + "?eui=" + eui;
    
    http.begin(server_path.c_str());

    // Send HTTP GET request
    const int http_response_code = http.GET();

    float temperature = 0.0;

    if (http_response_code > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(http_response_code);
        const String payload = http.getString();
        temperature = payload.toFloat();
        Serial.println(payload);
    }
    else {
        Serial.print("Error code: ");
        Serial.println(http_response_code);
    }
    // Free resources
    http.end();

    return temperature;
}

/* Entry point ----------------------------------------------------------------*/
void setup()
{}

/* The main loop -------------------------------------------------------------*/
void loop()
{
    connect_to_wifi();
    {
        const auto*display = new e_ink_display_project::display();
        display->write_lines();
        display->write_time(time_start_x, time_start_y);
        display->write_date(date_start_x, date_start_y);
        display->write_room("Wohnzimmer", fetch_data_from_api("0004A30B001FDC0B"), 0, (EPD_7IN5_V2_HEIGHT / 3));
        display->write_room("Simon", fetch_data_from_api("A84041000181854C"), (EPD_7IN5_V2_WIDTH / 3), (EPD_7IN5_V2_HEIGHT / 3));
        display->write_room("Badezimmer", fetch_data_from_api("0004A30B002240C2"), 2 * (EPD_7IN5_V2_WIDTH / 3), (EPD_7IN5_V2_HEIGHT / 3));
        display->write_room("Schlafzimmer", fetch_data_from_api("0004A30B001FB02B"), 0, 2 * (EPD_7IN5_V2_HEIGHT / 3));
        display->write_room("Balkon", fetch_data_from_api("0025CA0A00000476"), (EPD_7IN5_V2_WIDTH / 3), 2 * (EPD_7IN5_V2_HEIGHT / 3));
        display->flush();
    }
    ESP.deepSleep(60e6);
}
