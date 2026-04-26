#pragma once

namespace percussa
{
  namespace platform
  {
    struct Size
    {
      int width;
      int height;
    };

    namespace target
    {
      inline Size panelSize()
      {
#if defined(TARGET_XMX)
        return { 320, 240 };
#elif defined(TARGET_SSP)
        return { 1600, 480 };
#else
        return { 0, 0 };
#endif
      }

      inline Size framebufferSize()
      {
#if defined(TARGET_XMX)
        return { 320, 320 };
#elif defined(TARGET_SSP)
        return { 1600, 480 };
#else
        return panelSize();
#endif
      }
    } // namespace target
  } // namespace platform
} // namespace percussa