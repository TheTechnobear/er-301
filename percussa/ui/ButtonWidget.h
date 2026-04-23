#pragma once

#include <percussa/ui/Geometry.h>

#include <string>

namespace percussa
{
  namespace ui
  {
    struct ButtonWidget
    {
      ButtonWidget(const std::string &text, const Rect &rect, const std::string &groupName, bool isVisible = true) :
        label(text),
        bounds(rect),
        group(groupName),
        visible(isVisible)
      {
      }

      std::string label;
      Rect bounds;
      std::string group;
      bool visible;
    };
  }
}