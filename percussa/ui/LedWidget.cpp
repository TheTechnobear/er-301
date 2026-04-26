#include <percussa/ui/LedWidget.h>

#include <percussa/hw/PercussaState.h>
#include <percussa/ui/WidgetDrawing.h>

#include <algorithm>

namespace percussa
{
  namespace ui
  {
    void LedWidget::render(Olivec_Canvas canvas) const
    {
      bool active = gpioId < NUM_GPIO_IDS ? percussa_state_read(gpioId) : false;
      render(canvas, active);
    }

    void LedWidget::render(Olivec_Canvas canvas, bool active) const
    {
      int radius = std::max(4, bounds.h() / 2 - 2);
      int cy = bounds.y() + bounds.h() / 2;
      uint32_t c = color == Colour::Amber ? style::kAmberLed : style::kRed;

      int cx = bounds.x() + bounds.w() / 2;

      if (!label.empty())
      {
        int textW = 0;
        int textH = 0;
        percussa_od_text_metrics(label.c_str(), fontSize, &textW, &textH);
        if (labelSide == LabelRight)
        {
          cx = bounds.x() + 2 + radius;
        }
        else
        {
          cx = bounds.x() + bounds.w() - 2 - radius;
        }

        int tx = labelSide == LabelRight ? cx + radius + 6 : cx - radius - 6 - textW;
        int ty = bounds.y() + (bounds.h() - textH) / 2;
        drawing::drawIndicatorLight(canvas, cx, cy, radius, active, c);
        percussa_od_text(canvas, label.c_str(), tx, ty, fontSize, style::kAmberText);
        return;
      }

      drawing::drawIndicatorLight(canvas, cx, cy, radius, active, c);
    }
  } // namespace ui
} // namespace percussa