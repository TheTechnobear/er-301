#pragma once

#include <percussa/panel/Action.h>
#include <percussa/panel/Controller.h>

#include <memory>
#include <string>

namespace percussa
{
  namespace panel
  {
    class Panel;
  }

  namespace platform
  {
    class Platform;
  }

  namespace runtime
  {
    class Runtime
    {
    public:
      Runtime(std::unique_ptr<panel::Panel> panel, std::unique_ptr<platform::Platform> platform);

      int run();
      void handleAction(const input::Action &action);
      const panel::Panel &panel() const;

    private:
      std::unique_ptr<panel::Panel> mPanel;
      std::unique_ptr<panel::Controller> mController;
      std::unique_ptr<platform::Platform> mPlatform;
    };
  } // namespace runtime
} // namespace percussa