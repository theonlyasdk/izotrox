#include "AndroidDevice.hpp"
#include "Debug/Logger.hpp"
#include <cstdint>
#include <format>
#include <fstream>
#include <string>
#include <sys/types.h>

namespace Izo {

void AndroidDevice::set_brightness(uint8_t value) {
    Logger::the().info(std::format("Setting brightness to {}", value));
#ifdef __ANDROID__

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

void AndroidDevice::set_front_flash(bool enable) {
    Logger::the().info(std::format("Setting front flash to {}", enable));
#ifdef __ANDROID__
    std::ofstream f("/sys/class/camera/flash/front_flash");
    if (f.is_open()) {
        f << (enable ? "1" : "0");
    }
#endif
}

void AndroidDevice::set_back_flash(bool enable) {
    Logger::the().info(std::format("Setting back flash to {}", enable));
#ifdef __ANDROID__
    std::ofstream f("/sys/class/camera/flash/back_flash");
    if (f.is_open()) {
        f << (enable ? "1" : "0");
    }
#endif
}

} 
