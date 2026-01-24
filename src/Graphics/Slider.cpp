// Mozilla Public License version 2.0. (c) theonlyasdk 2026

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
    int trackH = 4;
    int ty = m_bounds.y + (m_bounds.h - trackH) / 2;
    
    painter.fill_rounded_rect({m_bounds.x, ty, m_bounds.w, trackH}, 2, ThemeDB::the().color("Slider.Track"));
    
    if (m_val > 0.0f) {
        int fillW = (int)(m_bounds.w * m_val);
        painter.fill_rounded_rect({m_bounds.x, ty, fillW, trackH}, 2, ThemeDB::the().color("Slider.Active"));
    }
    
    Image* imgToDraw = m_handle;
    if ((m_pressed || m_focused) && m_handle_focus && m_handle_focus->valid()) {
        imgToDraw = m_handle_focus;
    }

    if (imgToDraw && imgToDraw->valid()) {
        int hw = imgToDraw->width();
        int hh = imgToDraw->height();
        int hx = m_bounds.x + (int)(m_bounds.w * m_val) - hw / 2;
        int hy = m_bounds.y + (m_bounds.h - hh) / 2;
        imgToDraw->draw(painter, {hx, hy});
    } else {
        int hx = m_bounds.x + (int)(m_bounds.w * m_val);
        int hy = m_bounds.y + m_bounds.h / 2;
        painter.fill_rounded_rect({hx - 8, hy - 8, 16, 16}, 8, Color::White);
    }
}

void Slider::measure(int parent_w, int parent_h) {
    m_measured_size = {0, 0, 150, 40};
}

bool Slider::on_touch_event(IntPoint point, bool down) {
    // point is relative to m_bounds.
    
    int hw = 16, hh = 16;
    
    Image* img = m_handle;
    if ((m_pressed || m_focused) && m_handle_focus && m_handle_focus->valid()) img = m_handle_focus;
    
    if (img && img->valid()) {
        hw = img->width();
        hh = img->height();
    }
    
    // Handle pos relative to local 0,0
    int hx = (int)(m_bounds.w * m_val) - hw / 2;
    int hy = (m_bounds.h - hh) / 2;
    
    bool overHandle = (point.x >= hx && point.x < hx + hw && point.y >= hy && point.y < hy + hh);
    bool insideTrack = (point.x >= 0 && point.x < m_bounds.w && point.y >= 0 && point.y < m_bounds.h);
    
    if (down && !m_pressed) {
        // Initial press must be inside
        m_pressed = (overHandle || insideTrack);
    } else if (!down) {
        m_pressed = false;
    }
    
    // If pressed, we can drag anywhere
    if (m_pressed) {
        float relativeX = (float)point.x;
        float v = relativeX / (float)m_bounds.w;
        set_value(v);
        return true; 
    }
    
    return down && (overHandle || insideTrack);
}

} // namespace Izo
