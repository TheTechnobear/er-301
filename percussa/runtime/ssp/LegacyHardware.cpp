#include <percussa/runtime/ssp/LegacyHardware.h>

#include <percussa/hal/EncoderState.h>

#include <hal/gpio.h>

namespace percussa
{
  namespace runtime
  {
    namespace ssp
    {
      namespace
      {
        uint32_t mapButtonToGpio(input::HardwareButtonId button)
        {
          switch (button)
          {
          case input::HardwareButtonId::Button1:
            return BUTTON_MAIN1;
          case input::HardwareButtonId::Button2:
            return BUTTON_MAIN2;
          case input::HardwareButtonId::Button3:
            return BUTTON_MAIN3;
          case input::HardwareButtonId::Button4:
            return BUTTON_DIAL3;
          case input::HardwareButtonId::Button5:
            return BUTTON_MAIN4;
          case input::HardwareButtonId::Button6:
            return BUTTON_MAIN5;
          case input::HardwareButtonId::Button7:
            return BUTTON_MAIN6;
          case input::HardwareButtonId::Button8:
            return BUTTON_DIAL2;
          case input::HardwareButtonId::ShiftL:
            return BUTTON_ENTER;
          case input::HardwareButtonId::Up:
            return BUTTON_UP;
          case input::HardwareButtonId::ShiftR:
            return BUTTON_SHIFT;
          case input::HardwareButtonId::Left:
            return BUTTON_SUB1;
          case input::HardwareButtonId::Down:
            return BUTTON_SUB2;
          case input::HardwareButtonId::Right:
            return BUTTON_SUB3;
          case input::HardwareButtonId::P1:
            return BUTTON_SELECT1;
          case input::HardwareButtonId::P2:
            return BUTTON_SELECT2;
          case input::HardwareButtonId::P3:
            return BUTTON_SELECT3;
          case input::HardwareButtonId::P4:
            return BUTTON_SELECT4;
          case input::HardwareButtonId::Invalid:
          default:
            return NUM_GPIO_IDS;
          }
        }
      }

      void applyLegacyHardwareAction(const input::Action &action)
      {
        switch (action.type)
        {
        case input::ActionType::Button:
        {
          uint32_t gpioId = mapButtonToGpio(action.hardwareButton);
          if (gpioId < NUM_GPIO_IDS)
          {
            Gpio_write(gpioId, !action.pressed);
          }
          break;
        }
        case input::ActionType::EncoderTurn:
          if (action.hardwareEncoder == input::HardwareEncoderId::Encoder1)
          {
            PercussaEncoder_adjustValue(action.delta * 5);
          }
          break;
        case input::ActionType::EncoderPress:
          if (action.hardwareEncoder == input::HardwareEncoderId::Encoder1)
          {
            Gpio_write(BUTTON_DIAL1, !action.pressed);
          }
          break;
        default:
          break;
        }
      }
    }
  }
}