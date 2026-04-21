#pragma once

#include <functional>

namespace ssp
{
  enum class SSPButtonId
  {
    Soft1,
    Soft2,
    Soft3,
    Soft4,
    Soft5,
    Soft6,
    Soft7,
    Soft8,
    Up,
    Down,
    LShift,
    RShift,
    Left,
    Right,
    P1,
    P2,
    P3,
    P4,
    Invalid
  };

  enum class SSPEncoderId
  {
    Encoder1 = 0,
    Encoder2 = 1,
    Encoder3 = 2,
    Encoder4 = 3,
    Invalid = -1
  };

  class SSPButEnc
  {
  public:
    SSPButEnc() = default;
    ~SSPButEnc();

    bool init();

    void poll(
      const std::function<void(SSPButtonId, bool)> &onButton,
      const std::function<void(SSPEncoderId, int)> &onEncoderTurn,
      const std::function<void(SSPEncoderId, bool)> &onEncoderPress);

    bool isAvailable() const;

  private:
    SSPButtonId mapButtonCode(int code) const;

    static constexpr unsigned NUM_ENCODERS = 4;
    int encFD_[NUM_ENCODERS] = { -1, -1, -1, -1 };
    int encSwitchFD_ = -1;
    int buttonFD_ = -1;
    bool initialized_ = false;

    const int encMap_[NUM_ENCODERS] = { 1, 3, 0, 2 };
    const int encSwMap_[NUM_ENCODERS] = { 1, 3, 0, 2 };
  };
} // namespace ssp
