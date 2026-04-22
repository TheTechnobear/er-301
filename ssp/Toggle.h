#pragma once

#include <stdint.h>
#include <array>
#include <hal/gpio.h>
#include <ssp/olive_bridge.h>

namespace ssp
{

  struct Window;
  struct Toggle
  {
    static constexpr int STATE_UP = 0;
    static constexpr int STATE_MID = 1;
    static constexpr int STATE_DOWN = 2;

    Toggle(
      const char *label, uint32_t idA, uint32_t idB, const char *key, const char *up, const char *mid, const char *down)
        : label(label), idA(idA), idB(idB), key(key), up(up), mid(mid), down(down)
    {
    }

    int state() const
    {
      // its two switches in one..
      // idA = UP, idB = DOWN, neither = mid position
      // you cannot have idA && idB ;)
      if (Gpio_read(idA))
      {
        return STATE_UP;
      }
      if (Gpio_read(idB))
      {
        return STATE_DOWN;
      }
      return STATE_MID;
    }

    void setGeometry(int x_, int y_, int w_, int h_)
    {
      x = x_;
      y = y_;
      w = w_;
      h = h_;
    }

    bool hasGeometry() const
    {
      return w > 0 && h > 0;
    }

    void render(Olivec_Canvas canvas) const;
    static void applyDefaultLayout(std::array<Toggle, 2> &toggles);

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
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
  };

} // namespace ssp