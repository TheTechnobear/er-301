#include <percussa/platform/FbdevPlatform.h>

#include <percussa/hw/Framebuffer.h>
#include <percussa/input/LinuxInput.h>
#include <percussa/panel/Panel.h>
#include <percussa/platform/TargetDimensions.h>
#include <percussa/runtime/Runtime.h>
#include <percussa/ui/PanelRenderer.h>

#include <hal/display.h>
#include <hal/events.h>
#include <hal/log.h>

#include <chrono>
#include <cstring>
#include <thread>
#include <vector>

namespace percussa
{
  namespace platform
  {
    namespace
    {
      bool panelSizeMatchesTarget(const panel::Panel &panel)
      {
        Size expected = target::panelSize();
        return panel.width() == expected.width && panel.height() == expected.height;
      }

      void blitPanelToFramebuffer(const ui::RenderedPanel &rendered, Size framebufferSize, std::vector<uint32_t> &pixels)
      {
        std::memset(&pixels[0], 0, pixels.size() * sizeof(uint32_t));

        const int copyRows = rendered.height();
        const int rowPixels = rendered.width();
        for (int y = 0; y < copyRows; ++y)
        {
          std::memcpy(&pixels[(size_t)y * (size_t)framebufferSize.width],
                      rendered.data() + (size_t)y * (size_t)rowPixels,
                      (size_t)rowPixels * sizeof(uint32_t));
        }
      }
    } // namespace

    int FbdevPlatform::run(runtime::Runtime &runtime) const
    {
      const panel::Panel &panel = runtime.panel();
      if (!panelSizeMatchesTarget(panel))
      {
        Size expected = target::panelSize();
        logError("FbdevPlatform: panel size %dx%d does not match expected %dx%d.",
                 panel.width(),
                 panel.height(),
                 expected.width,
                 expected.height);
        return 1;
      }

      ui::PanelRenderer renderer;
      ui::RenderedPanel rendered = renderer.render(panel);
      Size framebufferSize = target::framebufferSize();
      std::vector<uint32_t> framebufferPixels((size_t)framebufferSize.width * (size_t)framebufferSize.height);

      hw::Framebuffer framebuffer(framebufferSize.width, framebufferSize.height);
      if (!framebuffer.init())
      {
        logError("FbdevPlatform: framebuffer init failed at %dx%d.",
                 framebufferSize.width,
                 framebufferSize.height);
        return 1;
      }

      input::LinuxInput input;
      input.init();

      blitPanelToFramebuffer(rendered, framebufferSize, framebufferPixels);
      framebuffer.present(&framebufferPixels[0]);

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

        input.poll([&](const input::Action &action) { runtime.handleAction(action); });

        DisplayBuffer *currentBuffer = Display_getLastPutBuffer();
        if (currentBuffer != lastPresentedBuffer)
        {
          rendered = renderer.render(panel);
          blitPanelToFramebuffer(rendered, framebufferSize, framebufferPixels);
          framebuffer.present(&framebufferPixels[0]);
          lastPresentedBuffer = currentBuffer;
        }

        Events_push(EVENT_DISPLAY_READY);

        double elapsedMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
        delayMs = (int)(targetMs - elapsedMs);
        if (delayMs < 0)
        {
          delayMs = 0;
        }
      }
    }
  } // namespace platform
} // namespace percussa