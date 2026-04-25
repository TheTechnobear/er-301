#include <percussa/panel/ssp/SspController.h>

#include <hal/encoder.h>
#include <hal/gpio.h>

#include <percussa/hw/PercussaState.h>
#include <percussa/panel/Panel.h>
#include <percussa/app/EncoderProxy.h>

#include <string>

namespace percussa
{
  namespace panel
  {
    namespace ssp
    {
      namespace
      {
        int clampOutput(int output)
        {
          if (output < 1)
          {
            return 1;
          }
          if (output > 4)
          {
            return 4;
          }
          return output;
        }

        uint32_t activeSelectGpio(int activeOutput)
        {
          switch (activeOutput)
          {
          case 1:
            return BUTTON_SELECT1;
          case 2:
            return BUTTON_SELECT2;
          case 3:
            return BUTTON_SELECT3;
          default:
            return BUTTON_SELECT4;
          }
        }
      } // namespace

      SspController::SspController(SspPanel &panel) : mPanel(panel), mStatusText("ready")
      {
        clearSelectButtons();
        setActiveOutput(1);
      }

      void SspController::handleAction(const input::Action &action)
      {
        (void)mPanel;
        mStatusText = input::describeAction(action);

        if (action.type == input::ActionType::Button)
        {
          uint32_t gpioId = mapButtonToGpio(action.hardwareButton);
          if (gpioId < NUM_GPIO_IDS)
          {
            percussa_state_write(gpioId, !action.pressed);
          }
        }

        if (action.type == input::ActionType::EncoderTurn)
        {
          switch (action.hardwareEncoder)
          {
          case input::HardwareEncoderId::Encoder1:
            PercussaEncoder_adjustValue(action.delta * 5);
            break;
          case input::HardwareEncoderId::Encoder2:
            switchToggle(TOGGLE_STORAGE_A, TOGGLE_STORAGE_B, action.delta);
            break;
          case input::HardwareEncoderId::Encoder3:
            switchToggle(TOGGLE_MODE_A, TOGGLE_MODE_B, action.delta);
            break;
          case input::HardwareEncoderId::Encoder4:
            if (action.delta > 0)
            {
              setActiveOutput(activeOutput() + 1);
            }
            else if (action.delta < 0)
            {
              setActiveOutput(activeOutput() - 1);
            }
            break;
          default:
            break;
          }
        }

        if (action.type == input::ActionType::EncoderPress)
        {
          switch (action.hardwareEncoder)
          {
          case input::HardwareEncoderId::Encoder1:
            percussa_state_write(BUTTON_DIAL1, !action.pressed);
            break;
          case input::HardwareEncoderId::Encoder2: break;
          case input::HardwareEncoderId::Encoder3: break;
          case input::HardwareEncoderId::Encoder4:
          {
            if (mActiveOutput > 3)
              return;
            uint32_t first = activeSelectGpio(activeOutput());
            uint32_t second = activeSelectGpio((activeOutput() + 1));
            percussa_state_write(first, !action.pressed);
            percussa_state_write(second, !action.pressed);
            break;
          }
          default:
            break;
          }
        }
      }

      const std::string &SspController::statusText() const
      {
        return mStatusText;
      }

      uint32_t SspController::mapButtonToGpio(input::HardwareButtonId button) const
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

      int SspController::activeOutput() const
      {
        return mActiveOutput;
      }

      void SspController::clearSelectButtons() const
      {
        percussa_state_write(BUTTON_SELECT1, true);
        percussa_state_write(BUTTON_SELECT2, true);
        percussa_state_write(BUTTON_SELECT3, true);
        percussa_state_write(BUTTON_SELECT4, true);
      }

      void SspController::setActiveOutput(int output)
      {
        int clamped = clampOutput(output);
        mActiveOutput = clamped;
        clearSelectButtons();
        percussa_state_write(activeSelectGpio(clamped), false);
      }

      void SspController::switchToggle(uint32_t idA, uint32_t idB, int delta) const
      {
        if (delta > 0)
        {
          if (percussa_state_read(idA))
          {
            percussa_state_write(idA, false);
          }
          else if (!percussa_state_read(idB))
          {
            percussa_state_write(idB, true);
          }
        }
        else if (delta < 0)
        {
          if (percussa_state_read(idB))
          {
            percussa_state_write(idB, false);
          }
          else if (!percussa_state_read(idA))
          {
            percussa_state_write(idA, true);
          }
        }
      }

      int SspController::toggleState(uint32_t idA, uint32_t idB) const
      {
        if (percussa_state_read(idA))
        {
          return 0;
        }
        if (percussa_state_read(idB))
        {
          return 2;
        }
        return 1;
      }
    } // namespace ssp
  } // namespace panel
} // namespace percussa