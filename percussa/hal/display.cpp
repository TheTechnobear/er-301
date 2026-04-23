#include <hal/display.h>

namespace
{
  struct DisplayLocals
  {
    DisplayBuffer buffers[2];
    DisplayBuffer *renderBuffer = 0;
    DisplayBuffer *lastPutBuffer = 0;
    bool started = false;
  } local;

  DisplayBuffer *otherBuffer(DisplayBuffer *buffer)
  {
    return buffer == &local.buffers[0] ? &local.buffers[1] : &local.buffers[0];
  }
}

extern "C"
{

  void Display_init(void)
  {
    local = DisplayLocals{};
    local.renderBuffer = &local.buffers[0];
  }

  void Display_deinit()
  {
    Display_stop();
  }

  void Display_start(void)
  {
    local.started = true;
  }

  void Display_stop(void)
  {
    local.started = false;
  }

  DisplayBuffer *Display_getBuffer()
  {
    if (local.started)
    {
      return local.renderBuffer;
    }
    return 0;
  }

  void Display_putBuffer(DisplayBuffer *buffer)
  {
    local.lastPutBuffer = buffer;
    local.renderBuffer = otherBuffer(buffer);
  }

  DisplayBuffer *Display_getLastPutBuffer()
  {
    return local.lastPutBuffer;
  }

}