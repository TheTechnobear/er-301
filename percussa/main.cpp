#include <percussa/panel/Panel.h>
#include <percussa/runtime/Runtime.h>

#include <percussa/app/Bootstrap.h>

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

  percussa::app::Bootstrap bootstrap(argc, argv);
  if (!bootstrap.initialize())
  {
    return 1;
  }

  std::unique_ptr<percussa::panel::Panel> panel = percussa::panel::createPanel();

  std::unique_ptr<percussa::platform::Platform> platform;
#if defined(PERCUSSA_PLATFORM_HOST_SDL)
  platform.reset(new percussa::platform::HostSdlPlatform(once));
#elif defined(PERCUSSA_PLATFORM_FBDEV)
  platform.reset(new percussa::platform::FbdevPlatform());
#elif defined(PERCUSSA_PLATFORM_PLUGIN)
  platform.reset(new percussa::platform::PluginPlatform());
#endif

  percussa::runtime::Runtime runtime(std::move(panel), std::move(platform));
  int result = runtime.run();
  bootstrap.finalize();
  return result;
}