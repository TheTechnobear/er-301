#include <percussa/app/EncoderProxy.h>

#include <hal/events.h>

#include <atomic>

namespace
{
  static std::atomic<int> gEncoderValue{0};
  static std::atomic<int> gLastEncoderValue{0};
}

namespace percussa
{
  namespace app
  {
    void initEncoderProxy()
    {
      gLastEncoderValue.store(gEncoderValue.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    int encoderProxyValue()
    {
      return gEncoderValue.load(std::memory_order_relaxed);
    }

    int encoderProxyChange()
    {
      int value = gEncoderValue.load(std::memory_order_relaxed);
      int previous = gLastEncoderValue.exchange(value, std::memory_order_relaxed);
      return value - previous;
    }
  }
}

extern "C"
{
  void PercussaEncoder_setValue(int value)
  {
    int previous = gEncoderValue.exchange(value, std::memory_order_relaxed);
    if (previous != value)
    {
      Events_push(EVENT_KNOB);
    }
  }

  void PercussaEncoder_adjustValue(int delta)
  {
    if (delta == 0)
    {
      return;
    }

    gEncoderValue.fetch_add(delta, std::memory_order_relaxed);
    Events_push(EVENT_KNOB);
  }
}