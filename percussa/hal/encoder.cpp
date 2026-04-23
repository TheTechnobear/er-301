#include <percussa/hal/EncoderState.h>

#include <hal/encoder.h>

namespace
{
  static int gEncoderValue = 0;
  static int gLastEncoderValue = 0;
}

extern "C"
{
  void Encoder_init(void)
  {
    gLastEncoderValue = gEncoderValue;
  }

  int Encoder_getValue(void)
  {
    return gEncoderValue;
  }

  int Encoder_getChange(void)
  {
    int value = gEncoderValue;
    int change = value - gLastEncoderValue;
    gLastEncoderValue = value;
    return change;
  }

  void PercussaEncoder_setValue(int value)
  {
    gEncoderValue = value;
  }

  void PercussaEncoder_adjustValue(int delta)
  {
    gEncoderValue += delta;
  }
}