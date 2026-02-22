#pragma once

#include <string>
#include <memory>
#include <queue>
#include <functional>

#include <functional>

namespace Izo {

class Font;
class Painter;

class Toast {
public:
    enum class State {
        FadeIn,
        Show,
        FadeOut,
        Done
    };

    Toast(const std::string& message, int duration_ms);
    
    void update(float delta);
    void draw(class Painter& painter, int screen_width, int screen_height);
    
    bool is_done() const { return m_state == State::Done; }
    
private:
    std::string m_message;
    Font* m_font;
    int m_duration_ms;
    State m_state;
    float m_timer;
    float m_alpha;
    int m_width;
    int m_height;
    
    static constexpr float FADE_DURATION = 200.0f;
};

class ToastManager {
public:
    static ToastManager& the() {
        static ToastManager instance;
        return instance;
    }
    
    void show(const std::string& message, int duration_ms = 2000);
    void update();
    void draw(class Painter& painter, int screen_width, int screen_height);
    
    void set_font(Font* font) { m_font = font; }
    Font* font() const { return m_font; }
    bool has_active_toast() const { return m_current != nullptr || !m_queue.empty(); }

private:
    ToastManager() = default;
    
    Font* m_font = nullptr;
    std::queue<std::unique_ptr<Toast>> m_queue;
    std::unique_ptr<Toast> m_current;
};

}
