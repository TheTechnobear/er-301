#pragma once

#include <percussa/platform/Platform.h>

namespace percussa
{
  namespace platform
  {
    class FbdevPlatform : public Platform
    {
    public:
      int run(runtime::Runtime &runtime) const;
    };
  }
}