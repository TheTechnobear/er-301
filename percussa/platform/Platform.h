#pragma once

#include <iosfwd>

#include <percussa/runtime/Runtime.h>

namespace percussa
{
  namespace runtime
  {
    class Runtime;
  }
  namespace platform
  {
    class Platform
    {
    public:
      virtual ~Platform()
      {
      }

      virtual int run(runtime::Runtime &runtime) const = 0;
    };
  } // namespace platform
} // namespace percussa