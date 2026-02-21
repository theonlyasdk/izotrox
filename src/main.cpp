/*
 * (c) theonlyasdk 2026
 *
 * Licensed under the Mozilla Public License 2.0
 * See the LICENSE file for more information
 */

#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <cstdlib>
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
#include "Graphics/Canvas.hpp"
#include "Graphics/Color.hpp"
#include "Graphics/Font.hpp"
#include "Graphics/Image.hpp"
#include "Graphics/Painter.hpp"
#include "Input/Input.hpp"
#include "Platform/Android/AndroidDevice.hpp"
#include "UI/Layout/LinearLayout.hpp"
#include "UI/View/View.hpp"
#include "UI/Widgets/Button.hpp"
#include "UI/Widgets/Label.hpp"
#include "UI/Widgets/ListBox.hpp"
#include "UI/Widgets/ListItem.hpp"
#include "UI/Widgets/ProgressBar.hpp"
#include "UI/Widgets/Slider.hpp"
#include "UI/Widgets/TextBox.hpp"
#include "UI/Widgets/Toast.hpp"
#include "UI/Widgets/Widget.hpp"
#include "Views/SecondView.hpp"
#include "Views/SplashScreen.hpp"
#include "Views/ThemePreviewView.hpp"

using namespace Izo;

void draw_debug_panel(Painter& painter, Font& font, float fps) {
    constexpr float UPDATE_FREQUENCY = 1.5f;  // every 1.5 seconds
    constexpr int PANEL_ROUNDNESS = 20;
    constexpr int PANEL_PADDING = 10;

    static float timer = 0;
    static std::string cached_text;
    static int cached_w = 0;
    static int cached_h = 0;

    int app_width = Application::the().width();
    int app_height = Application::the().height();

    timer += Application::the().delta();
    if (cached_text.empty() || timer >= UPDATE_FREQUENCY * 1000.0f) {
        float temp = SystemStats::cpu_temp();
        int mem = SystemStats::sys_free_memory_mb();
        int app_mem = SystemStats::app_memory_usage_mb();

        cached_text = std::format("FPS: {:.1f} | Temp: {:.1f}C | FreeMem: {}MB | AppMem: {}MB", fps, temp, mem, app_mem);
        cached_w = font.width(cached_text) + (PANEL_PADDING * 2);
        cached_h = font.height() + 10;
        timer = 0;
    }

    int pos_x = app_width / 2 - cached_w / 2;
    painter.fill_rounded_rect({pos_x, 10, cached_w, cached_h}, PANEL_ROUNDNESS, Color(0, 0, 0, 128));
    font.draw_text(painter, {PANEL_PADDING + pos_x, 15}, cached_text, Color::White);
}

const std::string try_parse_arguments(int argc, const char* argv[]) {
    // These variables will always be non-empty after parsing
    // The parser is guaranteed to populate these variables
    std::string theme_name = "default";
    std::string resource_root = "res";
    std::string save_theme_preview;
    bool debug_mode = false;

    ArgsParser parser("Izotrox - Experimental GUI engine for Android and Linux");
    parser.add_argument(theme_name, "theme", "t", "Name of the theme to load", false);
    parser.add_argument(resource_root, "resource-root", "r", "Resource root directory", false);
    parser.add_argument(save_theme_preview, "save-theme-preview", "p", "Save theme preview to file. Specify a custom theme using --theme", false);
    parser.add_argument(debug_mode, "debug", "d", "Enables debug mode", false);

    ArgsParser::ParseResult result = parser.parse(argc, argv);

    if (result == ArgsParser::ParseResult::ParseError) {
        return parser.last_error();
    } else if (result == ArgsParser::ParseResult::HelpRequested) {
        std::cout << parser.help_str();
        std::exit(0);
    } else {
        // Otherwise, it's always ParseResult::ParseOK
    }

    Settings::the().set<std::string>("theme-name", theme_name);

    if (!ResourceManagerBase::is_valid_resource_dir(resource_root)) {
        return "Invalid resource path: " + resource_root;
    }

    ResourceManagerBase::set_resource_root(resource_root);
    Settings::the().set<std::string>("resource-root", resource_root);

    if (!save_theme_preview.empty())
        Settings::the().set<std::string>("preview-path", save_theme_preview);

    Settings::the().set<bool>("debug", debug_mode);

    return "";
}

