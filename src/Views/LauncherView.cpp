#include "Views/LauncherView.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>

#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include "Core/ViewManager.hpp"
#include "Graphics/Color.hpp"
#include "Graphics/Font.hpp"
#include "Graphics/Painter.hpp"
#include "UI/View/View.hpp"
#include "UI/Widgets/Widget.hpp"

namespace Izo {

namespace {

constexpr int kNavBarHeightPx = 74;
constexpr int kOuterInsetPx = 14;
constexpr int kTopInsetPx = 18;
constexpr int kSectionGapPx = 14;
constexpr int kIconLabelGapPx = 8;
constexpr int kIconLabelHeightPx = 20;
constexpr int kIconRoundnessPx = 22;
constexpr int kNavButtonGapPx = 8;
constexpr int kRecentCardHeightPx = 58;
constexpr int kRecentCardGapPx = 10;
constexpr int kRecentHeaderHeightPx = 42;
constexpr int kMaxRecentEntries = 48;
constexpr float kAppOpenDurationMs = 340.0f;
constexpr float kAppCloseDurationMs = 260.0f;
constexpr float kRecentsDurationMs = 180.0f;

struct LauncherApp {
    const char* name;
    char glyph;
    Color color;
};

constexpr std::array<LauncherApp, 12> kDummyApps{{
    {"Dummy App 1", '1', Color(86, 133, 244)},
    {"Dummy App 2", '2', Color(66, 184, 131)},
    {"Dummy App 3", '3', Color(240, 173, 78)},
    {"Dummy App 4", '4', Color(231, 111, 81)},
    {"Dummy App 5", '5', Color(181, 131, 218)},
    {"Dummy App 6", '6', Color(47, 161, 214)},
    {"Dummy App 7", '7', Color(237, 125, 49)},
    {"Dummy App 8", '8', Color(99, 205, 218)},
    {"Dummy App 9", '9', Color(158, 119, 238)},
    {"Dummy App 10", 'A', Color(244, 114, 182)},
    {"Dummy App 11", 'B', Color(64, 193, 114)},
    {"Dummy App 12", 'C', Color(109, 135, 247)},
}};

enum class HitKind : uint8_t {
    None,
    Icon,
    NavBack,
    NavHome,
    NavRecents,
    RecentItem,
};

struct HitTarget {
    HitKind kind = HitKind::None;
    int index = -1;
};

static bool same_target(const HitTarget& lhs, const HitTarget& rhs) {
    return lhs.kind == rhs.kind && lhs.index == rhs.index;
}

static Color with_alpha_scale(Color color, float alpha_scale) {
    alpha_scale = std::clamp(alpha_scale, 0.0f, 1.0f);
    color.a = static_cast<uint8_t>(static_cast<float>(color.a) * alpha_scale);
    return color;
}

static Color lerp_color(const Color& from, const Color& to, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    auto lerp_channel = [t](uint8_t a, uint8_t b) -> uint8_t {
        return static_cast<uint8_t>(a + static_cast<float>(b - a) * t);
    };
    return Color(
        lerp_channel(from.r, to.r),
        lerp_channel(from.g, to.g),
        lerp_channel(from.b, to.b),
        lerp_channel(from.a, to.a));
}

static IntRect lerp_rect(const IntRect& from, const IntRect& to, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    auto lerp_value = [t](int a, int b) -> int {
        return static_cast<int>(std::lround(static_cast<float>(a) + static_cast<float>(b - a) * t));
    };
    return {
        lerp_value(from.x, to.x),
        lerp_value(from.y, to.y),
        std::max(1, lerp_value(from.w, to.w)),
        std::max(1, lerp_value(from.h, to.h)),
    };
}

class LauncherRoot : public Widget {
   public:
    LauncherRoot() {
        set_widget_type("LauncherRoot");
        set_width(WidgetSizePolicy::MatchParent);
        set_height(WidgetSizePolicy::MatchParent);
        set_show_focus_indicator(false);
        m_recents_progress.snap_to(0.0f);
        on_theme_update();
    }

