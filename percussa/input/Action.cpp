#include <percussa/input/Action.h>

#include <sstream>

namespace percussa
{
  namespace input
  {
    namespace
    {
      const char *buttonName(HardwareButtonId button)
      {
        switch (button)
        {
        case HardwareButtonId::Button1:
          return "Button1";
        case HardwareButtonId::Button2:
          return "Button2";
        case HardwareButtonId::Button3:
          return "Button3";
        case HardwareButtonId::Button4:
          return "Button4";
        case HardwareButtonId::Button5:
          return "Button5";
        case HardwareButtonId::Button6:
          return "Button6";
        case HardwareButtonId::Button7:
          return "Button7";
        case HardwareButtonId::Button8:
          return "Button8";
        case HardwareButtonId::Up:
          return "Up";
        case HardwareButtonId::Down:
          return "Down";
        case HardwareButtonId::Left:
          return "Left";
        case HardwareButtonId::Right:
          return "Right";
        case HardwareButtonId::ShiftL:
          return "LShift";
        case HardwareButtonId::ShiftR:
          return "RShift";
        case HardwareButtonId::P1:
          return "P1";
        case HardwareButtonId::P2:
          return "P2";
        case HardwareButtonId::P3:
          return "P3";
        case HardwareButtonId::P4:
          return "P4";
        case HardwareButtonId::Invalid:
        default:
          return "Invalid";
        }
      }

      const char *encoderName(HardwareEncoderId encoder)
      {
        switch (encoder)
        {
        case HardwareEncoderId::Encoder1:
          return "Encoder1";
        case HardwareEncoderId::Encoder2:
          return "Encoder2";
        case HardwareEncoderId::Encoder3:
          return "Encoder3";
        case HardwareEncoderId::Encoder4:
          return "Encoder4";
        case HardwareEncoderId::Invalid:
        default:
          return "Invalid";
        }
      }
    }

    std::string describeAction(const Action &action)
    {
      std::ostringstream out;
      switch (action.type)
      {
      case ActionType::Button:
        out << buttonName(action.hardwareButton) << (action.pressed ? " down" : " up");
        break;
      case ActionType::EncoderTurn:
        out << encoderName(action.hardwareEncoder) << " delta " << action.delta;
        break;
      case ActionType::EncoderPress:
        out << encoderName(action.hardwareEncoder) << (action.pressed ? " press" : " release");
        break;
      }
      return out.str();
    }
  }
}