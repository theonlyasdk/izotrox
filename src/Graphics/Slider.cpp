

#include "Slider.hpp"
#include "Core/ThemeDB.hpp"
#include <algorithm>

namespace Izo {

Slider::Slider(Image* handleImage, Image* handleFocusImage, float value) 
    : m_val(value), m_handle(handleImage), m_handle_focus(handleFocusImage) {
    set_focusable(true);
    set_show_focus_indicator(false);
}

void Slider::set_value(float v) {
    float new_p = std::clamp(v, 0.0f, 1.0f);
    if (new_p != m_val) {
        m_val = new_p;
        if (m_on_change) {
            m_on_change(m_val);
        }
    }
}

float Slider::value() const { return m_val; }

void Slider::draw_content(Painter& painter) {
    IntRect b = global_bounds();
    int trackH = 4;
    int ty = b.y + (b.h - trackH) / 2;

    int hw = 16;
    Image* imgToDraw = m_handle;
    if (m_pressed && m_handle_focus && m_handle_focus->valid()) {
        imgToDraw = m_handle_focus;
    }
    if (imgToDraw && imgToDraw->valid()) {
        hw = imgToDraw->width();
    }

    int available_travel = b.w - hw;
    int h_offset = hw / 2 + (int)(available_travel * m_val);

    int roundness = ThemeDB::the().get<int>("Looks", "Widget.Roundness", 6);
    Color color_slider_track = ThemeDB::the().get<Color>("Colors", "Slider.Track", Color(90));
    Color color_slider_track_active = ThemeDB::the().get<Color>("Colors", "Slider.Active", Color(90));

    // Track height is small, so we clamp radius to half height automatically in logic, but standard is 4px.
    // If track is thin, it becomes a stadium shape.
    
    painter.fill_rounded_rect({b.x, ty, b.w, trackH}, roundness, color_slider_track);

    if (m_val > 0.0f) {
        int fillW = h_offset;
        painter.fill_rounded_rect({b.x, ty, fillW, trackH}, roundness, color_slider_track_active);
    }

    if (imgToDraw && imgToDraw->valid()) {
        int hh = imgToDraw->height();
        int hx = b.x + h_offset - hw / 2;
        int hy = b.y + (b.h - hh) / 2;
        imgToDraw->draw(painter, {hx, hy});
    } else {
        int hx = b.x + h_offset;
        int hy = b.y + b.h / 2;
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
    int h_offset = hw / 2 + (int)(available_travel * m_val);

    int hx = h_offset - hw / 2;
    int hy = (m_bounds.h - hh) / 2;

    bool overHandle = (point.x >= hx && point.x < hx + hw && point.y >= hy && point.y < hy + hh);
    bool insideTrack = (point.x >= 0 && point.x < m_bounds.w && point.y >= 0 && point.y < m_bounds.h);

    if (down && !m_pressed) {

        m_pressed = (overHandle || insideTrack);
    } else if (!down) {
        m_pressed = false;
    }

    if (m_pressed) {
        float relativeX = (float)point.x;

        float available = (float)(m_bounds.w - hw);
        if (available > 0) {
            float v = (relativeX - hw/2.0f) / available;
            set_value(v);
        } else {
            set_value(0);
        }
        return true; 
    }

    return down && (overHandle || insideTrack);
}

} 
