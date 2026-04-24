#pragma once

#include <percussa/input/Action.h>
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
    struct RunOptions
    {
      bool once = false;
    };

    class Runtime
    {
    public:
      Runtime(std::unique_ptr<panel::Panel> panel,
              std::unique_ptr<panel::Controller> controller,
              std::unique_ptr<platform::Platform> platform,
              bool once);

      int run();
      void handleAction(const input::Action &action);
      const panel::Panel &panel() const;
      const RunOptions &options() const;
      const std::string &statusText() const;

    private:
      std::unique_ptr<panel::Panel> mPanel;
      std::unique_ptr<panel::Controller> mController;
      std::unique_ptr<platform::Platform> mPlatform;
      RunOptions mOptions;
    };
  }
}