    void on_theme_update() override {
        Widget::on_theme_update();

        const Color window_bg = ThemeDB::the().get<Color>("Colors", "Window.Background", Color(18, 21, 30));
        m_color_wallpaper_top = lerp_color(window_bg, Color(58, 92, 160), 0.28f);
        m_color_wallpaper_bottom = lerp_color(window_bg, Color(19, 27, 44), 0.64f);

        m_color_icon_label = ThemeDB::the().get<Color>("Colors", "Text.Primary", Color(245, 247, 252));
        m_color_icon_glyph = Color(250, 250, 252);
        m_color_nav_bg = Color(22, 24, 32, 190);
        m_color_nav_button = Color(40, 44, 58, 200);
        m_color_nav_button_active = Color(80, 95, 128, 220);
        m_color_nav_text = Color(242, 246, 250);

        m_color_app_text = Color(245, 246, 250);
        m_color_recents_header = Color(230, 233, 242);
        m_color_recent_card = Color(33, 38, 50, 222);
        m_color_recent_text = Color(235, 237, 245);
        m_color_recent_meta = Color(180, 186, 205);
        invalidate_visual();
    }

    void measure(int parent_w, int parent_h) override {
        m_measured_size = {0, 0, parent_w, parent_h};
    }

    void update() override {
        Widget::update();

        const float dt_ms = Application::the().delta();
        const float prev_app = m_app_progress.value();
        const float prev_recents = m_recents_progress.value();
        const bool app_running_before = m_app_progress.running();
        const bool recents_running_before = m_recents_progress.running();

        m_app_progress.update(dt_ms);
        m_recents_progress.update(dt_ms);

        if (!m_app_target_open && !m_app_progress.running() && m_app_progress.value() <= 0.001f) {
            m_active_app = -1;
            m_transition_app = -1;
        }

        if (app_running_before || recents_running_before ||
            std::abs(prev_app - m_app_progress.value()) > 0.0001f ||
            std::abs(prev_recents - m_recents_progress.value()) > 0.0001f) {
            invalidate_visual();
        }
    }

    bool has_running_animations() const override {
        return Widget::has_running_animations() ||
               m_app_progress.running() ||
               m_recents_progress.running();
    }

    bool on_touch_event(IntPoint point, bool down) override {
        IntPoint global_point{
            point.x + global_bounds().x,
            point.y + global_bounds().y,
        };

        if (down) {
            if (!m_pointer_down) {
                m_pointer_down = true;
                m_pressed_target = hit_test(global_point);
                invalidate_visual();
                return m_pressed_target.kind != HitKind::None;
            }
            return m_pressed_target.kind != HitKind::None;
        }

        if (!m_pointer_down) {
            return false;
        }

        HitTarget release_target = hit_test(global_point);
        HitTarget pressed_target = m_pressed_target;
        m_pointer_down = false;
        m_pressed_target = {};
        invalidate_visual();

        if (!same_target(pressed_target, release_target)) {
            return pressed_target.kind != HitKind::None;
        }

        trigger_target(pressed_target);
        return pressed_target.kind != HitKind::None;
    }

    void draw_content(Painter& painter) override {
        draw_wallpaper(painter);

        const float app_progress = m_app_progress.value();
        const float recents_progress = m_recents_progress.value();
        const float icon_alpha = std::clamp(1.0f - app_progress * 0.2f, 0.0f, 1.0f);

        draw_icon_grid(painter, icon_alpha);

        if (recents_progress > 0.001f) {
            draw_recents(painter, recents_progress);
        }

        if (app_progress > 0.001f && m_active_app >= 0 && m_transition_app >= 0) {
            draw_opening_app(painter, app_progress);
        }

        draw_nav_bar(painter);
    }

