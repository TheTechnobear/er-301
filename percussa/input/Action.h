#pragma once

#include <string>

namespace percussa
{
  namespace input
  {
    enum class HardwareButtonId
    {
      Button1,
      Button2,
      Button3,
      Button4,
      Button5,
      Button6,
      Button7,
      Button8,
      Up,
      Down,
      Left,
      Right,
      ShiftL,
      ShiftR,
      P1,
      P2,
      P3,
      P4,
      Invalid
    };

    enum class HardwareEncoderId
    {
      Encoder1 = 0,
      Encoder2 = 1,
      Encoder3 = 2,
      Encoder4 = 3,
      Invalid = -1
    };

    enum class ActionType
    {
      Button,
      EncoderTurn,
      EncoderPress
    };

    struct Action
    {
      ActionType type = ActionType::Button;
      HardwareButtonId hardwareButton = HardwareButtonId::Invalid;
      HardwareEncoderId hardwareEncoder = HardwareEncoderId::Invalid;
      bool pressed = false;
      int delta = 0;
    };

    std::string describeAction(const Action &action);
  }
}