#include <percussa/panel/xmx/XmxController.h>

namespace percussa
{
  namespace panel
  {
    namespace xmx
    {
      XmxController::XmxController(const Panel &panel) :
        mStatusText("ready")
      {
        (void)panel;
      }

      void XmxController::handleAction(const input::Action &action)
      {
        mStatusText = input::describeAction(action);
      }

      const std::string &XmxController::statusText() const
      {
        return mStatusText;
      }
    }
  }
}