#include "percussa/ui/LedWidget.h"
#include <percussa/panel/ssp/SspPanel.h>

#include <percussa/panel/ssp/SspController.h>

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
    namespace ssp
    {
      namespace
      {
        const int kScreenWidth = 1600;
        const int kScreenHeight = 480;

        const int kMargin = 32;
        const int kMainY = kMargin;
        const int kMainWidth = 256 * 3;
        const int kMainHeight = 64 * 3;

        const int kSubY = kMargin;
        const int kSubWidth = 128 * 3;
        const int kSubHeight = 64 * 3;

        const int kButtonWidth = 80;
        const int kButtonHeight = (66 * 9) / 10;
        const int kButtonGapX = 18;
        const int kButtonGapY = (22 * 9) / 10;
        const int kGridStartX = 870;

        const int kEncoderY = 364;
        const int kEncoderRadius = 62;

        const int kLedDefaultH = 28;
        const int kLedBlockW = 28;
        const int kLedAnchorOffset = 2 + 12;
        const int kLedGapX = 20;

        const int kLedFineX = 90 - kEncoderRadius;
        const int kLedFineY = kEncoderY + kEncoderRadius + 8;
        const int kLedFineCoarseW = 98;

        const int kToggleY = 334;
        const int kToggleW = 126;
        const int kToggleH = 104;
        const int kLedOutW = 70;
        const int kLedLinkW = 112;
        const int kLedRowSpacing = 44;
        const int kLedLinkOffsetY = 22;
        const int kLedCenterX = 90 + 3 * 200;
        const int kLedBaseY = (kScreenHeight - kLedDefaultH - 18) - 3 * kLedRowSpacing;

        int gridRowY(int index)
        {
          const int top = 293 + kButtonHeight / 2;
          return top + index * (kButtonHeight + kButtonGapY);
        }

        int columnX(int index)
        {
          return kGridStartX + index * (kButtonWidth + kButtonGapX);
        }

        int controlCenterX(int index)
        {
          return 90 + index * 200;
        }

        ui::Rect mainDisplayRect()
        {
          return ui::Rect(kMargin, kMainY, kMainWidth, kMainHeight);
        }

        ui::Rect subDisplayRect()
        {
          return ui::Rect(kMargin + kMainWidth + (kMargin * 2), kSubY, kSubWidth, kSubHeight);
        }

        ui::Rect buttonRect(int column, int row)
        {
          return ui::Rect(columnX(column), gridRowY(row), kButtonWidth, kButtonHeight);
        }

        ui::Rect encoderRect(int index)
        {
          int cx = controlCenterX(index);
          return ui::Rect(cx - kEncoderRadius, kEncoderY - kEncoderRadius, 2 * kEncoderRadius, 2 * kEncoderRadius);
        }

        ui::Rect toggleRect(int index)
        {
          return ui::Rect(controlCenterX(index) - kToggleW / 2, kToggleY, kToggleW, kToggleH);
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

      SspPanel::SspPanel()
      {
        mDisplays.push_back(std::shared_ptr<ui::DisplayWidget>(
          new ui::MainDisplayWidget(mainDisplayRect())));
        mDisplays.push_back(
          std::shared_ptr<ui::DisplayWidget>(new ui::SubDisplayWidget(subDisplayRect())));

        mButtons.push_back(
          ui::ButtonWidget("M1", buttonRect(0, 0), true, BUTTON_MAIN1));
        mButtons.push_back(
          ui::ButtonWidget("M2", buttonRect(1, 0), true, BUTTON_MAIN2));
        mButtons.push_back(
          ui::ButtonWidget("M3", buttonRect(2, 0), true, BUTTON_MAIN3));
        mButtons.push_back(
          ui::ButtonWidget("HOME", buttonRect(3, 0), true, BUTTON_DIAL3));
        mButtons.push_back(
          ui::ButtonWidget("ENTER", buttonRect(4, 0), true, BUTTON_ENTER));
        mButtons.push_back(
          ui::ButtonWidget("UP", buttonRect(5, 0), true, BUTTON_UP));
        mButtons.push_back(
          ui::ButtonWidget("SHIFT", buttonRect(6, 0), true, BUTTON_SHIFT));

        mButtons.push_back(
          ui::ButtonWidget("M4", buttonRect(0, 1), true, BUTTON_MAIN4));
        mButtons.push_back(
          ui::ButtonWidget("M5", buttonRect(1, 1), true, BUTTON_MAIN5));
        mButtons.push_back(
          ui::ButtonWidget("M6", buttonRect(2, 1), true, BUTTON_MAIN6));
        mButtons.push_back(
          ui::ButtonWidget("CAN", buttonRect(3, 1), true, BUTTON_DIAL2));
        mButtons.push_back(
          ui::ButtonWidget("S1", buttonRect(4, 1), true, BUTTON_SUB1));
        mButtons.push_back(
          ui::ButtonWidget("S2", buttonRect(5, 1), true, BUTTON_SUB2));
        mButtons.push_back(
          ui::ButtonWidget("S3", buttonRect(6, 1), true, BUTTON_SUB3));

        mEncoders.push_back(ui::EncoderWidget(encoderRect(0)));

        mLeds.push_back(ui::LedWidget(
          "fine", ui::Rect(kLedFineX, kLedFineY, kLedFineCoarseW, kLedDefaultH), ui::LedWidget::Red, LED_DIAL1));

        mToggles.push_back(ui::ToggleWidget("STORAGE",
                                            toggleRect(1),
                                            "user",
                                            "admin",
                                            "eject",
                                            TOGGLE_STORAGE_A,
                                            TOGGLE_STORAGE_B));
        mToggles.push_back(ui::ToggleWidget("MODE",
                                            toggleRect(2),
                                            "hold",
                                            "edit",
                                            "scope",
                                            TOGGLE_MODE_A,
                                            TOGGLE_MODE_B));

        mLeds.push_back(
          ui::LedWidget("1",
                        outputLedRect(0),
                        ui::LedWidget::Amber,
                        LED_OUT1,
                        16,
                        ui::LedWidget::LabelLeft));
        mLeds.push_back(
          ui::LedWidget("link",
                        linkLedRect(0),
                        ui::LedWidget::Red,
                        LED_LINK12,
                        16,
                        ui::LedWidget::LabelRight));
        mLeds.push_back(
          ui::LedWidget("2",
                        outputLedRect(1),
                        ui::LedWidget::Amber,
                        LED_OUT2,
                        16,
                        ui::LedWidget::LabelLeft));
        mLeds.push_back(
          ui::LedWidget("link",
                        linkLedRect(1),
                        ui::LedWidget::Red,
                        LED_LINK23,
                        16,
                        ui::LedWidget::LabelRight));
        mLeds.push_back(
          ui::LedWidget("3",
                        outputLedRect(2),
                        ui::LedWidget::Amber,
                        LED_OUT3,
                        16,
                        ui::LedWidget::LabelLeft));
        mLeds.push_back(
          ui::LedWidget("link",
                        linkLedRect(2),
                        ui::LedWidget::Red,
                        LED_LINK34,
                        16,
                        ui::LedWidget::LabelRight));
        mLeds.push_back(
          ui::LedWidget("4",
                        outputLedRect(3),
                        ui::LedWidget::Amber,
                        LED_OUT4,
                        16,
                        ui::LedWidget::LabelLeft));
      }

      int SspPanel::width() const
      {
        return kScreenWidth;
      }

      int SspPanel::height() const
      {
        return kScreenHeight;
      }

      void SspPanel::render(Olivec_Canvas canvas) const
      {
        olivec_fill(canvas, kBackground);

        const DisplayBuffer *buffer = Display_getLastPutBuffer();

        if (buffer)
        {
          mDisplays[0]->render(canvas, buffer->main);
          mDisplays[1]->render(canvas, buffer->sub);
        }

        for (auto &w : mEncoders)
        {
          w.render(canvas);
        }

        for (auto &w : mButtons)
        {
          w.render(canvas);
        }

        for (auto &w : mToggles)
        {
          w.render(canvas);
        }

        for (auto &w : mLeds)
        {
          w.render(canvas);
        }
      }

      std::unique_ptr<Controller> SspPanel::createController()
      {
        return std::unique_ptr<Controller>(new SspController(*this));
      }
    } // namespace ssp
  } // namespace panel
} // namespace percussa