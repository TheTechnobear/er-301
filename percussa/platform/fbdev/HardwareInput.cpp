#include <percussa/platform/fbdev/HardwareInput.h>
#include <percussa/platform/fbdev/ButtonMap.h>

#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

#include <bitset>
#include <cstdio>

// NOTE: The matrix keypad path intentionally forwards at most two simultaneous
// button presses. Tri-chords and larger are suppressed at this low level.
// This limitation is known on XMX (ghost-prone matrix combos) and currently
// untested on SSP.

namespace percussa
{
  namespace input
  {
    namespace
    {
      constexpr size_t kLogicalButtonCount = (size_t)HardwareButtonId::Invalid;

      struct MatrixButtonLimiter
      {
        std::bitset<kLogicalButtonCount> forwardedPressed;
        int activeCount = 0;
      };

      bool logicalButtonIndex(HardwareButtonId button, size_t &index)
      {
        if (button == HardwareButtonId::Invalid)
        {
          return false;
        }
        index = (size_t)button;
        return index < kLogicalButtonCount;
      }
    } // namespace

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
              static MatrixButtonLimiter limiter;
              size_t idx = 0;
              bool hasIndex = logicalButtonIndex(mapped, idx);

              // Handle key release first so we can swallow releases for dropped presses.
              if (ev.value == 0)
              {
                if (hasIndex && limiter.forwardedPressed.test(idx))
                {
                  limiter.forwardedPressed.reset(idx);
                  if (limiter.activeCount > 0)
                  {
                    limiter.activeCount--;
                  }

                  Action action;
                  action.type = ActionType::Button;
                  action.hardwareButton = mapped;
                  action.pressed = false;
                  onAction(action);
                }
                moreEvents = true;
                continue;
              }

              // Ignore key repeats (ev.value == 2) to keep matrix gating simple.
              if (ev.value != 1)
              {
                moreEvents = true;
                continue;
              }

              // For key presses, only allow up to two simultaneous keys.
              if (hasIndex)
              {
                bool alreadyPressed = limiter.forwardedPressed.test(idx);
                if (!alreadyPressed && limiter.activeCount >= 2)
                {
                  moreEvents = true;
                  continue;
                }

                if (!alreadyPressed)
                {
                  limiter.forwardedPressed.set(idx);
                  limiter.activeCount++;
                }
              }

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
