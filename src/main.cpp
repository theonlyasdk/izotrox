// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include <iostream>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>
#include <memory>
#include <string>

#include "Debug/Logger.hpp"
#include "Input/Input.hpp"
#include "Graphics/Canvas.hpp"
#include "Graphics/Painter.hpp"
#include "Graphics/Font.hpp"
#include "Graphics/Widget.hpp"
#include "Graphics/Container.hpp"
#include "Graphics/LinearLayout.hpp"
#include "Graphics/Label.hpp"
#include "Graphics/Button.hpp"
#include "Graphics/ProgressBar.hpp"
#include "Graphics/TextBox.hpp"
#include "Graphics/Slider.hpp"
#include "Graphics/Image.hpp"
#include "Graphics/ResourceManager.hpp"
#include "Graphics/SplashScreen.hpp"
#include "Graphics/View.hpp"
#include "Core/Application.hpp"
#include "Core/Theme.hpp"

using FontManager = Izo::ResourceManager<Izo::Font>;
using ImageManager = Izo::ResourceManager<Izo::Image>;

int main(int argc, char* argv[]) {
    Izo::Logger::instance().info("Izotrox Starting...");

    Izo::Theme::instance().load("res/theme/default.ini");

    int width = 800;
    int height = 600;

    Izo::Application app(width, height, "Izotrox");
    if (!app.init()) {
        Izo::Logger::instance().error("Failed to initialize application!");
        return 1;
    }
    
    width = app.width();
    height = app.height();

    auto backbuffer = std::make_unique<Izo::Canvas>(width, height);
    auto painter = std::make_unique<Izo::Painter>(*backbuffer);

    FontManager fonts;
    ImageManager images;

    Izo::Font* systemFont = fonts.load("system-ui", "res/fonts/Roboto-Regular.ttf", 24.0f);
    if (!systemFont) {
        Izo::Logger::instance().error("CRITICAL: Could not load system font!");
        return 1;
    }

    Izo::SplashScreen splash(app, *painter, *backbuffer, *systemFont);

    splash.set_total_steps(5); 
    
    splash.next_step("Initializing Input...");
    Izo::Input::instance().init();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    splash.next_step("Loading Images...");
    Izo::Image* sliderHandle = images.load("slider-handle", "res/icons/slider-handle.png");
    Izo::Image* sliderHandleFocus = images.load("slider-handle-focus", "res/icons/slider-handle-focus.png");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    splash.next_step("Loading Fonts...");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    splash.next_step("Building UI...");
    
    auto root = std::make_shared<Izo::LinearLayout>(Izo::Orientation::Vertical);
    root->set_width(Izo::MatchParent);
    root->set_height(Izo::MatchParent);

    // Create View
    Izo::View view(root);
    // Initial resize to setup layout
    view.resize(width, height);

    bool running = true;

    auto lbl = std::make_shared<Izo::Label>("Izotrox UI Demo", systemFont, Izo::Theme::instance().color("Label.Text"));
    lbl->set_width(Izo::MatchParent);
    root->add_child(lbl);

    auto btn1 = std::make_shared<Izo::Button>("Start Engine", systemFont);
    btn1->set_focusable(true);
    root->add_child(btn1);

    auto btn2 = std::make_shared<Izo::Button>("Settings", systemFont);
    btn2->set_focusable(true);
    root->add_child(btn2);

    auto btn3 = std::make_shared<Izo::Button>("Exit", systemFont);
    btn3->set_focusable(true);
    btn3->set_on_click([&running]() {
        Izo::Logger::instance().info("Exit clicked");
        running = false;
    });
    root->add_child(btn3);

    auto pb = std::make_shared<Izo::ProgressBar>(0.0f);
    root->add_child(pb);
    
    auto slider = std::make_shared<Izo::Slider>(sliderHandle, sliderHandleFocus, 0.5f);
    root->add_child(slider);

    auto tb = std::make_shared<Izo::TextBox>("Enter something...", systemFont);
    tb->set_focusable(true);
    root->add_child(tb);

    // Initial Layout via View resize?
    // We already called view.resize(). But we added children after.
    // Need to trigger resize/layout again.
    view.resize(width, height);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    splash.next_step("Ready!");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    app.set_on_resize([&](int w, int h) {
        width = w;
        height = h;
        backbuffer = std::make_unique<Izo::Canvas>(w, h);
        painter = std::make_unique<Izo::Painter>(*backbuffer);
        view.resize(w, h);
    });

    Izo::Logger::instance().info("Entering render loop...");

    root->invalidate();

    while (running) {
        if (!app.pump_events()) {
            running = false;
        }

        // Pass input to view
        int tx = Izo::Input::instance().touch_x();
        int ty = Izo::Input::instance().touch_y();
        bool down = Izo::Input::instance().touch_down();
        view.on_touch(tx, ty, down);
        
        int key = Izo::Input::instance().key();
        if (key > 0) view.on_key(key);
        
        view.update();

        if (Izo::Widget::is_dirty()) {
            backbuffer->clear(Izo::Theme::instance().color("Window.Background"));
            view.draw(*painter);

            app.present(*backbuffer);
            Izo::Widget::clear_dirty();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}
