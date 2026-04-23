#include <percussa/platform/PluginPlatform.h>

#include <percussa/panel/Panel.h>
#include <percussa/runtime/Runtime.h>

#include <iostream>
#include <ostream>

namespace percussa
{
  namespace platform
  {
    const char *PluginPlatform::name() const
    {
      return "plugin";
    }

    void PluginPlatform::describe(std::ostream &out) const
    {
      out << "host-provided callback entrypoints for display, audio, and control input";
    }

    int PluginPlatform::run(runtime::Runtime &runtime) const
    {
      const panel::Panel &panel = runtime.panel();
      std::cout << "Percussa scaffold" << std::endl;
      std::cout << "  panel: " << panel.name() << " (" << panel.width() << "x" << panel.height() << ")" << std::endl;
      std::cout << "  platform: " << name() << std::endl;
      std::cout << "  platform role: ";
      describe(std::cout);
      std::cout << std::endl;
      std::cout << "  status: " << runtime.statusText() << std::endl;
      return 0;
    }
  }
}