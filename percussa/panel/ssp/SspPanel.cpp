#include <percussa/panel/ssp/SspPanel.h>

#include <hal/display.h>

#include <percussa/ui/MainDisplayWidget.h>
#include <percussa/ui/SubDisplayWidget.h>

namespace
{
  const uint32_t kSspBackground = PERCUSSA_RGBA(10, 10, 10, 255);
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
        const int kMainX = kMargin;
        const int kMainY = kMargin;
        const int kMainScale = 3;
        const int kMainWidth = 256 * kMainScale;
        const int kMainHeight = 64 * kMainScale;

        const int kSubX = kMainX + kMainWidth + (kMargin * 2);
        const int kSubY = kMargin;
        const int kSubScale = 3;
        const int kSubWidth = 128 * kSubScale;
        const int kSubHeight = 64 * kSubScale;

        const int kButtonWidth = 80;
        const int kButtonHeight = (66 * 9) / 10;
        const int kButtonGapX = 18;
        const int kButtonGapY = (22 * 9) / 10;
        const int kGridStartX = 870;
        const int kGridTopY = 293 + kButtonHeight / 2;
        const int kGridBottomY = kGridTopY + kButtonHeight + kButtonGapY;

        const int kRightColumnX = 1470;
        const int kRightColumnTopY = (293 - (4 * kButtonHeight + 3 * kButtonGapY) - 8) + (kButtonHeight / 4);
        const int kRightColumnStepY = kButtonHeight + kButtonGapY;

        const int kEncoderY = 364;
        const int kEncoderRadius = 62;

        const int kLedFineX = 90 - kEncoderRadius;
        const int kLedFineY = kEncoderY + kEncoderRadius + 8;
        const int kLedFineCoarseW = 98;
        const int kLedDefaultH = 28;
        const int kLedOutW = 70;
        const int kLedLinkW = 112;
        const int kLedOutLinkStepY = 22;
        const int kLedOutColumnX = (90 + 200) - (kLedOutW + 4 + kLedLinkW) / 2;
        const int kLedLinkColumnX = kLedOutColumnX + kLedOutW + 4;
        const int kLedOut1Y = (kScreenHeight - kLedDefaultH - 18) - 6 * kLedOutLinkStepY;

        const int kToggleY = 334;
        const int kToggleW = 126;
        const int kToggleH = 104;
        const int kToggleStorageX = 90 + 2 * 200 - kToggleW / 2;
        const int kToggleModeX = 90 + 3 * 200 - kToggleW / 2;

        int columnX(int index)
        {
          return kGridStartX + index * (kButtonWidth + kButtonGapX);
        }

