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
#include "Graphics/ListView.hpp"
#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include "Platform/MobileDevice.hpp"

using namespace Izo;

using FontManager = ResourceManager<Font>;
using ImageManager = ResourceManager<Image>;

int main(int argc, char* argv[]) {
    Logger::the().info("Izotrox Booting...");

    ThemeDB::the().load("res/theme/default.ini");

    int width = 800;
    int height = 600;

    Application app(width, height, "Izotrox");
    if (!app.init()) {
        Logger::the().error("Failed to initialize application!");
        return 1;
    }
    
    width = app.width();
    height = app.height();

    auto backbuffer = std::make_unique<Canvas>(width, height);
    auto painter = std::make_unique<Painter>(*backbuffer);

    FontManager fonts;
    ImageManager manager;

    Font* systemFont = fonts.load("system-ui", "res/fonts/Roboto-Regular.ttf", 24.0f);
    if (!systemFont) {
        Logger::the().error("CRITICAL: Could not load system font!");
        return 1;
    }

    SplashScreen splash(app, *painter, *backbuffer, *systemFont);

    splash.set_total_steps(5); 
    
    splash.next_step("Initializing Input...");
    Input::the().init();

    splash.next_step("Loading Images...");

    Image* sliderHandle = manager.load("slider-handle", "res/icons/slider-handle.png");
    Image* sliderHandleFocus = manager.load("slider-handle-focus", "res/icons/slider-handle-focus.png");

    splash.next_step("Loading Fonts...");
    // load extra fonts here...

    splash.next_step("Building UI...");

    auto root = std::make_shared<LinearLayout>(Orientation::Vertical);
    root->set_width(MatchParent);
    root->set_height(MatchParent);

    // Create View
    View view(root);
    view.resize(width, height);

    bool running = true;

    auto lbl = std::make_shared<Label>("Izotrox UI Demo", systemFont, ThemeDB::the().color("Label.Text"));
    lbl->set_width(MatchParent);
    root->add_child(lbl);

    auto btn1 = std::make_shared<Button>("Start Engine", systemFont);
    btn1->set_focusable(true);
    root->add_child(btn1);

    auto btn2 = std::make_shared<Button>("Settings", systemFont);
    btn2->set_focusable(true);
    root->add_child(btn2);

    auto btn3 = std::make_shared<Button>("Exit", systemFont);
    btn3->set_focusable(true);
    btn3->set_on_click([&running]() {
        Logger::the().info("Exit clicked");
        running = false;
    });
    root->add_child(btn3);

    auto pb = std::make_shared<ProgressBar>(0.0f);
    root->add_child(pb);
    
    auto slider = std::make_shared<Slider>(sliderHandle, sliderHandleFocus, 0.5f);
    root->add_child(slider);

    auto tb = std::make_shared<TextBox>("Enter something...", systemFont);
    tb->set_focusable(true);
    root->add_child(tb);
    
    // Demo Multiline
    auto multiLbl = std::make_shared<Label>("Multi-line\nLabel Test", systemFont, ThemeDB::the().color("Label.Text"));
    root->add_child(multiLbl);
    
    // Demo Wrap
    auto wrapLbl = std::make_shared<Label>("This is a very long text that should automatically wrap to the next line if the container width is not enough to hold it in a single line.", systemFont, ThemeDB::the().color("Label.Text"));
    wrapLbl->set_width(MatchParent);
    wrapLbl->set_wrap(true);
    root->add_child(wrapLbl);

    // ListView Demo
    auto listView = std::make_shared<ListView>();
    listView->set_width(MatchParent);
    listView->set_height(400); 
    
    std::vector<std::string> items;
    for(int i=0; i<50; ++i) {
        items.push_back("List Item " + std::to_string(i));
    }
    listView->set_items(items);
    
    listView->set_item_drawer([systemFont](Painter& p, int i, int x, int y, int w, int h) {
        std::string text = "List Item " + std::to_string(i);
        int th = systemFont->height();
        int ty = y + (h - th) / 2;
        systemFont->draw_text(p, x + 15, ty, text, ThemeDB::the().color("ListView.Text"));
    });
    
    root->add_child(listView);

    view.resize(width, height);

    splash.next_step("Ready!");

    app.on_resize([&](int w, int h) {
        width = w;
        height = h;
        backbuffer = std::make_unique<Canvas>(w, h);
        painter = std::make_unique<Painter>(*backbuffer);
        view.resize(w, h);
    });

    root->invalidate();

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastTime).count();
        lastTime = currentTime;

        if (!app.pump_events()) {
            running = false;
        }

        int tx = Input::the().touch_x();
        int ty = Input::the().touch_y();
        bool down = Input::the().touch_down();
        view.on_touch(tx, ty, down);
        
        int key = Input::the().key();
        if (key > 0) view.on_key(key); 
        
        view.update();

        bool is_dirty = Widget::dirty();

        if (is_dirty) {
            backbuffer->clear(ThemeDB::the().color("Window.Background"));
            view.draw(*painter);

            app.present(*backbuffer);
            Widget::clear_dirty();
        }
    }

    Logger::the().info("Shutting down...");
    backbuffer->clear(Color::Black);
    app.present(*backbuffer);

    return 0;
}