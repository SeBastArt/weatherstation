// Display.cpp

#include "Display.h" // Inkludieren Sie die entsprechende Header-Datei
#include <dummy.h>
#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include <stdlib.h>
#include <math.h>
#include "time.h"

using namespace e_ink_display_project; // Namespace verwenden

auto display::update_date_time() -> void
{
    configTime(gmt_offset_sec_, daylight_offset_sec_, ntp_server_);
    if (!getLocalTime(&time_info)) {
        Serial.println("Failed to obtain time -2 ");
    }
}

uint8_t* display::create_full_image() const
{
    //Create a new image cache
    UBYTE* black_image;
    UWORD imagesize = EPD_7IN5_V2_WIDTH / 8 * EPD_7IN5_V2_HEIGHT;
    if ((black_image = static_cast<uint8_t*>(malloc(imagesize))) == nullptr) {
        Serial.println("Failed to apply for black memory...\r\n");
        while (true);
    }

    //Serial.println("Paint_NewImage\r\n");
    Paint_NewImage(black_image, EPD_7IN5_V2_WIDTH, EPD_7IN5_V2_HEIGHT, 0, WHITE);
    return black_image;
}

display::display(): height_(EPD_7IN5_V2_HEIGHT), width_(EPD_7IN5_V2_WIDTH), time_info()
{
    DEV_Module_Init();
    update_date_time();
    Serial.println("e-Paper Init and Clear...\r\n");
    EPD_7IN5_V2_Init();
    full_image_ = create_full_image();
    Paint_SelectImage(full_image_);
    Paint_Clear(WHITE);
}

auto display::flush() const -> void
{
    Paint_SetRotate(180);
    EPD_7IN5_V2_Display(full_image_);
}

display::~display()
{
    free(full_image_);
    full_image_ = nullptr;
    DEV_Delay_ms(1000);
    Serial.println("Goto Sleep...\r\n");
    EPD_7IN5_V2_Sleep();
}

void display::write_room(const char* name, const double temperature, const UWORD x_start, const UWORD y_start) const
{
    const uint16_t temp_start_x = x_start + 15;
    const uint16_t temp_start_y = y_start + 70;
    write_temperature(temperature, temp_start_x, temp_start_y);
    Paint_DrawString_EN(x_start + 20, y_start + 15, name, &Font24, WHITE, BLACK);
}

auto display::write_temperature(const double temperature, const UWORD x_start, const UWORD y_start) const -> void
{
    constexpr int decimal_places = 1;
    constexpr int distance_grad_celcius = 170;

    const int length = 1 + decimal_places + (temperature < 0 ? 1 : 0) + (std::abs(temperature) >= 1 ? static_cast<int>(std::log10(std::abs(temperature))) : 0);

    // Erstelle den Puffer mit der richtigen Größe
    const auto buffer = new char[length + 1]; // +1 für den Nullterminator
    dtostrf(temperature, 0, decimal_places, buffer);

    UWORD width_counter = 0;
    for (int i = 0; i <= length; ++i) {
        const char single_char[] = { buffer[i], '\0' };
        Paint_DrawString_EN(x_start + width_counter, y_start, single_char, &FontPoiretOne, WHITE, BLACK);

        const char* single_char_ptr = single_char;
        switch (*single_char_ptr)
        {
        case '0':
            width_counter += 60;
            break;
        case '1':
            width_counter += 25;
            break;
        case '.':
            width_counter += 10;
            break;
        case '-':
            width_counter += 30;
            break;
        default:
            width_counter += 45;
            break;
        }
    }

    Paint_DrawString_EN(x_start + distance_grad_celcius, y_start - 5, "o", &Font24, WHITE, BLACK);
    Paint_DrawString_EN(x_start + distance_grad_celcius + 13, y_start, "C", &FontPoiretOne, WHITE, BLACK);
}

auto display::write_date(const UWORD x_start, const UWORD y_start) const -> void
{
    // Erstelle einen formatierten Text im gewünschten Format
    // Annahme: Der formatierte Text passt in 50 Zeichen
    char formatted_time[50]; 
    strftime(formatted_time, sizeof(formatted_time), "%a,", &time_info);
    Paint_DrawString_EN(x_start, y_start, formatted_time, &Font24, WHITE, BLACK);
    strftime(formatted_time, sizeof(formatted_time), "%d.%m.%Y", &time_info);
    Paint_DrawString_EN(x_start + 70, y_start, formatted_time, &Font24, WHITE, BLACK);
}

auto display::write_time(const UWORD x_start, const UWORD y_start) const -> void
{
    char formattedTime[50]; // Annahme: Der formatierte Text passt in 50 Zeichen
    
    strftime(formattedTime, sizeof(formattedTime), "%02H", &time_info);
    Paint_DrawString_EN(x_start, y_start, formattedTime, &Font48, WHITE, BLACK);

    Paint_DrawString_EN(x_start + 60, y_start, ":", &Font48, WHITE, BLACK);

    strftime(formattedTime, sizeof(formattedTime), "%02M", &time_info);
    Paint_DrawString_EN(x_start + 90, y_start, formattedTime, &Font48, WHITE, BLACK);
}

auto display::write_lines() const -> void
{
    Paint_DrawLine(2 * (width_ / 3), height_, 2 * (width_ / 3), height_ / 3, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(width_ / 3, height_, width_ / 3, height_ / 3, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(0, height_ / 3, width_, height_ / 3, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(0, 2 * (height_ / 3), width_, 2 * (height_ / 3), BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
}

auto display::clear_screen() -> void
{
    EPD_7IN5_V2_Clear();
}


