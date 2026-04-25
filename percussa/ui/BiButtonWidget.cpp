#include <percussa/ui/BiButtonWidget.h>

#include <percussa/hw/PercussaState.h>
#include <percussa/ui/WidgetDrawing.h>

namespace percussa
{
  namespace ui
  {
    namespace
    {
      const uint32_t kButtonShadow = PERCUSSA_RGBA(0, 0, 0, 140);
      const uint32_t kButtonShell = PERCUSSA_RGBA(12, 13, 16, 255);
      const uint32_t kButtonFaceIdle = PERCUSSA_RGBA(26, 28, 32, 255);
      const uint32_t kButtonFacePressed = PERCUSSA_RGBA(255, 52, 44, 255);
      const uint32_t kButtonEdge = PERCUSSA_RGBA(8, 8, 10, 255);
      const uint32_t kButtonHighlight = PERCUSSA_RGBA(130, 136, 148, 100);
    }

    void BiButtonWidget::render(Olivec_Canvas canvas) const
    {
      render(canvas, false);
    }

    void BiButtonWidget::render(Olivec_Canvas canvas, bool active) const
    {
      if (!visible)
      {
        return;
      }

      uint32_t face = active ? kButtonFacePressed : kButtonFaceIdle;
      olivec_rect(canvas, bounds.x + 4, bounds.y + 6, bounds.w, bounds.h, kButtonShadow);
      olivec_rect(canvas, bounds.x, bounds.y, bounds.w, bounds.h, kButtonShell);
      olivec_rect(canvas, bounds.x + 4, bounds.y + 4, bounds.w - 8, bounds.h - 8, face);
      olivec_frame(canvas, bounds.x, bounds.y, bounds.w, bounds.h, 2, kButtonEdge);
      olivec_rect(canvas, bounds.x + 6, bounds.y + 6, bounds.w - 12, bounds.h / 6, kButtonHighlight);
      drawing::drawCenteredOdText(canvas, labels[(int)state], bounds, fontSize, drawing::kAmberText);
     }
  }
}