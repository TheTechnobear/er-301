#include <percussa/panel/xmx/XmxController.h>

#include <percussa/panel/Panel.h>

#include <string>

namespace percussa
{
  namespace panel
  {
    namespace xmx
    {
      namespace
      {
        int wrapIndex(int value, int size)
        {
          if (size <= 0)
          {
            return 0;
          }

          while (value < 0)
          {
            value += size;
          }

          while (value >= size)
          {
            value -= size;
          }

          return value;
        }

        int buttonSelection(input::HardwareButtonId button)
        {
          switch (button)
          {
          case input::HardwareButtonId::Button1:
            return 0;
          case input::HardwareButtonId::Button2:
            return 1;
          case input::HardwareButtonId::Button3:
            return 2;
          case input::HardwareButtonId::Button4:
            return 3;
          case input::HardwareButtonId::Button5:
            return 4;
          case input::HardwareButtonId::Button6:
            return 5;
          case input::HardwareButtonId::Button7:
            return 6;
          case input::HardwareButtonId::Button8:
            return 7;
          default:
            return -1;
          }
        }

        const char *tabName(int tab)
        {
          switch (tab)
          {
          case 0:
            return "main";
          case 1:
            return "mod";
          default:
            return "sys";
          }
        }

      }

      XmxController::XmxController(const Panel &panel) :
        mPanel(panel),
        mStatusText("ready")
      {
        refreshPresentationState();
      }

      void XmxController::handleAction(const input::Action &action)
      {
        mStatusText = input::describeAction(action);

        if (action.type == input::ActionType::Button && action.pressed)
        {
          int selection = buttonSelection(action.hardwareButton);
          if (selection >= 0)
          {
            mSelectedSoftButton = selection;
            mSelectedTab = selection / 3;
          }
        }

        if (action.type == input::ActionType::EncoderTurn)
        {
          switch (action.hardwareEncoder)
          {
          case input::HardwareEncoderId::Encoder1:
            mSelectedTab = wrapIndex(mSelectedTab + action.delta, 3);
            break;
          case input::HardwareEncoderId::Encoder2:
            mSelectedSoftButton = wrapIndex(mSelectedSoftButton + action.delta, 8);
            break;
          case input::HardwareEncoderId::Encoder3:
            mValue += action.delta;
            if (mValue < 0)
            {
              mValue = 0;
            }
            if (mValue > 99)
            {
              mValue = 99;
            }
            break;
          case input::HardwareEncoderId::Encoder4:
            mNavState = wrapIndex(mNavState + action.delta, 3);
            break;
          default:
            break;
          }
        }

        if (action.type == input::ActionType::EncoderPress && action.pressed)
        {
          switch (action.hardwareEncoder)
          {
          case input::HardwareEncoderId::Encoder1:
            mSelectedTab = wrapIndex(mSelectedTab + 1, 3);
            break;
          case input::HardwareEncoderId::Encoder4:
            mNavState = 1;
            break;
          default:
            break;
          }
        }

        refreshPresentationState();
      }

      const ui::PresentationState &XmxController::presentationState() const
      {
        return mPresentationState;
      }

      const std::string &XmxController::statusText() const
      {
        return mStatusText;
      }

      void XmxController::refreshPresentationState()
      {
        mPresentationState.statusText = mStatusText;
        mPresentationState.displays.resize(mPanel.displays().size());

        if (!mPresentationState.displays.empty())
        {
          mPresentationState.displays[0].title = std::string("xmx ") + tabName(mSelectedTab);
          mPresentationState.displays[0].line1 = std::string("soft s") + std::to_string(mSelectedSoftButton + 1);
          mPresentationState.displays[0].line2 = std::string("value ") + std::to_string(mValue);
        }

        mPresentationState.ledActive.assign(mPanel.leds().size(), false);
        if (!mPresentationState.ledActive.empty())
        {
          mPresentationState.ledActive[0] = (mValue % 2) == 1;
        }

        mPresentationState.togglePositions.assign(mPanel.toggles().size(), 1);
        if (!mPresentationState.togglePositions.empty())
        {
          mPresentationState.togglePositions[0] = mNavState;
        }
      }
    }
  }
}