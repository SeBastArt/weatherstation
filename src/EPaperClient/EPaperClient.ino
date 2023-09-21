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

#define USE_HSPI_FOR_EPD

// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 0


const char* ssid = "wifi_ssid";
const char* password = "wifi_password";
const String host = "https://host.de";
const String http_port = "443";
const String resource = "/api/resource/single";
const String array_resource = "/api/resource/array";

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

const char* fetch_data_array_from_api(const char* eui)
{
    HTTPClient http;
    const String server_path = host + ":" + http_port + array_resource + "?eui=" + eui;

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

auto write_date(const UWORD x_start, const UWORD y_start) -> void
{
    // Erstelle einen formatierten Text im gewünschten Format
    // Annahme: Der formatierte Text passt in 50 Zeichen
    char formatted_time[50];
    strftime(formatted_time, sizeof(formatted_time), "%a,%d.%m.%Y", &time_info);

    display.setRotation(0);
    display.setFont(&FreeSans18pt7b);

    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds(formatted_time, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t wh = FreeSans18pt7b.yAdvance;
    display.setPartialWindow(x_start, y_start - wh, tbw, wh + 10);

    display.setTextColor(GxEPD_BLACK);
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);

       
        display.setCursor(x_start, y_start);
        display.print(formatted_time);
    } while (display.nextPage());
}

auto write_time(const UWORD x_start, const UWORD y_start) -> void
{
    char formatted_time[50]; // Annahme: Der formatierte Text passt in 50 Zeichen
    strftime(formatted_time, sizeof(formatted_time), "%02H:%02M", &time_info);

    display.setRotation(0);
    display.setFont(&Oxygen_Bold36pt7b);

    int16_t tbx, tby; uint16_t tbw, tbh;
    display.getTextBounds(formatted_time, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t wh = Oxygen_Bold36pt7b.yAdvance;
    display.setPartialWindow(x_start, y_start - 52, display.width() / 3, 52 + 1);

    display.setTextColor(GxEPD_BLACK);
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
    	display.setCursor(x_start, y_start);
    	display.print(formatted_time);
    } while (display.nextPage());
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
    constexpr int distance_grad_celcius = 170;

    const int length = 1 + decimal_places + (temperature < 0 ? 1 : 0) + (std::abs(temperature) >= 1 ? static_cast<int>(std::log10(std::abs(temperature))) : 0);

    // create buffer with matching size
    const auto buffer = new char[length + 1]; // +1 für den Nullterminator
    dtostrf(temperature, 0, decimal_places, buffer);


    display.setRotation(0);
    display.setFont(&PoiretOne_Regular42pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setPartialWindow(x_start + 10, y_start + 5, display.width() / 3 - 20, display.height() / 3 - 10);
    do
    {
        display.fillScreen(GxEPD_WHITE);

    	display.setFont(&FreeSans12pt7b);
        display.setCursor(x_start + 20, y_start + 35);
        display.print(name);

        display.setFont(&PoiretOne_Regular42pt7b);
        display.setCursor(temp_start_x, temp_start_y);
        display.print(buffer);
        display.setCursor(temp_start_x + distance_grad_celcius + 10, temp_start_y);
        display.print("C");
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(temp_start_x + distance_grad_celcius, temp_start_y - 50);
        display.print("o");
    } while (display.nextPage());

}

void draw_lines()
{
    display.firstPage();
    do
    {
        //display.fillScreen(GxEPD_WHITE);
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
                display.drawPixel(x, y, GxEPD_BLACK);
            }
            else {
                display.drawPixel(x, y, GxEPD_WHITE);
            }
        }
    }

}

void draw_bar(const uint16_t x_start, const uint16_t y_start, const uint16_t height, const uint16_t width, const float value)
{

    constexpr int decimal_places = 0;
    constexpr int distance_grad_celcius = 170;

    const int length = 1 + decimal_places + (value < 0 ? 1 : 0) + (std::abs(value) >= 1 ? static_cast<int>(std::log10(std::abs(value))) : 0);

    // create buffer with matching size
    const auto buffer = new char[length + 1]; // +1 für den Nullterminator
    dtostrf(value, 0, decimal_places, buffer);

    draw_gray_rectangle(x_start, y_start - height, width, height, 4);

    if (x_start > display.width() / 2 + 50)
    {
	    display.setFont(&FreeMonoBold9pt7b);
	    display.setTextColor(GxEPD_BLACK);
	    uint16_t wh = FreeMonoBold9pt7b.yAdvance;
	    display.setCursor(x_start + 3, y_start - height - 10);
	    display.print(buffer);
    } 
}


