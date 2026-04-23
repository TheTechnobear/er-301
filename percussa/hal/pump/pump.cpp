#include <hal/constants.h>
#include <hal/pump.h>

#include <string.h>

extern "C" __attribute__((weak)) void Pump_callback(float *inputs, float *outputs)
{
  (void)inputs;
  memset(outputs, 0, sizeof(float) * MAX_AUDIO_FRAME_LENGTH * NUM_OUTPUT_CHANNELS);
}

#include "../../../hal/pump/pump.cpp"