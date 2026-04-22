#pragma once

#include <hal/display.h>

namespace ssp
{
  const int OVERSAMPLE = 4;

#if __APPLE__
  const int SCREEN_WIDTH = 1600;
  const int SCREEN_HEIGHT = 480;
#elif defined(TARGET_SSP)
  const int SCREEN_WIDTH = 1600;
  const int SCREEN_HEIGHT = 480;
#elif defined(TARGET_XMX)
  const int SCREEN_WIDTH = 320;
  const int SCREEN_HEIGHT = 320; // 240 ?
#else
  const int SCREEN_WIDTH = 1600;
  const int SCREEN_HEIGHT = 480;
#endif
  const int SCREEN_BRIGHTNESS = 15; // 1-16
  const float SCREEN_TINT = 0.85f;

  // panel color
  const int P_BACKGROUND = 10;

  const int ENCODER_SPEED = 5;

  // outer margin
  const int MARGIN = 16;

  // main display
  const int MAIN_X = MARGIN;
  const int MAIN_Y = MARGIN;
  const int MAIN_W = MAIN_HORIZONTAL_PIXELS;
  const int MAIN_H = MAIN_VERTICAL_PIXELS;
  const int MAIN_SCALE = 3;

  const double KNOB_SPEED = 360.0 * ENCODER_SPEED / 128.0;

  // sub display
  const int SUB_W = SUB_HORIZONTAL_PIXELS;
  const int SUB_H = SUB_VERTICAL_PIXELS;
  const int SUB_X = MAIN_X + (MAIN_HORIZONTAL_PIXELS * MAIN_SCALE) +( MARGIN * 2) ;
  const int SUB_Y = MARGIN;
  const int SUB_SCALE = 3;

} // namespace ssp