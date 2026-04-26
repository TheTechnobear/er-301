#include <percussa/ui/ButtonWidget.h>

#include <percussa/hw/PercussaState.h>
#include <percussa/ui/style.h>
#include <percussa/ui/WidgetDrawing.h>

namespace percussa
{
  namespace ui
  {
    void ButtonWidget::render(Olivec_Canvas canvas) const
    {
      bool active = gpioId < NUM_GPIO_IDS ? !percussa_state_read(gpioId) : false;
      render(canvas, active);
    }

    void ButtonWidget::render(Olivec_Canvas canvas, bool active) const
    {
      if (!visible)
      {
        return;
      }

      uint32_t face = active ? style::kButtonFacePressed : style::kButtonFaceIdle;
      olivec_rect(canvas, bounds.x() + 4, bounds.y() + 6, bounds.w(), bounds.h(), style::kButtonShadow);
      olivec_rect(canvas, bounds.x(), bounds.y(), bounds.w(), bounds.h(), style::kButtonShell);
      olivec_rect(canvas, bounds.x() + 4, bounds.y() + 4, bounds.w() - 8, bounds.h() - 8, face);
      olivec_frame(canvas, bounds.x(), bounds.y(), bounds.w(), bounds.h(), 2, style::kButtonEdge);
      olivec_rect(canvas, bounds.x() + 6, bounds.y() + 6, bounds.w() - 12, bounds.h() / 6, style::kButtonHighlight);
      drawing::drawCenteredOdText(canvas, label, bounds, fontSize, style::kAmberText);
    }
  } // namespace ui
} // namespace percussa