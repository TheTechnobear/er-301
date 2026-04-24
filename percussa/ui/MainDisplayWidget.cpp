#include <percussa/ui/MainDisplayWidget.h>

#include <percussa/ui/WidgetDrawing.h>

#include <hal/display.h>

namespace percussa
{
  namespace ui
  {
    namespace
    {
      void drawMainLabels(Olivec_Canvas canvas, const Rect &bounds)
      {
        const char *topLabels[3] = { "M1", "M2", "M3" };
        const char *bottomLabels[3] = { "M4", "M5", "M6" };
        int columnW = bounds.w / 6;
        for (int i = 0; i < 3; ++i)
        {
          int textW = 0;
          int textH = 0;
          percussa_od_text_metrics(topLabels[i], 16, &textW, &textH);
          int cx = bounds.x + i * columnW + columnW / 2;
          percussa_od_text(canvas, topLabels[i], cx - textW / 2, bounds.y - 22, 16, drawing::kAmberText);

          percussa_od_text_metrics(bottomLabels[i], 16, &textW, &textH);
          cx = bounds.x + (i + 3) * columnW + columnW / 2;
          percussa_od_text(canvas, bottomLabels[i], cx - textW / 2, bounds.y + bounds.h + 6, 16, drawing::kAmberText);
        }
      }
    }

    MainDisplayWidget::MainDisplayWidget(const Rect &rect) :
      DisplayWidget("main", rect, "main")
    {
    }

    void MainDisplayWidget::render(Olivec_Canvas canvas, const uint8_t *frame) const
    {
      renderGeneric(canvas);
      drawMainLabels(canvas, bounds);
      renderFrame(canvas, frame);
    }

    int MainDisplayWidget::sourceWidth() const
    {
      return MAIN_HORIZONTAL_PIXELS;
    }

    int MainDisplayWidget::sourceHeight() const
    {
      return MAIN_VERTICAL_PIXELS;
    }

    int MainDisplayWidget::pixelBrightness(const uint16_t *src, int srcX, int srcY) const
    {
      int yy = MAIN_VERTICAL_PIXELS - srcY - 1;
      int xx = MAIN_HORIZONTAL_PIXELS - srcX - 1;
      int stride = MAIN_HORIZONTAL_PIXELS >> 1;
      uint16_t cell = *(src + yy * stride + (xx >> 1));
      int shift = (((~xx) & 0b1) << 2);
      return (cell >> shift) & 0xF;
    }
  }
}