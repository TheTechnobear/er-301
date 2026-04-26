#pragma once

#include <percussa/platform/Platform.h>

namespace percussa
{
  namespace platform
  {
    class PluginPlatform : public Platform
    {
    public:
      int run(runtime::Runtime &runtime) const;
    };
  } // namespace platform
} // namespace percussa