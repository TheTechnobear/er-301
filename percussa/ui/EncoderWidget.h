#pragma once

#include <percussa/ui/Geometry.h>
#include <percussa/ui/olive_bridge.h>

#include <string>

namespace percussa
{
  namespace ui
  {
    class EncoderWidget
    {
    public:
      EncoderWidget(const Rect &rect, bool isVisible = true) : bounds(rect), visible(isVisible)
      {
      }

      void render(Olivec_Canvas canvas) const;

    private:
      std::string label;
      Rect bounds;
      bool visible;
    };
  } // namespace ui
} // namespace percussa