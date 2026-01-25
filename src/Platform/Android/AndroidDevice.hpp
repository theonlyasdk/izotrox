#pragma once

#include <cstdint>
namespace Izo {

class AndroidDevice {
public:
    static void set_brightness(uint8_t value);
    static void set_front_flash(bool enable);
    static void set_back_flash(bool enable);

};

} 
