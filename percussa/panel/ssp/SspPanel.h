#pragma once

#include <percussa/panel/Panel.h>

namespace percussa
{
  namespace panel
  {
    namespace ssp
    {
      class SspPanel : public Panel
      {
      public:
        SspPanel();

        const char *name() const;
        int width() const;
        int height() const;
        const std::vector<ui::DisplayWidget> &displays() const;
        const std::vector<ui::ButtonWidget> &buttons() const;
        const std::vector<ui::EncoderWidget> &encoders() const;
        const std::vector<ui::LedWidget> &leds() const;
        const std::vector<ui::ToggleWidget> &toggles() const;

      private:
        std::vector<ui::DisplayWidget> mDisplays;
        std::vector<ui::ButtonWidget> mButtons;
        std::vector<ui::EncoderWidget> mEncoders;
        std::vector<ui::LedWidget> mLeds;
        std::vector<ui::ToggleWidget> mToggles;
      };
    }
  }
}