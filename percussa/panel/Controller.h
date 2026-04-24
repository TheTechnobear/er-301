#pragma once

#include <percussa/input/Action.h>

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
      virtual const std::string &statusText() const = 0;
    };
  }
}