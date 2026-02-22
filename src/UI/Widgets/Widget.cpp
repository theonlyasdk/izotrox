#include "UI/Widgets/Widget.hpp"
#include "Core/Application.hpp"
#include "Core/ThemeDB.hpp"
#include "Core/ViewManager.hpp"
#include "Geometry/Primitives.hpp"
#include "Graphics/Font.hpp"
#include "Graphics/Painter.hpp"
#include "Input/Input.hpp"
#include <algorithm>
#include <vector>

namespace Izo {

static std::vector<Widget*>& widget_registry() {
    static std::vector<Widget*> s_widgets;
    return s_widgets;
}

Widget::Widget() : m_bounds{0, 0, 0, 0}, m_measured_size{0, 0, 0, 0}, m_focus_anim(0.0f), m_prev_touch_down(false), m_touch_started_inside(false), m_focusable(true) {
    widget_registry().push_back(this);
    on_theme_update();
}

Widget::~Widget() {
    auto& widgets = widget_registry();
    widgets.erase(std::remove(widgets.begin(), widgets.end(), this), widgets.end());
}

void Widget::notify_theme_update_all() {
    auto widgets = widget_registry();
    for (auto* widget : widgets) {
        if (widget) {
            widget->on_theme_update();
        }
    }
}

void Widget::draw(Painter& painter) {
    if (!m_visible) return;
    draw_content(painter);
    draw_debug_info(painter);
}

void Widget::draw_focus(Painter& painter) {
    if (!m_visible) return;
    if (m_focusable && m_show_focus_indicator) {
        draw_focus_outline(painter);
    }
}

void Widget::draw_debug_info(Painter& painter) {
    if (!Application::the().debug_mode()) return;

    painter.outline_rect(global_bounds(), Color::Red);

    int text_width = m_font->width(widget_type());
    int text_height = m_font->height();
    IntPoint txt_widget_type_pos = {
        global_bounds().x + global_bounds().w - text_width,
        global_bounds().y + global_bounds().h - text_height,
    };

    m_font->draw_text(painter, txt_widget_type_pos, widget_type(), Color::Yellow);
}

void Widget::on_theme_update() {
    m_font = FontManager::the().get_or_crash("system-ui");
    m_focus_outline_thickness = ThemeDB::the().get<int>("WidgetParams", "Widget.FocusThickness", 12);
    m_focus_roundness = ThemeDB::the().get<int>("WidgetParams", "Widget.Roundness", 6);
    m_focus_color = ThemeDB::the().get<Color>("Colors", "Widget.Focus", Color(0, 0, 255));
    m_focus_anim_duration = ThemeDB::the().get<int>("WidgetParams", "Widget.FocusAnimDuration", 300);
    invalidate_layout();
}

void Widget::update() {
    m_focus_anim.update(Application::the().delta());
    if (m_focus_anim.running()) {
        invalidate_visual();
    }
}

void Widget::measure(int parent_w, int parent_h) {
    int w = 0, h = 0;
    if (m_width == (int)WidgetSizePolicy::MatchParent)
        w = parent_w;
    else if (m_width > 0)
        w = m_width;

    if (m_height == (int)WidgetSizePolicy::MatchParent)
        h = parent_h;
    else if (m_height > 0)
        h = m_height;

    m_measured_size = {0, 0, w, h};
}

void Widget::handle_focus_logic(bool inside, bool down) {
    if (down && !m_prev_touch_down) {
        m_touch_started_inside = inside;
        m_gesture_cancelled = false;

        if (!inside) {
            set_focused(false);
        }
    }

    if (!down && m_prev_touch_down && !m_gesture_cancelled) {
        if (inside && m_touch_started_inside && m_focusable) {
            set_focused(true);
        }
    }

    if (!down) {
        m_touch_started_inside = false;
        m_gesture_cancelled = false;
    }

    m_prev_touch_down = down;
}

bool Widget::on_touch(IntPoint point, bool down, bool captured) {
    if (!m_visible) return false;

    IntRect b = global_bounds();
    bool inside = b.contains(point);
    handle_focus_logic(inside, down);

    if (!inside && !captured) return false;

    IntPoint local_point = {point.x - b.x, point.y - b.y};

    return on_touch_event(local_point, down);
}

void Widget::draw_focus_outline(Painter& painter) {
    float t = m_focus_anim.value();
    if (t > 0) {
        float expansion = m_focus_outline_thickness * (1.0f - t);
        uint8_t alpha = (uint8_t)(255 * t);
        Color color(m_focus_color.r, m_focus_color.g, m_focus_color.b, alpha);

        IntRect b = global_bounds();

        int draw_thickness = (int)(m_focus_outline_thickness * (1.0f - t)) + 1;
        if (draw_thickness < 1) draw_thickness = 1;
        int exp_int = (int)expansion;

        painter.draw_rounded_rect({b.x - exp_int,
                                   b.y - exp_int,
                                   b.w + exp_int * 2,
                                   b.h + exp_int * 2},
                                  m_focus_roundness, color, draw_thickness);
    }
}

void Widget::set_focused(bool focused) {
    if (m_focused != focused) {
        invalidate_visual();
        m_focused = focused;
        m_focus_anim.set_target(m_focused ? 1.0f : 0.0f, m_focus_anim_duration, Easing::EaseOutCubic);
        invalidate_visual();
    }
}

bool Widget::hovering() const {
    return global_bounds().contains(Input::the().touch_point());
}

void Widget::set_font(Font* font) {
    if (m_font == font) return;
    m_font = font;
    invalidate_layout();
}

void Widget::show() {
    if (m_visible) return;
    m_visible = true;
    invalidate_layout();
}

void Widget::hide() {
    if (!m_visible) return;
    IntRect old_bounds = global_bounds();
    m_visible = false;
    ViewManager::the().invalidate_rect(old_bounds);
    invalidate_layout();
}

void Widget::set_padding(Padding padding) {
    set_padding_ltrb(padding.left, padding.top, padding.right, padding.bottom);
}

void Widget::set_height(int h) {
    if (m_height == h) return;
    m_height = h;
    invalidate_layout();
}

void Widget::set_height(WidgetSizePolicy p) {
    set_height((int)p);
}

void Widget::set_bounds(const IntRect& bounds) {
    if (m_bounds.x == bounds.x && m_bounds.y == bounds.y && m_bounds.w == bounds.w && m_bounds.h == bounds.h) {
        return;
    }

    IntRect old_global_bounds = global_bounds();
    m_bounds = bounds;

    if (m_visible) {
        ViewManager::the().invalidate_rect(old_global_bounds);
        ViewManager::the().invalidate_rect(global_bounds());
    }
}

void Widget::set_width(WidgetSizePolicy p) {
    set_width((int)p);
}

void Widget::set_width(int w) {
    if (m_width == w) return;
    m_width = w;
    invalidate_layout();
}

void Widget::set_focusable(bool focusable) {
    if (m_focusable == focusable) return;
    m_focusable = focusable;
    if (!m_focusable) {
        set_focused(false);
    }
    invalidate_visual();
}

void Widget::set_padding_ltrb(int left, int top, int right, int bottom) {
    if (m_padding_left == left && m_padding_top == top && m_padding_right == right && m_padding_bottom == bottom) {
        return;
    }

    m_padding_left = left;
    m_padding_top = top;
    m_padding_right = right;
    m_padding_bottom = bottom;
    invalidate_layout();
}

void Widget::set_padding(int padding) {
    set_padding_ltrb(padding, padding, padding, padding);
}

void Widget::set_show_focus_indicator(bool show) {
    if (m_show_focus_indicator == show) return;
    m_show_focus_indicator = show;
    invalidate_visual();
}

void Widget::set_parent(Widget* parent) {
    if (m_parent == parent) return;
    m_parent = parent;
    invalidate_layout();
}

void Widget::set_layout_index(int index) {
    m_layout_index = index;
}

void Widget::invalidate_visual() {
    if (!m_visible) return;
    ViewManager::the().invalidate_rect(global_bounds());
}

void Widget::invalidate_layout() {
    if (!m_layout_dirty) {
        m_layout_dirty = true;
        if (m_parent) {
            m_parent->invalidate_layout();
        }
    }
    invalidate_visual();
}

bool Widget::has_running_animations() const {
    return m_focus_anim.running();
}

const IntRect Widget::global_bounds() const {
    IntRect bounds = m_bounds;
    const Widget* current_parent = m_parent;
    while (current_parent) {
        IntPoint offset = current_parent->content_scroll_offset();
        bounds.x += offset.x;
        bounds.y += offset.y;
        current_parent = current_parent->parent();
    }
    return bounds;
}



}  // namespace Izo
