#include <percussa/ui/EncoderWidget.h>

#include <percussa/ui/style.h>
#include <percussa/ui/WidgetDrawing.h>

#include <algorithm>

namespace percussa
{
  namespace ui
  {
    void EncoderWidget::render(Olivec_Canvas canvas) const
    {
      if (!visible)
      {
        return;
      }

      int cx = bounds.x() + bounds.w() / 2;
      int cy = bounds.y() + bounds.h() / 2;
      int r = std::min(bounds.w(), bounds.h()) / 2;
      drawing::drawFilledCircle(canvas, cx + 6, cy + 8, r + 2, style::kEncoderShadow);
      drawing::drawFilledCircle(canvas, cx, cy, r + 2, style::kEncoderShell);
      drawing::drawFilledCircle(canvas, cx, cy, r - 2, style::kEncoderFace);
      drawing::drawRing(canvas, cx, cy, r - 2, r - 4, style::kEncoderRim);
    }
  } // namespace ui
} // namespace percussa