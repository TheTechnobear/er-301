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
        std::string mStatusText;
      };
    }
  }
}