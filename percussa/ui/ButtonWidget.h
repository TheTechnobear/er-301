#pragma once

#include <hal/gpio.h>

#include <percussa/ui/Geometry.h>
#include <percussa/ui/olive_bridge.h>

#include <string>

namespace percussa
{
  namespace ui
  {
    class ButtonWidget
    {
    public:
      ButtonWidget(const std::string &text,
                   const Rect &rect,
                   bool isVisible = true,
                   uint32_t gpio = NUM_GPIO_IDS,
                   int fontSize = 16)
          : label(text), bounds(rect), visible(isVisible), gpioId(gpio), fontSize(fontSize)
      {
      }

      void render(Olivec_Canvas canvas) const;
      void render(Olivec_Canvas canvas, bool active) const;

    private:
      std::string label;
      Rect bounds;
      bool visible;
      uint32_t gpioId;
      int fontSize;
    };
  } // namespace ui
} // namespace percussa