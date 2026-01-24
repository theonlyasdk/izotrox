// Mozilla Public License version 2.0. (c) theonlyasdk 2026

#include "Input.hpp"
#include <Debug/Logger.hpp>
#include <fcntl.h>
#include <unistd.h>

// #define __ANDROID__

#ifdef __ANDROID__
#include <cstring>
#include <format>
#include <sys/poll.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#endif

/**
 * In the Input system, touch and mouse is considered similarly.
 */
namespace Izo {

Input& Input::the() {
    static Input g_instance;
    return g_instance;
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

void Input::set_touch(IntPoint point, bool down) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state.touch_point = point;
    m_state.touch_down = down;
}

void Input::set_key(KeyCode key) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state.last_key = key;
}

void Input::set_shift(bool down) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state.shift_down = down;
}

void Input::set_ctrl(bool down) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state.ctrl_down = down;
}

void Input::set_scroll(int y) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state.scroll_y += y;
}

KeyCode Input::key() {
    std::lock_guard<std::mutex> lock(m_mutex);
    KeyCode key = m_state.last_key;
    m_state.last_key = KeyCode::None;
    return key;
}

IntPoint Input::touch_point() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state.touch_point;
}

bool Input::touch_down() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state.touch_down;
}

bool Input::shift() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state.shift_down;
}

bool Input::ctrl() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state.ctrl_down;
}

int Input::scroll_y() {
    std::lock_guard<std::mutex> lock(m_mutex);
    int y = m_state.scroll_y;
    m_state.scroll_y = 0;
    return y;
}

#ifdef __ANDROID__
static KeyCode linux_code_to_ascii(int code, bool shift) {
    if (code >= KEY_A && code <= KEY_Z) {
        int val = 'a' + (code - KEY_A);
        if (shift) val -= 32;
        return (KeyCode)val;
    }
    if (code >= KEY_1 && code <= KEY_9) {
        if (!shift) return (KeyCode)('1' + (code - KEY_1));
        const char* syms = "!@#$%^&*(";
        return (KeyCode)syms[code - KEY_1];
    }
    if (code == KEY_0) return shift ? (KeyCode)')' : (KeyCode)'0';
    if (code == KEY_SPACE) return KeyCode::Space;
    if (code == KEY_ENTER) return KeyCode::Enter;
    if (code == KEY_BACKSPACE) return KeyCode::Backspace;
    return KeyCode::None; 
}
#endif

void Input::run_thread() {
#ifdef __ANDROID__
    Logger::the().info("Starting Android Input Thread");
    
    struct pollfd fds[16];
    int count = 0;

    for (int i = 0; i < 10; i++) {
        std::string path = std::format("/dev/input/event{}", std::to_string(i));

        int fd = open(path.c_str(), O_RDONLY);
        if (fd >= 0) {
            unsigned char evtype_b[EV_MAX/8 + 1];
            memset(evtype_b, 0, sizeof(evtype_b));
            ioctl(fd, EVIOCGBIT(0, sizeof(evtype_b)), evtype_b);

            bool keep = false;
            
            // Check for Touch (ABS_MT_POSITION_X)
            if ((evtype_b[EV_ABS/8] & (1<<(EV_ABS%8)))) {
                unsigned char abs_b[ABS_MAX/8 + 1];
                ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_b)), abs_b);
                if (abs_b[ABS_MT_POSITION_X/8] & (1<<(ABS_MT_POSITION_X%8))) {
                    keep = true;
                    Logger::the().info(std::format("Input '{}' is a Touchscreen", path));
                }
            }
            
            // Check for Keyboard (KEY_ENTER)
            if (!keep && (evtype_b[EV_KEY/8] & (1<<(EV_KEY%8)))) {
                 unsigned char key_b[KEY_MAX/8 + 1];
                 ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_b)), key_b);
                 if (key_b[KEY_ENTER/8] & (1<<(KEY_ENTER%8))) {
                     keep = true;
                     Logger::the().info(std::format("Input '{}' is a Keyboard", path));
                 }
            }

            if (keep) {
                fds[count].fd = fd;
                fds[count].events = POLLIN;
                count++;
            } else {
                close(fd);
            }
        }
    }

    if (count == 0) {
        Logger::the().error("No input devices found!");
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
                            KeyCode key = linux_code_to_ascii(ev.code, shift);
                            if (key != KeyCode::None) {
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
            set_touch(IntPoint(lx, ly), ldown);
        }
    }
    
    for (int i = 0; i < count; i++) {
        close(fds[i].fd);
    }
    Logger::the().info("Input thread stopped");
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
