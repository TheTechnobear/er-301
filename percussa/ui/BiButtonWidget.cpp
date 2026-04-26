#include <percussa/ui/BiButtonWidget.h>

#include <percussa/hw/PercussaState.h>
#include <percussa/ui/style.h>
#include <percussa/ui/WidgetDrawing.h>

namespace percussa
{
  namespace ui
  {
    void BiButtonWidget::render(Olivec_Canvas canvas) const
    {
      render(canvas, state);
    }

    void BiButtonWidget::render(Olivec_Canvas canvas, bool active) const
    {
      if (!visible)
      {
        return;
      }

      olivec_rect(canvas, bounds.x() + 4, bounds.y() + 6, bounds.w(), bounds.h(), style::kButtonShadow);
      olivec_rect(canvas, bounds.x(), bounds.y(), bounds.w(), bounds.h(), style::kButtonShell);
      olivec_rect(canvas, bounds.x() + 4, bounds.y() + 4, bounds.w() - 8, bounds.h() - 8, style::kButtonFaceIdle);
      olivec_frame(canvas, bounds.x(), bounds.y(), bounds.w(), bounds.h(), 2, style::kButtonEdge);
      olivec_rect(canvas, bounds.x() + 6, bounds.y() + 6, bounds.w() - 12, bounds.h() / 6, style::kButtonHighlight);
      drawing::drawCenteredOdText(canvas, labels[(int)active], bounds, fontSize, style::kAmberText);
    }
  } // namespace ui
} // namespace percussa