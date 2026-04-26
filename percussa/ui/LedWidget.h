#pragma once

#include <hal/gpio.h>

#include <percussa/ui/Geometry.h>
#include <percussa/ui/olive_bridge.h>

#include <string>

namespace percussa
{
  namespace ui
  {
    class LedWidget
    {
    public:
      enum Colour {
        Red,
        Amber
      };

      enum LabelSide {
        LabelLeft,
        LabelRight
      };

      LedWidget(const std::string &text,
                const Rect &rect,
                Colour color,
                uint32_t gpio = NUM_GPIO_IDS,
                int fontSize = 16,
                LabelSide side = LabelRight)
          : label(text), bounds(rect), color(color), gpioId(gpio), fontSize(fontSize), labelSide(side)
      {
      }

      void render(Olivec_Canvas canvas) const;
      void render(Olivec_Canvas canvas, bool active) const;

    private:
      std::string label;
      Rect bounds;
      Colour color;
      uint32_t gpioId;
      int fontSize;
      LabelSide labelSide;
    };
  } // namespace ui
} // namespace percussa