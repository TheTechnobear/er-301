#pragma once

#include <percussa/ui/Geometry.h>

#include <string>

namespace percussa
{
  namespace ui
  {
    struct LedWidget
    {
      LedWidget(const std::string &text, const Rect &rect, const std::string &colorName) :
        label(text),
        bounds(rect),
        color(colorName)
      {
      }

      std::string label;
      Rect bounds;
      std::string color;
    };
  }
}