#pragma once

#include <percussa/ui/olive_bridge.h>

namespace percussa
{
  namespace ui
  {
    namespace style
    {
      constexpr uint32_t kButtonShadow = PERCUSSA_RGBA(0, 0, 0, 140);
      constexpr uint32_t kButtonShell = PERCUSSA_RGBA(12, 13, 16, 255);
      constexpr uint32_t kButtonFaceIdle = PERCUSSA_RGBA(26, 28, 32, 255);
      constexpr uint32_t kButtonFacePressed = PERCUSSA_RGBA(255, 52, 44, 255);
      constexpr uint32_t kButtonEdge = PERCUSSA_RGBA(8, 8, 10, 255);
      constexpr uint32_t kButtonHighlight = PERCUSSA_RGBA(130, 136, 148, 100);

      constexpr uint32_t kDisplayFace = PERCUSSA_RGBA(9, 10, 12, 255);
      constexpr uint32_t kDisplayShadow = PERCUSSA_RGBA(0, 0, 0, 90);
      constexpr uint32_t kDisplayGlow = PERCUSSA_RGBA(24, 19, 8, 255);
      constexpr uint32_t kDisplayEdge = PERCUSSA_RGBA(106, 112, 126, 255);
      constexpr uint32_t kDisplayText = PERCUSSA_RGBA(245, 184, 40, 255);

      constexpr uint32_t kEncoderShadow = PERCUSSA_RGBA(0, 0, 0, 255);
      constexpr uint32_t kEncoderShell = PERCUSSA_RGBA(30, 30, 30, 255);
      constexpr uint32_t kEncoderFace = PERCUSSA_RGBA(150, 150, 150, 255);
      constexpr uint32_t kEncoderRim = PERCUSSA_RGBA(90, 90, 100, 255);

      constexpr uint32_t kAmberText = PERCUSSA_RGBA(225, 191, 0, 255);
      constexpr uint32_t kLedShadow = PERCUSSA_RGBA(0, 0, 0, 110);
      constexpr uint32_t kLedOffShell = PERCUSSA_RGBA(48, 36, 30, 255);
      constexpr uint32_t kLedOffCore = PERCUSSA_RGBA(18, 16, 14, 255);
      constexpr uint32_t kRed = PERCUSSA_RGBA(255, 52, 44, 255);
      constexpr uint32_t kAmberLed = PERCUSSA_RGBA(255, 176, 36, 255);
      constexpr uint32_t kToggleShadow = PERCUSSA_RGBA(0, 0, 0, 80);
      constexpr uint32_t kToggleLine = PERCUSSA_RGBA(110, 114, 126, 255);
    } // namespace style
  } // namespace ui
} // namespace percussa