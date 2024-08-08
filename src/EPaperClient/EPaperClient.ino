/* Includes ------------------------------------------------------------------*/
#include <dummy.h>
#include "EPD.h"
#include "DEV_Config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <ArduinoJson.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include "src/Display.h"
#include "src/GxEPD2_display_selection_new_style.h"
#include "src/fonts/WeatherIcons68pt7b.h"
#include "src/fonts/PoiretOne_Regular42pt7b.h"
#include "src/fonts/Oxygen_Bold36pt7b.h"
#include <vector>
#include <utility>

#define USE_HSPI_FOR_EPD

// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 0


const char* ssid = "Your WiFi Is In Another Castle";
const char* password = "65138264692885557410";
const String host = "https://weatherstation.wondering-developer.de";
const String http_port = "443";
const String temperature_resource = "/api/ThingsNetwork/GetTemperature";
const char* temperature_array_resource = "/api/ThingsNetwork/GetTemperaturesOfToday";
const char* humidity_array_resource = "/api/ThingsNetwork/GetHumidityOfToday";

constexpr int date_start_x = 20;
constexpr int date_start_y = 130;

constexpr int time_start_x = 20;
constexpr int time_start_y = 80;

const char* ntp_server_ = "pool.ntp.org";
const long  gmt_offset_sec_ = 3600;
const int   daylight_offset_sec_ = 3600;
struct tm time_info;

int time_to_sleep = 1;

const size_t capacity = JSON_ARRAY_SIZE(24) + 24 * sizeof(double);
DynamicJsonDocument doc(capacity);

/* Server and IP address ------------------------------------------------------*/
IPAddress my_ip;        // IP address in your local wifi net

#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
SPIClass hspi(HSPI);
#endif


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

const char* fetch_data_array_from_api(const char* eui, const char* ressource)
{
    HTTPClient http;
    const String server_path = host + ":" + http_port + ressource + "?eui=" + eui;

    http.begin(server_path.c_str());

    // Send HTTP GET request
    const int http_response_code = http.GET();

    String result;

    if (http_response_code > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(http_response_code);
        result = http.getString();
    }
    else {
        Serial.print("Error code: ");
        Serial.println(http_response_code);
    }
    // Free resources
    http.end();

    return result.c_str();
}


