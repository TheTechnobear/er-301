#include <ssp/Toggle.h>

#include <algorithm>

namespace ssp
{
  constexpr int kEncoderStartX = 90;
  constexpr int kEncoderStepX = 200;
  constexpr int kEncoderY = 392;

  constexpr int kToggleTitleFont = 16;
  constexpr int kToggleRowFont = 16;
  constexpr int kToggleDefaultY = 334;
  constexpr int kToggleDefaultW = 126;
  constexpr int kToggleDefaultH = 104;
  constexpr int kToggleStorageX = kEncoderStartX + 2 * kEncoderStepX - kToggleDefaultW / 2;
  constexpr int kToggleModeX = kEncoderStartX + 3 * kEncoderStepX - kToggleDefaultW / 2;
  constexpr int kTogglePaddingX = 4;
  constexpr int kTogglePaddingY = 6;
  constexpr int kToggleTextLineGap = 4;

  constexpr uint32_t kToggleTitleColor = SSP_RGBA(214, 216, 222, 255);
  constexpr uint32_t kToggleLineColor = SSP_RGBA(110, 114, 126, 255);
  constexpr uint32_t kToggleActiveColor = SSP_RGBA(255, 52, 44, 255);
  constexpr uint32_t kToggleInactiveShell = SSP_RGBA(48, 36, 30, 255);
  constexpr uint32_t kToggleInactiveCore = SSP_RGBA(18, 16, 14, 255);

  struct ToggleLayout
  {
    uint32_t idA;
    int x;
    int y;
    int w;
    int h;
  };

  static constexpr ToggleLayout gToggleLayouts[] = {
    { TOGGLE_STORAGE_A, kToggleStorageX, kToggleDefaultY, kToggleDefaultW, kToggleDefaultH },
    { TOGGLE_MODE_A, kToggleModeX, kToggleDefaultY, kToggleDefaultW, kToggleDefaultH },
  };

  static inline void drawLed(Olivec_Canvas canvas, int cx, int cy, int radius, bool isOn)
  {
    olivec_circle(canvas, cx, cy, radius, kToggleInactiveShell);
    olivec_circle(canvas, cx, cy, std::max(1, radius - 2), isOn ? kToggleActiveColor : kToggleInactiveCore);
  }

  void Toggle::applyDefaultLayout(std::array<Toggle, 2> &toggles)
  {
    for (const ToggleLayout &layout : gToggleLayouts)
    {
      for (Toggle &toggle : toggles)
      {
        if (toggle.idA == layout.idA)
        {
          toggle.setGeometry(layout.x, layout.y, layout.w, layout.h);
          break;
        }
      }
    }
  }

  void Toggle::render(Olivec_Canvas canvas) const
  {
    if (!hasGeometry())
    {
      return;
    }

    int titleW = 0;
    int titleH = 0;
    ssp_olive_od_text_metrics(label, kToggleTitleFont, &titleW, &titleH);
    ssp_olive_od_text(canvas, label, x + (w - titleW) / 2, y, kToggleTitleFont, kToggleTitleColor);

    int bodyTop = y + titleH + kTogglePaddingY;
    int rowHeight = std::max(20, (h - titleH - 2 * kTogglePaddingY) / 3);
    int ledRadius = std::max(4, std::min(8, rowHeight / 3));
    int ledCx = x + kTogglePaddingX + ledRadius;
    int lineX1 = ledCx + ledRadius + 6;
    int lineX2 = x + w - kTogglePaddingX;

    const char *rowLabels[3] = { up, mid, down };
    const int activeState = state();
    for (int i = 0; i < 3; i++)
    {
      int cy = bodyTop + i * rowHeight + rowHeight / 2;
      int textW = 0;
      int textH = 0;
      ssp_olive_od_text_metrics(rowLabels[i], kToggleRowFont, &textW, &textH);
      int textX = lineX1 + 2;
      int textY = cy - textH / 2;
      int gapLeft = textX - kToggleTextLineGap;
      int gapRight = textX + textW + kToggleTextLineGap;
      if (gapLeft > lineX1)
      {
        olivec_line(canvas, lineX1, cy, gapLeft, cy, kToggleLineColor);
      }
      if (gapRight < lineX2)
      {
        olivec_line(canvas, gapRight, cy, lineX2, cy, kToggleLineColor);
      }
      ssp_olive_od_text(canvas, rowLabels[i], textX, textY, kToggleRowFont, kToggleTitleColor);
      drawLed(canvas, ledCx, cy, ledRadius, activeState == i);
    }
  }
} // namespace ssp