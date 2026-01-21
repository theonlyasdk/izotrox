// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#pragma once

#include <atomic>
#include <mutex>
#include <thread>

namespace Izo {

struct InputState {
    int touch_x = 0;
    int touch_y = 0;
    bool touch_down = false;
    bool shift_down = false;
    
    int last_key = 0;
};

class Input {
public:
    enum Key {
        Backspace = 8,
        Enter = 13,
        Left = 200,
        Right = 201,
        Up = 202,
        Down = 203
    };

    static Input& the();

    void init();
    void update();

    int touch_x();
    int touch_y();
    bool touch_down();
    bool shift();
    
    int key();

    void set_touch(int x, int y, bool down);
    void set_key(int key);
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
