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
    : fd_(-1), fbp_(nullptr), width_(0), height_(0), double_buffered_(false), current_buffer_idx_(0) {
}

Framebuffer::~Framebuffer() {
    cleanup();
}

bool Framebuffer::init(const std::string& device) {
    Logger::instance().info("Initializing Framebuffer on " + device);
    
    fd_ = open(device.c_str(), O_RDWR);
    if (fd_ == -1) {
        // Fallback to standard linux fb0 if not android path
        if (device == "/dev/graphics/fb0") {
             fd_ = open("/dev/fb0", O_RDWR);
        }
        if (fd_ == -1) {
            Logger::instance().error("Error opening framebuffer device");
            return false;
        }
    }

    if (ioctl(fd_, FBIOGET_FSCREENINFO, &finfo_) == -1) {
        Logger::instance().error("Error reading fixed information");
        return false;
    }

    if (ioctl(fd_, FBIOGET_VSCREENINFO, &vinfo_) == -1) {
        Logger::instance().error("Error reading variable information");
        return false;
    }

    width_ = vinfo_.xres;
    height_ = vinfo_.yres;
    bpp_ = vinfo_.bits_per_pixel;
    line_length_ = finfo_.line_length;

    Logger::instance().info("FB Resolution: " + std::to_string(width_) + "x" + std::to_string(height_) + " " + std::to_string(bpp_) + "bpp");

    // Try to enable double buffering
    vinfo_.yres_virtual = vinfo_.yres * 2;
    if (ioctl(fd_, FBIOPUT_VSCREENINFO, &vinfo_) == 0) {
        ioctl(fd_, FBIOGET_VSCREENINFO, &vinfo_);
        if (vinfo_.yres_virtual >= vinfo_.yres * 2) {
             double_buffered_ = true;
             Logger::instance().info("Double buffering enabled");
        }
    }

    screensize_ = vinfo_.yres_virtual * finfo_.line_length;

    fbp_ = (uint8_t*)mmap(0, screensize_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    if ((intptr_t)fbp_ == -1) {
        Logger::instance().error("Error mapping framebuffer to memory");
        return false;
    }

    return true;
}

void Framebuffer::cleanup() {
    if (fbp_ && (intptr_t)fbp_ != -1) {
        munmap(fbp_, screensize_);
        fbp_ = nullptr;
    }
    if (fd_ > 0) {
        close(fd_);
        fd_ = -1;
    }
}

void Framebuffer::swap_buffers(Canvas& src) {
    if (!fbp_) return;

    int buf_idx = double_buffered_ ? (1 - current_buffer_idx_) : 0;
    int y_offset = buf_idx * height_;
    
    // Copy src canvas to FB
    // Handle BPP conversions if necessary. Assuming 32bpp for now (standard Android/Linux modern FB)
    if (bpp_ == 32) {
        uint8_t* dst_base = fbp_ + (y_offset * line_length_);
        const uint32_t* src_pixels = src.pixels();
        
        for (int y = 0; y < height_; ++y) {
            uint32_t* dst_row = (uint32_t*)(dst_base + y * line_length_);
            const uint32_t* src_row = src_pixels + y * width_; // Canvas width matches fb width hopefully
            
            // memcpy is fastest if formats match
            // Android often uses BGRA or RGBA. Our Color is RGBA (in memory).
            // If colors are wrong (blue/red swapped), we need a swizzle loop.
            // For now, let's assume they match or we accept the swap for this port.
            // Actually, we can check vinfo red offset.
            
            std::memcpy(dst_row, src_row, width_ * 4);
        }
    }

    if (double_buffered_) {
        vinfo_.yoffset = y_offset;
        ioctl(fd_, FBIOPAN_DISPLAY, &vinfo_);
        
        int arg = 0;
        ioctl(fd_, FBIO_WAITFORVSYNC, &arg);
        
        current_buffer_idx_ = buf_idx;
    }
}

} // namespace Izo
