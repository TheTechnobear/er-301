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
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace ssp
{
  static inline uint32_t scale8ToN(uint32_t value, uint32_t bits)
  {
    if (bits == 0)
    {
      return 0;
    }

    if (bits >= 8)
    {
      return (value << (bits - 8)) | (value >> (16 - bits));
    }

    const uint32_t maxValue = (1u << bits) - 1u;
    return (value * maxValue + 127u) / 255u;
  }

  HardwareFramebuffer::HardwareFramebuffer(int width, int height) : width_(width), height_(height)
  {
  }

  HardwareFramebuffer::~HardwareFramebuffer()
  {
    if (ttyfd_ >= 0)
    {
      if (ttyModeChanged_ && originalTtyMode_ >= 0)
      {
        if (ioctl(ttyfd_, KDSETMODE, originalTtyMode_) == -1)
        {
          logError("Failed to restore tty mode after framebuffer use.");
        }
      }

      close(ttyfd_);
      ttyfd_ = -1;
      originalTtyMode_ = -1;
      ttyModeChanged_ = false;
    }

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
    static const char *ttyDevices[] = { "/dev/tty0", "/dev/tty", "/dev/console", nullptr };
    for (const char **device = ttyDevices; *device != nullptr; ++device)
    {
      ttyfd_ = open(*device, O_RDWR | O_CLOEXEC);
      if (ttyfd_ >= 0)
      {
        if (ioctl(ttyfd_, KDGETMODE, &originalTtyMode_) == -1)
        {
          originalTtyMode_ = -1;
          logError("Unable to query tty mode on %s.", *device);
        }

        if (ioctl(ttyfd_, KDSETMODE, KD_GRAPHICS) == -1)
        {
          logError("Unable to set tty graphics mode on %s.", *device);
        }
        else
        {
          ttyModeChanged_ = true;
          logInfo("Using tty graphics mode via %s.", *device);
        }
        break;
      }
    }

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

    redOffset_ = varInfo.red.offset;
    redLength_ = varInfo.red.length;
    greenOffset_ = varInfo.green.offset;
    greenLength_ = varInfo.green.length;
    blueOffset_ = varInfo.blue.offset;
    blueLength_ = varInfo.blue.length;
    alphaOffset_ = varInfo.transp.offset;
    alphaLength_ = varInfo.transp.length;

    directCopy_ =
      redOffset_ == 0 && redLength_ == 8 &&
      greenOffset_ == 8 && greenLength_ == 8 &&
      blueOffset_ == 16 && blueLength_ == 8 &&
      (alphaLength_ == 0 || (alphaOffset_ == 24 && alphaLength_ == 8));

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
    logInfo(
      "Using hardware framebuffer at %dx%d (stride=%d, R:%u/%u G:%u/%u B:%u/%u A:%u/%u, directCopy=%d).",
      width_,
      height_,
      strideBytes_,
      redOffset_,
      redLength_,
      greenOffset_,
      greenLength_,
      blueOffset_,
      blueLength_,
      alphaOffset_,
      alphaLength_,
      directCopy_ ? 1 : 0);
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
      const uint32_t *src = frame + (size_t)y * (size_t)width_;
      uint8_t *dst = mapped_ + (size_t)y * (size_t)strideBytes_;

      if (directCopy_)
      {
        std::memcpy(dst, src, rowBytes);
        continue;
      }

      uint32_t *dst32 = (uint32_t *)dst;
      for (int x = 0; x < width_; x++)
      {
        const uint32_t pixel = src[x];
        const uint32_t r = (pixel >> 0) & 0xFFu;
        const uint32_t g = (pixel >> 8) & 0xFFu;
        const uint32_t b = (pixel >> 16) & 0xFFu;
        const uint32_t a = (pixel >> 24) & 0xFFu;

        uint32_t packed = 0;
        packed |= scale8ToN(r, redLength_) << redOffset_;
        packed |= scale8ToN(g, greenLength_) << greenOffset_;
        packed |= scale8ToN(b, blueLength_) << blueOffset_;
        if (alphaLength_ > 0)
        {
          packed |= scale8ToN(a, alphaLength_) << alphaOffset_;
        }
        dst32[x] = packed;
      }
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
