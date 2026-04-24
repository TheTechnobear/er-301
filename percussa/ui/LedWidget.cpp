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
      int radius = std::max(2, std::min(bounds.h / 2 - 2, bounds.w / 6));
      int cx = bounds.x + 2 + radius;
      int cy = bounds.y + bounds.h / 2;
      uint32_t color = colorName() == "amber" ? drawing::kAmberLed : drawing::kRed;

      drawing::drawIndicatorLight(canvas, cx, cy, radius, active, color);

      if (!label.empty())
      {
        int textW = 0;
        int textH = 0;
        percussa_od_text_metrics(label.c_str(), 16, &textW, &textH);
        int tx = cx + radius + 6;
        int ty = bounds.y + (bounds.h - textH) / 2;
        percussa_od_text(canvas, label.c_str(), tx, ty, 16, drawing::kAmberText);
      }
    }

    const std::string &LedWidget::colorName() const
    {
      return color;
    }
  }
}