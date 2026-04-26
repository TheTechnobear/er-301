#pragma once

#include <hal/gpio.h>

#include <percussa/ui/Geometry.h>
#include <percussa/ui/olive_bridge.h>

#include <string>
#include <vector>

namespace percussa
{
  namespace ui
  {
    class BiButtonWidget
    {
    public:
      BiButtonWidget(const std::string &textA,
                     const std::string &textB,
                     const Rect &rect,
                     bool isVisible = true,
                     int fontSize = 16)
          : bounds(rect), visible(isVisible), fontSize(fontSize), state(false)
      {
        labels.push_back(textA);
        labels.push_back(textB);
      }

      void render(Olivec_Canvas canvas) const;
      void render(Olivec_Canvas canvas, bool active) const;

    private:
      std::vector<std::string> labels;
      Rect bounds;
      std::string group;
      bool visible;
      int fontSize;
      bool state;
    };
  } // namespace ui
} // namespace percussa