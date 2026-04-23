#include <percussa/runtime/Runtime.h>

#include <percussa/input/Action.h>
#include <percussa/panel/Panel.h>
#include <percussa/platform/Platform.h>

#if defined(PERCUSSA_PANEL_SSP)
#include <percussa/runtime/ssp/LegacyHardware.h>
#endif

namespace percussa
{
  namespace runtime
  {
    Runtime::Runtime(std::unique_ptr<panel::Panel> panel,
                     std::unique_ptr<panel::Controller> controller,
                     std::unique_ptr<platform::Platform> platform,
                     bool once) :
      mPanel(std::move(panel)),
      mController(std::move(controller)),
      mPlatform(std::move(platform))
    {
      mOptions.once = once;
    }

    int Runtime::run()
    {
      return mPlatform->run(*this);
    }

    void Runtime::handleAction(const input::Action &action)
    {
#if defined(PERCUSSA_PANEL_SSP)
      ssp::applyLegacyHardwareAction(action);
#endif
      mController->handleAction(action);
    }

    const panel::Panel &Runtime::panel() const
    {
      return *mPanel;
    }

    const RunOptions &Runtime::options() const
    {
      return mOptions;
    }

    const ui::PresentationState &Runtime::presentationState() const
    {
      return mController->presentationState();
    }

    const std::string &Runtime::statusText() const
    {
      return mController->statusText();
    }
  }
}