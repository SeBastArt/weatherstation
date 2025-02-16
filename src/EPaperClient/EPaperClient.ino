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
#define INIT_INTERVAL 15  // Alle 15 Minuten komplette Display-Init
RTC_DATA_ATTR int wakeup_count = 0;  // Bleibt im Speicher erhalten


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

bool fetch_data_array_from_api(const char* eui, const char* resource, String& response) {
    HTTPClient http;
    const String server_path = host + ":" + http_port + resource + "?eui=" + eui;

    Serial.println("Fetching from: " + server_path);

    http.begin(server_path.c_str());
    int http_response_code = http.GET();

    if (http_response_code > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(http_response_code);
        response = http.getString();
        http.end();
        return true;
    }
    else {
        Serial.print("HTTP Error code: ");
        Serial.println(http_response_code);
        http.end();
        return false;
    }
}


float fetch_data_from_api(const char* eui)
{
    HTTPClient http;
    const String server_path = host + ":" + http_port + temperature_resource + "?eui=" + eui;

    http.begin(server_path.c_str());

    // Send HTTP GET request
    const int http_response_code = http.GET();

    if (http_response_code > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(http_response_code);
        const String payload = http.getString();
        Serial.println(payload);
        return payload.toFloat();
    }
    else {
        Serial.print("Error code: ");
        Serial.println(http_response_code);
        http.end();
        return NAN;
    }
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
    if (std::isnan(temperature)) {
        Serial.println("Fehler: Temperaturwert ist ungültig.");
        return;
    }


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

void draw_bar(uint16_t x_start, uint16_t y_bottom, uint16_t bar_height, uint16_t bar_width, float value)
{
    // Text-Puffer für die Balkenbeschriftung
    constexpr int DECIMAL_PLACES = 0;
    constexpr size_t BUFFER_SIZE = 8;
    std::array<char, BUFFER_SIZE> buffer{};
    dtostrf(value, 0, DECIMAL_PLACES, buffer.data());

    // Graues Rechteck von (y_bottom - bar_height) bis y_bottom
    // x_start bis x_start + bar_width
    draw_gray_rectangle(x_start, y_bottom - bar_height, bar_width, bar_height, 4);

    // Nur rechts im Diagramm beschriften (Beispiel)
    if (x_start > display.width() / 2 + 50) {
        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(GxEPD_BLACK);
        int16_t cursor_offset = calculate_cursor_offset(strlen(buffer.data()));
        // Schrift soll oberhalb des Balkens erscheinen
        display.setCursor(static_cast<int16_t>(x_start + cursor_offset),
            static_cast<int16_t>(y_bottom - bar_height - 10));
        display.print(buffer.data());
    }
}


double mapValue(double value, double in_min, double in_max, double out_min, double out_max) {
    if (in_max == in_min) return out_min; // Verhindert Division durch Null
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Function to interpolate between points using Catmull-Rom spline
float interpolate(float p0, float p1, float p2, float p3, float t) {
    // Katmull-Rom-Spline-Interpolation für sanftere Kurven
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5f * ((2.0f * p1) + (-p0 + p2) * t + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

void drawSmoothLine(const float* line_values, size_t num_line_values, uint16_t x_graphic_start_x, uint16_t height, uint16_t bar_size) {
    if (num_line_values < 9) return;

    float from_value_y = height - mapValue(line_values[0], 0.0, 100.0, 0.0, static_cast<double>(height));
    float from_value_x = x_graphic_start_x + bar_size / 2 + 9 * (bar_size + 5);
    float step = 1.0 / 20; // Feinere Interpolation für eine glattere Linie

    for (size_t i = 9; i < num_line_values - 1; i++) {
        for (float t = 0; t < 1; t += step) {
            uint16_t to_value_x = x_graphic_start_x + (i + t + 1) * (bar_size + 5) + bar_size / 2;
            uint16_t to_value_y = static_cast<uint16_t>(height - mapValue(
                interpolate(line_values[i - 1], line_values[i], line_values[i + 1], line_values[i + 2], t),
                0.0, 100.0, 0.0, static_cast<double>(height)
            ));

            display.drawLine(
                static_cast<int16_t>(from_value_x), static_cast<int16_t>(from_value_y + 40),
                static_cast<int16_t>(to_value_x), static_cast<int16_t>(to_value_y + 40),
                GxEPD_BLACK
            );

            from_value_x = to_value_x;
            from_value_y = to_value_y;
        }
    }
}

/**
 * Mappt einen numerischen Wert aus einem Eingangsbereich [in_min, in_max]
 * auf einen Bildschirmbereich [out_min, out_max].
 *
 * @param value   Der Wert, der gemappt werden soll.
 * @param in_min  Minimaler Eingabewert (z.B. Temperatur min oder Luftfeuchtigkeit min).
 * @param in_max  Maximaler Eingabewert (z.B. Temperatur max oder Luftfeuchtigkeit max).
 * @param out_min Obere Grenze des Bildschirmbereichs (kleinere y-Koordinate, da oben).
 * @param out_max Untere Grenze des Bildschirmbereichs (größere y-Koordinate, da unten).
 * @return        Die entsprechende Y-Koordinate auf dem Bildschirm.
 */
static int16_t mapToScreen(float value, float in_min, float in_max, int16_t out_min, int16_t out_max) {
    if (fabs(in_max - in_min) < 1e-9) return out_max; // Schutz vor Division durch 0

    float fraction = (value - in_min) / (in_max - in_min); // Wert normalisieren
    float range = static_cast<float>(out_max - out_min);   // Bildschirmbereich

    // Umwandlung in Display-Koordinaten (y nimmt nach unten zu)
    float mapped = out_max - fraction * range;

    return static_cast<int16_t>(roundf(mapped));
}

/**
 * Zeichnet diskrete Humidity-Marker entlang der X-Achse.
 *
 * @param humidity_values Array der Luftfeuchtigkeitswerte (0–100%).
 * @param num_values      Anzahl der Werte.
 * @param x_start         Linke obere x-Koordinate des Zeichenbereichs.
 * @param y_start         Untere Grenze des Zeichenbereichs (z.B. y_start = untere Grenze).
 * @param width           Gesamte Breite des Zeichenbereichs.
 * @param height          Gesamte Höhe des Zeichenbereichs.
 */
void drawHumidityMarkers(const float* humidity_values, size_t num_values,
    uint16_t x_start, uint16_t y_start, uint16_t width, uint16_t height)
{
    if (num_values == 0) return;

    // Definiere den vertikalen Zeichenbereich:
    int16_t top = y_start - height;
    int16_t bottom = y_start;

    // Wir gehen davon aus, dass die Luftfeuchtigkeit zwischen 0 und 100 liegt.
    float in_min = 0.0f, in_max = 100.0f;

    // Berechne den horizontalen Abstand zwischen den Markern
    uint16_t spacing = width / num_values;

    // Definiere den Radius der Marker
    const uint8_t markerRadius = 4;

    // Zeichne Marker entlang der X-Achse
    for (size_t i = 0; i < num_values; i++) {
        // Berechne X-Position für den Marker (gleichmäßige Verteilung)
        uint16_t markerX = x_start + i * spacing + spacing / 2 - 3;

        if (markerX > display.width() / 3) {
            // Berechne Y-Position basierend auf dem Humidity-Wert
            int16_t markerY = mapToScreen(humidity_values[i], in_min, in_max, top, bottom);

            // Zeichne einen gefüllten Kreis an der berechneten Position
            display.fillCircle(markerX, markerY, markerRadius, GxEPD_WHITE);
            display.drawCircle(markerX, markerY, markerRadius, GxEPD_BLACK);
        }
    }
}



void draw_bar_histogram(
    const float* bar_values, size_t num_bar_values,
    const float* line_values, size_t num_line_values,
    uint16_t x_start, uint16_t y_start,
    uint16_t width, uint16_t height)
{
    if (num_bar_values == 0) return;

    int16_t top = y_start - height;  // Obere Grenze des Diagramms
    int16_t bottom = y_start;        // Untere Grenze des Diagramms

    float max_value = *std::max_element(bar_values, bar_values + num_bar_values);
    float min_value = *std::min_element(bar_values, bar_values + num_bar_values);

    uint16_t bar_size = (width / num_bar_values) - 5;
    int16_t zero_line = mapToScreen(0.0f, min_value, max_value, top, bottom);

    display.setPartialWindow(x_start, top, width, height);
    display.firstPage();

    do {
        display.fillScreen(GxEPD_WHITE);

        uint16_t x_current = x_start;

        for (size_t i = 0; i < num_bar_values; i++) {
            int16_t bar_top = mapToScreen(bar_values[i], min_value, max_value, top, bottom);
            int16_t bar_height = abs(zero_line - bar_top);
            bar_height = std::max<int16_t>(bar_height, static_cast<int16_t>(2));// Mindestens 2 Pixel hoch

            draw_bar(x_current, std::max(bar_top, zero_line), bar_height, bar_size, bar_values[i]);

            x_current += (bar_size + 5);
        }

        // Falls sowohl positive als auch negative Werte vorhanden sind -> Null-Linie zeichnen
        if (min_value < 0.0f && max_value > 0.0f) {
            uint16_t start_x = x_start + (width / 3);  // Linie startet bei 1/3 der X-Achse
            display.drawFastHLine(start_x, zero_line, width - (width / 3), GxEPD_BLACK);
        }

        // Zeichne Humidity-Marker falls genügend Werte vorhanden sind
        if (num_line_values >= 9) {
            drawHumidityMarkers(line_values, num_line_values, x_start, y_start, width, height);
        }

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

bool fetch_and_deserialize(const char* eui, const char* resource, DynamicJsonDocument& doc, String& json_response) {
    if (!fetch_data_array_from_api(eui, resource, json_response)) {
        Serial.println("fetch_data_array_from_api() failed.");
        return false;
    }

    Serial.println("JSON Response: " + json_response);

    DeserializationError error = deserializeJson(doc, json_response);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return false;
    }

    return true;
}

void extract_data(const JsonArray& array, float* data_array, int max_size) {
    int array_size = array.size();
    for (int i = 0; i < array_size && i < max_size; i++) {
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

bool find_min_max(const JsonArray& temp_array, float& min_value, float& max_value) {
    if (temp_array.size() == 0) {
        Serial.println("Error: Empty array in find_min_max()");
        return false;
    }

    // Initialisieren mit dem ersten Wert
    min_value = temp_array[0].as<float>();
    max_value = temp_array[0].as<float>();

    // Durch das Array iterieren und Min/Max finden
    for (int i = 1; i < temp_array.size(); i++) {
        float value = temp_array[i].as<float>();
        if (value < min_value) min_value = value;
        if (value > max_value) max_value = value;
    }

    return true;
}

void fetch_and_process(const char* eui, const char* resource, std::vector<float>& data_vector, float& min_value, float& max_value) {
    DynamicJsonDocument doc(1024);
    String json_response;

    if (!fetch_and_deserialize(eui, resource, doc, json_response)) {
        Serial.println("Fehler beim Abrufen der Daten.");
        return;
    }

    JsonArray json_array = doc.as<JsonArray>();

    if (json_array.isNull() || json_array.size() == 0) {
        Serial.println("Leere oder ungültige JSON-Antwort.");
        data_vector.clear();
        return;
    }

    data_vector.resize(json_array.size());
    for (size_t i = 0; i < json_array.size(); i++) {
        data_vector[i] = json_array[i].as<float>();
    }

    if (!data_vector.empty()) {
        min_value = *std::min_element(data_vector.begin(), data_vector.end());
        max_value = *std::max_element(data_vector.begin(), data_vector.end());
        Serial.printf("Min: %.2f, Max: %.2f\n", min_value, max_value);
    }
    else {
        Serial.println("Keine gültigen Werte erhalten.");
    }
}


void update_display()
{
    std::vector<float> temperature_data, humidity_data;
    float min_temp, max_temp, min_humidity, max_humidity;

    fetch_and_process("0004A30B001FB02B", "/api/ThingsNetwork/GetTemperaturesOfToday", temperature_data, min_temp, max_temp);
    fetch_and_process("0025CA0A00000476", "/api/ThingsNetwork/GetHumidityOfToday", humidity_data, min_humidity, max_humidity);

    if (temperature_data.empty() || humidity_data.empty()) {
        Serial.println("Fehler: Keine gültigen Daten erhalten.");
        return;  // Abbrechen, wenn keine Daten verfügbar sind.
    }

    display_min_max_values(min_temp, max_temp);


   
   /* std::vector<float> temperature_dummy = {
    -3.5, -2.8, -1.2, 0.0, 1.5, 3.2, 4.8, 5.6, 6.3, 7.1, 8.2, 9.0,
    8.5, 7.3, 6.1, 5.0, 4.2, 3.5, 2.0, 1.2, 0.0, -1.5, -2.7, -3.9
    };

    std::vector<float> humidity_dummy = {
        30.2, 32.8, 35.0, 38.1, 40.5, 42.0, 45.6, 47.2, 50.1, 52.8, 55.4, 57.0,
        58.3, 57.5, 56.1, 54.8, 52.0, 50.3, 47.1, 45.5, 42.0, 40.2, 38.0, 35.5
    };
   */


    draw_bar_histogram(temperature_data.data(), temperature_data.size(), humidity_data.data(), humidity_data.size(), 0, display.height() / 3, display.width(), display.height() / 3);

    write_room("Wohnzimmer", fetch_data_from_api("0004A30B001FDC0B"), 0, (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Simon", fetch_data_from_api("A84041000181854C"), (EPD_7IN5_V2_WIDTH / 3), (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Badezimmer", fetch_data_from_api("0004A30B002240C2"), 2 * (EPD_7IN5_V2_WIDTH / 3), (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Schlafzimmer", fetch_data_from_api("0025CA0A00000476"), 0, 2 * (EPD_7IN5_V2_HEIGHT / 3));
    write_room("Balkon", fetch_data_from_api("0004A30B001FB02B"), (EPD_7IN5_V2_WIDTH / 3), 2 * (EPD_7IN5_V2_HEIGHT / 3));
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
    static unsigned long lastUpdate = 0;
    const unsigned long updateInterval = 60000;  // 60 Sekunden

    // Falls 15 Minuten vergangen sind, mache eine vollständige Display-Init
    if (wakeup_count >= INIT_INTERVAL) {
        Serial.println("15-Minuten-Intervall erreicht -> Vollständige Display-Init");
        initialize_display();
        wakeup_count = 0;  // Zähler zurücksetzen
    }

    if (millis() - lastUpdate >= updateInterval) {
        wakeup_count++;
        lastUpdate = millis();
        update_date_time();
        update_display();
    }
}
