/*
 * (c) theonlyasdk 2026
 *
 * Licensed under the Mozilla Public License 2.0
 * See the LICENSE file for more information
 */

#include <unistd.h>

#include <chrono>
#include <format>
#include <iostream>
#include <memory>
#include <string>

#include "Core/Application.hpp"
#include "Core/ArgsParser.hpp"
#include "Core/ResourceManager.hpp"
#include "Core/Settings.hpp"
#include "Core/SystemStats.hpp"
#include "Core/ThemeDB.hpp"
#include "Core/ViewManager.hpp"
#include "Debug/IzoShell.hpp"
#include "Debug/Izometa.hpp"
#include "Debug/Logger.hpp"
#include "UI/Widgets/Button.hpp"
#include "Graphics/Canvas.hpp"
#include "Graphics/Color.hpp"
#include "Graphics/Font.hpp"
#include "Graphics/Image.hpp"
#include "UI/Widgets/Label.hpp"
#include "Graphics/Painter.hpp"
#include "UI/Widgets/ProgressBar.hpp"
#include "UI/Widgets/Slider.hpp"
#include "UI/Widgets/TextBox.hpp"
#include "UI/Widgets/Toast.hpp"
#include "UI/View/View.hpp"
#include "Input/Input.hpp"
#include "Platform/Android/AndroidDevice.hpp"
#include "Views/SecondView.hpp"
#include "Views/SplashScreen.hpp"
#include "Views/ThemePreviewView.hpp"
#include "UI/Layout/LinearLayout.hpp"
#include "UI/Widgets/ListBox.hpp"
#include "UI/Widgets/ListItem.hpp"
#include "UI/Widgets/Widget.hpp"

using namespace Izo;

void draw_debug_panel(Painter& painter, Font& font, float fps) {
    static float timer = 0;
    static std::string cached_text;
    static int cached_w = 0;
    static int cached_h = 0;

    constexpr int update_interval_s = 1;
    timer += Application::the().delta();
    if (cached_text.empty() || timer >= update_interval_s * 1000.0f) {
        float temp = SystemStats::cpu_temp();
        int mem = SystemStats::free_memory_mb();
        int app_mem = SystemStats::app_memory_usage_mb();

        cached_text = std::format("FPS: {:.1f} | Temp: {:.1f}C | FreeMem: {}MB | AppMem: {}MB", fps, temp, mem, app_mem);
        cached_w = font.width(cached_text) + 20;
        cached_h = font.height() + 10;
        timer = 0;
    }

    painter.fill_rect({10, 10, cached_w, cached_h}, Color(0, 0, 0, 128));
    font.draw_text(painter, {20, 15}, cached_text, Color::White);
}

const std::string try_parse_arguments(int argc, const char* argv[]) {
    ArgsParser parser("Izotrox - Advanced UI Framework");
    parser.add_argument("theme", "t", "Theme name to load", false);
    parser.add_argument("resource-root", "r", "Resource root directory", false);
    parser.add_argument("save-theme-preview", "p", "Save theme preview to file", false);

    if (!parser.parse(argc, argv)) {
        return parser.get_error();
    }

    if (parser.help_requested()) {
        std::cout << parser.help_str();
        std::exit(0);
    }

    if (auto theme = parser.value("theme")) {
        Settings::the().set<std::string>("theme-name", theme.value());
    }

    if (auto root = parser.value("resource-root")) {
        ResourceManagerBase::set_resource_root(root.value());
        Settings::the().set<std::string>("resource-root", root.value());
    }

    if (auto preview = parser.value("save-theme-preview")) {
        Settings::the().set<std::string>("preview-path", preview.value());
    }

    return "";
}

std::unique_ptr<Canvas> canvas;
std::unique_ptr<Painter> painter;

