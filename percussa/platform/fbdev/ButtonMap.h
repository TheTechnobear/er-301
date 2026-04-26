#pragma once

#include <percussa/panel/Action.h>

namespace percussa
{
  namespace input
  {
    HardwareButtonId mapButtonCode(int code);
    extern const int kEncoderMultiplier;
  }
}
