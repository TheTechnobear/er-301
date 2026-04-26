#include <percussa/ui/SubDisplayWidget.h>

#include <percussa/ui/WidgetDrawing.h>

#include <hal/display.h>

namespace percussa
{
  namespace ui
  {

    SubDisplayWidget::SubDisplayWidget(const Rect &rect,int fontSize) :
      DisplayWidget(rect), fontSize((fontSize))
    {
    }

    void SubDisplayWidget::drawSubLabels(Olivec_Canvas canvas) const
    {
      const char *labels[3] = { "S1", "S2", "S3" };
      int columnW = bounds.w() / 3;
      for (int i = 0; i < 3; ++i)
      {
        int textW = 0;
        int textH = 0;
        percussa_od_text_metrics(labels[i], fontSize, &textW, &textH);
        int cx = bounds.x() + i * columnW + columnW / 2;
        percussa_od_text(canvas, labels[i], cx - textW / 2, bounds.y() + bounds.h() + 6, fontSize, style::kAmberText);
      }
    }


    void SubDisplayWidget::render(Olivec_Canvas canvas, const uint8_t *frame) const
    {
      renderGeneric(canvas);
      drawSubLabels(canvas);
      renderFrame(canvas, frame);
    }

    int SubDisplayWidget::sourceWidth() const
    {
      return SUB_HORIZONTAL_PIXELS;
    }

    int SubDisplayWidget::sourceHeight() const
    {
      return SUB_VERTICAL_PIXELS;
    }

    int SubDisplayWidget::pixelBrightness(const uint16_t *src, int srcX, int srcY) const
    {
      int yy = SUB_VERTICAL_PIXELS - srcY - 1;
      int xx = SUB_HORIZONTAL_PIXELS - srcX - 1;
      int shift = yy & 0b111;
      uint16_t cell = *(src + ((yy >> 3) << 7) + xx);
      return ((cell >> shift) & 0b1) ? 0xF : 0;
    }
  }
}