        ui::Rect encoderRect(int index)
        {
          int cx = 90 + index * 200;
          return ui::Rect(cx - kEncoderRadius, kEncoderY - kEncoderRadius, 2 * kEncoderRadius, 2 * kEncoderRadius);
        }
      }

      SspPanel::SspPanel()
      {
        mDisplays.push_back(std::shared_ptr<ui::DisplayWidget>(new ui::MainDisplayWidget(ui::Rect(kMainX, kMainY, kMainWidth, kMainHeight))));
        mDisplays.push_back(std::shared_ptr<ui::DisplayWidget>(new ui::SubDisplayWidget(ui::Rect(kSubX, kSubY, kSubWidth, kSubHeight))));

        mButtons.push_back(ui::ButtonWidget("M1", ui::Rect(columnX(0), kGridTopY, kButtonWidth, kButtonHeight), "main", true, BUTTON_MAIN1));
        mButtons.push_back(ui::ButtonWidget("M2", ui::Rect(columnX(1), kGridTopY, kButtonWidth, kButtonHeight), "main", true, BUTTON_MAIN2));
        mButtons.push_back(ui::ButtonWidget("M3", ui::Rect(columnX(2), kGridTopY, kButtonWidth, kButtonHeight), "main", true, BUTTON_MAIN3));
        mButtons.push_back(ui::ButtonWidget("HOME", ui::Rect(columnX(3), kGridTopY, kButtonWidth, kButtonHeight), "dial", true, BUTTON_DIAL3));
        mButtons.push_back(ui::ButtonWidget("ENTER", ui::Rect(columnX(4), kGridTopY, kButtonWidth, kButtonHeight), "action", true, BUTTON_ENTER));
        mButtons.push_back(ui::ButtonWidget("UP", ui::Rect(columnX(5), kGridTopY, kButtonWidth, kButtonHeight), "action", true, BUTTON_UP));
        mButtons.push_back(ui::ButtonWidget("SHIFT", ui::Rect(columnX(6), kGridTopY, kButtonWidth, kButtonHeight), "action", true, BUTTON_SHIFT));

        mButtons.push_back(ui::ButtonWidget("M4", ui::Rect(columnX(0), kGridBottomY, kButtonWidth, kButtonHeight), "main", true, BUTTON_MAIN4));
        mButtons.push_back(ui::ButtonWidget("M5", ui::Rect(columnX(1), kGridBottomY, kButtonWidth, kButtonHeight), "main", true, BUTTON_MAIN5));
        mButtons.push_back(ui::ButtonWidget("M6", ui::Rect(columnX(2), kGridBottomY, kButtonWidth, kButtonHeight), "main", true, BUTTON_MAIN6));
        mButtons.push_back(ui::ButtonWidget("CAN", ui::Rect(columnX(3), kGridBottomY, kButtonWidth, kButtonHeight), "dial", true, BUTTON_DIAL2));
        mButtons.push_back(ui::ButtonWidget("S1", ui::Rect(columnX(4), kGridBottomY, kButtonWidth, kButtonHeight), "sub", true, BUTTON_SUB1));
        mButtons.push_back(ui::ButtonWidget("S2", ui::Rect(columnX(5), kGridBottomY, kButtonWidth, kButtonHeight), "sub", true, BUTTON_SUB2));
        mButtons.push_back(ui::ButtonWidget("S3", ui::Rect(columnX(6), kGridBottomY, kButtonWidth, kButtonHeight), "sub", true, BUTTON_SUB3));

        // mButtons.push_back(ui::ButtonWidget("1", ui::Rect(kRightColumnX, kRightColumnTopY + 0 * kRightColumnStepY, kButtonWidth, kButtonHeight), "select", true, BUTTON_SELECT1));
        // mButtons.push_back(ui::ButtonWidget("2", ui::Rect(kRightColumnX, kRightColumnTopY + 1 * kRightColumnStepY, kButtonWidth, kButtonHeight), "select", true, BUTTON_SELECT2));
        // mButtons.push_back(ui::ButtonWidget("3", ui::Rect(kRightColumnX, kRightColumnTopY + 2 * kRightColumnStepY, kButtonWidth, kButtonHeight), "select", true, BUTTON_SELECT3));
        // mButtons.push_back(ui::ButtonWidget("4", ui::Rect(kRightColumnX, kRightColumnTopY + 3 * kRightColumnStepY, kButtonWidth, kButtonHeight), "select", true, BUTTON_SELECT4));

        mEncoders.push_back(ui::EncoderWidget("DATA", encoderRect(0)));
        mEncoders.push_back(ui::EncoderWidget("OUT", encoderRect(1)));
        mEncoders.push_back(ui::EncoderWidget("STORE", encoderRect(2)));
        mEncoders.push_back(ui::EncoderWidget("MODE", encoderRect(3)));

        mLeds.push_back(ui::LedWidget("fine", ui::Rect(kLedFineX, kLedFineY, kLedFineCoarseW, kLedDefaultH), "red", LED_DIAL1));
        // mLeds.push_back(ui::LedWidget("coarse", ui::Rect(kLedFineX + kLedFineCoarseW + 4, kLedFineY, kLedFineCoarseW, kLedDefaultH), "red", LED_DIAL2));
        // mLeds.push_back(ui::LedWidget("I/O", ui::Rect(kLedFineX, kLedFineY + 30, kLedFineCoarseW, kLedDefaultH), "red", LED_IO));
        // mLeds.push_back(ui::LedWidget("safe", ui::Rect(kLedFineX, kLedFineY + 56, kLedFineCoarseW, kLedDefaultH), "red", LED_SAFE));

        mLeds.push_back(ui::LedWidget("1", ui::Rect(kLedOutColumnX, kLedOut1Y + 0 * kLedOutLinkStepY, kLedOutW, kLedDefaultH), "amber", LED_OUT1));
        mLeds.push_back(ui::LedWidget("link", ui::Rect(kLedLinkColumnX, kLedOut1Y + 1 * kLedOutLinkStepY, kLedLinkW, kLedDefaultH), "red", LED_LINK12));
        mLeds.push_back(ui::LedWidget("2", ui::Rect(kLedOutColumnX, kLedOut1Y + 2 * kLedOutLinkStepY, kLedOutW, kLedDefaultH), "amber", LED_OUT2));
        mLeds.push_back(ui::LedWidget("link", ui::Rect(kLedLinkColumnX, kLedOut1Y + 3 * kLedOutLinkStepY, kLedLinkW, kLedDefaultH), "red", LED_LINK23));
        mLeds.push_back(ui::LedWidget("3", ui::Rect(kLedOutColumnX, kLedOut1Y + 4 * kLedOutLinkStepY, kLedOutW, kLedDefaultH), "amber", LED_OUT3));
        mLeds.push_back(ui::LedWidget("link", ui::Rect(kLedLinkColumnX, kLedOut1Y + 5 * kLedOutLinkStepY, kLedLinkW, kLedDefaultH), "red", LED_LINK34));
        mLeds.push_back(ui::LedWidget("4", ui::Rect(kLedOutColumnX, kLedOut1Y + 6 * kLedOutLinkStepY, kLedOutW, kLedDefaultH), "amber", LED_OUT4));

        mToggles.push_back(ui::ToggleWidget("STORAGE", ui::Rect(kToggleStorageX, kToggleY, kToggleW, kToggleH), "user", "admin", "eject", TOGGLE_STORAGE_A, TOGGLE_STORAGE_B));
        mToggles.push_back(ui::ToggleWidget("MODE", ui::Rect(kToggleModeX, kToggleY, kToggleW, kToggleH), "hold", "edit", "scope", TOGGLE_MODE_A, TOGGLE_MODE_B));
      }

      const char *SspPanel::name() const
      {
        return "ssp";
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
        olivec_fill(canvas, kSspBackground);

        const DisplayBuffer *buffer = Display_getLastPutBuffer();
        for (size_t i = 0; i < mDisplays.size(); ++i)
        {
          const uint8_t *frame = 0;
          if (buffer)
          {
            if (i == 0)
            {
              frame = buffer->main;
            }
            else if (i == 1)
            {
              frame = buffer->sub;
            }
          }
          mDisplays[i]->render(canvas, frame);
        }

        if (!mEncoders.empty())
        {
          mEncoders[0].render(canvas);
        }

        for (size_t i = 0; i < mButtons.size(); ++i)
        {
          mButtons[i].render(canvas);
        }

        for (size_t i = 0; i < mToggles.size(); ++i)
        {
          mToggles[i].render(canvas);
        }

        for (size_t i = 0; i < mLeds.size(); ++i)
        {
          mLeds[i].render(canvas);
        }
      }

      const std::vector<std::shared_ptr<ui::DisplayWidget>> &SspPanel::displays() const
      {
        return mDisplays;
      }

      const std::vector<ui::ButtonWidget> &SspPanel::buttons() const
      {
        return mButtons;
      }

      const std::vector<ui::EncoderWidget> &SspPanel::encoders() const
      {
        return mEncoders;
      }

      const std::vector<ui::LedWidget> &SspPanel::leds() const
      {
        return mLeds;
      }

      const std::vector<ui::ToggleWidget> &SspPanel::toggles() const
      {
        return mToggles;
      }
    }
  }
}