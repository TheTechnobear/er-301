#include <percussa/panel/xmx/XmxPanel.h>

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
        // note: XMX hardware FB is is 320x320, but displayable is 320x240
        // so height() retuns 320 for FB pixel buffer, but we only draw 240
        const int kScreenWidth = 320;
        const int kScreenHeight = 240;
      
        const int kMargin = 8;
        const int kMainX = kMargin;
        const int kMainY = kMargin * 2;
        const int kMainScale = 1;
        const int kMainWidth = 256 * kMainScale;
        const int kMainHeight = 64 * kMainScale;

        const int kSubX = kMargin ; 
        const int kSubY = kMainY + kMainHeight + kMargin;
        const int kSubScale = 1;
        const int kSubWidth = 128 * kSubScale;
        const int kSubHeight = 64 * kSubScale;

        const int kEncoderRadius = (kScreenWidth - (5 * kMargin))  / 4;
        const int kEncoderY = kScreenHeight - kEncoderRadius - kMargin;

        const int kButtonWidth = 45;
        const int kButtonHeight = 25;
        const int kButtonGapX = 4;
        const int kButtonGapY = 4;
        const int kGridTopY = kScreenHeight - kMargin - (2 * kButtonHeight) + kButtonGapY;
        const int kGridBottomY = kGridTopY + kButtonHeight + kButtonGapY;

        const int kRightColumnX = kScreenWidth - kButtonWidth - kMargin;
        const int kRightColumnTopY = 0 + kMargin;
        const int kRightColumnStepY = kButtonHeight + kButtonGapY;


        int fontSize = 12;

        ui::Rect encoderRect(int index)
        {
          int cx = ((kEncoderRadius + kMargin) * index) + kMargin;
          return ui::Rect(cx , kEncoderY , kEncoderRadius, kEncoderRadius);
        }

        const int kLedDefaultH = 10;

        auto outRect = encoderRect(3);
        const int kLedOutW = outRect.w / 2;
        const int kLedOutColumnX = outRect.x;
        const int kLedLinkColumnX = kLedOutColumnX + kLedOutW - 5;

        const int kLedLinkW = outRect.w / 2;
        const int kLedOutLinkStepY = 8; // text h
        const int kLedOut1Y = kScreenHeight - outRect.h + kMargin;

        auto toggleStorage = encoderRect(2);
        auto toggleMode = encoderRect(3);

        const int kToggleY = kSubY + ( kMargin * 2);
        const int kToggleW = toggleStorage.w;
        const int kToggleH = toggleStorage.h;
        const int kToggleStorageX = toggleStorage.x;
        const int kToggleModeX = toggleMode.x;

        int columnX(int index)
        {
          return kMargin + index * (kButtonWidth + kButtonGapX);
        }

        auto ledFine = encoderRect(0);
        const int kLedFineCoarseW = ledFine.w;
        const int kLedFineX = ledFine.x;
        const int kLedFineY = kScreenHeight - ledFine.h;
        // const int kLedFineY = kScreenHeight - kMargin;
      }

      XmxPanel::XmxPanel()
      {
        mDisplays.push_back(std::shared_ptr<ui::DisplayWidget>(new ui::MainDisplayWidget(ui::Rect(kMainX, kMainY, kMainWidth, kMainHeight), fontSize)));
        mDisplays.push_back(std::shared_ptr<ui::DisplayWidget>(new ui::SubDisplayWidget(ui::Rect(kSubX, kSubY, kSubWidth, kSubHeight),fontSize)));

        mBiButtons.push_back(ui::BiButtonWidget("M1","S1",ui::Rect(columnX(0), kGridTopY, kButtonWidth, kButtonHeight), "main", true,fontSize));
        mBiButtons.push_back(ui::BiButtonWidget("M2","S2", ui::Rect(columnX(1), kGridTopY, kButtonWidth, kButtonHeight), "main", true,fontSize));
        mBiButtons.push_back(ui::BiButtonWidget("M3","S3", ui::Rect(columnX(2), kGridTopY, kButtonWidth, kButtonHeight), "main", true,fontSize));
        mBiButtons.push_back(ui::BiButtonWidget("Ent", "Can", ui::Rect(columnX(3), kGridTopY, kButtonWidth, kButtonHeight), "dial", true,fontSize));

        mBiButtons.push_back(ui::BiButtonWidget("M4", "", ui::Rect(columnX(0), kGridBottomY, kButtonWidth, kButtonHeight), "main", true,fontSize));
        mBiButtons.push_back(ui::BiButtonWidget("M5", "", ui::Rect(columnX(1), kGridBottomY, kButtonWidth, kButtonHeight), "main", true,fontSize));
        mBiButtons.push_back(ui::BiButtonWidget("M6", "", ui::Rect(columnX(2), kGridBottomY, kButtonWidth, kButtonHeight), "main", true,fontSize));
        mBiButtons.push_back(ui::BiButtonWidget("Fn", "Fn", ui::Rect(columnX(3), kGridBottomY, kButtonWidth, kButtonHeight), "dial", true,fontSize));

        mBiButtons.push_back(ui::BiButtonWidget("Up","Home",ui::Rect(kRightColumnX, kRightColumnTopY + 0 * kRightColumnStepY, kButtonWidth, kButtonHeight), "select", true,fontSize));
        mBiButtons.push_back(ui::BiButtonWidget("Shft", "", ui::Rect(kRightColumnX, kRightColumnTopY + 1 * kRightColumnStepY, kButtonWidth, kButtonHeight), "select", true,fontSize));


        mLeds.push_back(ui::LedWidget("fine", ui::Rect(kLedFineX, kLedFineY, kLedFineCoarseW, kLedDefaultH), "red", LED_DIAL1,fontSize));

        mLeds.push_back(ui::LedWidget("1", ui::Rect(kLedOutColumnX, kLedOut1Y + 0 * kLedOutLinkStepY, kLedOutW, kLedDefaultH), "amber", LED_OUT1,fontSize));
        mLeds.push_back(ui::LedWidget("link", ui::Rect(kLedLinkColumnX, kLedOut1Y + 1 * kLedOutLinkStepY, kLedLinkW, kLedDefaultH), "red", LED_LINK12,fontSize));
        mLeds.push_back(ui::LedWidget("2", ui::Rect(kLedOutColumnX, kLedOut1Y + 2 * kLedOutLinkStepY, kLedOutW, kLedDefaultH), "amber", LED_OUT2,fontSize));
        mLeds.push_back(ui::LedWidget("link", ui::Rect(kLedLinkColumnX, kLedOut1Y + 3 * kLedOutLinkStepY, kLedLinkW, kLedDefaultH), "red", LED_LINK23,fontSize));
        mLeds.push_back(ui::LedWidget("3", ui::Rect(kLedOutColumnX, kLedOut1Y + 4 * kLedOutLinkStepY, kLedOutW, kLedDefaultH), "amber", LED_OUT3,fontSize));
        mLeds.push_back(ui::LedWidget("link", ui::Rect(kLedLinkColumnX, kLedOut1Y + 5 * kLedOutLinkStepY, kLedLinkW, kLedDefaultH), "red", LED_LINK34,fontSize));
        mLeds.push_back(ui::LedWidget("4", ui::Rect(kLedOutColumnX, kLedOut1Y + 6 * kLedOutLinkStepY, kLedOutW, kLedDefaultH), "amber", LED_OUT4,fontSize));

        mToggles.push_back(ui::ToggleWidget("STORAGE", ui::Rect(kToggleStorageX, kToggleY, kToggleW, kToggleH), "user", "admin", "eject", TOGGLE_STORAGE_A, TOGGLE_STORAGE_B,fontSize));
        mToggles.push_back(ui::ToggleWidget("MODE", ui::Rect(kToggleModeX, kToggleY, kToggleW, kToggleH), "hold", "edit", "scope", TOGGLE_MODE_A, TOGGLE_MODE_B,fontSize));
      }

      void XmxPanel::setFnShift(bool s) {
        for (size_t i = 0; i < mBiButtons.size(); ++i)
        {
          mBiButtons[i].setState(s);
        }
      }

      const char *XmxPanel::name() const
      {
        return "xmx";
      }

      int XmxPanel::width() const
      {
        return kScreenWidth;
      }

      int XmxPanel::height() const
      {
#if defined(__linux__) && defined(TARGET_XMX)
        // xmx hardware fb is 320, but only 240 is displayed!.
        return 320;
#else         
        return kScreenHeight;
#endif 
      }

      void XmxPanel::render(Olivec_Canvas canvas) const
      {
        olivec_fill(canvas, kBackground);

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

        for (size_t i = 0; i < mEncoders.size(); ++i) {
          mEncoders[i].render(canvas);
        }

        for (size_t i = 0; i < mButtons.size(); ++i)
        {
          mButtons[i].render(canvas);
        }

        for (size_t i = 0; i < mBiButtons.size(); ++i)
        {
          mBiButtons[i].render(canvas);
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

      const std::vector<std::shared_ptr<ui::DisplayWidget>> &XmxPanel::displays() const
      {
        return mDisplays;
      }

      const std::vector<ui::ButtonWidget> &XmxPanel::buttons() const
      {
        return mButtons;
      }

      const std::vector<ui::EncoderWidget> &XmxPanel::encoders() const
      {
        return mEncoders;
      }

      const std::vector<ui::LedWidget> &XmxPanel::leds() const
      {
        return mLeds;
      }

      const std::vector<ui::ToggleWidget> &XmxPanel::toggles() const
      {
        return mToggles;
      }
    }
  }
}