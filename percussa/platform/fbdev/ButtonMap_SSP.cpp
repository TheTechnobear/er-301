#include <percussa/platform/fbdev/ButtonMap.h>

namespace percussa
{
  namespace input
  {
    const int kEncoderMultiplier = 1;

    HardwareButtonId mapButtonCode(int code)
    {
      switch (code)
      {
      case 88:
        return HardwareButtonId::Button1;
      case 87:
        return HardwareButtonId::Button2;
      case 68:
        return HardwareButtonId::Button3;
      case 67:
        return HardwareButtonId::Button4;
      case 64:
        return HardwareButtonId::Button5;
      case 63:
        return HardwareButtonId::Button6;
      case 62:
        return HardwareButtonId::Button7;
      case 61:
        return HardwareButtonId::Button8;
      case 65:
        return HardwareButtonId::Up;
      case 59:
        return HardwareButtonId::Down;
      case 66:
        return HardwareButtonId::ShiftL;
      case 187:
        return HardwareButtonId::ShiftR;
      case 60:
        return HardwareButtonId::Left;
      case 188:
        return HardwareButtonId::Right;
      case 183:
        return HardwareButtonId::P1;
      case 184:
        return HardwareButtonId::P2;
      case 185:
        return HardwareButtonId::P3;
      case 186:
        return HardwareButtonId::P4;
      default:
        return HardwareButtonId::Invalid;
      }
    }
  }
}
