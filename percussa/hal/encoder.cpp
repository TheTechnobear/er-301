#include <percussa/app/EncoderProxy.h>

#include <hal/encoder.h>

extern "C"
{
  void Encoder_init(void)
  {
    percussa::app::initEncoderProxy();
  }

  int Encoder_getValue(void)
  {
    return percussa::app::encoderProxyValue();
  }

  int Encoder_getChange(void)
  {
    return percussa::app::encoderProxyChange();
  }
}