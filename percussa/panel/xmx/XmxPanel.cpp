#include <percussa/panel/xmx/XmxPanel.h>

namespace
{
  const uint32_t kBackground = PERCUSSA_RGBA(31, 34, 40, 255);
}

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
      }

      XmxPanel::XmxPanel()
      {
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

      void XmxPanel::render(Olivec_Canvas canvas) const
      {
        (void)mDisplays;
        olivec_fill(canvas, kBackground);
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