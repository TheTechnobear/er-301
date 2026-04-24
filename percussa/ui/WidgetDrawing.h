#pragma once

#include <percussa/ui/Geometry.h>
#include <percussa/ui/olive_bridge.h>

#include <algorithm>
#include <cmath>
#include <string>

namespace percussa
{
  namespace ui
  {
    namespace drawing
    {
      static const uint32_t kAmberText = PERCUSSA_RGBA(225, 191, 0, 255);
      static const uint32_t kLedOffShell = PERCUSSA_RGBA(48, 36, 30, 255);
      static const uint32_t kLedOffCore = PERCUSSA_RGBA(18, 16, 14, 255);
      static const uint32_t kRed = PERCUSSA_RGBA(255, 52, 44, 255);
      static const uint32_t kAmberLed = PERCUSSA_RGBA(255, 176, 36, 255);
      static const uint32_t kToggleLine = PERCUSSA_RGBA(110, 114, 126, 255);

      inline void drawCenteredOdText(Olivec_Canvas canvas, const std::string &text, const Rect &rect, int size, uint32_t color)
      {
        int textWidth = 0;
        int textHeight = 0;
        percussa_od_text_metrics(text.c_str(), size, &textWidth, &textHeight);
        int x = rect.x + (rect.w - textWidth) / 2;
        int y = rect.y + (rect.h - textHeight) / 2;
        percussa_od_text(canvas, text.c_str(), x, y, size, color);
      }

      inline void putPixel(Olivec_Canvas canvas, int x, int y, uint32_t color)
      {
        if (x < 0 || y < 0)
        {
          return;
        }
        if ((size_t)x >= canvas.width || (size_t)y >= canvas.height)
        {
          return;
        }
        canvas.pixels[(size_t)y * canvas.stride + (size_t)x] = color;
      }

      inline void drawFilledCircle(Olivec_Canvas canvas, int cx, int cy, int r, uint32_t color)
      {
        if (r <= 0)
        {
          return;
        }

        for (int yy = -r; yy <= r; yy++)
        {
          int xx = (int)std::sqrt((double)(r * r - yy * yy));
          int y = cy + yy;
          for (int x = cx - xx; x <= cx + xx; x++)
          {
            putPixel(canvas, x, y, color);
          }
        }
      }

      inline void drawRing(Olivec_Canvas canvas, int cx, int cy, int rOuter, int rInner, uint32_t color)
      {
        if (rOuter <= 0)
        {
          return;
        }

        if (rInner < 0)
        {
          rInner = 0;
        }

        int ro2 = rOuter * rOuter;
        int ri2 = rInner * rInner;
        for (int yy = -rOuter; yy <= rOuter; yy++)
        {
          int y2 = yy * yy;
          for (int xx = -rOuter; xx <= rOuter; xx++)
          {
            int d2 = xx * xx + y2;
            if (d2 <= ro2 && d2 >= ri2)
            {
              putPixel(canvas, cx + xx, cy + yy, color);
            }
          }
        }
      }

      inline void drawIndicatorLight(Olivec_Canvas canvas, int cx, int cy, int radius, bool active, uint32_t onColor)
      {
        olivec_circle(canvas, cx, cy, radius, kLedOffShell);
        olivec_circle(canvas, cx, cy, std::max(1, radius - 2), active ? onColor : kLedOffCore);
      }
    }
  }
}