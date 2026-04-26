#include <percussa/platform/PluginPlatform.h>

#include <percussa/panel/Panel.h>
#include <percussa/platform/TargetDimensions.h>
#include <percussa/runtime/Runtime.h>

#include <iostream>
#include <ostream>

namespace percussa
{
  namespace platform
  {
    int PluginPlatform::run(runtime::Runtime &runtime) const
    {
      const panel::Panel &panel = runtime.panel();
      Size expected = target::panelSize();
      std::cout << "Percussa scaffold" << std::endl;
      std::cout << "  panel size: " << panel.width() << "x" << panel.height() << std::endl;
      std::cout << "  platform role: plugin" << std::endl;
      if (panel.width() != expected.width || panel.height() != expected.height)
      {
        std::cout << "  panel size mismatch: expected " << expected.width << "x" << expected.height << std::endl;
        return 1;
      }
      return 0;
    }
  }
}