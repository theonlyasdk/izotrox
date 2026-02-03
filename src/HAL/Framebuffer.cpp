#include "Framebuffer.hpp"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <Debug/Logger.hpp>

namespace Izo {

Framebuffer::Framebuffer() 
    : m_fd(-1), m_fbp(nullptr), m_width(0), m_height(0), m_double_buffered(false), m_current_buffer_idx(0) {
}

Framebuffer::~Framebuffer() {
    cleanup();
}

bool Framebuffer::init(const std::string& device) {
    LogInfo("Initializing Framebuffer on {}", device);

    m_fd = open(device.c_str(), O_RDWR);
    if (m_fd == -1) {
        if (device == "/dev/graphics/fb0") {
             m_fd = open("/dev/fb0", O_RDWR);
        }
        if (m_fd == -1) {
            LogError("Error opening framebuffer device");
            return false;
        }
    }

    if (ioctl(m_fd, FBIOGET_FSCREENINFO, &m_finfo) == -1) {
        LogError("Error reading fixed information");
        return false;
    }

    if (ioctl(m_fd, FBIOGET_VSCREENINFO, &m_vinfo) == -1) {
        LogError("Error reading variable information");
        return false;
    }

    LogInfo("Framebuffer ID: {}", std::string(m_finfo.id));
    LogInfo("Memory start: {}", std::to_string(m_finfo.smem_start));
    LogInfo("Memory length: {}", std::to_string(m_finfo.smem_len));
    LogInfo("Line length: {}", std::to_string(m_finfo.line_length));
    LogInfo("Resolution: {}x{}", std::to_string(m_vinfo.xres), std::to_string(m_vinfo.yres));
    LogInfo("Virtual resolution: {}x{}", std::to_string(m_vinfo.xres_virtual), std::to_string(m_vinfo.yres_virtual));
    LogInfo("Bits per pixel: {}", std::to_string(m_vinfo.bits_per_pixel));
    LogInfo("Red: len={} offset={}", std::to_string(m_vinfo.red.length), std::to_string(m_vinfo.red.offset));
    LogInfo("Green: len={} offset={}", std::to_string(m_vinfo.green.length), std::to_string(m_vinfo.green.offset));
    LogInfo("Blue: len={} offset={}", std::to_string(m_vinfo.blue.length), std::to_string(m_vinfo.blue.offset));
    LogInfo("Alpha: len={} offset={}", std::to_string(m_vinfo.transp.length), std::to_string(m_vinfo.transp.offset));

    m_width = m_vinfo.xres;
    m_height = m_vinfo.yres;
    m_bpp = m_vinfo.bits_per_pixel;
    m_line_length = m_finfo.line_length;

    m_vinfo.yres_virtual = m_vinfo.yres * 2;
    if (ioctl(m_fd, FBIOPUT_VSCREENINFO, &m_vinfo) == 0) {
        ioctl(m_fd, FBIOGET_VSCREENINFO, &m_vinfo);
        if (m_vinfo.yres_virtual >= m_vinfo.yres * 2) {
             m_double_buffered = true;
             LogInfo("Double buffering enabled");
        }
    }

    m_screensize = m_vinfo.yres_virtual * m_finfo.line_length;

    m_fbp = (uint8_t*)mmap(0, m_screensize, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
    if ((intptr_t)m_fbp == -1) {
        LogError("Error mapping framebuffer to memory");
        return false;
    }

    return true;
}

void Framebuffer::cleanup() {
    if (m_fbp && (intptr_t)m_fbp != -1) {
        munmap(m_fbp, m_screensize);
        m_fbp = nullptr;
    }
    if (m_fd > 0) {
        close(m_fd);
        m_fd = -1;
    }
}

void Framebuffer::swap_buffers(Canvas& src) {
    if (!m_fbp) return;

    int buf_idx = m_double_buffered ? (1 - m_current_buffer_idx) : 0;
    int y_offset = buf_idx * m_height;

    if (m_bpp == 32) {
        uint8_t* dst_base = m_fbp + (y_offset * m_line_length);
        const uint32_t* src_pixels = src.pixels();

        for (int y = 0; y < m_height; ++y) {
            uint32_t* dst_row = (uint32_t*)(dst_base + y * m_line_length);
            const uint32_t* src_row = src_pixels + y * m_width; 

            std::memcpy(dst_row, src_row, m_width * 4);
        }
    }

    if (m_double_buffered) {
        m_vinfo.yoffset = y_offset;
        ioctl(m_fd, FBIOPAN_DISPLAY, &m_vinfo);

        int arg = 0;
        ioctl(m_fd, FBIO_WAITFORVSYNC, &arg);

        m_current_buffer_idx = buf_idx;
    }
}

} 