float fetch_data_from_api(const char* eui)
{
    HTTPClient http;
    const String server_path = host + ":" + http_port + temperature_resource + "?eui=" + eui;
    
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

auto write_date(const UWORD x_start, const UWORD y_start) -> void
{
    // Erstelle einen formatierten Text im gew�nschten Format
    // Annahme: Der formatierte Text passt in 50 Zeichen
    char formatted_time[50];
    strftime(formatted_time, sizeof(formatted_time), "%a,%d.%m.%Y", &time_info);

    display.setRotation(0);
    display.setFont(&FreeSans18pt7b);

    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds(formatted_time, 0, 0, &tbx, &tby, &tbw, &tbh);

    display.setTextColor(GxEPD_BLACK);
    display.setCursor(static_cast<int16_t>(x_start), static_cast<int16_t>(y_start));
    display.print(formatted_time);
}

auto write_time(const UWORD x_start, const UWORD y_start) -> void
{
    char formatted_time[50]; // Annahme: Der formatierte Text passt in 50 Zeichen
    strftime(formatted_time, sizeof(formatted_time), "%02H:%02M", &time_info);

    display.setRotation(0);
    display.setFont(&Oxygen_Bold36pt7b);

    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds(formatted_time, 0, 0, &tbx, &tby, &tbw, &tbh);

    display.setTextColor(GxEPD_BLACK);
    display.setCursor(static_cast<int16_t>(x_start), static_cast<int16_t>(y_start));
    display.print(formatted_time);
}

auto update_date_time() -> void
{
    configTime(gmt_offset_sec_, daylight_offset_sec_, ntp_server_);
    if (!getLocalTime(&time_info)) {
        Serial.println("Failed to obtain time -2 ");
    }
}

auto write_room(const char* name, const double temperature, const UWORD x_start, const UWORD y_start) -> void
{
    const uint16_t temp_start_x = x_start + 15;
    const uint16_t temp_start_y = y_start + 130;

    constexpr int decimal_places = 1;

    const int length = 1 + decimal_places + (temperature < 0 ? 1 : 0) + (std::abs(temperature) >= 1 ? static_cast<int>(std::log10(std::abs(temperature))) : 0);

    // create buffer with matching size
    const auto buffer = new char[length + 1]; // +1 f�r den Nullterminator
    dtostrf(temperature, 0, decimal_places, buffer);


    display.setRotation(0);
    display.setFont(&PoiretOne_Regular42pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setPartialWindow(x_start + 10, y_start + 5, display.width() / 3 - 20, display.height() / 3 - 10);
    do
    {
        constexpr int distance_grad_celsius = 170;
        display.fillScreen(GxEPD_WHITE);

    	display.setFont(&FreeSans12pt7b);
        display.setCursor(x_start + 20, y_start + 35);
        display.print(name);

        display.setFont(&PoiretOne_Regular42pt7b);
        display.setCursor(temp_start_x, temp_start_y);
        display.print(buffer);
        display.setCursor(temp_start_x + distance_grad_celsius + 10, temp_start_y);
        display.print("C");
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(temp_start_x + distance_grad_celsius, temp_start_y - 50);
        display.print("o");
    } while (display.nextPage());

}

void draw_lines()
{
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.drawFastVLine(display.width() / 3, display.height() / 3, 2 * display.height() / 3, GxEPD_BLACK);
        display.drawFastVLine(2 * display.width() / 3, display.height() / 3, 2 * display.height() / 3, GxEPD_BLACK);
        display.drawFastHLine(0, display.height() / 3, display.width(), GxEPD_BLACK);
        display.drawFastHLine(0, 2 * display.height() / 3, display.width(), GxEPD_BLACK);
    } while (display.nextPage());
}


void draw_gray_rectangle(const uint16_t x_start, const uint16_t y_start, const uint16_t width, const uint16_t height, const uint16_t light_level)
{

    for (int x = x_start; x < x_start + width; x++) {
        for (int y = y_start; y < y_start + height; y++) {
            if ((x + y) % light_level == 0) {
                display.drawPixel(static_cast<int16_t>(x), static_cast<int16_t>(y), GxEPD_BLACK);
            }
            else {
                display.drawPixel(static_cast<int16_t>(x), static_cast<int16_t>(y), GxEPD_WHITE);
            }
        }
    }

}

int calculate_buffer_length(const float value, const int decimal_places) {
    return 1 + decimal_places + (value < 0 ? 1 : 0) + (std::abs(value) >= 1 ? static_cast<int>(std::log10(std::abs(value))) : 0);
}

int16_t calculate_cursor_offset(const int length) {
    switch (length) {
    case 1: return 9;
    case 2: return 3;
    case 3: return -3;
    default: return 3;
    }
}

void draw_bar(const uint16_t x_start, const uint16_t y_start, const uint16_t height, const uint16_t width, const float value) {
    constexpr int decimal_places = 0;
    const int length = calculate_buffer_length(value, decimal_places);

    const std::unique_ptr<char[]> buffer(new char[length + 1]);
    dtostrf(value, 0, decimal_places, buffer.get());

    draw_gray_rectangle(x_start, y_start - height, width, height, 4);

    if (x_start > display.width() / 2 + 50) {
        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(GxEPD_BLACK);
        const int16_t cursor_offset = calculate_cursor_offset(length);
        display.setCursor(static_cast<int16_t>(x_start + cursor_offset), static_cast<int16_t>(y_start - height - 10));
        display.print(buffer.get());
    }
}

double mapValue(double value, double in_min, double in_max, double out_min, double out_max) {
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Function to interpolate between points using Catmull-Rom spline
float interpolate(float p0, float p1, float p2, float p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5 * ((2 * p1) + (-p0 + p2) * t + (2*p0 - 5*p1 + 4*p2 - p3) * t2 + (-p0 + 3*p1 - 3*p2 + p3) * t3);
}

void drawSmoothLine(const float* line_values, size_t num_line_values, uint16_t x_graphic_start_x, uint16_t height, uint16_t bar_size) {
    if (num_line_values < 9) {
        // Not enough points for spline interpolation
        return;
    }

    float from_value_y = height - mapValue(line_values[0], 0.0, 100.0, 0.0, static_cast<double>(height));
    float from_value_x = x_graphic_start_x + bar_size / 2 + 9 * (bar_size + 5);
    float step = 1.0 / 20; // Increase to get more smoothness

    for (size_t i = 9; i < num_line_values - 1; i++) {
        for (float t = 0; t < 1; t += step) {
            uint16_t to_value_x = x_graphic_start_x + (i + t + 1) * (bar_size + 5) + bar_size / 2;
            uint16_t to_value_y = static_cast<uint16_t>(height - mapValue(interpolate(line_values[i - 1], line_values[i], line_values[i + 1], line_values[i + 2], t), 0.0, 100.0, 0.0, static_cast<double>(height)));

            display.drawLine(static_cast<int16_t>(from_value_x), static_cast<int16_t>(from_value_y + 40), static_cast<int16_t>(to_value_x), static_cast<int16_t>(to_value_y + 40), GxEPD_BLACK);

            from_value_x = to_value_x;
            from_value_y = to_value_y;
        }
    }
}

void draw_bar_histogram(const float* bar_values, size_t num_bar_values, const float* line_values, size_t num_line_values, const uint16_t x_start, const uint16_t y_start, const uint16_t width, const uint16_t height) {

    uint16_t bar_size = width / num_bar_values - 5;
    uint16_t x_graphic_start_x = x_start;
    
    display.setPartialWindow(x_start, y_start - height, width, height);
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);

        int max_value = static_cast<int>(*std::max_element(bar_values, bar_values + num_bar_values));
        int min_value = static_cast<int>(*std::min_element(bar_values, bar_values + num_bar_values));

        for (size_t i = 0; i < num_bar_values; i++) {
	        uint16_t x = x_graphic_start_x + i * (bar_size + 5); // Calculate x-coordinate for the bar
	        draw_bar(x, y_start, map(bar_values[i], min_value, max_value + 5, 0, height), bar_size, bar_values[i]);
	    }
        
        drawSmoothLine(line_values, num_line_values, x_graphic_start_x, height, bar_size);

        write_time(time_start_x, time_start_y);
        write_date(date_start_x, date_start_y);

    } while (display.nextPage());
}

