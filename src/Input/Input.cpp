// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Input.hpp"
#include <Debug/Logger.hpp>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#ifdef __ANDROID__
#include <sys/poll.h>
#include <linux/input.h>
#endif

namespace Izo {

Input& Input::instance() {
    static Input s_instance;
    return s_instance;
}

Input::Input() {
}

Input::~Input() {
    if (m_running) {
        m_running = false;
        if (m_worker_thread.joinable()) {
            m_worker_thread.join();
        }
    }
}

void Input::set_touch(int x, int y, bool down) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state.touch_x = x;
    m_state.touch_y = y;
    m_state.touch_down = down;
}

void Input::set_key(int key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state.last_key = key;
}

void Input::set_shift(bool down) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state.shift_down = down;
}

int Input::key() {
    std::lock_guard<std::mutex> lock(m_mutex);
    int key = m_state.last_key;
    m_state.last_key = 0;
    return key;
}

int Input::touch_x() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state.touch_x;
}

int Input::touch_y() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state.touch_y;
}

bool Input::touch_down() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state.touch_down;
}

bool Input::shift() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state.shift_down;
}

#ifdef __ANDROID__
static int linux_code_to_ascii(int code, bool shift) {
    if (code >= KEY_A && code <= KEY_Z) {
        int val = 'a' + (code - KEY_A);
        if (shift) val -= 32;
        return val;
    }
    if (code >= KEY_1 && code <= KEY_9) {
        if (!shift) return '1' + (code - KEY_1);
        const char* syms = "!@#$%^&*(";
        return syms[code - KEY_1];
    }
    if (code == KEY_0) return shift ? ')' : '0';
    if (code == KEY_SPACE) return ' ';
    if (code == KEY_ENTER) return 13;
    if (code == KEY_BACKSPACE) return 8;
    return 0; 
}
#endif

void Input::run_thread() {
#ifdef __ANDROID__
    Logger::instance().info("Starting Android Input Thread");
    
    struct pollfd fds[16];
    int count = 0;

    for (int i = 0; i < 10; i++) {
        std::string path = "/dev/input/event" + std::to_string(i);
        int fd = open(path.c_str(), O_RDONLY);
        if (fd >= 0) {
            Logger::instance().info("Opened input: " + path);
            fds[count].fd = fd;
            fds[count].events = POLLIN;
            count++;
        }
    }

    if (count == 0) {
        Logger::instance().error("No input devices found!");
        return;
    }

    struct input_event ev;
    bool shift = false;
    int lx = 0, ly = 0; 
    bool ldown = false;

    while (m_running) {
        int ret = poll(fds, count, 100); 
        if (ret == 0) continue; 
        if (ret < 0) break; 

        bool touch_updated = false;

        for (int i = 0; i < count; i++) {
            if (fds[i].revents & POLLIN) {
                if (read(fds[i].fd, &ev, sizeof(struct input_event)) > 0) {
                    if (ev.type == EV_KEY) {
                        if (ev.code == KEY_LEFTSHIFT || ev.code == KEY_RIGHTSHIFT) {
                            shift = (ev.value != 0);
                            set_shift(shift);
                        } else if (ev.code == BTN_TOUCH || ev.code == BTN_LEFT) {
                            ldown = (ev.value != 0);
                            touch_updated = true;
                        } else if (ev.value == 1 || ev.value == 2) { 
                            int key = linux_code_to_ascii(ev.code, shift);
                            if (key > 0) {
                                set_key(key);
                            }
                        }
                    } else if (ev.type == EV_ABS) {
                        if (ev.code == ABS_MT_POSITION_X || ev.code == ABS_X) {
                            lx = ev.value;
                            touch_updated = true;
                        } else if (ev.code == ABS_MT_POSITION_Y || ev.code == ABS_Y) {
                            ly = ev.value;
                            touch_updated = true;
                        } else if (ev.code == ABS_MT_TRACKING_ID) {
                            ldown = (ev.value != -1);
                            touch_updated = true;
                        }
                    }
                }
            }
        }
        
        if (touch_updated) {
            set_touch(lx, ly, ldown);
        }
    }
    
    for (int i = 0; i < count; i++) {
        close(fds[i].fd);
    }
    Logger::instance().info("Input thread stopped");
#endif
}

void Input::init() {
#ifdef __ANDROID__
    if (!m_running) {
        m_running = true;
        m_worker_thread = std::thread(&Input::run_thread, this);
    }
#endif
}

void Input::update() {
}

} // namespace Izo