   private:
    void trigger_target(const HitTarget& target) {
        switch (target.kind) {
            case HitKind::Icon:
                open_app(target.index);
                break;
            case HitKind::NavBack:
                handle_back();
                break;
            case HitKind::NavHome:
                handle_home();
                break;
            case HitKind::NavRecents:
                toggle_recents();
                break;
            case HitKind::RecentItem:
                open_app(target.index);
                break;
            default:
                break;
        }
    }

    void open_app(int app_index) {
        if (app_index < 0 || app_index >= static_cast<int>(kDummyApps.size())) return;

        add_recent(app_index);
        set_recents_visible(false);

        m_active_app = app_index;
        m_transition_app = app_index;
        m_app_target_open = true;
        m_app_progress.set_target(1.0f, kAppOpenDurationMs, Easing::EaseOutQuart);
        invalidate_visual();
    }

    void close_app() {
        if (m_active_app < 0 && m_app_progress.value() <= 0.001f) return;

        m_app_target_open = false;
        m_app_progress.set_target(0.0f, kAppCloseDurationMs, Easing::EaseInOutCubic);
        invalidate_visual();
    }

    void handle_home() {
        set_recents_visible(false);
        close_app();
    }

    void handle_back() {
        if (m_app_progress.value() > 0.001f || m_active_app >= 0) {
            close_app();
            return;
        }

        if (m_recents_visible || m_recents_progress.value() > 0.001f) {
            set_recents_visible(false);
            return;
        }

        ViewManager::the().pop(ViewTransition::PushRight);
    }

    void toggle_recents() {
        const bool target_visible = !(m_recents_visible || m_recents_progress.value() > 0.5f);
        set_recents_visible(target_visible);
        if (target_visible) {
            close_app();
        }
    }

    void set_recents_visible(bool visible) {
        m_recents_visible = visible;
        m_recents_progress.set_target(
            visible ? 1.0f : 0.0f,
            kRecentsDurationMs,
            Easing::EaseOutCubic);
        invalidate_visual();
    }

    void add_recent(int app_index) {
        if (app_index < 0 || app_index >= static_cast<int>(kDummyApps.size())) return;

        m_recent_ring[m_recent_head] = static_cast<uint8_t>(app_index);
        m_recent_head = (m_recent_head + 1) % kMaxRecentEntries;
        if (m_recent_count < kMaxRecentEntries) {
            ++m_recent_count;
        }
    }

    int recent_app_from_newest(int newest_offset) const {
        if (newest_offset < 0 || newest_offset >= m_recent_count) {
            return -1;
        }

        int pos = m_recent_head - 1 - newest_offset;
        if (pos < 0) {
            pos += kMaxRecentEntries;
        }
        return static_cast<int>(m_recent_ring[static_cast<size_t>(pos)]);
    }

    HitTarget hit_test(const IntPoint& global_point) const {
        for (int i = 0; i < 3; ++i) {
            IntRect nav_button = nav_button_rect(i);
            if (nav_button.contains(global_point)) {
                if (i == 0) return {HitKind::NavBack, 0};
                if (i == 1) return {HitKind::NavHome, 0};
                return {HitKind::NavRecents, 0};
            }
        }

        if (m_recents_progress.value() > 0.05f) {
            const int visible_count = recents_visible_count();
            for (int i = 0; i < visible_count; ++i) {
                IntRect card = recents_card_rect(i);
                if (!card.contains(global_point)) continue;

                int app_index = recent_app_from_newest(i);
                if (app_index >= 0) {
                    return {HitKind::RecentItem, app_index};
                }
                return {};
            }
            return {};
        }

        if (m_app_progress.value() > 0.03f) {
            return {};
        }

        for (int i = 0; i < static_cast<int>(kDummyApps.size()); ++i) {
            if (icon_rect_for_index(i).contains(global_point)) {
                return {HitKind::Icon, i};
            }
        }

        return {};
    }

