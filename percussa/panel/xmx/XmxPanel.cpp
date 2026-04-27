#include <percussa/panel/xmx/XmxPanel.h>

#include <percussa/panel/xmx/XmxController.h>

#include <hal/display.h>

#include <percussa/ui/MainDisplayWidget.h>
#include <percussa/ui/SubDisplayWidget.h>

namespace
{
  const uint32_t kBackground = PERCUSSA_RGBA(10, 10, 10, 255);
}

namespace percussa
{
  namespace panel
  {
    namespace xmx
    {
      namespace
      {
        // The logical XMX panel is 320x240. The linux framebuffer quirk is handled in FbdevPlatform.
        const int kScreenWidth = 320;
        const int kScreenHeight = 240;

        const int kMargin = 8;
        const int kMainY = kMargin * 2;
        const int kMainWidth = 256;
        const int kMainHeight = 64;

        const int kSubY = kMainY + kMainHeight + kMargin;
        const int kSubWidth = 128;
        const int kSubHeight = 64;

        const int kEncoderRadius = (kScreenWidth - (5 * kMargin)) / 4;

        const int kButtonWidth = 45;
        const int kButtonHeight = 25;
        const int kButtonGapX = 4;
        const int kButtonGapY = 4;

        const int kRightColumnX = kScreenWidth - kButtonWidth - kMargin;
        const int kFontSize = 12;

        int encoderX(int index)
        {
          return ((kEncoderRadius + kMargin) * index) + kMargin;
        }

        int buttonColumnX(int index)
        {
          return kMargin + index * (kButtonWidth + kButtonGapX);
        }

        int buttonRowY(int index)
        {
          const int top = kScreenHeight - kMargin - (2 * kButtonHeight) + kButtonGapY;
          return top + index * (kButtonHeight + kButtonGapY);
        }

        int rightColumnY(int index)
        {
          return kMargin + index * (kButtonHeight + kButtonGapY);
        }

        ui::Rect mainDisplayRect()
        {
          return ui::Rect(kMargin, kMainY, kMainWidth, kMainHeight);
        }

        ui::Rect subDisplayRect()
        {
          return ui::Rect(kMargin, kSubY, kSubWidth, kSubHeight);
        }

        ui::Rect gridButtonRect(int column, int row)
        {
          return ui::Rect(buttonColumnX(column), buttonRowY(row), kButtonWidth, kButtonHeight);
        }

        ui::Rect rightButtonRect(int row)
        {
          return ui::Rect(kRightColumnX, rightColumnY(row), kButtonWidth, kButtonHeight);
        }

        const int kLedDefaultH = 10;
        const int kLedBlockW = 12;

        const int kLedRowSpacing = 16;
        const int kLedLinkOffsetY = 8;
        const int kLedAnchorOffset = 2 + 4;
        const int kLedCenterX = encoderX(3) + kEncoderRadius / 2;
        const int kLedGapX = 8;
        const int kLedBaseY = kScreenHeight - kEncoderRadius + kMargin;

        const int kToggleY = kSubY + (kMargin * 2);

        ui::Rect toggleRect(int index)
        {
          return ui::Rect(encoderX(index), kToggleY, kEncoderRadius, kEncoderRadius);
        }

        ui::Rect fineLedRect()
        {
          return ui::Rect(encoderX(0), kScreenHeight - kEncoderRadius, kEncoderRadius, kLedDefaultH);
        }

        ui::Rect outputLedRect(int outputIndex)
        {
          return ui::Rect(kLedCenterX - kLedGapX / 2 - kLedBlockW + kLedAnchorOffset,
                          kLedBaseY + outputIndex * kLedRowSpacing,
                          kLedBlockW,
                          kLedDefaultH);
        }

        ui::Rect linkLedRect(int linkIndex)
        {
          return ui::Rect(kLedCenterX + kLedGapX / 2 - kLedAnchorOffset,
                          kLedBaseY + kLedLinkOffsetY + linkIndex * kLedRowSpacing,
                          kLedBlockW,
                          kLedDefaultH);
        }
      } // namespace