int main(int argc, const char* argv[]) {
    Settings::the().set<std::string>("theme-name", "default");
    Settings::the().set<std::string>("resource-root", "res");

    auto parse_error = try_parse_arguments(argc, argv);
    if (!parse_error.empty()) {
        LogError("Error parsing arguments: {}", parse_error);
        return 1;
    }

    Logger::the().enable_logging_to_file();

    LogInfo("Izotrox v{}.{}.{} Booting... (compiled on {}, {})", IZO_VERSION_MAJOR, IZO_VERSION_MINOR, IZO_VERSION_REVISION, IZO_BUILD_DATE, IZO_BUILD_TIME);

    bool headless = Settings::the().has("preview-path");
    LogTrace("Headless mode: {}", headless);
    std::string theme_name = Settings::the().get<std::string>("theme-name");
    std::string theme_path = "theme/" + theme_name + ".ini";

    if (!ThemeDB::the().load(theme_path)) {
        LogWarn("Failed to load theme '{}', falling back to 'default.ini'", theme_path);

        if (!ThemeDB::the().load("theme/default.ini")) {
            LogError("Unable to load the default theme! Izotrox might behave unexpectedly, and some colors might be missing!");
        }
    }

    int width = 800;
    int height = 600;
    bool running = true;

    Application app(width, height, "Izotrox");
    if (!app.init()) {
        LogFatal("Failed to initialize application!");
        return 1;
    }

    width = app.width();
    height = app.height();

    canvas = std::make_unique<Canvas>(width, height);
    painter = std::make_unique<Painter>(*canvas);

    std::string fontFamily = ThemeDB::the().get<std::string>("System", "FontFamily", "fonts/Inter-Bold.ttf");
    float fontSize = ThemeDB::the().get<float>("System", "FontSize", 64.0);
    auto systemFont = FontManager::the().load("system-ui", fontFamily, fontSize).get();

    if (!systemFont) {
        LogFatal("Could not load system font!");
        return 1;
    }

    // Load earlier for preview
    auto sliderHandle = ImageManager::the().load("slider-handle", "icons/slider-handle.png").get();
    auto sliderHandleFocus = ImageManager::the().load("slider-handle-focus", "icons/slider-handle-focus.png").get();

    if (headless) {
        std::string preview_path = Settings::the().get<std::string>("preview-path");
        auto preview_view = ThemePreviewView::create(systemFont);
        canvas->clear(ThemeDB::the().get<Color>("Colors", "Window.Background", Color(255)));

        // Layout the view
        int w = canvas->width();   // Fixed size for preview
        int h = canvas->height();  // Fixed size for preview
        if (auto viewPtr = std::dynamic_pointer_cast<View>(preview_view)) {
            ViewManager::the().resize(w, h);
            ViewManager::the().push(preview_view, ViewTransition::None);
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
    
        LogInfo("Theme preview saved to '{}'", preview_path);
        return 0;
    }

    SplashScreen splash(app, *painter, *canvas, *systemFont);

    splash.set_total_steps(5);

    splash.next_step("Initializing Input...");
    Input::the().init();

    splash.next_step("Loading Images...");

    splash.next_step("Loading Fonts...");

    auto toast_font = FontManager::the().load("toast", fontFamily, fontSize / 2).get();
    auto inconsolata = FontManager::the().load("inconsolata", "fonts/Inconsolata-Regular.ttf", 18.0f).get();
    ToastManager::the().set_font(toast_font);

    splash.next_step("Building UI...");

    auto root = std::make_shared<LinearLayout>(Orientation::Vertical);
    root->set_width(WidgetSizePolicy::MatchParent);
    root->set_height(WidgetSizePolicy::MatchParent);
    root->set_show_focus_indicator(false);
    root->set_padding(20);

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

    auto slider_demo = std::make_shared<Slider>(0.5f);
    slider_demo->set_width(WidgetSizePolicy::MatchParent);
    slider_demo->set_on_change([&](float v) {
        pb_demo->set_progress(v);
        AndroidDevice::set_brightness((v / 100) * 255);
    });
    root->add_child(slider_demo);

    auto tb_demo = std::make_shared<TextBox>("Shell> ", systemFont);
    tb_demo->set_focusable(true);
    tb_demo->set_width(WidgetSizePolicy::MatchParent);
    tb_demo->set_on_submit([&](const std::string& text) {
        if (!text.empty()) {
            LogInfo("Shell> {}", text);
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

    auto listview = std::make_shared<ListBox>();
    listview->set_height(400);
    listview->set_width(WidgetSizePolicy::MatchParent);

    size_t MAX_LIST_ITEMS = 1000;
    for (size_t i = 0; i < MAX_LIST_ITEMS; ++i) {
        auto item = std::make_shared<ListItem>(Orientation::Vertical);

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

    LogInfo("Bye!");
    canvas->clear(Color::Black);
    app.present(*canvas);

    return 0;
}