void on_sigint(int) {
    Application::the().quit(0);
}

static void register_signal_handlers() {
    // std::atexit(on_exit);
    struct sigaction sa{};
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, nullptr);
}

int main(int argc, const char* argv[]) {
    register_signal_handlers();

    Settings::the().set<std::string>("theme-name", "default");
    Settings::the().set<std::string>("resource-root", "res");

    auto parse_error = try_parse_arguments(argc, argv);
    if (!parse_error.empty()) {
        LogError("Error parsing arguments: {}", parse_error);
        return 1;
    }

    // Logger::the().enable_logging_to_file();

    LogInfo("Izotrox v{}.{}.{} Booting... (compiled on {}, {})",
            IZO_VERSION_MAJOR, IZO_VERSION_MINOR, IZO_VERSION_REVISION,
            IZO_BUILD_DATE, IZO_BUILD_TIME);

    bool headless = Settings::the().has("preview-path");
    LogTrace("Headless mode: {}", headless);
    auto theme_name = Settings::the().get<std::string>("theme-name");
    std::string theme_path = "themes/" + theme_name + ".ini";

    if (!ThemeDB::the().load(theme_path)) {
        LogWarn("Failed to load theme '{}', falling back to 'default.ini'", theme_path);

        if (!ThemeDB::the().load("themes/default.ini")) {
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

    app.set_debug(Settings::the().get_or<bool>("debug", true));

    auto canvas = std::make_unique<Canvas>(width, height);
    Painter painter(std::move(canvas));

    auto systemFont = FontManager::the().get_or_crash("system-ui");

    if (!systemFont) {
        LogFatal("Could not load system font!");
        return 1;
    }

    ImageManager::the().load("slider-handle", "icons/slider-handle.png");
    ImageManager::the().load("slider-handle-focus", "icons/slider-handle-focus.png");

    if (headless) {
        auto preview_path = Settings::the().get<std::string>("preview-path");
        auto preview_view = ThemePreviewView::create();
        painter.canvas()->clear(ThemeDB::the().get<Color>("Colors", "Window.Background", Color(255)));

        int w = painter.canvas()->width();
        int h = painter.canvas()->height();
        ViewManager::the().resize(w, h);
        ViewManager::the().push(std::move(preview_view), ViewTransition::None);
        ViewManager::the().update();
        ViewManager::the().draw(painter);

        // Save to file
        if (painter.canvas()->save_to_file(preview_path)) {
            LogInfo("Theme preview saved to {}", preview_path);
            return 0;
        } else {
            LogError("Failed to save theme preview to {}", preview_path);
            return 1;
        }
    }

    SplashScreen splash(app, painter, *painter.canvas(), *systemFont);

    splash.set_total_steps(5);

    splash.next_step("Initializing Input...");
    Input::the().init();

    splash.next_step("Loading Images...");

    splash.next_step("Loading Fonts...");

    auto inconsolata = FontManager::the().load("inconsolata", "fonts/Inconsolata-Regular.ttf", 18.0f);

    ToastManager::the().set_font(FontManager::the().get("system-ui"));

    splash.next_step("Building UI...");

    auto root = std::make_unique<LinearLayout>(Orientation::Vertical);
    root->set_width(WidgetSizePolicy::MatchParent);
    root->set_height(WidgetSizePolicy::MatchParent);
    root->set_show_focus_indicator(false);
    root->set_padding(20);

    auto lbl_title = std::make_unique<Label>("Izotrox UI Demo");
    lbl_title->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(std::move(lbl_title));

    auto btn_start_engine = std::make_unique<Button>("Start Engine");
    btn_start_engine->set_focusable(true);
    root->add_child(std::move(btn_start_engine));

    auto btn_settings = std::make_unique<Button>("Settings");
    btn_settings->set_focusable(true);
    root->add_child(std::move(btn_settings));

    auto btn_crash_app = std::make_unique<Button>("Crash App");
    btn_crash_app->set_focusable(true);
    btn_crash_app->set_on_click([&running]() {
        /* LogFatal automatically calls the application destructor and
        terminates the app (if LOG_FATAL_TERMINATE_APP is enabled)
        with error code defined in LOG_FATAL_EXIT_CODE */
        LogFatal("UwU app crashed");
    });
    root->add_child(std::move(btn_crash_app));

    auto btn_second_view = std::make_unique<Button>("Go to Second View");
    btn_second_view->set_focusable(true);
    btn_second_view->set_width(WidgetSizePolicy::MatchParent);
    btn_second_view->set_on_click([]() {
        auto secondView = SecondView::create();
        ViewManager::the().push(std::move(secondView));
    });
    root->add_child(std::move(btn_second_view));

    auto pb_demo = std::make_unique<ProgressBar>(0.0f);
    ProgressBar* pb_demo_ptr = pb_demo.get();  // Keep raw pointer for lambda capture
    root->add_child(std::move(pb_demo));

    auto slider_demo = std::make_unique<Slider>(0.5f);
    slider_demo->set_width(WidgetSizePolicy::MatchParent);
    slider_demo->set_on_change([pb_demo_ptr](float v) {
        pb_demo_ptr->set_progress(v);
    });
    root->add_child(std::move(slider_demo));

    auto tb_demo = std::make_unique<TextBox>("Shell> ");
    TextBox* tb_demo_ptr = tb_demo.get();  // Keep raw pointer for lambda capture
    tb_demo->set_focusable(true);
    tb_demo->set_width(WidgetSizePolicy::MatchParent);
    tb_demo->set_on_submit([tb_demo_ptr](const std::string& text) {
        if (!text.empty()) {
            LogInfo("Shell> {}", text);
            IzoShell::the().execute(text);
            tb_demo_ptr->clear();
        }
    });
    root->add_child(std::move(tb_demo));

    auto lbl_multiline_demo = std::make_unique<Label>("Multi-line\nLabel Test");
    root->add_child(std::move(lbl_multiline_demo));

    auto lbl_wrap_demo = std::make_unique<Label>("This is a very long text that should automatically wrap to the next line if the container width is not enough to hold it in a single line.");
    lbl_wrap_demo->set_width(WidgetSizePolicy::MatchParent);
    lbl_wrap_demo->set_wrap(true);
    root->add_child(std::move(lbl_wrap_demo));

    auto listview = std::make_unique<ListBox>();
    listview->set_height(400);
    listview->set_width(WidgetSizePolicy::MatchParent);

    size_t MAX_LIST_ITEMS = 1000;
    for (size_t i = 0; i < MAX_LIST_ITEMS; ++i) {
        auto item = std::make_unique<ListItem>(Orientation::Vertical);

        auto label = std::make_unique<Label>("Item " + std::to_string(i));
        label->set_focusable(false);

        auto subLabel = std::make_unique<Label>("Details for " + std::to_string(i));
        subLabel->set_color_variant(ColorVariant::Secondary);
        subLabel->set_focusable(false);

        item->add_child(std::move(label));
        item->add_child(std::move(subLabel));

        listview->add_item(std::move(item));
    }
    root->add_child(std::move(listview));

    auto mainView = std::make_unique<View>(std::move(root));
    ViewManager::the().push(std::move(mainView), ViewTransition::None);
    ViewManager::the().resize(width, height);

    splash.next_step("Ready!");

    app.on_resize([&](int w, int h) {
        width = w;
        height = h;
        painter.canvas()->resize(w, h);
        painter.reset_clips_and_transform();
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
        ToastManager::the().update();

        painter.canvas()->clear(ThemeDB::the().get<Color>("Colors", "Window.Background", Color(255)));
        ViewManager::the().draw(painter);
        ToastManager::the().draw(painter, width, height);

        draw_debug_panel(painter, *inconsolata, current_fps);

        app.present(*painter.canvas());
    }

    LogInfo("Bye!");
    painter.canvas()->clear(Color::Black);
    app.present(*painter.canvas());

    return 0;
}
