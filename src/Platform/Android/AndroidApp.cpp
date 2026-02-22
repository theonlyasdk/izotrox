#ifdef __ANDROID__

#include "Platform/Android/AndroidApp.hpp"

#include <cstdlib>

#include "Graphics/Canvas.hpp"
#include "Platform/Android/AndroidDevice.hpp"

namespace Izo {

AndroidApp::AndroidApp(int width, int height, std::string)
    : m_width(static_cast<uint32_t>(width)),
      m_height(static_cast<uint32_t>(height)) {
}

AndroidApp::~AndroidApp() {
    AndroidDevice::set_brightness(0);
}

bool AndroidApp::init() {
    std::system("stop");
    if (!m_fb.init()) {
        return false;
    }

    m_width = static_cast<uint32_t>(m_fb.width());
    m_height = static_cast<uint32_t>(m_fb.height());
    if (m_on_resize) {
        m_on_resize(static_cast<int>(m_width), static_cast<int>(m_height));
    }
    return true;
}

bool AndroidApp::pump_events() {
    return true;
}

void AndroidApp::present(Canvas& canvas, std::span<const IntRect> dirty_rects) {
    if (m_fb.valid()) {
        m_fb.swap_buffers(canvas, dirty_rects);
    }
}

void AndroidApp::quit(int exit_code) {
    AndroidDevice::set_brightness(0);
    std::system("start");
    std::exit(exit_code);
}

void AndroidApp::show() {
}

}

#endif
