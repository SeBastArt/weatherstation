// Display.h

#ifndef DISPLAY_H // Header Guard, um Doppeldefinitionen zu verhindern
#define DISPLAY_H

#include <dummy.h>
#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "time.h"

namespace e_ink_display_project
{
    extern "C" bool getLocalTime(struct tm* info, uint32_t ms = 5000);

    class display {

        const char* ntp_server_ = "pool.ntp.org";
        const long  gmt_offset_sec_ = 3600;
        const int   daylight_offset_sec_ = 3600;
        UBYTE* full_image_{}; // Pointer auf ein Byte, das auf 0 zeigt
        uint16_t height_;
        uint16_t width_;
  
        auto update_date_time() -> void;
        UBYTE* create_full_image() const;
    public:
        struct tm time_info;
        display();
        ~display();
        auto write_room(const char* name, const double temperature, uint16_t x_start, uint16_t y_start) const -> void;
        auto write_temperature(double temperature, UWORD x_start, UWORD y_start) const -> void;
        auto write_date(UWORD x_start, UWORD y_start) const -> void;
        auto write_time(UWORD x_start, UWORD y_start) const -> void;
        auto write_lines() const -> void;
        static auto clear_screen() -> void;
        auto flush() const -> void;
    };
}

#endif // DISPLAY_H