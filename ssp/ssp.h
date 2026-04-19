#pragma once

#include <hal/display.h>

namespace ssp
{
  // Thread-safe.
  void putDisplayBuffer(DisplayBuffer *buffer);
  DisplayBuffer *getDisplayBuffer();
  int getEncoderValue();
  bool isRearCardPresent();
  bool isFrontCardPresent();
}