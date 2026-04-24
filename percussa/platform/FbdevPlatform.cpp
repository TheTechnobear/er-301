#include <percussa/platform/FbdevPlatform.h>

#include <percussa/hw/Framebuffer.h>
#include <percussa/input/LinuxInput.h>
#include <percussa/panel/Panel.h>
#include <percussa/runtime/Runtime.h>
#include <percussa/ui/PanelRenderer.h>

#include <hal/display.h>
#include <hal/events.h>

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
      ui::RenderedPanel rendered = renderer.render(runtime.panel());

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

      DisplayBuffer *lastPresentedBuffer = Display_getLastPutBuffer();
      int delayMs = 0;
      const double targetMs = 1000.0 / 70.0;

      while (true)
      {
        if (delayMs > 0)
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
        }

        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

        input.poll(
          [&](const input::Action &action)
          {
            runtime.handleAction(action);
          });

        DisplayBuffer *currentBuffer = Display_getLastPutBuffer();
        if (currentBuffer != lastPresentedBuffer)
        {
          rendered = renderer.render(runtime.panel());
          framebuffer.present(rendered.pixels.data());
          lastPresentedBuffer = currentBuffer;
        }

        Events_push(EVENT_DISPLAY_READY);

        double elapsedMs = std::chrono::duration<double, std::milli>(
          std::chrono::steady_clock::now() - start).count();
        delayMs = (int)(targetMs - elapsedMs);
        if (delayMs < 0)
        {
          delayMs = 0;
        }
      }
    }
  }
}