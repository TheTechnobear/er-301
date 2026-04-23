#include <percussa/input/LinuxInput.h>

#if defined(__linux__)

#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

#include <cstdio>

namespace percussa
{
  namespace input
  {
    namespace
    {
      static constexpr int kEncoderMultiplier = 1;
    }

    LinuxInput::LinuxInput()
    {
    }

    LinuxInput::~LinuxInput()
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

    bool LinuxInput::init()
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

    bool LinuxInput::isAvailable() const
    {
      return mInitialized;
    }

    HardwareButtonId LinuxInput::mapButtonCode(int code) const
    {
#if defined(PERCUSSA_PANEL_SSP)
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
#elif defined(PERCUSSA_PANEL_XMX)
      switch (code)
      {
      case 59:
        return HardwareButtonId::Button1;
      case 60:
        return HardwareButtonId::Button2;
      case 61:
        return HardwareButtonId::Button3;
      case 62:
        return HardwareButtonId::Button4;
      case 64:
        return HardwareButtonId::Button5;
      case 65:
        return HardwareButtonId::Button6;
      case 66:
        return HardwareButtonId::Button7;
      case 67:
        return HardwareButtonId::Button8;
      case 68:
        return HardwareButtonId::Up;
      case 63:
        return HardwareButtonId::Down;
      default:
        return HardwareButtonId::Invalid;
      }
#else
#error "No percussa panel selected at build time."
#endif
    }

    void LinuxInput::poll(const std::function<void(const Action &)> &onAction)
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

#else

namespace percussa
{
  namespace input
  {
    LinuxInput::LinuxInput()
    {
    }

    LinuxInput::~LinuxInput()
    {
    }

    bool LinuxInput::init()
    {
      mInitialized = false;
      return false;
    }

    bool LinuxInput::isAvailable() const
    {
      return false;
    }

    HardwareButtonId LinuxInput::mapButtonCode(int code) const
    {
      (void)code;
      return HardwareButtonId::Invalid;
    }

    void LinuxInput::poll(const std::function<void(const Action &)> &onAction)
    {
      (void)onAction;
    }
  }
}

#endif