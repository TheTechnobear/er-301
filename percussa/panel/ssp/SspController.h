#pragma once

#include <percussa/panel/Controller.h>

#include <stdint.h>

#include "SspPanel.h"

namespace percussa
{
  namespace panel
  {
    namespace ssp
    {
      class SspController : public Controller
      {
      public:
        explicit SspController(SspPanel &panel);

        void handleAction(const input::Action &action);
      private:
        SspPanel &mPanel;
        int mActiveOutput = 1;

        uint32_t mapButtonToGpio(input::HardwareButtonId button) const;
        void clearSelectButtons() const;
        void setActiveOutput(int output);
        void switchToggle(uint32_t idA, uint32_t idB, int delta) const;
      };
    } // namespace ssp
  } // namespace panel
} // namespace percussa