#include <percussa/panel/ssp/SspController.h>

#include <percussa/panel/Panel.h>

#include <string>

namespace percussa
{
  namespace panel
  {
    namespace ssp
    {
      namespace
      {
        const char *togglePositionName(int position)
        {
          switch (position)
          {
          case 0:
            return "low";
          case 2:
            return "high";
          default:
            return "mid";
          }
        }
      }

      SspController::SspController(const Panel &panel) :
        mPanel(panel),
        mStatusText("ready")
      {
        refreshPresentationState();
      }

      void SspController::handleAction(const input::Action &action)
      {
        mStatusText = input::describeAction(action);

        if (action.type == input::ActionType::Button)
        {
          mFrontPanelState.handleButton(action.hardwareButton, action.pressed);
        }

        if (action.type == input::ActionType::EncoderTurn)
        {
          mFrontPanelState.handleEncoderTurn(action.hardwareEncoder, action.delta);
        }

        if (action.type == input::ActionType::EncoderPress)
        {
          mFrontPanelState.handleEncoderPress(action.hardwareEncoder, action.pressed);
        }

        refreshPresentationState();
      }

      const ui::PresentationState &SspController::presentationState() const
      {
        return mPresentationState;
      }

      const std::string &SspController::statusText() const
      {
        return mStatusText;
      }

      void SspController::refreshPresentationState()
      {
        mPresentationState.statusText = mStatusText;
        mPresentationState.displays.resize(mPanel.displays().size());
        mPresentationState.buttonActive.assign(mFrontPanelState.buttonActive().begin(), mFrontPanelState.buttonActive().end());

        if (!mPresentationState.displays.empty())
        {
          mPresentationState.displays[0].title = "ssp";
          mPresentationState.displays[0].line1 = std::string("out ") + std::to_string(mFrontPanelState.activeOutput());
          mPresentationState.displays[0].line2 = std::string("data ") + std::to_string(mFrontPanelState.encoderValue());
        }

        if (mPresentationState.displays.size() > 1)
        {
          const std::array<int, SspFrontPanelState::kToggleCount> &toggles = mFrontPanelState.togglePositions();
          mPresentationState.displays[1].title = "panel";
          mPresentationState.displays[1].line1 = std::string("store ") + togglePositionName(toggles[0]);
          mPresentationState.displays[1].line2 = std::string("mode ") + togglePositionName(toggles[1]);
        }

        for (size_t i = 2; i < mPresentationState.displays.size(); ++i)
        {
          mPresentationState.displays[i].title = mPanel.displays()[i].role;
          mPresentationState.displays[i].line1 = "shared state";
          mPresentationState.displays[i].line2 = mStatusText;
        }

        mPresentationState.ledActive.assign(mFrontPanelState.ledActive().begin(), mFrontPanelState.ledActive().end());
        mPresentationState.togglePositions.assign(mFrontPanelState.togglePositions().begin(), mFrontPanelState.togglePositions().end());
      }
    }
  }
}