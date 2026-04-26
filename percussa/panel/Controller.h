#pragma once

#include <percussa/panel/Action.h>

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
    };
  } // namespace panel
} // namespace percussa