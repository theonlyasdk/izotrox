#pragma once

#include <cstdint>
namespace Izo {

class AndroidDevice {
public:
    static void set_brightness(uint8_t value); // 0-255
};

} // namespace Izo
