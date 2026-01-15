// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Slider.hpp"
#include "../Core/Theme.hpp"
#include <algorithm>

namespace Izo {

Slider::Slider(Image* handleImage, Image* handleFocusImage, float value) 
    : m_val(value), m_handle(handleImage), m_handle_focus(handleFocusImage) {
    set_focusable(true);
}

void Slider::set_value(float v) {
    float new_p = std::clamp(v, 0.0f, 1.0f);
    if (new_p != m_val) {
        m_val = new_p;
        Widget::invalidate();
    }
}

float Slider::value() const { return m_val; }

void Slider::draw_content(Painter& painter) {
    int trackH = 4;
    int ty = m_bounds.y + (m_bounds.h - trackH) / 2;
    
    painter.fill_rounded_rect(m_bounds.x, ty, m_bounds.w, trackH, 2, Theme::instance().color("Slider.Track"));
    
    if (m_val > 0.0f) {
        int fillW = (int)(m_bounds.w * m_val);
        painter.fill_rounded_rect(m_bounds.x, ty, fillW, trackH, 2, Theme::instance().color("Slider.Active"));
    }
    
    Image* imgToDraw = m_handle;
    if ((m_is_pressed || m_is_focused) && m_handle_focus && m_handle_focus->valid()) {
        imgToDraw = m_handle_focus;
    }

    if (imgToDraw && imgToDraw->valid()) {
        int hw = imgToDraw->width();
        int hh = imgToDraw->height();
        int hx = m_bounds.x + (int)(m_bounds.w * m_val) - hw / 2;
        int hy = m_bounds.y + (m_bounds.h - hh) / 2;
        imgToDraw->draw(painter, hx, hy);
    } else {
        int hx = m_bounds.x + (int)(m_bounds.w * m_val);
        int hy = m_bounds.y + m_bounds.h / 2;
        painter.fill_rounded_rect(hx - 8, hy - 8, 16, 16, 8, Color::White);
    }
}

void Slider::measure(int parent_w, int parent_h) {
    m_measured_size = {0, 0, 150, 40};
}

bool Slider::on_touch_event(int local_x, int local_y, bool down) {
    // local_x/y are relative to m_bounds.
    
    int hw = 16, hh = 16;
    
    Image* img = m_handle;
    if ((m_is_pressed || m_is_focused) && m_handle_focus && m_handle_focus->valid()) img = m_handle_focus;
    
    if (img && img->valid()) {
        hw = img->width();
        hh = img->height();
    }
    
    // Handle pos relative to local 0,0
    int hx = (int)(m_bounds.w * m_val) - hw / 2;
    int hy = (m_bounds.h - hh) / 2;
    
    bool overHandle = (local_x >= hx && local_x < hx + hw && local_y >= hy && local_y < hy + hh);
    bool insideTrack = (local_x >= 0 && local_x < m_bounds.w && local_y >= 0 && local_y < m_bounds.h);
    
    m_is_pressed = down && (overHandle || insideTrack || m_is_pressed);

    if (down) {
        if (overHandle || m_is_focused || insideTrack) { 
            float relativeX = (float)local_x;
            float v = relativeX / (float)m_bounds.w;
            set_value(v);
        }
    }
    
    return down && (overHandle || insideTrack);
}

} // namespace Izo