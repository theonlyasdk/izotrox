#pragma once
#include "Canvas.hpp"
#include <string>
#include <linux/fb.h>

namespace Izo {

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    bool init(const std::string& device = "/dev/graphics/fb0");
    void cleanup();

    // Swaps buffers if double buffering is enabled
    void swap_buffers(Canvas& src);

    int width() const { return width_; }
    int height() const { return height_; }
    bool valid() const { return fd_ > 0; }
    
    uint32_t* buffer() { return (uint32_t*)fbp_; }

private:
    int fd_;
    uint8_t* fbp_;
    size_t screensize_;
    int width_, height_;
    int bpp_;
    long int line_length_;
    
    struct fb_var_screeninfo vinfo_;
    struct fb_fix_screeninfo finfo_;

    bool double_buffered_;
    int current_buffer_idx_;
};

} // namespace Izo
