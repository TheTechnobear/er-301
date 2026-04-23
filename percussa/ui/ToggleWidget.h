#pragma once

#include <percussa/ui/Geometry.h>

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
                   const std::string &highLabel) :
        label(text),
        bounds(rect),
        low(lowLabel),
        mid(midLabel),
        high(highLabel)
      {
      }

      std::string label;
      Rect bounds;
      std::string low;
      std::string mid;
      std::string high;
    };
  }
}