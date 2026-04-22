#pragma once

#include <ssp/constants.h>
#include <ssp/olive_bridge.h>
#include <stdint.h>
#include <string>
#include <array>
#include <hal/gpio.h>

namespace ssp
{
  struct Window;
  struct Led
  {
    Led(const char *label, uint32_t id, uint32_t color) : label(label), id(id), color(color) {};

    int state() const
    {
      return Gpio_read(id);
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
    static void applyDefaultLayout(std::array<Led, 11> &leds);

    std::string label;
    uint32_t id;
    uint32_t color;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
  };

  struct RedLed : public Led
  {
    RedLed(const char *label, uint32_t id) : Led(label, id, SSP_RGBA(255, 52, 44, 255))
    {
    }
  };

  struct OrangeLed : public Led
  {
    OrangeLed(const char *label,uint32_t id) : Led(label, id, SSP_RGBA(255, 144, 32, 255))
    {
    }
  };

} // namespace ssp