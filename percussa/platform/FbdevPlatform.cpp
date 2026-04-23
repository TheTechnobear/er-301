#include <percussa/platform/FbdevPlatform.h>

#include <percussa/hw/Framebuffer.h>
#include <percussa/input/LinuxInput.h>
#include <percussa/panel/Panel.h>
#include <percussa/runtime/Runtime.h>
#include <percussa/ui/PanelRenderer.h>

#include <chrono>
#include <iostream>
#include <ostream>
#include <thread>

namespace percussa
{
  namespace platform
  {
    const char *FbdevPlatform::name() const
    {
      return "fbdev";
    }

    void FbdevPlatform::describe(std::ostream &out) const
    {
      out << "standalone linux entrypoint with direct framebuffer presentation and hardware-backed audio/input";
    }

    int FbdevPlatform::run(runtime::Runtime &runtime) const
    {
      ui::PanelRenderer renderer;
      ui::RenderedPanel rendered = renderer.render(runtime.panel(), runtime.presentationState());

      hw::Framebuffer framebuffer(rendered.width, rendered.height);
      if (!framebuffer.init())
      {
        return 1;
      }

      input::LinuxInput input;
      input.init();

      framebuffer.present(rendered.pixels.data());
      if (runtime.options().once)
      {
        return 0;
      }

      while (true)
      {
        input.poll(
          [&](const input::Action &action)
          {
            runtime.handleAction(action);
            rendered = renderer.render(runtime.panel(), runtime.presentationState());
            framebuffer.present(rendered.pixels.data());
          });

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
  }
}