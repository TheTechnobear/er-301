#pragma once

#include <cstddef>
#include <cstdint>

namespace ssp
{
    class HardwareFramebuffer
    {
    public:
        HardwareFramebuffer(int width, int height);
        ~HardwareFramebuffer();

        bool init();
        void present(const uint32_t *frame);

    private:
        int width_ = 0;
        int height_ = 0;

        int fbfd_ = -1;
        int strideBytes_ = 0;
        size_t mapSize_ = 0;
        uint8_t *mapped_ = nullptr;
    };
} // namespace ssp