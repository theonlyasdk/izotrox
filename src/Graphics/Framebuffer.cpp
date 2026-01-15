#include "Framebuffer.hpp"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
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
    Logger::instance().info("Initializing Framebuffer on " + device);
    
    m_fd = open(device.c_str(), O_RDWR);
    if (m_fd == -1) {
        // Fallback to standard linux fb0 if not android path
        if (device == "/dev/graphics/fb0") {
             m_fd = open("/dev/fb0", O_RDWR);
        }
        if (m_fd == -1) {
            Logger::instance().error("Error opening framebuffer device");
            return false;
        }
    }

    if (ioctl(m_fd, FBIOGET_FSCREENINFO, &m_finfo) == -1) {
        Logger::instance().error("Error reading fixed information");
        return false;
    }

    if (ioctl(m_fd, FBIOGET_VSCREENINFO, &m_vinfo) == -1) {
        Logger::instance().error("Error reading variable information");
        return false;
    }

    m_width = m_vinfo.xres;
    m_height = m_vinfo.yres;
    m_bpp = m_vinfo.bits_per_pixel;
    m_line_length = m_finfo.line_length;

    Logger::instance().info("FB Resolution: " + std::to_string(m_width) + "x" + std::to_string(m_height) + " " + std::to_string(m_bpp) + "bpp");

    // Try to enable double buffering
    m_vinfo.yres_virtual = m_vinfo.yres * 2;
    if (ioctl(m_fd, FBIOPUT_VSCREENINFO, &m_vinfo) == 0) {
        ioctl(m_fd, FBIOGET_VSCREENINFO, &m_vinfo);
        if (m_vinfo.yres_virtual >= m_vinfo.yres * 2) {
             m_double_buffered = true;
             Logger::instance().info("Double buffering enabled");
        }
    }

    m_screensize = m_vinfo.yres_virtual * m_finfo.line_length;

    m_fbp = (uint8_t*)mmap(0, m_screensize, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
    if ((intptr_t)m_fbp == -1) {
        Logger::instance().error("Error mapping framebuffer to memory");
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
    
    // Copy src canvas to FB
    // Handle BPP conversions if necessary. Assuming 32bpp for now (standard Android/Linux modern FB)
    if (m_bpp == 32) {
        uint8_t* dst_base = m_fbp + (y_offset * m_line_length);
        const uint32_t* src_pixels = src.pixels();
        
        for (int y = 0; y < m_height; ++y) {
            uint32_t* dst_row = (uint32_t*)(dst_base + y * m_line_length);
            const uint32_t* src_row = src_pixels + y * m_width; // Canvas width matches fb width hopefully
            
            // memcpy is fastest if formats match
            // Android often uses BGRA or RGBA. Our Color is RGBA (in memory).
            // If colors are wrong (blue/red swapped), we need a swizzle loop.
            // For now, let's assume they match or we accept the swap for this port.
            // Actually, we can check vinfo red offset.
            
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

} // namespace Izo