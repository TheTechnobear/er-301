#include <percussa/platform/fbdev/ButtonMap.h>

namespace percussa
{
  namespace input
  {
    const int kEncoderMultiplier = -1;

    HardwareButtonId mapButtonCode(int code)
    {
      switch (code)
      {
      case 59:
        return HardwareButtonId::Button1;
      case 60:
        return HardwareButtonId::Button2;
      case 61:
        return HardwareButtonId::Button3;
      case 62:
        return HardwareButtonId::Button4;
      case 64:
        return HardwareButtonId::Button5;
      case 65:
        return HardwareButtonId::Button6;
      case 66:
        return HardwareButtonId::Button7;
      case 67:
        return HardwareButtonId::Button8;
      case 68:
        return HardwareButtonId::Up;
      case 63:
        return HardwareButtonId::Down;
      default:
        return HardwareButtonId::Invalid;
      }
    }
  }
}
