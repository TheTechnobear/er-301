#include <percussa/ui/EncoderWidget.h>

#include <percussa/ui/WidgetDrawing.h>

#include <algorithm>

namespace percussa
{
  namespace ui
  {
    namespace
    {
      const uint32_t kEncoderShadow = PERCUSSA_RGBA(0, 0, 0, 255);
      const uint32_t kEncoderShell = PERCUSSA_RGBA(28, 30, 36, 255);
      const uint32_t kEncoderFace = PERCUSSA_RGBA(232, 236, 246, 255);
      const uint32_t kEncoderRim = PERCUSSA_RGBA(86, 92, 108, 255);
    }

    void EncoderWidget::render(Olivec_Canvas canvas) const
    {
      if (!visible)
      {
        return;
      }

      int cx = bounds.x + bounds.w / 2;
      int cy = bounds.y + bounds.h / 2;
      int r = std::min(bounds.w, bounds.h) / 2;
      drawing::drawFilledCircle(canvas, cx + 6, cy + 8, r + 2, kEncoderShadow);
      drawing::drawFilledCircle(canvas, cx, cy, r + 2, kEncoderShell);
      drawing::drawFilledCircle(canvas, cx, cy, r - 2, kEncoderFace);
      drawing::drawRing(canvas, cx, cy, r - 2, r - 4, kEncoderRim);
    }
  }
}