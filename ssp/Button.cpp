#include <ssp/Button.h>

namespace ssp
{
  constexpr int kButtonWidth = 80;
  constexpr int kButtonHeightBase = 66;
  constexpr int kButtonHeight = (kButtonHeightBase * 9) / 10;
  constexpr int kButtonGapX = 18;
  constexpr int kButtonGapYBase = 22;
  constexpr int kButtonGapY = (kButtonGapYBase * 9) / 10;

  constexpr int kGridStartX = 870;
  constexpr int kGridTopYBase = 293;
  constexpr int kGridYOffset = kButtonHeight / 2;
  constexpr int kGridTopY = kGridTopYBase + kGridYOffset;
  constexpr int kGridBottomY = kGridTopY + kButtonHeight + kButtonGapY;

  constexpr int kRightColumnX = 1470;
  constexpr int kRightColumnBottomGap = 8;
  constexpr int kRightColumnYOffset = kGridYOffset / 2;
  constexpr int kRightColumnTopYBase = kGridTopYBase - (4 * kButtonHeight + 3 * kButtonGapY) - kRightColumnBottomGap;
  constexpr int kRightColumnTopY = kRightColumnTopYBase + kRightColumnYOffset;
  constexpr int kRightColumnStepY = kButtonHeight + kButtonGapY;

  constexpr int kLabelFontSmall = 16;
  constexpr int kLabelFontLarge = 16;
  constexpr int kLargeLabelThreshold = 90;

  constexpr int kShadowOffsetX = 4;
  constexpr int kShadowOffsetY = 6;
  constexpr int kFaceInset = 4;
  constexpr int kFrameThickness = 2;
  constexpr int kHighlightInset = 6;
  constexpr int kHighlightHeight = 10;

  constexpr uint32_t kColorShadow = SSP_RGBA(0, 0, 0, 140);
  constexpr uint32_t kColorShell = SSP_RGBA(12, 13, 16, 255);
  constexpr uint32_t kColorFaceIdle = SSP_RGBA(26, 28, 32, 255);
  constexpr uint32_t kColorFacePressed = SSP_RGBA(255, 52, 44, 255);
  constexpr uint32_t kColorEdge = SSP_RGBA(8, 8, 10, 255);
  constexpr uint32_t kColorHighlight = SSP_RGBA(130, 136, 148, 100);
  constexpr uint32_t kColorLabel = SSP_RGBA(245, 52, 44, 255);

  constexpr int colX(int index)
  {
    return kGridStartX + index * (kButtonWidth + kButtonGapX);
  }

  void Button::renderCenteredLabel(Olivec_Canvas canvas) const
  {
    // const char *text = buttonLabelForRender(button);
    const char *text = label.c_str();
    if (text[0] == '\0')
    {
      return;
    }

    int fontSize = h >= kLargeLabelThreshold ? kLabelFontLarge : kLabelFontSmall;
    int textW = 0;
    int textH = 0;
    ssp_olive_od_text_metrics(text, fontSize, &textW, &textH);
    int tx = x + (w - textW) / 2;
    int ty = y + (h - textH) / 2;

    ssp_olive_od_text(canvas, text, tx, ty, fontSize, kColorLabel);
  }

  struct RectButtonLayout
  {
    uint32_t id;
    int x;
    int y;
    int w;
    int h;
  };

  static constexpr RectButtonLayout gRectButtons[] = {
    { BUTTON_MAIN1, colX(0), kGridTopY, kButtonWidth, kButtonHeight },
    { BUTTON_MAIN2, colX(1), kGridTopY, kButtonWidth, kButtonHeight },
    { BUTTON_MAIN3, colX(2), kGridTopY, kButtonWidth, kButtonHeight },
    { BUTTON_DIAL3, colX(3), kGridTopY, kButtonWidth, kButtonHeight },
    { BUTTON_ENTER, colX(4), kGridTopY, kButtonWidth, kButtonHeight },
    { BUTTON_UP, colX(5), kGridTopY, kButtonWidth, kButtonHeight },
    { BUTTON_SHIFT, colX(6), kGridTopY, kButtonWidth, kButtonHeight },

    { BUTTON_MAIN4, colX(0), kGridBottomY, kButtonWidth, kButtonHeight },
    { BUTTON_MAIN5, colX(1), kGridBottomY, kButtonWidth, kButtonHeight },
    { BUTTON_MAIN6, colX(2), kGridBottomY, kButtonWidth, kButtonHeight },
    { BUTTON_DIAL2, colX(3), kGridBottomY, kButtonWidth, kButtonHeight },
    { BUTTON_SUB1, colX(4), kGridBottomY, kButtonWidth, kButtonHeight },
    { BUTTON_SUB2, colX(5), kGridBottomY, kButtonWidth, kButtonHeight },
    { BUTTON_SUB3, colX(6), kGridBottomY, kButtonWidth, kButtonHeight },

    { BUTTON_SELECT1, kRightColumnX, kRightColumnTopY + 0 * kRightColumnStepY, kButtonWidth, kButtonHeight },
    { BUTTON_SELECT2, kRightColumnX, kRightColumnTopY + 1 * kRightColumnStepY, kButtonWidth, kButtonHeight },
    { BUTTON_SELECT3, kRightColumnX, kRightColumnTopY + 2 * kRightColumnStepY, kButtonWidth, kButtonHeight },
    { BUTTON_SELECT4, kRightColumnX, kRightColumnTopY + 3 * kRightColumnStepY, kButtonWidth, kButtonHeight },
  };

  void Button::applyDefaultRectLayout(std::array<Button, 19> &buttons)
  {
    for (const RectButtonLayout &layout : gRectButtons)
    {
      for (Button &button : buttons)
      {
        if (button.id == layout.id)
        {
          button.setGeometry(layout.x, layout.y, layout.w, layout.h);
          break;
        }
      }
    }
  }

  void Button::render(Olivec_Canvas canvas) const
  {
    if (!hasGeometry())
    {
      return;
    }

    uint32_t face = isPressed() ? kColorFacePressed : kColorFaceIdle;

    olivec_rect(canvas, x + kShadowOffsetX, y + kShadowOffsetY, w, h, kColorShadow);
    olivec_rect(canvas, x, y, w, h, kColorShell);
    olivec_rect(canvas, x + kFaceInset, y + kFaceInset, w - 2 * kFaceInset, h - 2 * kFaceInset, face);
    olivec_frame(canvas, x, y, w, h, kFrameThickness, kColorEdge);
    olivec_rect(
      canvas, x + kHighlightInset, y + kHighlightInset, w - 2 * kHighlightInset, kHighlightHeight, kColorHighlight);
    renderCenteredLabel(canvas);
  }
} // namespace ssp
