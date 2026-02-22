#include "UI/Widgets/Toast.hpp"
#include "Graphics/Font.hpp"
#include "Graphics/Painter.hpp"
#include "Core/ThemeDB.hpp"

namespace Izo {

Toast::Toast(const std::string& message, int duration_ms)
    : m_message(message), m_duration_ms(duration_ms)
    , m_state(State::FadeIn), m_timer(0.0f), m_alpha(0.0f) {
    m_font = ToastManager::the().font();
    // Size will be calculated in draw() based on screen constraints
    m_width = 0;
    m_height = 0;
}

void Toast::update(float delta) {
    switch (m_state) {
        case State::FadeIn:
            m_timer += delta;
            m_alpha = std::min(m_timer / kFadeDuration, 1.0f);
            if (m_timer >= kFadeDuration) {
                m_state = State::Show;
                m_timer = 0.0f;
                m_alpha = 1.0f;
            }
            break;
        case State::Show:
            m_timer += delta;
            if (m_timer >= m_duration_ms) {
                m_state = State::FadeOut;
                m_timer = 0.0f;
            }
            break;
        case State::FadeOut:
            m_timer += delta;
            m_alpha = std::max(1.0f - (m_timer / kFadeDuration), 0.0f);
            if (m_timer >= kFadeDuration) {
                m_state = State::Done;
                m_alpha = 0.0f;
            }
            break;
        case State::Done:
            break;
    }
}

void Toast::draw(Painter& painter, int screen_width, int screen_height) {
    if (m_state == State::Done || !m_font) return;

    int max_w = screen_width - 60; // 30px padding on sides
    int text_w = 0, text_h = 0;
 
    int internal_padding = 40;
    int max_text_width = max_w - internal_padding;
    if (max_text_width < 100) max_text_width = 100;

    m_font->measure_multiline(m_message, text_w, text_h, max_text_width);
    
    m_width = text_w + internal_padding;
    m_height = text_h + 20;

    int x = (screen_width - m_width) / 2;
    // Position from bottom: margin 100px.
    // Grows upwards means the y position depends on height.
    int y = screen_height - m_height - 100;

    Color bg = ThemeDB::the().get<Color>("Colors", "Toast.Background", Color(100));
    bg.a = (uint8_t)(bg.a * m_alpha);
    
    int roundness = ThemeDB::the().get<int>("WidgetParams", "Toast.Roundness", 12);
    int border_thickness = ThemeDB::the().get<int>("WidgetParams", "Toast.BorderThickness", 12);

    painter.fill_rounded_rect({x, y, m_width, m_height}, roundness, bg);
    
    Color border = ThemeDB::the().get<Color>("Colors", "Toast.Border", Color(50));
    border.a = (uint8_t)(border.a * m_alpha);
    painter.draw_rounded_rect({x, y, m_width, m_height}, roundness, border, border_thickness);
    
    Color text_c = ThemeDB::the().get<Color>("Colors", "Toast.Text", Color(0));
    text_c.a = (uint8_t)(text_c.a * m_alpha);
    
    // Draw text centered in the toast
    // Multiline draw needs top-left of text area
    int tx = x + (m_width - text_w) / 2;
    int ty = y + (m_height - text_h) / 2;
    
    m_font->draw_text_multiline(painter, {tx, ty}, m_message, text_c, max_text_width);
}

void ToastManager::show(const std::string& message, int duration_ms) {
    m_queue.push(std::make_unique<Toast>(message, duration_ms));
}

void ToastManager::update() {
    float delta = Application::the().delta();
    if (!m_current && !m_queue.empty()) {
        m_current = std::move(m_queue.front());
        m_queue.pop();
    }
    
    if (m_current) {
        m_current->update(delta);
        if (m_current->is_done()) {
            m_current = nullptr;
        }
    }
}

void ToastManager::draw(Painter& painter, int screen_width, int screen_height) {
    if (m_current) {
        m_current->draw(painter, screen_width, screen_height);
    }
}

}
