#include <percussa/hw/Framebuffer.h>

#include <cstring>
#include <cstdio>

#if defined(__linux__)

#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace percussa
{
  namespace hw
  {
    namespace
    {
      uint32_t scale8ToN(uint32_t value, uint32_t bits)
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
    }

    Framebuffer::Framebuffer(int width, int height) :
      width_(width),
      height_(height)
    {
    }

    Framebuffer::~Framebuffer()
    {
      if (ttyfd_ >= 0)
      {
        if (ttyModeChanged_ && originalTtyMode_ >= 0)
        {
          if (ioctl(ttyfd_, KDSETMODE, originalTtyMode_) == -1)
          {
            std::fprintf(stderr, "Failed to restore tty mode after framebuffer use.\n");
          }
        }

        close(ttyfd_);
      }

      if (mapped_)
      {
        munmap(mapped_, mapSize_);
      }

      if (fbfd_ >= 0)
      {
        close(fbfd_);
      }
    }

    bool Framebuffer::init()
    {
      static const char *ttyDevices[] = { "/dev/tty0", "/dev/tty", "/dev/console", nullptr };
      for (const char **device = ttyDevices; *device != nullptr; ++device)
      {
        ttyfd_ = open(*device, O_RDWR | O_CLOEXEC);
        if (ttyfd_ < 0)
        {
          continue;
        }

        if (ioctl(ttyfd_, KDGETMODE, &originalTtyMode_) != -1 && ioctl(ttyfd_, KDSETMODE, KD_GRAPHICS) != -1)
        {
          ttyModeChanged_ = true;
          break;
        }
      }

      fbfd_ = open("/dev/fb0", O_RDWR);
      if (fbfd_ < 0)
      {
        std::fprintf(stderr, "Failed to open /dev/fb0.\n");
        return false;
      }

      fb_fix_screeninfo fixInfo;
      fb_var_screeninfo varInfo;
      if (ioctl(fbfd_, FBIOGET_FSCREENINFO, &fixInfo) == -1)
      {
        std::fprintf(stderr, "Failed to read framebuffer fixed info.\n");
        return false;
      }

      if (ioctl(fbfd_, FBIOGET_VSCREENINFO, &varInfo) == -1)
      {
        std::fprintf(stderr, "Failed to read framebuffer variable info.\n");
        return false;
      }

      if ((int)varInfo.xres != width_ || (int)varInfo.yres != height_)
      {
        std::fprintf(stderr, "Framebuffer size mismatch. Expected %dx%d, got %ux%u.\n", width_, height_, varInfo.xres, varInfo.yres);
        return false;
      }

      if (varInfo.bits_per_pixel != 32)
      {
        std::fprintf(stderr, "Unsupported framebuffer format: %u bpp. Expected 32 bpp.\n", varInfo.bits_per_pixel);
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
        std::fprintf(stderr, "Failed to map framebuffer memory.\n");
        return false;
      }

      std::memset(mapped_, 0, mapSize_);
      return true;
    }

    void Framebuffer::present(const uint32_t *frame)
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
  }
}

#else

namespace percussa
{
  namespace hw
  {
    Framebuffer::Framebuffer(int width, int height) :
      width_(width),
      height_(height)
    {
    }

    Framebuffer::~Framebuffer()
    {
    }

    bool Framebuffer::init()
    {
      std::fprintf(stderr, "Framebuffer is unavailable on this platform.\n");
      return false;
    }

    void Framebuffer::present(const uint32_t *frame)
    {
      (void)frame;
    }
  }
}

#endif