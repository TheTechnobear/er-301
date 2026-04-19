#include <hal/log.h>
#include <hal/encoder.h>
#include <ssp/ssp.h>

extern "C"
{

  static int lastValue = 0;

  void Encoder_init(void)
  {
    lastValue = ssp::getEncoderValue();
  }

  int Encoder_getValue(void)
  {
    return ssp::getEncoderValue();
  }

  int Encoder_getChange(void)
  {
    int value = ssp::getEncoderValue();
    int change = value - lastValue;
    lastValue = value;
    return change;
  }
}