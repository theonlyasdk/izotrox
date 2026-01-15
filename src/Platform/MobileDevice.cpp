#include "MobileDevice.hpp"
#include <fstream>
#include <string>

namespace Izo {

void MobileDevice::set_brightness(int value) {
#ifdef __ANDROID__
    // Try common brightness paths
    const char* paths[] = {
        "/sys/class/backlight/panel0-backlight/brightness",
        "/sys/class/leds/lcd-backlight/brightness",
        "/sys/class/backlight/intel_backlight/brightness"
    };

    for (const char* path : paths) {
        std::ofstream f(path);
        if (f.is_open()) {
            f << value;
            return;
        }
    }
#endif
}

} // namespace Izo
