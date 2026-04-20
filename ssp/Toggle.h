#pragma once

#include <SDL2/SDL.h>
#include <stdint.h>
#include <hal/gpio.h>

namespace ssp
{

  class Window;
  struct Toggle
  {
    Toggle(
      const char *label, uint32_t idA, uint32_t idB, const char *key, const char *up, const char *mid, const char *down)
        : label(label), idA(idA), idB(idB), key(key), up(up), mid(mid), down(down)
    {
    }

    int state()
    {
      // its two switches in one..
      // idA = UP, idB = DOWN, neither = mid position
      // you cannot have idA && idB ;)
      return Gpio_read(idA) ? 3 : (Gpio_read(idA) ? 1 : 2);
    }

    void switchDown()
    {
      if (Gpio_read(idA))
        Gpio_write(idA, false);
      else if (Gpio_read(idB))
        return;
      else
        Gpio_write(idB, true);
    }

    void switchUp()
    {
      if (Gpio_read(idA))
        return;
      else if (Gpio_read(idB))
        Gpio_write(idB, false);
      else
        Gpio_write(idA, true);
    }


    const char *label;
    uint32_t idA;
    uint32_t idB;
    const char *key;
    const char *up;
    const char *mid;
    const char *down;
  };

} // namespace ssp