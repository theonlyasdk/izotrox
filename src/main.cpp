/*
 * (c) theonlyasdk 2026
 *
 * Licensed under the Mozilla Public License 2.0
 * See the LICENSE file for more information
 */

#include <unistd.h>
#include <chrono>
#include <memory>
#include <string>
#include <format>

#include "Core/Izotrox.hpp"
#include "Debug/Logger.hpp"
#include "Graphics/Color.hpp"
#include "Input/Input.hpp"
#include "Graphics/Canvas.hpp"
#include "Graphics/Painter.hpp"
#include "Graphics/Font.hpp"
#include "Graphics/Widget.hpp"
#include "Graphics/LinearLayout.hpp"
#include "Graphics/Label.hpp"
#include "Graphics/Button.hpp"
#include "Graphics/ProgressBar.hpp"
#include "Graphics/TextBox.hpp"
#include "Graphics/Toast.hpp"
#include "Graphics/Slider.hpp"
#include "Graphics/Image.hpp"
#include "Core/ResourceManager.hpp"
#include "Views/SplashScreen.hpp"
#include "Graphics/View.hpp"
#include "Graphics/ListView.hpp"
#include "Graphics/ListItem.hpp"
#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include "Core/ViewManager.hpp"
#include "Core/IzoShell.hpp"
#include "Platform/Android/AndroidDevice.hpp"
#include "Core/SystemStats.hpp"
#include "Views/SecondView.hpp"
#include "Views/ThemePreviewView.hpp"

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

    painter.fill_rect({10, 10, cached_w, cached_h}, Color(0, 0, 0, 128));
    font.draw_text(painter, {20, 15}, cached_text, Color::White);
}