    IntRect nav_bar_rect() const {
        IntRect b = global_bounds();
        int width = std::max(1, b.w - kOuterInsetPx * 2);
        int x = b.x + (b.w - width) / 2;
        int y = b.bottom() - kNavBarHeightPx - kOuterInsetPx;
        return {x, y, width, kNavBarHeightPx};
    }

    IntRect nav_button_rect(int button_index) const {
        IntRect bar = nav_bar_rect();
        int usable_w = std::max(1, bar.w - kNavButtonGapPx * 2);
        int button_w = std::max(1, usable_w / 3);
        int x = bar.x + button_index * (button_w + kNavButtonGapPx);
        int y = bar.y + 8;
        int h = std::max(1, bar.h - 16);
        return {x, y, button_w, h};
    }

    IntRect launcher_content_rect() const {
        IntRect b = global_bounds();
        IntRect nav = nav_bar_rect();
        int x = b.x + kOuterInsetPx;
        int y = b.y + kTopInsetPx;
        int w = std::max(1, b.w - kOuterInsetPx * 2);
        int h = std::max(1, nav.y - y - kSectionGapPx);
        return {x, y, w, h};
    }

    IntRect icon_rect_for_index(int index) const {
        IntRect area = launcher_content_rect();
        constexpr int kColumns = 4;
        const int app_count = static_cast<int>(kDummyApps.size());
        const int rows = (app_count + kColumns - 1) / kColumns;

        int horizontal_gap = std::max(8, area.w / 36);
        int cell_w = (area.w - horizontal_gap * (kColumns - 1)) / kColumns;
        int icon_size = std::clamp(cell_w, 52, 96);
        int row_step = icon_size + kIconLabelGapPx + kIconLabelHeightPx + std::max(8, icon_size / 7);
        int total_h = rows * row_step;

        int start_y = area.y + std::max(0, (area.h - total_h) / 2);

        int row = index / kColumns;
        int col = index % kColumns;
        int x = area.x + col * (cell_w + horizontal_gap) + std::max(0, (cell_w - icon_size) / 2);
        int y = start_y + row * row_step;
        return {x, y, icon_size, icon_size};
    }

    IntRect app_target_rect() const {
        IntRect b = global_bounds();
        IntRect nav = nav_bar_rect();
        int x = b.x + 8;
        int y = b.y + 8;
        int w = std::max(1, b.w - 16);
        int h = std::max(1, nav.y - y - 8);
        return {x, y, w, h};
    }

    int recents_visible_count() const {
        IntRect area = launcher_content_rect();
        int available_h = std::max(1, area.h - kRecentHeaderHeightPx);
        int per_card = kRecentCardHeightPx + kRecentCardGapPx;
        int max_cards = std::max(0, available_h / per_card);
        return std::min(m_recent_count, max_cards);
    }

    IntRect recents_card_rect(int position) const {
        IntRect area = launcher_content_rect();
        int x = area.x + 8;
        int y = area.y + kRecentHeaderHeightPx + position * (kRecentCardHeightPx + kRecentCardGapPx);
        int w = std::max(1, area.w - 16);
        return {x, y, w, kRecentCardHeightPx};
    }

    void draw_wallpaper(Painter& painter) {
        IntRect b = global_bounds();
        constexpr int kSteps = 28;
        for (int i = 0; i < kSteps; ++i) {
            float t0 = static_cast<float>(i) / static_cast<float>(kSteps);
            float t1 = static_cast<float>(i + 1) / static_cast<float>(kSteps);
            int y = b.y + static_cast<int>(std::floor(static_cast<float>(b.h) * t0));
            int next_y = b.y + static_cast<int>(std::ceil(static_cast<float>(b.h) * t1));
            int h = std::max(1, next_y - y);
            painter.fill_rect({b.x, y, b.w, h}, lerp_color(m_color_wallpaper_top, m_color_wallpaper_bottom, t0));
        }
    }

