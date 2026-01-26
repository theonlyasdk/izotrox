#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include "Geometry/Primitives.hpp"
#include "KeyCode.hpp"

namespace Izo {

struct InputState {
    IntPoint touch_point = {0, 0};
    bool touch_down = false;
    bool shift_down = false;
    bool ctrl_down = false;
    int scroll_y = 0;

    KeyCode last_key = KeyCode::None;
};

class Input {
public:
    static Input& the();

    void init();
    void update();

    IntPoint touch_point();
    bool touch_down();
    bool shift();
    bool ctrl();
    int scroll_y();

    KeyCode key();

    void set_touch(IntPoint point, bool down);
    void set_key(KeyCode key);
    void set_shift(bool down);
    void set_ctrl(bool down);
    void set_scroll(int y);

private:
    Input(); 
    ~Input();

    void run_thread();

    InputState m_state;
    std::mutex m_mutex;

    std::atomic<bool> m_running{false};
    std::thread m_worker_thread;
};

} 
