#include <ssp/hw/Display.h>

#include <hal/log.h>

#include <cstring>

#ifndef SSP_USE_SDL
#if defined(__APPLE__)
#define SSP_USE_SDL 1
#else
#define SSP_USE_SDL 0
#endif
#endif

#if !SSP_USE_SDL

#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace ssp
{
  HardwareFramebuffer::HardwareFramebuffer(int width, int height) : width_(width), height_(height)
  {
  }

  HardwareFramebuffer::~HardwareFramebuffer()
  {
    if (mapped_)
    {
      munmap(mapped_, mapSize_);
      mapped_ = nullptr;
    }

    if (fbfd_ >= 0)
    {
      close(fbfd_);
      fbfd_ = -1;
    }
  }

  bool HardwareFramebuffer::init()
  {
    fbfd_ = open("/dev/fb0", O_RDWR);
    if (fbfd_ < 0)
    {
      logError("Failed to open /dev/fb0.");
      return false;
    }

    fb_fix_screeninfo fixInfo;
    fb_var_screeninfo varInfo;
    if (ioctl(fbfd_, FBIOGET_FSCREENINFO, &fixInfo) == -1)
    {
      logError("Failed to read framebuffer fixed info.");
      return false;
    }

    if (ioctl(fbfd_, FBIOGET_VSCREENINFO, &varInfo) == -1)
    {
      logError("Failed to read framebuffer variable info.");
      return false;
    }

    if ((int)varInfo.xres != width_ || (int)varInfo.yres != height_)
    {
      logError(
        "Framebuffer size mismatch. Expected %dx%d, got %ux%u.", width_, height_, varInfo.xres, varInfo.yres);
      return false;
    }

    if (varInfo.bits_per_pixel != 32)
    {
      logError("Unsupported framebuffer format: %u bpp. Expected 32 bpp.", varInfo.bits_per_pixel);
      return false;
    }

    strideBytes_ = fixInfo.line_length;
    mapSize_ = fixInfo.smem_len;
    mapped_ = (uint8_t *)mmap(nullptr, mapSize_, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd_, 0);
    if (mapped_ == MAP_FAILED)
    {
      mapped_ = nullptr;
      logError("Failed to map framebuffer memory.");
      return false;
    }

    std::memset(mapped_, 0, mapSize_);
    logInfo("Using hardware framebuffer at %dx%d (stride=%d).", width_, height_, strideBytes_);
    return true;
  }

  void HardwareFramebuffer::present(const uint32_t *frame)
  {
    if (!mapped_ || !frame)
    {
      return;
    }

    const size_t rowBytes = (size_t)width_ * sizeof(uint32_t);
    for (int y = 0; y < height_; y++)
    {
      const uint8_t *src = (const uint8_t *)(frame + (size_t)y * (size_t)width_);
      uint8_t *dst = mapped_ + (size_t)y * (size_t)strideBytes_;
      std::memcpy(dst, src, rowBytes);
    }
  }
} // namespace ssp

#else

namespace ssp
{
  HardwareFramebuffer::HardwareFramebuffer(int width, int height) : width_(width), height_(height)
  {
  }

  HardwareFramebuffer::~HardwareFramebuffer()
  {
  }

  bool HardwareFramebuffer::init()
  {
    logError("HardwareFramebuffer is disabled because SSP_USE_SDL=1.");
    return false;
  }

  void HardwareFramebuffer::present(const uint32_t *frame)
  {
    (void)frame;
  }
} // namespace ssp

#endif
