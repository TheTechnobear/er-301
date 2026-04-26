#pragma once

#include <percussa/platform/Platform.h>

namespace percussa
{
  namespace platform
  {
    class HostSdlPlatform : public Platform
    {
    public:
      explicit HostSdlPlatform(bool once = false) : mOnce(once)
      {
      }

      int run(runtime::Runtime &runtime) const;

    private:
      bool mOnce;
    };
  } // namespace platform
} // namespace percussa