#pragma once

#include <percussa/panel/Controller.h>
#include <percussa/panel/ssp/SspFrontPanelState.h>

namespace percussa
{
  namespace panel
  {
    class Panel;

    namespace ssp
    {
      class SspController : public Controller
      {
      public:
        explicit SspController(const Panel &panel);

        void handleAction(const input::Action &action);
        const ui::PresentationState &presentationState() const;
        const std::string &statusText() const;

      private:
        const Panel &mPanel;
        SspFrontPanelState mFrontPanelState;
        ui::PresentationState mPresentationState;
        std::string mStatusText;

        void refreshPresentationState();
      };
    }
  }
}