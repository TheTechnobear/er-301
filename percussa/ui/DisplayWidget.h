#pragma once

#include <percussa/ui/Geometry.h>

#include <string>

namespace percussa
{
  namespace ui
  {
    struct DisplayWidget
    {
      DisplayWidget(const std::string &text, const Rect &rect, const std::string &roleName) :
        label(text),
        bounds(rect),
        role(roleName)
      {
      }

      std::string label;
      Rect bounds;
      std::string role;
    };
  }
}