    void draw_icon_grid(Painter& painter, float alpha_scale) {
        if (!m_font) return;

        for (int i = 0; i < static_cast<int>(kDummyApps.size()); ++i) {
            const LauncherApp& app = kDummyApps[static_cast<size_t>(i)];
            IntRect icon = icon_rect_for_index(i);

            Color icon_color = with_alpha_scale(app.color, alpha_scale);
            painter.draw_drop_shadow_rect(icon, 10, with_alpha_scale(Color(0, 0, 0, 110), alpha_scale), kIconRoundnessPx, {0, 4});
            painter.fill_rounded_rect(icon, kIconRoundnessPx, icon_color);

            std::string glyph(1, app.glyph);
            int glyph_x = icon.x + (icon.w - m_font->width(glyph)) / 2;
            int glyph_y = icon.y + (icon.h - m_font->height()) / 2;
            m_font->draw_text(painter, {glyph_x, glyph_y}, glyph, with_alpha_scale(m_color_icon_glyph, alpha_scale));

            int label_w = m_font->width(app.name);
            int label_x = icon.x + (icon.w - label_w) / 2;
            int label_y = icon.bottom() + kIconLabelGapPx;
            m_font->draw_text(painter, {label_x, label_y}, app.name, with_alpha_scale(m_color_icon_label, alpha_scale));
        }
    }

    void draw_opening_app(Painter& painter, float progress) {
        if (!m_font || m_active_app < 0 || m_transition_app < 0) return;

        const LauncherApp& app = kDummyApps[static_cast<size_t>(m_active_app)];
        IntRect source = icon_rect_for_index(m_transition_app);
        IntRect target = app_target_rect();
        IntRect rect = lerp_rect(source, target, progress);
        int roundness = std::max(2, static_cast<int>(std::lround((1.0f - progress) * 24.0f)));

        painter.draw_drop_shadow_rect(rect, 18, Color(0, 0, 0, 120), std::max(2, roundness + 8), {0, 12});
        painter.fill_rounded_rect(rect, roundness, app.color);

        Color top_bar = lerp_color(with_alpha_scale(app.color, 1.0f), Color(255, 255, 255, 255), 0.12f);
        painter.fill_rect({rect.x, rect.y, rect.w, 42}, top_bar);

        float text_alpha = std::clamp((progress - 0.30f) / 0.70f, 0.0f, 1.0f);
        Color text_color = with_alpha_scale(m_color_app_text, text_alpha);

        int title_w = m_font->width(app.name);
        int title_x = rect.x + (rect.w - title_w) / 2;
        int title_y = rect.y + 12;
        m_font->draw_text(painter, {title_x, title_y}, app.name, text_color);

        for (int i = 0; i < 5; ++i) {
            int row_y = rect.y + 62 + i * 34;
            if (row_y + 22 >= rect.bottom()) break;

            int row_w = std::max(40, rect.w - 46 - (i % 2) * 38);
            painter.fill_rounded_rect(
                {rect.x + 18, row_y, row_w, 22},
                8,
                with_alpha_scale(Color(255, 255, 255, 40), text_alpha));
        }
    }

    void draw_recents(Painter& painter, float recents_progress) {
        if (!m_font) return;

        IntRect area = launcher_content_rect();
        painter.fill_rect(area, with_alpha_scale(Color(0, 0, 0, 120), recents_progress));

        m_font->draw_text(
            painter,
            {area.x + 10, area.y + 10},
            "Recent Apps",
            with_alpha_scale(m_color_recents_header, recents_progress));

        const int visible_count = recents_visible_count();
        for (int i = 0; i < visible_count; ++i) {
            int app_index = recent_app_from_newest(i);
            if (app_index < 0 || app_index >= static_cast<int>(kDummyApps.size())) continue;

            const LauncherApp& app = kDummyApps[static_cast<size_t>(app_index)];
            IntRect card = recents_card_rect(i);

            painter.fill_rounded_rect(card, 12, with_alpha_scale(m_color_recent_card, recents_progress));
            painter.fill_rounded_rect({card.x + 8, card.y + 8, 42, card.h - 16}, 10, with_alpha_scale(app.color, recents_progress));

            int title_x = card.x + 58;
            int title_y = card.y + 11;
            int subtitle_y = card.y + 31;
            m_font->draw_text(painter, {title_x, title_y}, app.name, with_alpha_scale(m_color_recent_text, recents_progress));
            m_font->draw_text(painter, {title_x, subtitle_y}, "Tap to reopen", with_alpha_scale(m_color_recent_meta, recents_progress));
        }

        if (visible_count == 0) {
            m_font->draw_text(
                painter,
                {area.x + 12, area.y + kRecentHeaderHeightPx + 16},
                "No recent apps yet",
                with_alpha_scale(m_color_recent_meta, recents_progress));
        }
    }

