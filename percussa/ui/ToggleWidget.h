#pragma once

#include <hal/gpio.h>

#include <percussa/ui/Geometry.h>
#include <percussa/ui/olive_bridge.h>

#include <string>

namespace percussa
{
  namespace ui
  {
    struct ToggleWidget
    {
      ToggleWidget(const std::string &text,
                   const Rect &rect,
                   const std::string &lowLabel,
                   const std::string &midLabel,
                   const std::string &highLabel,
                   uint32_t gpioLow = NUM_GPIO_IDS,
                   uint32_t gpioHigh = NUM_GPIO_IDS) :
        label(text),
        bounds(rect),
        low(lowLabel),
        mid(midLabel),
        high(highLabel),
        lowGpioId(gpioLow),
        highGpioId(gpioHigh)
      {
      }

      std::string label;
      Rect bounds;
      std::string low;
      std::string mid;
      std::string high;
      uint32_t lowGpioId;
      uint32_t highGpioId;

      void render(Olivec_Canvas canvas) const;
      void render(Olivec_Canvas canvas, int position) const;
    };
  }
}