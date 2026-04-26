#pragma once

#include <percussa/panel/Controller.h>
#include "XmxPanel.h"

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
        explicit XmxController(XmxPanel &panel);

        void handleAction(const input::Action &action);

      private:
        XmxPanel &mPanel;
        int mActiveOutput = 1;

        uint32_t mapButtonToGpio(input::HardwareButtonId button) const;
        void clearSelectButtons() const;
        void setActiveOutput(int output);
        void switchToggle(uint32_t idA, uint32_t idB, int delta) const;

        bool fnState = false;
      };
    } // namespace xmx
  } // namespace panel
} // namespace percussa