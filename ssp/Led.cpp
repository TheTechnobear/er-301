#include <ssp/Led.h>
#include "constants.h"

#include <algorithm>

namespace ssp
{
  constexpr int kEncoderStartX = 90;
  constexpr int kEncoderStepX = 200;
  constexpr int kEncoderY = 364;
  constexpr int kEncoderRadius = 62;
  constexpr int kScreenBottom = SCREEN_HEIGHT;

  constexpr int kLedLabelFont = 16;
  constexpr int kLedCirclePadding = 2;
  constexpr int kLedLabelGap = 6;
  constexpr int kLedFineCoarseW = 98;
  constexpr int kLedFineCoarseGap = 4;
  constexpr int kLedFineX = kEncoderStartX - kEncoderRadius;
  constexpr int kLedFineY = kEncoderY + kEncoderRadius + 8;

  constexpr int kLedOutW = 70;
  constexpr int kLedLinkW = 112;
  constexpr int kLedDefaultH = 28;
  constexpr int kLedOutLinkGapX = 4;
  constexpr int kLedOutLinkStepY = 22;
  constexpr int kLedOutBaseY = kScreenBottom - kLedDefaultH - 18;
  constexpr int kLedOutCenterX = kEncoderStartX + kEncoderStepX;
  constexpr int kLedOutColumnX = kLedOutCenterX - (kLedOutW + kLedOutLinkGapX + kLedLinkW) / 2;
  constexpr int kLedLinkColumnX = kLedOutColumnX + kLedOutW + kLedOutLinkGapX;
  constexpr int kLedOut1Y = kLedOutBaseY - 6 * kLedOutLinkStepY;

  constexpr uint32_t kLedOffShell = SSP_RGBA(48, 36, 30, 255);
  constexpr uint32_t kLedOffCore = SSP_RGBA(18, 16, 14, 255);
  constexpr uint32_t kLedLabelColor = SSP_RGBA(MAIN_DISPLAY_AMBER_R, MAIN_DISPLAY_AMBER_G, 0, 255);

  struct LedLayout
  {
    uint32_t id;
    int x;
    int y;
    int w;
    int h;
  };

  static constexpr LedLayout gLedLayouts[] = {
    { LED_DIAL1, kLedFineX, kLedFineY, kLedFineCoarseW, kLedDefaultH },
    { LED_DIAL2, kLedFineX + kLedFineCoarseW + kLedFineCoarseGap, kLedFineY, kLedFineCoarseW,
      kLedDefaultH },
    { LED_IO, kLedFineX, kLedFineY + 30, kLedFineCoarseW, kLedDefaultH },
    { LED_SAFE, kLedFineX, kLedFineY + 56, kLedFineCoarseW, kLedDefaultH },
    { LED_OUT1, kLedOutColumnX, kLedOut1Y + 0 * kLedOutLinkStepY, kLedOutW, kLedDefaultH },
    { LED_OUT2, kLedOutColumnX, kLedOut1Y + 2 * kLedOutLinkStepY, kLedOutW, kLedDefaultH },
    { LED_OUT3, kLedOutColumnX, kLedOut1Y + 4 * kLedOutLinkStepY, kLedOutW, kLedDefaultH },
    { LED_OUT4, kLedOutColumnX, kLedOut1Y + 6 * kLedOutLinkStepY, kLedOutW, kLedDefaultH },
    { LED_LINK12, kLedLinkColumnX, kLedOut1Y + 1 * kLedOutLinkStepY, kLedLinkW, kLedDefaultH },
    { LED_LINK23, kLedLinkColumnX, kLedOut1Y + 3 * kLedOutLinkStepY, kLedLinkW, kLedDefaultH },
    { LED_LINK34, kLedLinkColumnX, kLedOut1Y + 5 * kLedOutLinkStepY, kLedLinkW, kLedDefaultH },
  };

  void Led::applyDefaultLayout(std::array<Led, 11> &leds)
  {
    for (const LedLayout &layout : gLedLayouts)
    {
      for (Led &led : leds)
      {
        if (led.id == layout.id)
        {
          led.setGeometry(layout.x, layout.y, layout.w, layout.h);
          break;
        }
      }
    }
  }

  void Led::render(Olivec_Canvas canvas) const
  {
    if (!hasGeometry())
    {
      return;
    }

    int radius = std::max(2, std::min(h / 2 - kLedCirclePadding, w / 6));
    int cx = x + kLedCirclePadding + radius;
    int cy = y + h / 2;

    olivec_circle(canvas, cx, cy, radius, kLedOffShell);
    if (state())
    {
      olivec_circle(canvas, cx, cy, std::max(1, radius - 2), color);
    }
    else
    {
      olivec_circle(canvas, cx, cy, std::max(1, radius - 2), kLedOffCore);
    }

    if (!label.empty())
    {
      int textW = 0;
      int textH = 0;
      ssp_olive_od_text_metrics(label.c_str(), kLedLabelFont, &textW, &textH);
      int tx = cx + radius + kLedLabelGap;
      int ty = y + (h - textH) / 2;
      ssp_olive_od_text(canvas, label.c_str(), tx, ty, kLedLabelFont, kLedLabelColor);
    }
  }
} // namespace ssp