      XmxPanel::XmxPanel()
      {
        mDisplays.push_back(std::shared_ptr<ui::DisplayWidget>(
          new ui::MainDisplayWidget(mainDisplayRect(), kFontSize)));
        mDisplays.push_back(std::shared_ptr<ui::DisplayWidget>(
          new ui::SubDisplayWidget(subDisplayRect(), kFontSize)));

        mBiButtons.push_back(
          ui::BiButtonWidget("M1", "S1", gridButtonRect(0, 0), true, kFontSize));
        mBiButtons.push_back(
          ui::BiButtonWidget("M2", "S2", gridButtonRect(1, 0), true, kFontSize));
        mBiButtons.push_back(
          ui::BiButtonWidget("M3", "S3", gridButtonRect(2, 0), true, kFontSize));
        mBiButtons.push_back(ui::BiButtonWidget(
          "Ent", "Can", gridButtonRect(3, 0), true, kFontSize));

        mBiButtons.push_back(ui::BiButtonWidget(
          "M4", "", gridButtonRect(0, 1), true, kFontSize));
        mBiButtons.push_back(ui::BiButtonWidget(
          "M5", "", gridButtonRect(1, 1), true, kFontSize));
        mBiButtons.push_back(ui::BiButtonWidget(
          "M6", "", gridButtonRect(2, 1), true, kFontSize));
        mBiButtons.push_back(ui::BiButtonWidget(
          "Fn", "Fn", gridButtonRect(3, 1), true, kFontSize));

        mBiButtons.push_back(ui::BiButtonWidget(
          "Up",
          "Home",
          rightButtonRect(0),
          true,
          kFontSize));
        mBiButtons.push_back(ui::BiButtonWidget(
          "Shft",
          "Shft",
          rightButtonRect(1),
          true,
          kFontSize));


        mLeds.push_back(ui::LedWidget("fine",
                                      fineLedRect(),
                                      ui::LedWidget::Red,
                                      LED_DIAL1,
                                      kFontSize));

        mLeds.push_back(
          ui::LedWidget("1",
                        outputLedRect(0),
                        ui::LedWidget::Amber,
                        LED_OUT1,
                        kFontSize,
                        ui::LedWidget::LabelLeft));
        mLeds.push_back(
          ui::LedWidget("link",
                        linkLedRect(0),
                        ui::LedWidget::Red,
                        LED_LINK12,
                        kFontSize,
                        ui::LedWidget::LabelRight));
        mLeds.push_back(
          ui::LedWidget("2",
                        outputLedRect(1),
                        ui::LedWidget::Amber,
                        LED_OUT2,
                        kFontSize,
                        ui::LedWidget::LabelLeft));
        mLeds.push_back(
          ui::LedWidget("link",
                        linkLedRect(1),
                        ui::LedWidget::Red,
                        LED_LINK23,
                        kFontSize,
                        ui::LedWidget::LabelRight));
        mLeds.push_back(
          ui::LedWidget("3",
                        outputLedRect(2),
                        ui::LedWidget::Amber,
                        LED_OUT3,
                        kFontSize,
                        ui::LedWidget::LabelLeft));
        mLeds.push_back(
          ui::LedWidget("link",
                        linkLedRect(2),
                        ui::LedWidget::Red,
                        LED_LINK34,
                        kFontSize,
                        ui::LedWidget::LabelRight));
        mLeds.push_back(
          ui::LedWidget("4",
                        outputLedRect(3),
                        ui::LedWidget::Amber,
                        LED_OUT4,
                        kFontSize,
                        ui::LedWidget::LabelLeft));

        mToggles.push_back(ui::ToggleWidget("STORAGE",
                                            toggleRect(2),
                                            "user",
                                            "admin",
                                            "eject",
                                            TOGGLE_STORAGE_A,
                                            TOGGLE_STORAGE_B,
                                            kFontSize));
        mToggles.push_back(ui::ToggleWidget("MODE",
                                            toggleRect(3),
                                            "hold",
                                            "edit",
                                            "scope",
                                            TOGGLE_MODE_A,
                                            TOGGLE_MODE_B,
                                            kFontSize));
      }

      void XmxPanel::setFnShift(bool s)
      {
        mFnShift = s;
      }

      int XmxPanel::width() const
      {
        return kScreenWidth;
      }

      int XmxPanel::height() const
      {
        return kScreenHeight;
      }

      void XmxPanel::render(Olivec_Canvas canvas) const
      {
        olivec_fill(canvas, kBackground);
        const DisplayBuffer *buffer = Display_getLastPutBuffer();

        if (buffer)
        {
          mDisplays[0]->render(canvas, buffer->main);
          mDisplays[1]->render(canvas, buffer->sub);
        }

        for (auto &w : mToggles)
        {
          w.render(canvas);
        }

        for (auto &w : mLeds)
        {
          w.render(canvas);
        }
        for (auto &w : mBiButtons)
        {
          w.render(canvas, mFnShift);
        }
      }

      std::unique_ptr<Controller> XmxPanel::createController()
      {
        return std::unique_ptr<Controller>(new XmxController(*this));
      }
    } // namespace xmx
  } // namespace panel
} // namespace percussa