#pragma once

#include <percussa/ui/Geometry.h>

#include <string>

namespace percussa
{
  namespace ui
  {
    struct EncoderWidget
    {
      EncoderWidget(const std::string &text, const Rect &rect, bool isVisible = true) :
        label(text),
        bounds(rect),
        visible(isVisible)
      {
      }

      std::string label;
      Rect bounds;
      bool visible;
    };
  }
}