#pragma once

#include <cstdint>
namespace Izo {

class AndroidDevice {
public:
    static void set_brightness(uint8_t value);
    static void set_front_flash(bool enable);
    static void set_back_flash(bool enable);
    static bool try_enable_otg();
    static bool try_disable_hw_overlays();
    static void set_screen_on(bool enable);
};

} 
