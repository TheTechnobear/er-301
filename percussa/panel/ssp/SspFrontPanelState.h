#pragma once

#include <percussa/input/Action.h>

#include <hal/gpio.h>

#include <array>

namespace percussa
{
  namespace panel
  {
    namespace ssp
    {
      class SspFrontPanelState
      {
      public:
        static constexpr size_t kButtonCount = 18;
        static constexpr size_t kLedCount = 11;
        static constexpr size_t kToggleCount = 2;

        SspFrontPanelState();

        void handleButton(input::HardwareButtonId button, bool pressed);
        void handleEncoderTurn(input::HardwareEncoderId encoder, int delta);
        void handleEncoderPress(input::HardwareEncoderId encoder, bool pressed);

        const std::array<bool, kButtonCount> &buttonActive() const;
        const std::array<bool, kLedCount> &ledActive() const;
        const std::array<int, kToggleCount> &togglePositions() const;
        int activeOutput() const;
        int encoderValue() const;

      private:
        enum ButtonWidgetIndex
        {
          BTN_M1 = 0,
          BTN_M2,
          BTN_M3,
          BTN_HOME,
          BTN_ENTER,
          BTN_UP,
          BTN_SHIFT,
          BTN_M4,
          BTN_M5,
          BTN_M6,
          BTN_CAN,
          BTN_S1,
          BTN_S2,
          BTN_S3,
          BTN_SEL1,
          BTN_SEL2,
          BTN_SEL3,
          BTN_SEL4
        };

        enum LedWidgetIndex
        {
          WIDGET_LED_FINE = 0,
          WIDGET_LED_COARSE,
          WIDGET_LED_IO,
          WIDGET_LED_SAFE,
          WIDGET_LED_OUT1,
          WIDGET_LED_LINK1,
          WIDGET_LED_OUT2,
          WIDGET_LED_LINK2,
          WIDGET_LED_OUT3,
          WIDGET_LED_LINK3,
          WIDGET_LED_OUT4
        };

        enum ToggleWidgetIndex
        {
          TOGGLE_STORAGE = 0,
          TOGGLE_MODE = 1
        };

        static constexpr int kToggleUp = 0;
        static constexpr int kToggleMid = 1;
        static constexpr int kToggleDown = 2;

        std::array<bool, kButtonCount> mButtonActive = { false };
        std::array<bool, kLedCount> mLedActive = { false };
        std::array<int, kToggleCount> mTogglePositions = { kToggleMid, kToggleMid };
        int mActiveOutput = 1;
        int mEncoderValue = 0;
        bool mLinkGestureActive = false;

        uint32_t mapButtonToGpio(input::HardwareButtonId button) const;
        void setActiveOutput(int output);
        void switchToggle(uint32_t idA, uint32_t idB, int delta);
        int toggleState(uint32_t idA, uint32_t idB) const;
        void refreshIndicators();
      };
    }
  }
}