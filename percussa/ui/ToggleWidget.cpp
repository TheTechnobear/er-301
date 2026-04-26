#include <percussa/ui/ToggleWidget.h>

#include <percussa/hw/PercussaState.h>
#include <percussa/ui/WidgetDrawing.h>

#include <algorithm>

namespace percussa
{
  namespace ui
  {
    void ToggleWidget::render(Olivec_Canvas canvas) const
    {
      int position = 1;
      if (lowGpioId < NUM_GPIO_IDS && percussa_state_read(lowGpioId))
      {
        position = 0;
      }
      else if (highGpioId < NUM_GPIO_IDS && percussa_state_read(highGpioId))
      {
        position = 2;
      }

      render(canvas, position);
    }

    void ToggleWidget::render(Olivec_Canvas canvas, int position) const
    {
      int titleW = 0;
      int titleH = 0;
      percussa_od_text_metrics(label.c_str(), fontSize, &titleW, &titleH);
      percussa_od_text(
        canvas, label.c_str(), bounds.x() + (bounds.w() - titleW) / 2, bounds.y(), fontSize, style::kAmberText);

      int bodyTop = bounds.y() + titleH + 6;
      int rowHeight = (bounds.h() - titleH - 12) / 3;
      int ledRadius = std::max(4, std::min(8, rowHeight / 3));
      int ledCx = bounds.x() + 4 + ledRadius;
      int lineX1 = ledCx + ledRadius + 6;
      int lineX2 = bounds.x() + bounds.w() - 4;

      const char *rows[3] = { low.c_str(), mid.c_str(), high.c_str() };
      for (int i = 0; i < 3; ++i)
      {
        int cy = bodyTop + i * rowHeight + rowHeight / 2;
        int textW = 0;
        int textH = 0;
        percussa_od_text_metrics(rows[i], fontSize, &textW, &textH);
        int textX = lineX1 + 2;
        int textY = cy - textH / 2;
        int gapLeft = textX - 4;
        int gapRight = textX + textW + 4;
        if (gapLeft > lineX1)
        {
          olivec_line(canvas, lineX1, cy, gapLeft, cy, style::kToggleLine);
        }
        if (gapRight < lineX2)
        {
          olivec_line(canvas, gapRight, cy, lineX2, cy, style::kToggleLine);
        }
        percussa_od_text(canvas, rows[i], textX, textY, fontSize, style::kAmberText);
        drawing::drawIndicatorLight(canvas, ledCx, cy, ledRadius, i == position, style::kRed);
      }
    }
  } // namespace ui
} // namespace percussa