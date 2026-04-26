#pragma once

#ifdef __cplusplus
namespace percussa
{
  namespace app
  {
    void initEncoderProxy();
    int encoderProxyValue();
    int encoderProxyChange();
  }
}

extern "C"
{
#endif

  void PercussaEncoder_adjustValue(int delta);

#ifdef __cplusplus
}
#endif