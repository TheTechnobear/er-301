#pragma once

#include <hal/gpio.h>

#include <percussa/ui/Geometry.h>
#include <percussa/ui/olive_bridge.h>

#include <string>

namespace percussa
{
  namespace ui
  {
    struct ButtonWidget
    {
      ButtonWidget(const std::string &text,
                   const Rect &rect,
                   const std::string &groupName,
                   bool isVisible = true,
                   uint32_t gpio = NUM_GPIO_IDS) :
        label(text),
        bounds(rect),
        group(groupName),
        visible(isVisible),
        gpioId(gpio)
      {
      }

      std::string label;
      Rect bounds;
      std::string group;
      bool visible;
      uint32_t gpioId;

      void render(Olivec_Canvas canvas) const;
      void render(Olivec_Canvas canvas, bool active) const;
    };
  }
}