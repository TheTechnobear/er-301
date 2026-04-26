#pragma once

#include <percussa/panel/Panel.h>

#include <memory>

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

        int width() const;
        int height() const;
        void render(Olivec_Canvas canvas) const;
        std::unique_ptr<Controller> createController();

      private:
        std::vector<std::shared_ptr<ui::DisplayWidget>> mDisplays;
        std::vector<ui::ButtonWidget> mButtons;
        std::vector<ui::EncoderWidget> mEncoders;
        std::vector<ui::LedWidget> mLeds;
        std::vector<ui::ToggleWidget> mToggles;
      };
    } // namespace ssp
  } // namespace panel
} // namespace percussa