#pragma once

#include <percussa/input/Action.h>
#include <percussa/ui/PresentationState.h>

#include <string>

namespace percussa
{
  namespace panel
  {
    class Controller
    {
    public:
      virtual ~Controller()
      {
      }

      virtual void handleAction(const input::Action &action) = 0;
      virtual const ui::PresentationState &presentationState() const = 0;
      virtual const std::string &statusText() const = 0;
    };
  }
}