void draw_bar_histogram(const float* bar_values, size_t num_bar_values, const float* line_values, size_t num_line_values, const uint16_t x_start, const uint16_t y_start, const uint16_t width, const uint16_t height) {

    uint16_t bar_size = width / num_bar_values - 5;
    uint16_t x_graphic_start_x = x_start;


    int16_t tbx, tby; uint16_t tbw, tbh;
    char formatted_time[50];

    display.setPartialWindow(x_start, y_start - height, width, height);
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
	    for (size_t i = 0; i < num_bar_values; i++) {
	        uint16_t x = x_graphic_start_x + i * (bar_size + 5); // Calculate x-coordinate for the bar
	        draw_bar(x, y_start, map(bar_values[i], 0.0, 40, 0, height), bar_size, bar_values[i]);
	    }

        float from_value_y = height - map(line_values[0], 0.0, 100, 0, height);
        float from_value_x = x_graphic_start_x + bar_size / 2;
        for (size_t i = 1; i < num_line_values; i++) {
            uint16_t to_value_x = x_graphic_start_x + i * (bar_size + 5) + bar_size / 2;
            uint16_t to_value_y =  height - map(line_values[i], 0.0, 100, 0, height);
            //display.drawLine(from_value_x, from_value_y, to_value_x, to_value_y, GxEPD_BLACK);
            from_value_x = to_value_x;
            from_value_y = to_value_y;
        }

        strftime(formatted_time, sizeof(formatted_time), "%02H:%02M", &time_info);

        display.setRotation(0);
        display.setFont(&Oxygen_Bold36pt7b);
        display.setTextColor(GxEPD_BLACK);
        display.getTextBounds(formatted_time, 0, 0, &tbx, &tby, &tbw, &tbh);

        display.setCursor(time_start_x, time_start_y);
        display.print(formatted_time);

       
        strftime(formatted_time, sizeof(formatted_time), "%a,%d.%m.%Y", &time_info);

        display.setRotation(0);
        display.setFont(&FreeSans18pt7b);
        display.setTextColor(GxEPD_BLACK);
       
        display.getTextBounds(formatted_time, 0, 0, &tbx, &tby, &tbw, &tbh);

        display.setCursor(date_start_x, date_start_y);
        display.print(formatted_time);

    } while (display.nextPage());
}



/* Entry point ----------------------------------------------------------------*/
void setup()
{
    connect_to_wifi();
    update_date_time();
#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
    hspi.begin(13, 12, 14, 15); // remap hspi for EPD (swap pins)
    display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
#endif
    // *** end of special handling for Waveshare ESP32 Driver board *** //
    // **************************************************************** //
    display.init(115200);
    draw_lines();

    float lineData[] =  { 51.8, 45.7, 35.6, 61.2, 72.1, 57.3, 76.6, 64.9, 40.5, 38.2, 59.7, 68.3, 33.9, 55.6, 71.0, 47.4, 63.8, 41.2, 76.3, 54.7, 69.5, 49.1, 43.8, 31.4 };
    size_t numLineValues = sizeof(lineData) / sizeof(lineData[0]);

    const char* jsonResponse = fetch_data_array_from_api("0004A30B001FB02B");
    DeserializationError error = deserializeJson(doc, jsonResponse);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    JsonArray array = doc.as<JsonArray>();
    int numberOfElements = array.size();
    float values[numberOfElements];
    for (int i = 0; i < array.size(); i++) {
        values[i] = array[i].as<float>();
    }
    //outside: 0004A30B001FB02B
    draw_bar_histogram(values, numberOfElements, lineData, numLineValues, 0 , display.height() / 3, display.width(), display.height() / 3);
   
    //write_time(time_start_x, time_start_y);
	//write_date(date_start_x, date_start_y);

    write_room("Wohnzimmer", fetch_data_from_api("0004A30B001FDC0B"), 0, (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Simon", fetch_data_from_api("A84041000181854C"), (EPD_7IN5_V2_WIDTH / 3), (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Badezimmer", fetch_data_from_api("0004A30B002240C2"), 2 * (EPD_7IN5_V2_WIDTH / 3), (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Schlafzimmer", fetch_data_from_api("0025CA0A00000476"), 0, 2 * (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Balkon", fetch_data_from_api("0004A30B001FB02B"), (EPD_7IN5_V2_WIDTH / 3), 2 * (EPD_7IN5_V2_HEIGHT / 3));

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
        display.print("Min: 16.5");

        display.setCursor(display.width() / 3 * 2 + 150, display.height() / 3 * 2 + 105);
        display.print("Max: 24.7");
    } while (display.nextPage());
}



/* The main loop -------------------------------------------------------------*/
void loop()
{
    delay(60000);
    update_date_time();

    float lineData[] = { 51.8, 45.7, 35.6, 61.2, 72.1, 57.3, 76.6, 64.9, 40.5, 38.2, 59.7, 68.3, 33.9, 55.6, 71.0, 47.4, 63.8, 41.2, 76.3, 54.7, 69.5, 49.1, 43.8, 31.4 };

    const char* jsonResponse = fetch_data_array_from_api("0004A30B001FB02B");
    DeserializationError error = deserializeJson(doc, jsonResponse);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    JsonArray array = doc.as<JsonArray>();
    int numberOfElements = array.size();
    float values[numberOfElements];
    for (int i = 0; i < array.size(); i++) {
        values[i] = array[i].as<float>();
    }

    size_t numLineValues = sizeof(lineData) / sizeof(lineData[0]);

    draw_bar_histogram(values, numberOfElements, lineData, numLineValues, 0, display.height() / 3, display.width(), display.height() / 3);

    //write_time(time_start_x, time_start_y);
    //write_date(date_start_x, date_start_y);

    write_room("Wohnzimmer", fetch_data_from_api("0004A30B001FDC0B"), 0, (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Simon", fetch_data_from_api("A84041000181854C"), (EPD_7IN5_V2_WIDTH / 3), (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Badezimmer", fetch_data_from_api("0004A30B002240C2"), 2 * (EPD_7IN5_V2_WIDTH / 3), (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Schlafzimmer", fetch_data_from_api("0025CA0A00000476"), 0, 2 * (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Balkon", fetch_data_from_api("0004A30B001FB02B"), (EPD_7IN5_V2_WIDTH / 3), 2 * (EPD_7IN5_V2_HEIGHT / 3));


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
        display.print("Min: 16.5");

        display.setCursor(display.width() / 3 * 2 + 150, display.height() / 3 * 2 + 105);
        display.print("Max: 24.7");
    } while (display.nextPage());
}
