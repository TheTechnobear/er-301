#pragma once

#include <percussa/input/Action.h>

#include <functional>

namespace percussa
{
  namespace input
  {
    class LinuxInput
    {
    public:
      LinuxInput();
      ~LinuxInput();

      bool init();
      bool isAvailable() const;
      void poll(const std::function<void(const Action &)> &onAction);

    private:
      HardwareButtonId mapButtonCode(int code) const;

      static constexpr unsigned kEncoderCount = 4;
      int mEncoderFd[kEncoderCount] = { -1, -1, -1, -1 };
      int mEncoderSwitchFd = -1;
      int mButtonFd = -1;
      bool mInitialized = false;

      const int mEncoderMap[kEncoderCount] = { 1, 3, 0, 2 };
      const int mEncoderSwitchMap[kEncoderCount] = { 1, 3, 0, 2 };
    };
  }
}