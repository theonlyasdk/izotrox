// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>
#include <memory>
#include <string>
#include <format>
#include <sstream>

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
#include "Views/SplashScreen.hpp"
#include "Graphics/View.hpp"
#include "Graphics/ListView.hpp"
#include "Graphics/ListItem.hpp"
#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include "Platform/Android/AndroidDevice.hpp"
#include "Core/SystemStats.hpp"

using namespace Izo;

using FontManager = ResourceManager<Font>;
using ImageManager = ResourceManager<Image>;

void draw_debug_panel(Painter& painter, Font& font, float fps) {
    static float timer = 0;
    static std::string cached_text;
    static int cached_w = 0;
    static int cached_h = 0;

    timer += Application::the().delta();
    if (cached_text.empty() || timer >= 1000.0f) {
        float temp = SystemStats::get_cpu_temp();
        int mem = SystemStats::get_free_memory_mb();

        cached_text = std::format("FPS: {:.1f} | Temp: {:.1f}C | Mem: {}MB", fps, temp, mem);
        cached_w = font.width(cached_text) + 20;
        cached_h = font.height() + 10;
        timer = 0;
    }
    
    painter.fill_rect(10, 10, cached_w, cached_h, Color(0, 0, 0, 128)); 
    font.draw_text(painter, 20, 15, cached_text, Color::White);
}

int main(int argc, char* argv[]) {
    Logger::the().info("Izotrox Booting...");

    ThemeDB::the().load("res/theme/nord.ini");

    int width = 800;
    int height = 600;

    Application app(width, height, "Izotrox");
    if (!app.init()) {
        Logger::the().error("Failed to initialize application!");
        return 1;
    }
    
    width = app.width();
    height = app.height();

    auto canvas = std::make_unique<Canvas>(width, height);
    auto painter = std::make_unique<Painter>(*canvas);

    FontManager fonts;
    ImageManager manager;

    std::string fontFamily = ThemeDB::the().string_value("FontFamily", "res/fonts/Roboto-Regular.ttf");
    float fontSize = std::stof(ThemeDB::the().string_value("FontSize", "24.0"));

    Font* systemFont = fonts.load("system-ui", fontFamily, fontSize);
    Font* inconsolata = fonts.load("inconsolata", "res/fonts/Inconsolata-Regular.ttf", 18.0f);

    if (!systemFont) {
        Logger::the().error("Could not load system font!");
        return 1;
    }

    SplashScreen splash(app, *painter, *canvas, *systemFont);

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
    root->set_width(WidgetSizePolicy::MatchParent);
    root->set_height(WidgetSizePolicy::MatchParent);
    root->set_show_focus_indicator(false);

    // Create View
    View view(root);
    view.resize(width, height);

    bool running = true;

    auto lbl_title = std::make_shared<Label>("Izotrox UI Demo", systemFont, ThemeDB::the().color("Label.Text"));
    lbl_title->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(lbl_title);

    auto btn_start_engine = std::make_shared<Button>("Start Engine", systemFont);
    btn_start_engine->set_focusable(true);
    root->add_child(btn_start_engine);

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

    auto pb_demo = std::make_shared<ProgressBar>(0.0f);
    root->add_child(pb_demo);
    
    auto slider_demo = std::make_shared<Slider>(sliderHandle, sliderHandleFocus, 0.5f);
    slider_demo->set_width(WidgetSizePolicy::MatchParent);
    slider_demo->set_on_change([&](float v) {
        pb_demo->set_progress(v);
        AndroidDevice::set_brightness((v/100)*255)
    });
    root->add_child(slider_demo);

    auto tb_demo = std::make_shared<TextBox>("Enter something...", systemFont);
    tb_demo->set_focusable(true);
    tb_demo->set_width(WidgetSizePolicy::MatchParent);
    tb_demo->set_on_change([&](const std::string& text) {
        Logger::the().info("Text changed: " + text);
    });
    root->add_child(tb_demo);
    
    // Demo Multiline
    auto lbl_multiline_demo = std::make_shared<Label>("Multi-line\nLabel Test", systemFont, ThemeDB::the().color("Label.Text"));
    root->add_child(lbl_multiline_demo);
    
    // Demo Wrap
    auto lbl_wrap_demo = std::make_shared<Label>("This is a very long text that should automatically wrap to the next line if the container width is not enough to hold it in a single line.", systemFont, ThemeDB::the().color("Label.Text"));
    lbl_wrap_demo->set_width(WidgetSizePolicy::MatchParent);
    lbl_wrap_demo->set_wrap(true);
    root->add_child(lbl_wrap_demo);

    // ListView Demo
    auto listview = std::make_shared<ListView>();
    listview->set_height(400);
    listview->set_width(WidgetSizePolicy::MatchParent);

    for (int i = 0; i < 30; ++i) {
        auto item = std::make_shared<ListItem>(Orientation::Vertical);
        // item->set_height(60); 
        item->set_padding(10, 10, 5, 5);

        auto label = std::make_shared<Label>("Item #" + std::to_string(i), systemFont, ThemeDB::the().color("ListView.Text"));
        label->set_focusable(false);
        auto subLabel = std::make_shared<Label>("Details for " + std::to_string(i), systemFont, Color(150, 150, 150));
        subLabel->set_focusable(false);
        
        item->add_child(label);
        item->add_child(subLabel);

        listview->add_item(item);
    }
    root->add_child(listview);
    view.resize(width, height);

    splash.next_step("Ready!");

    app.on_resize([&](int w, int h) {
        width = w;
        height = h;
        canvas->resize(w, h);
        painter->set_canvas(*canvas);
        view.resize(w, h);
    });

    auto last_time = std::chrono::high_resolution_clock::now();
    int frame_count = 0;
    float fps_timer = 0;
    float current_fps = 0;

    while (running) {
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(now - last_time).count();
        last_time = now;

        app.set_delta(dt);

        frame_count++;
        fps_timer += dt;
        if (fps_timer >= 1000.0f) {
            current_fps = frame_count * 1000.0f / fps_timer;
            frame_count = 0;
            fps_timer = 0;
        }

        if (!app.pump_events()) {
            running = false;
        }

        int tx = Input::the().touch_x();
        int ty = Input::the().touch_y();
        bool down = Input::the().touch_down();
        view.on_touch(tx, ty, down);
        
        KeyCode key = Input::the().key();
        if (key != KeyCode::None) view.on_key(key); 
        
        view.update();

        canvas->clear(ThemeDB::the().color("Window.Background"));
        view.draw(*painter);

        draw_debug_panel(*painter, *inconsolata, current_fps);

        app.present(*canvas);
    }

    Logger::the().info("Bye!");
    canvas->clear(Color::Black);
    app.present(*canvas);

    return 0;
}
