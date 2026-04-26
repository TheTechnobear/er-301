#include <percussa/runtime/Runtime.h>

#include <percussa/panel/Action.h>
#include <percussa/panel/Panel.h>
#include <percussa/platform/Platform.h>

namespace percussa
{
  namespace runtime
  {
    Runtime::Runtime(std::unique_ptr<panel::Panel> panel, std::unique_ptr<platform::Platform> platform)
        : mPanel(std::move(panel)), mController(mPanel->createController()), mPlatform(std::move(platform))
    {
    }

    int Runtime::run()
    {
      return mPlatform->run(*this);
    }

    void Runtime::handleAction(const input::Action &action)
    {
      mController->handleAction(action);
    }

    const panel::Panel &Runtime::panel() const
    {
      return *mPanel;
    }
  } // namespace runtime
} // namespace percussa