int main(int argc, char* argv[]) {
    Logger::the().enable_logging_to_file();

    LogInfo("Izotrox v{}.{}.{} Booting... (compiled on {}, {})", IZO_VERSION_MAJOR, IZO_VERSION_MINOR, IZO_VERSION_REVISION, IZO_BUILD_DATE, IZO_BUILD_TIME);

    /* TODO: Improve command line argument handling, possibly with a better library */
    std::string theme_name = "default";
    std::string preview_path;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--theme" && i + 1 < argc) {
            theme_name = argv[++i];
        } else if (arg == "--resource-root" && i + 1 < argc) {
            std::string root = argv[++i];
            ResourceManagerBase::set_resource_root(root);
        } else if (arg == "--save-theme-preview" && i + 1 < argc) {
            preview_path = argv[++i];
        }
    }

    int width = 800;
    int height = 600;

    bool headless = !preview_path.empty();
    std::string theme_path = "theme/" + theme_name + ".ini";

    if (!ThemeDB::the().load(theme_path)) {
        LogWarn("Failed to load theme '{}', falling back to 'default.ini'", theme_path);

        if (!ThemeDB::the().load("theme/default.ini")) {
            LogError("Unable to load the default theme! Izotrox might behave unexpectedly, and some colors might be missing!");
        }
    }

    bool running = true;

    Application app(width, height, "Izotrox");
    if (!app.init()) {
        LogFatal("Failed to initialize application!");
        return 1;
    }

    width = app.width();
    height = app.height();

    auto canvas = std::make_unique<Canvas>(width, height);
    auto painter = std::make_unique<Painter>(*canvas);

    FontManager fonts;
    ImageManager manager;

    std::string fontFamily = ThemeDB::the().get<std::string>("System", "FontFamily", "fonts/Inter-Bold.ttf");
    float fontSize = ThemeDB::the().get<float>("System", "FontSize", 64.0);
    Font* systemFont = fonts.load("system-ui", fontFamily, fontSize);

    if (!systemFont) {
        LogFatal("Could not load system font!");
        return 1;
    }
    
    // Load earlier for preview
    Image* sliderHandle = manager.load("slider-handle", "icons/slider-handle.png");
    Image* sliderHandleFocus = manager.load("slider-handle-focus", "icons/slider-handle-focus.png");

    if (!preview_path.empty()) {

        // Setup minimal environment for preview
        auto previewView = ThemePreviewView::create(systemFont, sliderHandle, sliderHandleFocus);
        
        // We need to simulate one frame
        canvas->clear(ThemeDB::the().get<Color>("Colors", "Window.Background", Color(255)));
        
        // Layout the view
        int w = width;
        int h = height; // Fixed size for preview
        
        // Manually setup view for rendering
        if (auto viewPtr = std::dynamic_pointer_cast<View>(previewView)) {
             // We need to set resizing
             // Since ViewManager manages resizing usually...
             // Let's just create ViewManager but push previewView
             ViewManager::the().resize(w, h);
             ViewManager::the().push(previewView, ViewTransition::None);
             
             // Update once
             ViewManager::the().update();
             ViewManager::the().draw(*painter);
        }
        
        // Save to file
        if (canvas->save_to_file(preview_path)) {
             LogInfo("Theme preview saved to {}", preview_path);
             return 0;
        } else {
             LogError("Failed to save theme preview to {}", preview_path);
             return 1;
        }
    }

    SplashScreen splash(app, *painter, *canvas, *systemFont);

    splash.set_total_steps(5);

    splash.next_step("Initializing Input...");
    Input::the().init();

    splash.next_step("Loading Images...");

    splash.next_step("Loading Fonts...");

    Font* toast_font = fonts.load("toast", fontFamily, fontSize/2);

    Font* inconsolata = fonts.load("inconsolata", "fonts/Inconsolata-Regular.ttf", 18.0f);
    ToastManager::the().set_font(toast_font);

    splash.next_step("Building UI...");

    auto root = std::make_shared<LinearLayout>(Orientation::Vertical);
    root->set_width(WidgetSizePolicy::MatchParent);
    root->set_height(WidgetSizePolicy::MatchParent);
    root->set_show_focus_indicator(false);

    auto lbl_title = std::make_shared<Label>("Izotrox UI Demo", systemFont);
    lbl_title->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(lbl_title);

    auto btn_start_engine = std::make_shared<Button>("Start Engine", systemFont);
    btn_start_engine->set_focusable(true);
    root->add_child(btn_start_engine);

    auto btn_settings = std::make_shared<Button>("Settings", systemFont);
    btn_settings->set_focusable(true);
    root->add_child(btn_settings);

    auto btn_crash_app = std::make_shared<Button>("Crash App", systemFont);
    btn_crash_app->set_focusable(true);
    btn_crash_app->set_on_click([&running]() {
        /* LogFatal automatically calls the application destructor and
        terminates the app (if LOG_FATAL_TERMINATE_APP is enabled)
        with error code defined in LOG_FATAL_EXIT_CODE */
        LogFatal("UwU app crashed");
        /* For now, we manually call Application::quit(1) */
        Application::the().quit(1);
    });
    root->add_child(btn_crash_app);

    auto btn_second_view = std::make_shared<Button>("Go to Second View", systemFont);
    btn_second_view->set_focusable(true);
    btn_second_view->set_width(WidgetSizePolicy::MatchParent);
    btn_second_view->set_on_click([systemFont]() {
        auto secondView = SecondView::create(systemFont);
        ViewManager::the().push(secondView);
    });
    root->add_child(btn_second_view);

    auto pb_demo = std::make_shared<ProgressBar>(0.0f);
    root->add_child(pb_demo);

    auto slider_demo = std::make_shared<Slider>(sliderHandle, sliderHandleFocus, 0.5f);
    slider_demo->set_width(WidgetSizePolicy::MatchParent);
    slider_demo->set_on_change([&](float v) {
        pb_demo->set_progress(v);
        AndroidDevice::set_brightness((v/100)*255);
    });
    root->add_child(slider_demo);

    auto tb_demo = std::make_shared<TextBox>("Shell> ", systemFont);
    tb_demo->set_focusable(true);
    tb_demo->set_width(WidgetSizePolicy::MatchParent);
    tb_demo->set_on_submit([&](const std::string& text) {
        if (!text.empty()) {
            Logger::the().info("Shell> " + text);
            IzoShell::the().execute(text);
            tb_demo->clear();
        }
    });
    root->add_child(tb_demo);

    auto lbl_multiline_demo = std::make_shared<Label>("Multi-line\nLabel Test", systemFont);
    root->add_child(lbl_multiline_demo);

    auto lbl_wrap_demo = std::make_shared<Label>("This is a very long text that should automatically wrap to the next line if the container width is not enough to hold it in a single line.", systemFont);
    lbl_wrap_demo->set_width(WidgetSizePolicy::MatchParent);
    lbl_wrap_demo->set_wrap(true);
    root->add_child(lbl_wrap_demo);

    auto listview = std::make_shared<ListView>();
    listview->set_height(400);
    listview->set_width(WidgetSizePolicy::MatchParent);

    size_t MAX_LIST_ITEMS = 1000;
    for (size_t i = 0; i < MAX_LIST_ITEMS; ++i) {
        auto item = std::make_shared<ListItem>(Orientation::Horizontal);

        item->set_padding(10, 10, 10, 10);

        auto label = std::make_shared<Label>("Item " + std::to_string(i), systemFont);
        label->set_focusable(false);

        auto subLabel = std::make_shared<Label>("Details for " + std::to_string(i), systemFont);
        subLabel->set_color_variant(ColorVariant::Secondary);
        subLabel->set_focusable(false);

        item->add_child(label);
        item->add_child(subLabel);

        listview->add_item(item);
    }
    root->add_child(listview);

    auto mainView = std::make_shared<View>(root);
    ViewManager::the().push(mainView, ViewTransition::None);
    ViewManager::the().resize(width, height);

    splash.next_step("Ready!");

    app.on_resize([&](int w, int h) {
        width = w;
        height = h;
        canvas->resize(w, h);
        painter->set_canvas(*canvas);
        ViewManager::the().resize(w, h);
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

        IntPoint tp = Input::the().touch_point();
        bool down = Input::the().touch_down();
        ViewManager::the().on_touch(tp, down);

        KeyCode key = Input::the().key();
        if (key != KeyCode::None) ViewManager::the().on_key(key);

        ViewManager::the().update();
        ToastManager::the().update(dt);

        canvas->clear(ThemeDB::the().get<Color>("Colors", "Window.Background", Color(255)));
        ViewManager::the().draw(*painter);
        ToastManager::the().draw(*painter, width, height);

        draw_debug_panel(*painter, *inconsolata, current_fps);

        app.present(*canvas);
    }

    Logger::the().info("Bye!");
    canvas->clear(Color::Black);
    app.present(*canvas);

    return 0;

}
