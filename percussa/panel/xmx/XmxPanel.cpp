#include <percussa/panel/xmx/XmxPanel.h>

namespace percussa
{
  namespace panel
  {
    namespace xmx
    {
      namespace
      {
        const int kScreenWidth = 520;
        const int kScreenHeight = 430;

        const int kDisplayX = 100;
        const int kDisplayY = 24;
        const int kDisplayW = 320;
        const int kDisplayH = 240;

        const int kSoftButtonW = 64;
        const int kSoftButtonH = 34;
        const int kSoftButtonGap = 10;
        const int kTopButtonY = 32;

        const int kLeftButtonX = 20;
        const int kRightButtonX = 436;

        const int kEncoderY = 316;
        const int kEncoderRadius = 28;
        const int kEncoderGap = 26;
        const int kEncoderStartX = 76;

        ui::Rect encoderRect(int index)
        {
          int x = kEncoderStartX + index * (2 * kEncoderRadius + kEncoderGap);
          return ui::Rect(x, kEncoderY, 2 * kEncoderRadius, 2 * kEncoderRadius);
        }
      }

      XmxPanel::XmxPanel()
      {
        mDisplays.push_back(ui::DisplayWidget("main", ui::Rect(kDisplayX, kDisplayY, kDisplayW, kDisplayH), "main"));

        mButtons.push_back(ui::ButtonWidget("S1", ui::Rect(kLeftButtonX, kTopButtonY + 0 * (kSoftButtonH + kSoftButtonGap), kSoftButtonW, kSoftButtonH), "soft"));
        mButtons.push_back(ui::ButtonWidget("S2", ui::Rect(kLeftButtonX, kTopButtonY + 1 * (kSoftButtonH + kSoftButtonGap), kSoftButtonW, kSoftButtonH), "soft"));
        mButtons.push_back(ui::ButtonWidget("S3", ui::Rect(kLeftButtonX, kTopButtonY + 2 * (kSoftButtonH + kSoftButtonGap), kSoftButtonW, kSoftButtonH), "soft"));
        mButtons.push_back(ui::ButtonWidget("S4", ui::Rect(kLeftButtonX, kTopButtonY + 3 * (kSoftButtonH + kSoftButtonGap), kSoftButtonW, kSoftButtonH), "soft"));

        mButtons.push_back(ui::ButtonWidget("S5", ui::Rect(kRightButtonX, kTopButtonY + 0 * (kSoftButtonH + kSoftButtonGap), kSoftButtonW, kSoftButtonH), "soft"));
        mButtons.push_back(ui::ButtonWidget("S6", ui::Rect(kRightButtonX, kTopButtonY + 1 * (kSoftButtonH + kSoftButtonGap), kSoftButtonW, kSoftButtonH), "soft"));
        mButtons.push_back(ui::ButtonWidget("S7", ui::Rect(kRightButtonX, kTopButtonY + 2 * (kSoftButtonH + kSoftButtonGap), kSoftButtonW, kSoftButtonH), "soft"));
        mButtons.push_back(ui::ButtonWidget("S8", ui::Rect(kRightButtonX, kTopButtonY + 3 * (kSoftButtonH + kSoftButtonGap), kSoftButtonW, kSoftButtonH), "soft"));

        mEncoders.push_back(ui::EncoderWidget("E1", encoderRect(0)));
        mEncoders.push_back(ui::EncoderWidget("E2", encoderRect(1)));
        mEncoders.push_back(ui::EncoderWidget("E3", encoderRect(2)));
        mEncoders.push_back(ui::EncoderWidget("E4", encoderRect(3)));

        mLeds.push_back(ui::LedWidget("state", ui::Rect(244, 282, 12, 12), "amber"));

        mToggles.push_back(ui::ToggleWidget("NAV", ui::Rect(404, 298, 88, 70), "up", "hold", "down"));
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
        return kScreenHeight;
      }

      const std::vector<ui::DisplayWidget> &XmxPanel::displays() const
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