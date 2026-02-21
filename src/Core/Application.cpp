#include "Core/Application.hpp"

#ifdef __ANDROID__
#include "Platform/Android/AndroidApp.hpp"
#else
#include "Platform/Linux/DesktopApp.hpp"
#endif

#include <utility>

#include "Graphics/Canvas.hpp"

namespace Izo {

Application* Application::_instance = nullptr;

Application::Application(int width, int height, const char* title)
    : m_width(static_cast<uint32_t>(width)),
      m_height(static_cast<uint32_t>(height)) {
#ifdef __ANDROID__
    m_backend = std::make_unique<AndroidApp>(width, height, title);
#else
    m_backend = std::make_unique<DesktopApp>(title, width, height);
#endif
    _instance = this;
}

Application::~Application() = default;

bool Application::init() {
    if (!m_backend) return false;

    m_backend->set_on_resize([this](int w, int h) {
        m_width = static_cast<uint32_t>(w);
        m_height = static_cast<uint32_t>(h);
        if (m_on_resize) {
            m_on_resize(w, h);
        }
    });

    if (!m_backend->init()) {
        return false;
    }

    m_width = m_backend->width();
    m_height = m_backend->height();
    return true;
}

bool Application::pump_events() {
    return m_backend ? m_backend->pump_events() : false;
}

void Application::present(Canvas& canvas) {
    if (m_backend) {
        m_backend->present(canvas);
    }
}

void Application::on_resize(std::function<void(int, int)> callback) {
    m_on_resize = std::move(callback);
}

void Application::quit(int exit_code) {
    if (m_backend) {
        m_backend->quit(exit_code);
    }
}

void Application::show() {
    if (m_backend) {
        m_backend->show();
    }
}

}
