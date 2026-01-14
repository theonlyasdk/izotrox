#include <iostream>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>

#include "Debug/Logger.hpp"
#include "Input/Input.hpp"
#include "Graphics/Canvas.hpp"
#include "Graphics/Painter.hpp"
#include "Graphics/Font.hpp"
#include "Graphics/Widget.hpp"
#include "Graphics/Container.hpp"
#include "Graphics/Label.hpp"
#include "Graphics/Button.hpp"
#include "Graphics/ProgressBar.hpp"
#include "Graphics/TextBox.hpp"

#ifdef __ANDROID__
    #include "Graphics/Framebuffer.hpp"
#else
    #include "Platform/Linux/SDLApplication.hpp"
#endif

using namespace Izo;

int main(int argc, char* argv[]) {
    Logger::instance().info("Izotrox Starting...");

    int width = 800;
    int height = 600;

#ifdef __ANDROID__
    Framebuffer fb;
    if (!fb.init()) {
        Logger::instance().error("Failed to initialize framebuffer!");
        return 1;
    }
    width = fb.width();
    height = fb.height();
#else
    SDLApplication app("Izotrox", width, height);
#endif

    // Create a backbuffer (Canvas)
    Canvas backbuffer(width, height);
    Painter painter(backbuffer);

    Font font("res/fonts/Roboto-Regular.ttf", 24.0f);
    if (!font.valid()) {
        Logger::instance().warn("Could not load font: res/fonts/Roboto-Regular.ttf");
    }

    Container root;
    root.set_pos(50, 50);
    root.set_size(width - 100, height - 100);

    auto lbl = std::make_shared<Label>("Izotrox UI Demo", &font, Color::White);
    root.add_child(lbl);

    auto btn1 = std::make_shared<Button>("Start Engine", &font);
    root.add_child(btn1);

    auto btn2 = std::make_shared<Button>("Settings", &font);
    root.add_child(btn2);

    auto btn3 = std::make_shared<Button>("Exit", &font);
    root.add_child(btn3);

    auto pb = std::make_shared<ProgressBar>(0.0f);
    root.add_child(pb);

    auto tb = std::make_shared<TextBox>("Enter something...", &font);
    root.add_child(tb);

    // Initial Layout
    root.layout();

    Input::instance().init();

    Logger::instance().info("Entering render loop...");

    int frames = 0;
    int display_fps = 0;
    auto last_time = std::chrono::steady_clock::now();

    // Force first draw
    Widget::invalidate();

    bool running = true;
    while (running) {
        
#ifndef __ANDROID__
        if (!app.pump_events()) {
            running = false;
        }
#endif
        
        root.update();

        if (Widget::is_dirty()) {
            backbuffer.clear(Color(20, 20, 20));

            root.draw(painter);

            if (font.valid()) {
                 std::string fps_text = "FPS: " + std::to_string(display_fps);
                 font.draw_text(painter, 10, 10, fps_text, Color::Green);
            }

#ifdef __ANDROID__
            if (fb.valid()) fb.swap_buffers(backbuffer);
#else
            app.present(backbuffer.pixels(), width, height);
#endif
            Widget::clear_dirty();
            
            frames++;
        }
        
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_time).count() >= 1) {
            display_fps = frames;
            frames = 0;
            last_time = now;
            // Only log FPS, don't force redraw just for FPS number update unless something else changes
            // Logger::instance().info("FPS: " + std::to_string(display_fps));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}