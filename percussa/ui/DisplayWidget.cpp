#include <percussa/ui/DisplayWidget.h>

#include <percussa/ui/WidgetDrawing.h>

#include <cctype>

namespace percussa
{
  namespace ui
  {
    namespace
    {
      const uint32_t kDisplayFace = PERCUSSA_RGBA(9, 10, 12, 255);
      const uint32_t kDisplayGlow = PERCUSSA_RGBA(24, 19, 8, 255);
      const uint32_t kDisplayEdge = PERCUSSA_RGBA(106, 112, 126, 255);
      const uint32_t kDisplayText = PERCUSSA_RGBA(245, 184, 40, 255);
      const int kSspBrightness = 15;

      std::string upper(const std::string &value)
      {
        std::string result = value;
        for (size_t i = 0; i < result.size(); ++i)
        {
          result[i] = (char)std::toupper((unsigned char)result[i]);
        }
        return result;
      }

      uint32_t grayColor(int value)
      {
        return PERCUSSA_RGBA(value, (int)(value * 0.85f), 0, 255);
      }

      void drawCenteredText(Olivec_Canvas canvas, const std::string &text, const Rect &rect, int size, uint32_t color)
      {
        int textWidth = 0;
        int textHeight = 0;
        percussa_olive_text_metrics(text.c_str(), size, &textWidth, &textHeight);
        int x = rect.x + (rect.w - textWidth) / 2;
        int y = rect.y + (rect.h - textHeight) / 2;
        olivec_text(canvas, text.c_str(), x, y, percussa_olive_default_font(), (size_t)size, color);
      }
    }

    void DisplayWidget::render(Olivec_Canvas canvas, const uint8_t *frame) const
    {
      (void)frame;
      renderGeneric(canvas);
    }

    void DisplayWidget::renderGeneric(Olivec_Canvas canvas) const
    {
      olivec_rect(canvas, bounds.x, bounds.y, bounds.w, bounds.h, kDisplayGlow);
      olivec_rect(canvas, bounds.x + 4, bounds.y + 4, bounds.w - 8, bounds.h - 8, kDisplayFace);
      olivec_frame(canvas, bounds.x, bounds.y, bounds.w, bounds.h, 2, kDisplayEdge);

      Rect labelRect(bounds.x + 8, bounds.y + 6, bounds.w - 16, 14);
      drawCenteredText(canvas, upper(label), labelRect, 1, kDisplayText);
    }

    void DisplayWidget::renderFrame(Olivec_Canvas canvas, const uint8_t *frame) const
    {
      if (!frame || sourceWidth() <= 0 || sourceHeight() <= 0)
      {
        return;
      }

      const uint16_t *src = (const uint16_t *)frame;
      int scale = bounds.w / sourceWidth();
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
              drawing::putPixel(canvas, bounds.x + srcX * scale + dx, bounds.y + srcY * scale + dy, color);
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
  }
}