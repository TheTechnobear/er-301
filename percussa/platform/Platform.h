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

      virtual const char *name() const = 0;
      virtual void describe(std::ostream &out) const = 0;
      virtual int run(runtime::Runtime &runtime) const = 0;
    };
  }
}