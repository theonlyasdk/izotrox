#include "Application.hpp"
#include "Platform/PlatformMacros.hpp"
#include <cstdint>

#ifdef __ANDROID__
    #include "Device/Framebuffer.hpp"
#else
    #include "Platform/Linux/SDLApplication.hpp"
#endif

namespace Izo {

/**
 * Application platform implementation
 */
struct Application::Impl {
    int width, height;
    std::function<void(int, int)> on_resize;

    IF_ANDROID(
        Framebuffer fb;
    )
    IF_DESKTOP(
        std::unique_ptr<SDLApplication> sdl_app;
    )

    Impl(int w, int h, const char* title) : width(w), height(h) {
        IF_DESKTOP(
            sdl_app = std::make_unique<SDLApplication>(title, w, h);
        )
    }
};

Application::Application(int width, int height, const char* title)
    : impl(std::make_unique<Impl>(width, height, title)) {
}

Application::~Application() {
    IF_ANDROID(
        AndroidDevice::set_brightness(0);
    )
}

bool Application::init() {
    IF_ANDROID(
        system("stop");
        if (!impl->fb.init()) 
            return false;
        impl->width = impl->fb.width();
        impl->height = impl->fb.height();
    )    
    return true;
}

bool Application::pump_events() {
    IF_ANDROID(
        // On Android, we just keep running until external kill or back button logic
        // For now always true
        return true;
    )
    IF_DESKTOP(
        if (impl->sdl_app) {
            return impl->sdl_app->pump_events();
        }
        return false;
    )
}

void Application::present(Canvas& canvas) {
    IF_ANDROID(
        if (impl->fb.valid()) 
            impl->fb.swap_buffers(canvas);
    )
    IF_DESKTOP(
        if (impl->sdl_app) {
            impl->sdl_app->present(canvas.pixels(), impl->width, impl->height);

            if (impl->sdl_app->width() != impl->width || 
                impl->sdl_app->height() != impl->height) {
                impl->width = impl->sdl_app->width();
                impl->height = impl->sdl_app->height();
                if (impl->on_resize) 
                    impl->on_resize(impl->width, impl->height);
            }
        }
    )
}

uint32_t Application::width() const { return impl->width; }
uint32_t Application::height() const { return impl->height; }

void Application::on_resize(std::function<void(int, int)> callback) {
    impl->on_resize = callback;
}

} // namespace Izo