void display_min_max_values(float min_value, float max_value) {
    char min_str[10];
    char max_str[10];
    
    // Konvertiere die Float-Werte in Strings mit einer Nachkommastelle
    dtostrf(min_value, 4, 1, min_str);
    dtostrf(max_value, 4, 1, max_str);
    
    display.setPartialWindow(display.width() / 3 * 2 + 10, display.height() / 3 * 2 + 10, display.width() / 3, display.height() / 3);
    display.firstPage();
    do {
        display.setFont(&WeatherIcons68pt7b);
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(display.width() / 3 * 2 + 30, display.height() / 3 * 2 + 110);
        display.print("f");

        display.setFont(&FreeSans12pt7b);
        display.setTextColor(GxEPD_BLACK);

        display.setCursor(display.width() / 3 * 2 + 150, display.height() / 3 * 2 + 65);
        display.print("Min: ");
        display.print(min_str);

        display.setCursor(display.width() / 3 * 2 + 150, display.height() / 3 * 2 + 105);
        display.print("Max: ");
        display.print(max_str);
    } while (display.nextPage());
}

const char* fetch_and_deserialize(const char* eui, const char* resource, DynamicJsonDocument& doc) {
    const char* json_response = fetch_data_array_from_api(eui, resource);
    if (DeserializationError error = deserializeJson(doc, json_response))
    {
        json_response = fetch_data_array_from_api(eui, resource);
        if (DeserializationError error = deserializeJson(doc, json_response)) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return nullptr;
        }
    }
    return json_response;
}

void extract_data(const JsonArray& array, float* data_array) {
    for (int i = 0; i < array.size(); i++) {
        data_array[i] = array[i].as<float>();
    }
}

void initialize_display()
{
#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
    hspi.begin(13, 12, 14, 15); // remap hspi for EPD (swap pins)
    display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
#endif
    // *** end of special handling for Waveshare ESP32 Driver board *** //
    // **************************************************************** //
    display.init(115200);
    draw_lines();
}

void update_display()
{
    const char* temperature_json_response = fetch_and_deserialize("0004A30B001FB02B", temperature_array_resource, doc);
    if (!temperature_json_response) return;

    JsonArray temp_array = doc.as<JsonArray>();
    int temp_count = temp_array.size();
    float temperature_data[temp_count];
    extract_data(temp_array, temperature_data);

    float min_value = temp_array[0].as<float>();
    float max_value = temp_array[0].as<float>();

    for (int i = 1; i < temp_array.size(); i++) {
        float value = temp_array[i].as<float>();
        if (value < min_value) {
            min_value = value;
        }
        if (value > max_value) {
            max_value = value;
        }
    }
    
    const char* humidity_json_response = fetch_and_deserialize("0025CA0A00000476", humidity_array_resource, doc);
    if (!humidity_json_response) return;

    JsonArray humidity_array = doc.as<JsonArray>();
    int humidity_count = humidity_array.size();
    float humidity_data[humidity_count];
    extract_data(humidity_array, humidity_data);

    draw_bar_histogram(temperature_data, temp_count, humidity_data, humidity_count, 0, display.height() / 3, display.width(), display.height() / 3);
    
    write_room("Wohnzimmer", fetch_data_from_api("0004A30B001FDC0B"), 0, (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Simon", fetch_data_from_api("A84041000181854C"), (EPD_7IN5_V2_WIDTH / 3), (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Badezimmer", fetch_data_from_api("0004A30B002240C2"), 2 * (EPD_7IN5_V2_WIDTH / 3), (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Schlafzimmer", fetch_data_from_api("0025CA0A00000476"), 0, 2 * (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Balkon", fetch_data_from_api("0004A30B001FB02B"), (EPD_7IN5_V2_WIDTH / 3), 2 * (EPD_7IN5_V2_HEIGHT / 3));

    display_min_max_values(min_value, max_value);
}

void setup()
{
    connect_to_wifi();
    update_date_time();
    initialize_display();
    update_display();
}

void loop()
{
    delay(60000);
    update_date_time();
    update_display();
}
