#pragma once
#include "Graphics/Canvas.hpp"

#include <string>
#include <linux/fb.h>

namespace Izo {

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    bool init(const std::string& device = "/dev/graphics/fb0");
    void cleanup();

    void swap_buffers(Canvas& src);

    int width() const { return m_width; }
    int height() const { return m_height; }
    bool valid() const { return m_fd > 0; }

    uint32_t* buffer() { return (uint32_t*)m_fbp; }

private:
    int m_fd;
    uint8_t* m_fbp;
    size_t m_screensize;
    int m_width, m_height;
    int m_bpp;
    long int m_line_length;

    struct fb_var_screeninfo m_vinfo;
    struct fb_fix_screeninfo m_finfo;

    bool m_double_buffered;
    int m_current_buffer_idx;
};

} 
