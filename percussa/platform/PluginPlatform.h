#pragma once

#include <percussa/platform/Platform.h>

namespace percussa
{
  namespace platform
  {
    class PluginPlatform : public Platform
    {
    public:
      const char *name() const;
      void describe(std::ostream &out) const;
      int run(runtime::Runtime &runtime) const;
    };
  }
}