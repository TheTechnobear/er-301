#pragma once

#include <percussa/ui/DisplayWidget.h>

namespace percussa
{
  namespace ui
  {
    class MainDisplayWidget : public DisplayWidget
    {
    public:
      MainDisplayWidget(const Rect &rect);

      void render(Olivec_Canvas canvas, const uint8_t *frame) const;

    protected:
      int sourceWidth() const;
      int sourceHeight() const;
      int pixelBrightness(const uint16_t *src, int srcX, int srcY) const;
    };
  }
}