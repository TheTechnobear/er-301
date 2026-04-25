#pragma once

#include <hal/gpio.h>

#include <percussa/ui/Geometry.h>
#include <percussa/ui/olive_bridge.h>

#include <string>

namespace percussa
{
  namespace ui
  {
    struct LedWidget
    {
      LedWidget(const std::string &text,
                const Rect &rect,
                const std::string &colorName,
                uint32_t gpio = NUM_GPIO_IDS,
                int fontSize=16) :
        label(text),
        bounds(rect),
        color(colorName),
        gpioId(gpio),
        fontSize(fontSize)
      {
      }

      std::string label;
      Rect bounds;
      std::string color;
      uint32_t gpioId;
      int fontSize;

      void render(Olivec_Canvas canvas) const;
      void render(Olivec_Canvas canvas, bool active) const;
      const std::string &colorName() const;
    };
  }
}