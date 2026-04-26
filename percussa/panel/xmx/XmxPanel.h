#pragma once

#include <percussa/panel/Panel.h>

#include <percussa/ui/BiButtonWidget.h>
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

        int width() const;
        int height() const;
        void render(Olivec_Canvas canvas) const;
        std::unique_ptr<Controller> createController();
        void setFnShift(bool s);

      private:
        std::vector<std::shared_ptr<ui::DisplayWidget>> mDisplays;
        std::vector<ui::LedWidget> mLeds;
        std::vector<ui::ToggleWidget> mToggles;
        std::vector<ui::BiButtonWidget> mBiButtons;
        bool mFnShift = false;
      };
    } // namespace xmx
  } // namespace panel
} // namespace percussa