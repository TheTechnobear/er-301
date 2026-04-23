#pragma once

#include <percussa/panel/Controller.h>

namespace percussa
{
  namespace panel
  {
    class Panel;

    namespace xmx
    {
      class XmxController : public Controller
      {
      public:
        explicit XmxController(const Panel &panel);

        void handleAction(const input::Action &action);
        const ui::PresentationState &presentationState() const;
        const std::string &statusText() const;

      private:
        const Panel &mPanel;
        ui::PresentationState mPresentationState;
        std::string mStatusText;
        int mSelectedTab = 0;
        int mSelectedSoftButton = 0;
        int mValue = 0;
        int mNavState = 1;

        void refreshPresentationState();
      };
    }
  }
}