    void draw_nav_bar(Painter& painter) {
        if (!m_font) return;

        IntRect nav = nav_bar_rect();
        painter.draw_drop_shadow_rect(nav, 10, Color(0, 0, 0, 110), 18, {0, 5});
        painter.fill_rounded_rect(nav, 18, m_color_nav_bg);

        const std::array<const char*, 3> labels{"B", "H", "R"};
        for (int i = 0; i < 3; ++i) {
            IntRect btn = nav_button_rect(i);
            bool pressed = m_pointer_down &&
                           ((i == 0 && m_pressed_target.kind == HitKind::NavBack) ||
                            (i == 1 && m_pressed_target.kind == HitKind::NavHome) ||
                            (i == 2 && m_pressed_target.kind == HitKind::NavRecents));

            Color fill = pressed ? m_color_nav_button_active : m_color_nav_button;

            if (!pressed && i == 1 && m_app_progress.value() <= 0.001f && m_recents_progress.value() <= 0.001f) {
                fill = lerp_color(m_color_nav_button, m_color_nav_button_active, 0.38f);
            }
            if (!pressed && i == 2 && m_recents_progress.value() > 0.001f) {
                fill = lerp_color(m_color_nav_button, m_color_nav_button_active, 0.55f);
            }

            painter.fill_rounded_rect(btn, 12, fill);

            int label_w = m_font->width(labels[static_cast<size_t>(i)]);
            int label_x = btn.x + (btn.w - label_w) / 2;
            int label_y = btn.y + (btn.h - m_font->height()) / 2;
            m_font->draw_text(painter, {label_x, label_y}, labels[static_cast<size_t>(i)], m_color_nav_text);
        }
    }

    Animator<float> m_app_progress{0.0f};
    Animator<float> m_recents_progress{0.0f};

    bool m_app_target_open = false;
    bool m_recents_visible = false;
    bool m_pointer_down = false;

    int m_active_app = -1;
    int m_transition_app = -1;
    HitTarget m_pressed_target{};

    std::array<uint8_t, kMaxRecentEntries> m_recent_ring{};
    int m_recent_head = 0;
    int m_recent_count = 0;

    Color m_color_wallpaper_top{56, 86, 145};
    Color m_color_wallpaper_bottom{18, 26, 41};
    Color m_color_icon_label{245, 247, 252};
    Color m_color_icon_glyph{250, 250, 252};
    Color m_color_nav_bg{22, 24, 32, 190};
    Color m_color_nav_button{40, 44, 58, 200};
    Color m_color_nav_button_active{80, 95, 128, 220};
    Color m_color_nav_text{242, 246, 250};
    Color m_color_app_text{245, 246, 250};
    Color m_color_recents_header{230, 233, 242};
    Color m_color_recent_card{33, 38, 50, 222};
    Color m_color_recent_text{235, 237, 245};
    Color m_color_recent_meta{180, 186, 205};
};

}  // namespace

std::unique_ptr<View> LauncherView::create() {
    auto root = std::make_unique<LauncherRoot>();
    return std::make_unique<View>(std::move(root));
}

}  // namespace Izo
