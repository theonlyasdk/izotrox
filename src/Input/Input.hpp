// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include "KeyCode.hpp"

namespace Izo {

struct InputState {
    int touch_x = 0;
    int touch_y = 0;
    bool touch_down = false;
    bool shift_down = false;
    
    KeyCode last_key = KeyCode::None;
};

class Input {
public:
    static Input& the();

    void init();
    void update();

    int touch_x();
    int touch_y();
    bool touch_down();
    bool shift();
    
    KeyCode key();

    void set_touch(int x, int y, bool down);
    void set_key(KeyCode key);
    void set_shift(bool down);

private:
    Input(); 
    ~Input();

    void run_thread();

    InputState m_state;
    std::mutex m_mutex;
    
    std::atomic<bool> m_running{false};
    std::thread m_worker_thread;
};

} // namespace Izo
