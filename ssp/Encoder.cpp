#include <ssp/Encoder.h>

#include <cmath>

namespace ssp
{
  constexpr int kEncoderCount = 4;
  constexpr int kEncoderStartX = 90;
  constexpr int kEncoderStepX = 200;
  constexpr int kEncoderY = 392;
  constexpr int kEncoderRadius = 62;

  constexpr int kShadowOffsetX = 6;
  constexpr int kShadowOffsetY = 8;
  constexpr int kOuterRadiusOffset = 2;
  constexpr int kFaceRadiusInset = 2;
  constexpr int kRimOuterInset = 2;
  constexpr int kRimInnerInset = 4;

  constexpr uint32_t kColorShadow = SSP_RGBA(0, 0, 0, 255);
  constexpr uint32_t kColorShell = SSP_RGBA(28, 30, 36, 255);
  constexpr uint32_t kColorFace = SSP_RGBA(232, 236, 246, 255);
  constexpr uint32_t kColorRim = SSP_RGBA(86, 92, 108, 255);
  constexpr uint32_t kColorGlow = SSP_RGBA(255, 52, 44, 255);

  constexpr int encoderCenterX(int index)
  {
    return kEncoderStartX + index * kEncoderStepX;
  }

  struct EncoderLayout
  {
    int cx;
    int cy;
    int r;
  };

  static constexpr EncoderLayout gEncoders[] = {
    { encoderCenterX(0), kEncoderY, kEncoderRadius },
    { encoderCenterX(1), kEncoderY, kEncoderRadius },
    { encoderCenterX(2), kEncoderY, kEncoderRadius },
    { encoderCenterX(3), kEncoderY, kEncoderRadius },
  };

  static inline void putPixel(Olivec_Canvas canvas, int x, int y, uint32_t color)
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

  static inline void drawFilledCircle(Olivec_Canvas canvas, int cx, int cy, int r, uint32_t color)
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

  static inline void drawRing(Olivec_Canvas canvas, int cx, int cy, int rOuter, int rInner, uint32_t color)
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

  void Encoder::applyDefaultLayout(std::array<Encoder, 4> &encoders)
  {
    for (int i = 0; i < kEncoderCount; i++)
    {
      if ((size_t)i >= encoders.size())
      {
        break;
      }
      encoders[i].setGeometry(gEncoders[i].cx, gEncoders[i].cy, gEncoders[i].r);
    }
  }

  void Encoder::render(Olivec_Canvas canvas) const
  {
    if (!hasGeometry())
    {
      return;
    }

    drawFilledCircle(canvas, cx + kShadowOffsetX, cy + kShadowOffsetY, r + kOuterRadiusOffset, kColorShadow);
    drawFilledCircle(canvas, cx, cy, r + kOuterRadiusOffset, kColorShell);
    drawFilledCircle(canvas, cx, cy, r - kFaceRadiusInset, kColorFace);
    drawRing(canvas, cx, cy, r - kRimOuterInset, r - kRimInnerInset, kColorRim);
    if (isPressed())
    {
      drawFilledCircle(canvas, cx, cy, r / 3, kColorGlow);
    }
  }
}
