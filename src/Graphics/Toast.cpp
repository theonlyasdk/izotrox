#include "Toast.hpp"
#include "Painter.hpp"
#include "Core/ThemeDB.hpp"

namespace Izo {

Toast::Toast(const std::string& message, Font* font, int duration_ms)
    : m_message(message), m_font(font), m_duration_ms(duration_ms)
    , m_state(State::FadeIn), m_timer(0.0f), m_alpha(0.0f) {
    if (m_font) {
        m_width = m_font->width(m_message) + 40;
        m_height = m_font->height() + 20;
    }
}

void Toast::update(float delta) {
    switch (m_state) {
        case State::FadeIn:
            m_timer += delta;
            m_alpha = std::min(m_timer / FADE_DURATION, 1.0f);
            if (m_timer >= FADE_DURATION) {
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
            m_alpha = std::max(1.0f - (m_timer / FADE_DURATION), 0.0f);
            if (m_timer >= FADE_DURATION) {
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

    int x = (screen_width - m_width) / 2;
    int y = screen_height - m_height - 100;

    Color bg = ThemeDB::the().color("Toast.Background");
    bg.a = (uint8_t)(bg.a * m_alpha);
    
    painter.fill_rounded_rect({x, y, m_width, m_height}, 10, bg);
    
    Color text_c = ThemeDB::the().color("Label.Text");
    text_c.a = (uint8_t)(text_c.a * m_alpha);
    
    int text_w = m_font->width(m_message);
    int text_h = m_font->height();
    int tx = x + (m_width - text_w) / 2;
    int ty = y + (m_height - text_h) / 2;
    
    m_font->draw_text(painter, {tx, ty}, m_message, text_c);
}

ToastManager& ToastManager::the() {
    static ToastManager instance;
    return instance;
}

void ToastManager::show(const std::string& message, int duration_ms) {
    m_queue.push(std::make_shared<Toast>(message, m_font, duration_ms));
}

void ToastManager::update(float delta) {
    if (!m_current && !m_queue.empty()) {
        m_current = m_queue.front();
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
