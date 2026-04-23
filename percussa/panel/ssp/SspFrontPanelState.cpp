#include <percussa/panel/ssp/SspFrontPanelState.h>

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
            return 4;
          }
          if (output > 4)
          {
            return 1;
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
      }

      SspFrontPanelState::SspFrontPanelState()
      {
        Gpio_init();
        setActiveOutput(1);
        refreshIndicators();
      }

      void SspFrontPanelState::handleButton(input::HardwareButtonId button, bool pressed)
      {
        uint32_t gpioId = mapButtonToGpio(button);
        if (gpioId < NUM_GPIO_IDS)
        {
          Gpio_write(gpioId, !pressed);
        }

        switch (button)
        {
        case input::HardwareButtonId::P1:
          if (pressed)
          {
            setActiveOutput(1);
          }
          break;
        case input::HardwareButtonId::P2:
          if (pressed)
          {
            setActiveOutput(2);
          }
          break;
        case input::HardwareButtonId::P3:
          if (pressed)
          {
            setActiveOutput(3);
          }
          break;
        case input::HardwareButtonId::P4:
          if (pressed)
          {
            setActiveOutput(4);
          }
          break;
        default:
          break;
        }

        refreshIndicators();
      }

      void SspFrontPanelState::handleEncoderTurn(input::HardwareEncoderId encoder, int delta)
      {
        switch (encoder)
        {
        case input::HardwareEncoderId::Encoder1:
          mEncoderValue += delta * 5;
          break;
        case input::HardwareEncoderId::Encoder2:
          if (delta > 0)
          {
            setActiveOutput(mActiveOutput + 1);
          }
          else if (delta < 0)
          {
            setActiveOutput(mActiveOutput - 1);
          }
          break;
        case input::HardwareEncoderId::Encoder3:
          switchToggle(TOGGLE_STORAGE_A, TOGGLE_STORAGE_B, delta);
          break;
        case input::HardwareEncoderId::Encoder4:
          switchToggle(TOGGLE_MODE_A, TOGGLE_MODE_B, delta);
          break;
        default:
          break;
        }

        refreshIndicators();
      }

      void SspFrontPanelState::handleEncoderPress(input::HardwareEncoderId encoder, bool pressed)
      {
        switch (encoder)
        {
        case input::HardwareEncoderId::Encoder1:
          Gpio_write(BUTTON_DIAL1, !pressed);
          break;
        case input::HardwareEncoderId::Encoder2:
          mLinkGestureActive = pressed;
          if (pressed)
          {
            uint32_t first = activeSelectGpio(mActiveOutput);
            uint32_t second = activeSelectGpio((mActiveOutput % 4) + 1);
            Gpio_write(first, false);
            Gpio_write(second, false);
          }
          else
          {
            setActiveOutput(mActiveOutput);
          }
          break;
        case input::HardwareEncoderId::Encoder3:
        case input::HardwareEncoderId::Encoder4:
        default:
          break;
        }

        refreshIndicators();
      }

      const std::array<bool, SspFrontPanelState::kButtonCount> &SspFrontPanelState::buttonActive() const
      {
        return mButtonActive;
      }

      const std::array<bool, SspFrontPanelState::kLedCount> &SspFrontPanelState::ledActive() const
      {
        return mLedActive;
      }

      const std::array<int, SspFrontPanelState::kToggleCount> &SspFrontPanelState::togglePositions() const
      {
        return mTogglePositions;
      }

      int SspFrontPanelState::activeOutput() const
      {
        return mActiveOutput;
      }

      int SspFrontPanelState::encoderValue() const
      {
        return mEncoderValue;
      }

      uint32_t SspFrontPanelState::mapButtonToGpio(input::HardwareButtonId button) const
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

      void SspFrontPanelState::setActiveOutput(int output)
      {
        mActiveOutput = clampOutput(output);
        Gpio_write(BUTTON_SELECT1, true);
        Gpio_write(BUTTON_SELECT2, true);
        Gpio_write(BUTTON_SELECT3, true);
        Gpio_write(BUTTON_SELECT4, true);
        Gpio_write(activeSelectGpio(mActiveOutput), false);
      }

      void SspFrontPanelState::switchToggle(uint32_t idA, uint32_t idB, int delta)
      {
        if (delta > 0)
        {
          if (Gpio_read(idA))
          {
            Gpio_write(idA, false);
          }
          else if (!Gpio_read(idB))
          {
            Gpio_write(idB, true);
          }
        }
        else if (delta < 0)
        {
          if (Gpio_read(idB))
          {
            Gpio_write(idB, false);
          }
          else if (!Gpio_read(idA))
          {
            Gpio_write(idA, true);
          }
        }
      }

      int SspFrontPanelState::toggleState(uint32_t idA, uint32_t idB) const
      {
        if (Gpio_read(idA))
        {
          return kToggleUp;
        }
        if (Gpio_read(idB))
        {
          return kToggleDown;
        }
        return kToggleMid;
      }

      void SspFrontPanelState::refreshIndicators()
      {
        mButtonActive[BTN_M1] = Button_pressed(BUTTON_MAIN1);
        mButtonActive[BTN_M2] = Button_pressed(BUTTON_MAIN2);
        mButtonActive[BTN_M3] = Button_pressed(BUTTON_MAIN3);
        mButtonActive[BTN_HOME] = Button_pressed(BUTTON_DIAL3);
        mButtonActive[BTN_ENTER] = Button_pressed(BUTTON_ENTER);
        mButtonActive[BTN_UP] = Button_pressed(BUTTON_UP);
        mButtonActive[BTN_SHIFT] = Button_pressed(BUTTON_SHIFT);
        mButtonActive[BTN_M4] = Button_pressed(BUTTON_MAIN4);
        mButtonActive[BTN_M5] = Button_pressed(BUTTON_MAIN5);
        mButtonActive[BTN_M6] = Button_pressed(BUTTON_MAIN6);
        mButtonActive[BTN_CAN] = Button_pressed(BUTTON_DIAL2);
        mButtonActive[BTN_S1] = Button_pressed(BUTTON_SUB1);
        mButtonActive[BTN_S2] = Button_pressed(BUTTON_SUB2);
        mButtonActive[BTN_S3] = Button_pressed(BUTTON_SUB3);
        mButtonActive[BTN_SEL1] = Button_pressed(BUTTON_SELECT1);
        mButtonActive[BTN_SEL2] = Button_pressed(BUTTON_SELECT2);
        mButtonActive[BTN_SEL3] = Button_pressed(BUTTON_SELECT3);
        mButtonActive[BTN_SEL4] = Button_pressed(BUTTON_SELECT4);

        Gpio_write(LED_OUT1, mActiveOutput == 1);
        Gpio_write(LED_OUT2, mActiveOutput == 2);
        Gpio_write(LED_OUT3, mActiveOutput == 3);
        Gpio_write(LED_OUT4, mActiveOutput == 4);
        Gpio_write(LED_LINK12, mLinkGestureActive && mActiveOutput == 1);
        Gpio_write(LED_LINK23, mLinkGestureActive && mActiveOutput == 2);
        Gpio_write(LED_LINK34, mLinkGestureActive && mActiveOutput >= 3);
        Gpio_write(LED_DIAL1, mEncoderValue < 0);
        Gpio_write(LED_DIAL2, mEncoderValue > 0);
        Gpio_write(LED_IO, toggleState(TOGGLE_STORAGE_A, TOGGLE_STORAGE_B) != kToggleMid);
        Gpio_write(LED_SAFE, toggleState(TOGGLE_MODE_A, TOGGLE_MODE_B) == kToggleDown);

        mLedActive[WIDGET_LED_FINE] = Gpio_read(LED_DIAL1);
        mLedActive[WIDGET_LED_COARSE] = Gpio_read(LED_DIAL2);
        mLedActive[WIDGET_LED_IO] = Gpio_read(LED_IO);
        mLedActive[WIDGET_LED_SAFE] = Gpio_read(LED_SAFE);
        mLedActive[WIDGET_LED_OUT1] = Gpio_read(LED_OUT1);
        mLedActive[WIDGET_LED_LINK1] = Gpio_read(LED_LINK12);
        mLedActive[WIDGET_LED_OUT2] = Gpio_read(LED_OUT2);
        mLedActive[WIDGET_LED_LINK2] = Gpio_read(LED_LINK23);
        mLedActive[WIDGET_LED_OUT3] = Gpio_read(LED_OUT3);
        mLedActive[WIDGET_LED_LINK3] = Gpio_read(LED_LINK34);
        mLedActive[WIDGET_LED_OUT4] = Gpio_read(LED_OUT4);

        mTogglePositions[TOGGLE_STORAGE] = toggleState(TOGGLE_STORAGE_A, TOGGLE_STORAGE_B);
        mTogglePositions[TOGGLE_MODE] = toggleState(TOGGLE_MODE_A, TOGGLE_MODE_B);
      }
    }
  }
}