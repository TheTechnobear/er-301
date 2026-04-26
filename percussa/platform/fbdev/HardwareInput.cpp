#include <percussa/platform/fbdev/HardwareInput.h>
#include <percussa/platform/fbdev/ButtonMap.h>

#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

#include <cstdio>

namespace percussa
{
  namespace input
  {
    HardwareInput::HardwareInput()
    {
    }

    HardwareInput::~HardwareInput()
    {
      for (int i = 0; i < (int)kEncoderCount; i++)
      {
        if (mEncoderFd[i] >= 0)
        {
          close(mEncoderFd[i]);
        }
      }

      if (mEncoderSwitchFd >= 0)
      {
        close(mEncoderSwitchFd);
      }

      if (mButtonFd >= 0)
      {
        close(mButtonFd);
      }
    }

    bool HardwareInput::init()
    {
      for (int i = 0; i < (int)kEncoderCount; i++)
      {
        static constexpr unsigned kMaxPath = 100;
        char path[kMaxPath];
        std::snprintf(path, kMaxPath, "/dev/input/by-path/platform-rotary@%d-event", i);
        mEncoderFd[i] = open(path, O_RDONLY | O_NONBLOCK);
        if (mEncoderFd[i] < 0)
        {
          return false;
        }
      }

      mEncoderSwitchFd = open("/dev/input/by-path/platform-gpio-keys-event", O_RDONLY | O_NONBLOCK);
      if (mEncoderSwitchFd < 0)
      {
        return false;
      }

      mButtonFd = open("/dev/input/by-path/platform-matrix-keypad-event", O_RDONLY | O_NONBLOCK);
      if (mButtonFd < 0)
      {
        return false;
      }

      mInitialized = true;
      return true;
    }

    bool HardwareInput::isAvailable() const
    {
      return mInitialized;
    }

    void HardwareInput::poll(const std::function<void(const Action &)> &onAction)
    {
      if (!mInitialized)
      {
        return;
      }

      bool moreEvents = true;
      while (moreEvents)
      {
        input_event ev;
        moreEvents = false;

        for (int i = 0; i < (int)kEncoderCount; i++)
        {
          if (read(mEncoderFd[i], &ev, sizeof(ev)) == sizeof(ev))
          {
            if (ev.type == EV_REL && ev.code == REL_X)
            {
              const int mapped = mEncoderMap[i];
              const int delta = ev.value * kEncoderMultiplier;
              if (delta != 0 && mapped >= 0 && mapped < (int)kEncoderCount)
              {
                Action action;
                action.type = ActionType::EncoderTurn;
                action.hardwareEncoder = (HardwareEncoderId)mapped;
                action.delta = delta;
                onAction(action);
              }
            }
            moreEvents = true;
          }
        }

        if (read(mEncoderSwitchFd, &ev, sizeof(ev)) == sizeof(ev))
        {
          if (ev.type == EV_KEY)
          {
            const int logicalIndex = ev.code - 2;
            if (logicalIndex >= 0 && logicalIndex < (int)kEncoderCount)
            {
              const int mapped = mEncoderSwitchMap[logicalIndex];
              if (mapped >= 0 && mapped < (int)kEncoderCount)
              {
                Action action;
                action.type = ActionType::EncoderPress;
                action.hardwareEncoder = (HardwareEncoderId)mapped;
                action.pressed = ev.value != 0;
                onAction(action);
              }
            }
          }
          moreEvents = true;
        }

        if (read(mButtonFd, &ev, sizeof(ev)) == sizeof(ev))
        {
          if (ev.type == EV_KEY)
          {
            const HardwareButtonId mapped = mapButtonCode(ev.code);
            if (mapped != HardwareButtonId::Invalid)
            {
              Action action;
              action.type = ActionType::Button;
              action.hardwareButton = mapped;
              action.pressed = ev.value != 0;
              onAction(action);
            }
          }
          moreEvents = true;
        }
      }
    }
  }
}
