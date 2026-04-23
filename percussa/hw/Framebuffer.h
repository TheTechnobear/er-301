#pragma once

#include <cstdint>
#include <cstddef>

namespace percussa
{
  namespace hw
  {
    class Framebuffer
    {
    public:
      Framebuffer(int width, int height);
      ~Framebuffer();

      bool init();
      void present(const uint32_t *frame);

    private:
      int width_ = 0;
      int height_ = 0;

      bool directCopy_ = true;
      uint32_t redOffset_ = 0;
      uint32_t redLength_ = 8;
      uint32_t greenOffset_ = 8;
      uint32_t greenLength_ = 8;
      uint32_t blueOffset_ = 16;
      uint32_t blueLength_ = 8;
      uint32_t alphaOffset_ = 24;
      uint32_t alphaLength_ = 8;

      int fbfd_ = -1;
      int ttyfd_ = -1;
      int originalTtyMode_ = -1;
      bool ttyModeChanged_ = false;
      int strideBytes_ = 0;
      size_t mapSize_ = 0;
      uint8_t *mapped_ = nullptr;
    };
  }
}