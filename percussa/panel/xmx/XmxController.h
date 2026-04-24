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
        const std::string &statusText() const;

      private:
        const Panel &mPanel;
        std::string mStatusText;
        int mActiveOutput = 1;
        bool mLinkGestureActive = false;

        uint32_t mapButtonToGpio(input::HardwareButtonId button) const;
        int activeOutput() const;
        void clearSelectButtons() const;
        void setActiveOutput(int output);
        void switchToggle(uint32_t idA, uint32_t idB, int delta) const;
        int toggleState(uint32_t idA, uint32_t idB) const;
        void syncIndicators() const;
      };
    }
  }
}