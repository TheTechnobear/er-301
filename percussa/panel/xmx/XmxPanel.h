#pragma once

#include <percussa/panel/Panel.h>

#include <memory>

namespace percussa
{
  namespace panel
  {
    namespace xmx
    {
      class XmxPanel : public Panel
      {
      public:
        XmxPanel();

        const char *name() const;
        int width() const;
        int height() const;
        void render(Olivec_Canvas canvas) const;
        const std::vector<std::shared_ptr<ui::DisplayWidget>> &displays() const;
        const std::vector<ui::ButtonWidget> &buttons() const;
        const std::vector<ui::EncoderWidget> &encoders() const;
        const std::vector<ui::LedWidget> &leds() const;
        const std::vector<ui::ToggleWidget> &toggles() const;

      private:
        std::vector<std::shared_ptr<ui::DisplayWidget>> mDisplays;
        std::vector<ui::ButtonWidget> mButtons;
        std::vector<ui::EncoderWidget> mEncoders;
        std::vector<ui::LedWidget> mLeds;
        std::vector<ui::ToggleWidget> mToggles;
      };
    }
  }
}