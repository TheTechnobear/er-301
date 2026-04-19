#include "SSPCore.h"
#include <ssp/ssp.h>
#include <ssp/SSPCore.h>

namespace ssp
{
  static SSPCore sspcore;

  DisplayBuffer *getDisplayBuffer()
  {
    return sspcore.getDisplayBuffer();
  }

  void putDisplayBuffer(DisplayBuffer *buffer)
  {
    sspcore.putDisplayBuffer(buffer);
  }

  int getEncoderValue()
  {
    return sspcore.getEncoderValue();
  }

  bool isRearCardPresent()
  {
    return sspcore.isRearCardPresent();
  }

  bool isFrontCardPresent()
  {
    return sspcore.isFrontCardPresent();
  }
}

int main(int argc, char **argv)
{
  return ssp::sspcore.run(argc, argv);
}
