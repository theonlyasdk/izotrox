
#include <algorithm>

#include "UI/Widgets/Slider.hpp"
#include "Core/ThemeDB.hpp"
#include "Core/ResourceManager.hpp"
#include "Graphics/Image.hpp"
#include "Graphics/Painter.hpp"

namespace Izo {

Slider::Slider(float value) : m_value(value) {
    m_handle        = ImageManager::the().get_or_crash("slider-handle");
    m_handle_focus  = ImageManager::the().get_or_crash("slider-handle-focus");

    set_focusable(true);
    set_show_focus_indicator(false);
}

void Slider::set_value(float value) {
    float new_value = std::clamp(value, 0.0f, 1.0f);
    if (new_value != m_value) {
        m_value = new_value;

        if (m_on_change) {
            m_on_change(m_value);
        }
    }
}

void Slider::draw_content(Painter& painter) {
    int roundness                   = ThemeDB::the().get<int>("Looks", "Widget.Roundness", 6);
    Color color_slider_track        = ThemeDB::the().get<Color>("Colors", "Slider.Track", Color(90));
    Color color_slider_track_active = ThemeDB::the().get<Color>("Colors", "Slider.Active", Color(90));

    IntRect bounds = global_bounds();
    int track_h = 4;
    int ty = bounds.y + (bounds.h - track_h) / 2;

    int hw = 16;
    Image* imgToDraw = m_handle;
    if (m_pressed && m_handle_focus && m_handle_focus->valid()) {
        imgToDraw = m_handle_focus;
    }
    if (imgToDraw && imgToDraw->valid()) {
        hw = imgToDraw->width();
    }

    int available_travel = bounds.w - hw;
    int h_offset = hw / 2 + (int)(available_travel * m_value);

    // Track height is small, so we clamp radius to half height automatically in logic
    // If track is thin, it becomes a stadium shape.
    painter.fill_rounded_rect({bounds.x, ty, bounds.w, track_h}, roundness, color_slider_track);

    if (m_value > 0.0f) {
        int fillW = h_offset;
        painter.fill_rounded_rect({bounds.x, ty, fillW, track_h}, roundness, color_slider_track_active);
    }

    if (imgToDraw && imgToDraw->valid()) {
        int hh = imgToDraw->height();
        int hx = bounds.x + h_offset - hw / 2;
        int hy = bounds.y + (bounds.h - hh) / 2;
        imgToDraw->draw(painter, {hx, hy});
    } else {
        int hx = bounds.x + h_offset;
        int hy = bounds.y + bounds.h / 2;
        painter.fill_rect({hx - 8, hy - 8, 16, 16}, Color::White);
    }
}

void Slider::measure(int parent_w, int parent_h) {
    m_measured_size = {0, 0, 150, 40};
}

bool Slider::on_touch_event(IntPoint point, bool down) {
    if (!m_pressed && !content_box().contains(point)) return false;

    int hw = 16, hh = 16;

    Image* img = m_handle;
    if ((m_pressed || m_focused) && m_handle_focus && m_handle_focus->valid()) img = m_handle_focus;

    if (img && img->valid()) {
        hw = img->width();
        hh = img->height();
    }

    int available_travel = m_bounds.w - hw;
    int h_offset = hw / 2 + (int)(available_travel * m_value);

    int hx = h_offset - hw / 2;
    int hy = (m_bounds.h - hh) / 2;

    bool over_handle = (point.x >= hx && point.x < hx + hw && point.y >= hy && point.y < hy + hh);
    bool inside_track = (point.x >= 0 && point.x < m_bounds.w && point.y >= 0 && point.y < m_bounds.h);

    if (down && !m_pressed) {
        m_pressed = (over_handle || inside_track);
    } else if (!down) {
        m_pressed = false;
    }

    if (m_pressed) {
        float relative_x = point.x;
        float available_w = (float)(m_bounds.w - hw);
    
        if (available_w > 0) {
            float v = (relative_x - hw/2.0f) / available_w;
            set_value(v);
        } else {
            set_value(0);
        }
        return true; 
    }

    return down && (over_handle || inside_track);
}


} 
