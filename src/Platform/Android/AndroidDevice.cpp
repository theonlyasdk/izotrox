#include "AndroidDevice.hpp"
#include "Debug/Logger.hpp"
#include <cstdint>
#include <cstdlib>
#include <format>
#include <fstream>
#include <string>
#include <sys/types.h>

namespace Izo {

void AndroidDevice::set_brightness(uint8_t value) {
    LogDebug("Setting brightness to {}", value);
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
    LogDebug("Setting front flash to {}", enable);
#ifdef __ANDROID__
    std::ofstream f("/sys/class/camera/flash/front_flash");
    if (f.is_open()) {
        f << (enable ? "1" : "0");
    }
#endif
}

void AndroidDevice::set_back_flash(bool enable) {
    LogDebug("Setting back flash to {}", enable);
#ifdef __ANDROID__
    std::ofstream f("/sys/class/camera/flash/back_flash");
    if (f.is_open()) {
        f << (enable ? "1" : "0");
    }
#endif
}

bool AndroidDevice::try_disable_hw_overlays() {
    LogDebug("Attempting to disable HW overlays...");
#ifdef __ANDROID__
    int ret = std::system("service call SurfaceFlinger 1008 i32 1");
    if (ret != 0) {
        LogWarn("Service call failed (Android stopped?), trying manual wipe...");
        return false;
    }
    return true;
#else
    return false;
#endif
}

bool AndroidDevice::try_enable_otg() {
    LogDebug("Attempting to enable OTG...");
#ifdef __ANDROID__
    bool success = false;

    auto write_to_file = [&](const std::string& path, const std::string& value, const std::string& log_msg) {
        std::ofstream f(path);
        if (f.is_open()) {
            f << value;
            LogDebug("OTG: {}", log_msg);
            success = true;
        }
    };

    // Samsung specific
    write_to_file("/sys/class/sec/switch/otg_cable_type", "on", "sec/switch enabled");

    // MTK specific
    write_to_file("/sys/devices/platform/mt_usb/musb-hdrc.0/mode", "b_host", "musb-hdrc b_host");

    // For other/generic vendors
    write_to_file("/sys/class/power_supply/usb/otg_switch", "1", "power_supply switch enabled");

    return success;
#else
    return false;
#endif
}

void AndroidDevice::set_screen_on(bool enable) {
    LogDebug("Setting screen to {}", enable);
    set_brightness(enable ? 255 : 0);
}

} 
