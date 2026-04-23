#include <hal/modulation.h>

#include <hal/channels.h>
#include <hal/constants.h>

#include <string.h>

static struct
{
  uint32_t range0[MOD_NUM_CHANNELS_PER_ADC];
  uint32_t range1[MOD_NUM_CHANNELS_PER_ADC];
  float frame[MAX_AUDIO_FRAME_LENGTH * NUM_INPUT_CHANNELS];
} local;

static float getRangeGain(uint32_t range)
{
  switch (range)
  {
  case MOD_BIPOLAR_1250:
    return 2.0f;
  case MOD_BIPOLAR_0625:
    return 4.0f;
  default:
    return 1.0f;
  }
}

static float getInputGain(uint32_t channel)
{
  if (channel < MOD_NUM_CHANNELS_PER_ADC)
  {
    return getRangeGain(local.range0[channel]);
  }

  if (channel < MOD_NUM_CHANNELS)
  {
    return getRangeGain(local.range1[channel - MOD_NUM_CHANNELS_PER_ADC]);
  }

  return 1.0f;
}

static float s32ToInputVolts(int sample)
{
  return ((float)sample / 2147483648.0f) * FULLSCALE_IN_VOLTS;
}

void Modulation_init()
{
  memset(&local, 0, sizeof(local));

  for (uint32_t i = 0; i < MOD_NUM_CHANNELS_PER_ADC; i++)
  {
    local.range0[i] = MOD_BIPOLAR_2500;
    local.range1[i] = MOD_BIPOLAR_2500;
  }
}

void Modulation_start(void)
{
}

void Modulation_restart(void)
{
}

void Modulation_setChannelRange(uint32_t channel, uint32_t range)
{
  if (channel >= MOD_NUM_CHANNELS)
  {
    return;
  }

  if (channel < MOD_NUM_CHANNELS_PER_ADC)
  {
    local.range0[channel] = range;
  }
  else
  {
    channel -= MOD_NUM_CHANNELS_PER_ADC;
    local.range1[channel] = range;
  }
}

uint32_t Modulation_getChannelRange(uint32_t channel)
{
  if (channel >= MOD_NUM_CHANNELS)
  {
    return MOD_BIPOLAR_2500;
  }

  if (channel < MOD_NUM_CHANNELS_PER_ADC)
  {
    return local.range0[channel];
  }
  else
  {
    channel -= MOD_NUM_CHANNELS_PER_ADC;
    return local.range1[channel];
  }
}

void SspModulation_ingestInterleavedS32(const int *samples, uint32_t frames, uint32_t channels)
{
  if (frames > MAX_AUDIO_FRAME_LENGTH)
  {
    frames = MAX_AUDIO_FRAME_LENGTH;
  }

  for (uint32_t i = 0; i < frames; i++)
  {
    uint32_t srcBase = i * channels;
    uint32_t dstBase = i * NUM_INPUT_CHANNELS;

    for (uint32_t ch = 0; ch < NUM_INPUT_CHANNELS; ch++)
    {
      int sample = 0;
      if (samples && ch < channels)
      {
        sample = samples[srcBase + ch];
      }

      float x = s32ToInputVolts(sample);
      local.frame[dstBase + ch] = x * getInputGain(ch);
    }
  }
}

void SspModulation_copyFrame(float *dst, uint32_t frames)
{
  if (frames > MAX_AUDIO_FRAME_LENGTH)
  {
    frames = MAX_AUDIO_FRAME_LENGTH;
  }

  memcpy(dst, local.frame, sizeof(float) * frames * NUM_INPUT_CHANNELS);
}