#pragma once

#include <atomic>
#include <mutex>

namespace Izo {

struct InputState {
    int touch_x = 0;
    int touch_y = 0;
    bool touch_down = false;
    
    int last_key = 0;
    
    // For internal tracking
    int mouse_x = 0;
    int mouse_y = 0;
};

class Input {
public:
    static Input& instance();

    void init();
    void update();

    int touch_x() const { return state_.touch_x; }
    int touch_y() const { return state_.touch_y; }
    bool touch_down() const { return state_.touch_down; }
    
    int key();

    void set_touch(int x, int y, bool down);
    void set_key(int key);

private:
    Input() = default;
    ~Input();

    InputState state_;
    std::mutex mutex_;
    
#ifdef __ANDROID__
    bool running_ = false;
    // Thread handle would be here (pthread or std::thread)
    // We'll use std::thread in cpp
#endif
};

} // namespace Izo
