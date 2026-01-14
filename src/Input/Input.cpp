#include "Input.hpp"
#include <Debug/Logger.hpp>
#include <thread>
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

Input::~Input() {
#ifdef __ANDROID__
    running_ = false;
    // Detach or join thread if we stored it (we are leaking it for now as per simple port)
#endif
}

void Input::set_touch(int x, int y, bool down) {
    std::lock_guard<std::mutex> lock(mutex_);
    state_.touch_x = x;
    state_.touch_y = y;
    state_.touch_down = down;
}

void Input::set_key(int key) {
    std::lock_guard<std::mutex> lock(mutex_);
    state_.last_key = key;
}

int Input::key() {
    std::lock_guard<std::mutex> lock(mutex_);
    int key = state_.last_key;
    state_.last_key = 0;
    return key;
}

#ifdef __ANDROID__
static int linux_code_to_ascii(int code, bool shift) {
    // Basic mapping similar to ankrypton
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

void android_input_thread() {
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
    
    // Local state tracking to minimize lock contention
    int lx = 0, ly = 0; 
    bool ldown = false;

    while (true) {
        int ret = poll(fds, count, -1);
        if (ret <= 0) continue;

        bool touch_updated = false;

        for (int i = 0; i < count; i++) {
            if (fds[i].revents & POLLIN) {
                if (read(fds[i].fd, &ev, sizeof(struct input_event)) > 0) {
                    if (ev.type == EV_KEY) {
                        if (ev.code == KEY_LEFTSHIFT || ev.code == KEY_RIGHTSHIFT) {
                            shift = (ev.value != 0);
                        } else if (ev.code == BTN_TOUCH || ev.code == BTN_LEFT) {
                            ldown = (ev.value != 0);
                            touch_updated = true;
                        } else if (ev.value == 1 || ev.value == 2) { // Press or Repeat
                            int key = linux_code_to_ascii(ev.code, shift);
                            if (key > 0) {
                                Input::instance().set_key(key);
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
            Input::instance().set_touch(lx, ly, ldown);
        }
    }
}
#endif

void Input::init() {
#ifdef __ANDROID__
    running_ = true;
    std::thread t(android_input_thread);
    t.detach(); 
#endif
}

void Input::update() {
    // Nothing to poll on Android (thread handles it)
    // Nothing on Linux (SDL pumps events)
}

} // namespace Izo
