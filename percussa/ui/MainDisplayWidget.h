#pragma once

#include <percussa/ui/DisplayWidget.h>

namespace percussa
{
  namespace ui
  {
    class MainDisplayWidget : public DisplayWidget
    {
    public:
      MainDisplayWidget(const Rect &rect, int fontSize=16);

      void render(Olivec_Canvas canvas, const uint8_t *frame) const;

    protected:
      void drawMainLabels(Olivec_Canvas canvas) const;
      int sourceWidth() const;
      int sourceHeight() const;
      int pixelBrightness(const uint16_t *src, int srcX, int srcY) const;
      int fontSize;
    };
  }
}