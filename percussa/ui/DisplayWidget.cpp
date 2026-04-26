#include <percussa/ui/DisplayWidget.h>

#include <percussa/ui/style.h>
#include <percussa/ui/WidgetDrawing.h>

#include <cctype>

namespace percussa
{
  namespace ui
  {
    namespace
    {
      const int kSspBrightness = 15;


      uint32_t grayColor(int value)
      {
        return PERCUSSA_RGBA(value, (int)(value * 0.85f), 0, 255);
      }

    } // namespace

    void DisplayWidget::render(Olivec_Canvas canvas, const uint8_t *frame) const
    {
      (void)frame;
      renderGeneric(canvas);
    }

    void DisplayWidget::renderGeneric(Olivec_Canvas canvas) const
    {
    }

    void DisplayWidget::renderFrame(Olivec_Canvas canvas, const uint8_t *frame) const
    {
      if (!frame || sourceWidth() <= 0 || sourceHeight() <= 0)
      {
        return;
      }

      const uint16_t *src = (const uint16_t *)frame;
      int scale = bounds.w() / sourceWidth();
      for (int srcY = 0; srcY < sourceHeight(); ++srcY)
      {
        for (int srcX = 0; srcX < sourceWidth(); ++srcX)
        {
          int value = pixelBrightness(src, srcX, srcY) * kSspBrightness;
          uint32_t color = grayColor(value);
          for (int dy = 0; dy < scale; ++dy)
          {
            for (int dx = 0; dx < scale; ++dx)
            {
              drawing::putPixel(canvas, bounds.x() + srcX * scale + dx, bounds.y() + srcY * scale + dy, color);
            }
          }
        }
      }
    }

    int DisplayWidget::sourceWidth() const
    {
      return 0;
    }

    int DisplayWidget::sourceHeight() const
    {
      return 0;
    }

    int DisplayWidget::pixelBrightness(const uint16_t *src, int srcX, int srcY) const
    {
      (void)src;
      (void)srcX;
      (void)srcY;
      return 0;
    }
  } // namespace ui
} // namespace percussa