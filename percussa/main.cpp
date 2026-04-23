#include <percussa/panel/Family.h>
#include <percussa/runtime/Runtime.h>

#if defined(PERCUSSA_PANEL_SSP)
#include <percussa/runtime/ssp/SspBootstrap.h>
#endif

#if defined(PERCUSSA_PLATFORM_HOST_SDL)
#include <percussa/platform/HostSdlPlatform.h>
#elif defined(PERCUSSA_PLATFORM_FBDEV)
#include <percussa/platform/FbdevPlatform.h>
#elif defined(PERCUSSA_PLATFORM_PLUGIN)
#include <percussa/platform/PluginPlatform.h>
#else
#error "No percussa platform selected at build time."
#endif

#include <memory>

int main(int argc, char **argv)
{
  bool once = false;

  for (int i = 1; i < argc; ++i)
  {
    if (std::string(argv[i]) == "--once")
    {
      once = true;
    }
  }

#if defined(PERCUSSA_PANEL_SSP)
  percussa::runtime::ssp::SspBootstrap bootstrap(argc, argv);
  if (!bootstrap.initialize())
  {
    return 1;
  }
#endif

  percussa::panel::Family family = percussa::panel::createFamily();

  std::unique_ptr<percussa::platform::Platform> platform;
#if defined(PERCUSSA_PLATFORM_HOST_SDL)
  platform.reset(new percussa::platform::HostSdlPlatform());
#elif defined(PERCUSSA_PLATFORM_FBDEV)
  platform.reset(new percussa::platform::FbdevPlatform());
#elif defined(PERCUSSA_PLATFORM_PLUGIN)
  platform.reset(new percussa::platform::PluginPlatform());
#endif

  percussa::runtime::Runtime runtime(std::move(family.panel), std::move(family.controller), std::move(platform), once);
  int result = runtime.run();
#if defined(PERCUSSA_PANEL_SSP)
  bootstrap.finalize();
#endif
  return result;
}