#pragma once

#include <stdint.h>

namespace ssp
{
  class Window;
  struct Jack
  {
    Jack(uint32_t channel, int x, int y);
    void draw(Window *window);

    uint32_t channel;
    int x;
    int y;
  };

} // namespace ssp