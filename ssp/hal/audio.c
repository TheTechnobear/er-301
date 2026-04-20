#include <SDL2/SDL.h>
#include <hal/audio.h>
#include <hal/log.h>
#include <hal/constants.h>
#include <hal/pump.h>
#include <od/config.h>
#include <string.h>

#define MAX_CAPTURE_CHANNELS 32
#define MAX_PLAYBACK_CHANNELS 32

void SspModulation_ingestInterleavedS32(const int *samples, uint32_t frames, uint32_t channels);
void SspModulation_copyFrame(float *dst, uint32_t frames);

static struct AudioLocals {
  int captureBuffer[MAX_AUDIO_FRAME_LENGTH * MAX_CAPTURE_CHANNELS] __attribute__((aligned(CACHELINE_SIZE_MAX)));
  int captureMapped[MAX_AUDIO_FRAME_LENGTH * NUM_INPUT_CHANNELS] __attribute__((aligned(CACHELINE_SIZE_MAX)));
  float inFrame[MAX_AUDIO_FRAME_LENGTH * NUM_INPUT_CHANNELS] __attribute__((aligned(CACHELINE_SIZE_MAX)));
  float outFrame[MAX_AUDIO_FRAME_LENGTH * NUM_OUTPUT_CHANNELS] __attribute__((aligned(CACHELINE_SIZE_MAX)));

  SDL_AudioSpec playSpec;
  SDL_AudioSpec capSpec;
  SDL_AudioDeviceID playDev;
  SDL_AudioDeviceID capDev;
  char playDevName[64];
  char capDevName[64];
  uint32_t debugFrameCounter;
} local;

// Logical ER-301 input index -> host capture device channel index.
static int inputChannelMap[NUM_INPUT_CHANNELS] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

// Logical ER-301 outputs OUT1..OUT4 -> host playback device channel index.
static int outputChannelMap[NUM_OUTPUT_CHANNELS] = {0, 1, 2, 3};

static SDL_AudioDeviceID openCaptureDevice(const char *deviceName,
                                           int sampleRate,
                                           int frameLength,
                                           int requestedChannels,
                                           SDL_AudioSpec *obtained)
{
  SDL_AudioSpec want;
  SDL_zero(want);
  want.freq = sampleRate;
  want.format = AUDIO_S32;
  want.channels = requestedChannels;
  want.samples = frameLength;
  want.callback = NULL;

  return SDL_OpenAudioDevice(deviceName,
                             true,
                             &want,
                             obtained,
                             SDL_AUDIO_ALLOW_CHANNELS_CHANGE |
                                 SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
}

static void playCallback(void *userdata, Uint8 *stream, int len)
{
  (void)userdata;

  uint32_t frameLength = globalConfig.frameLength;
  uint32_t outChannels = local.playSpec.channels ? local.playSpec.channels : 1;
  uint32_t outFramesAvailable = (uint32_t)len / (sizeof(int) * outChannels);
  uint32_t captureBytes = 0;
  int got = 0;
  if (frameLength > outFramesAvailable)
  {
    frameLength = outFramesAvailable;
  }

  if (local.capDev)
  {
    uint32_t captureChannels = local.capSpec.channels;
    if (captureChannels > MAX_CAPTURE_CHANNELS)
    {
      captureChannels = MAX_CAPTURE_CHANNELS;
    }

    captureBytes = frameLength * captureChannels * sizeof(int);
    // int queuedBefore = SDL_GetQueuedAudioSize(local.capDev);
    got = SDL_DequeueAudio(local.capDev, local.captureBuffer, captureBytes);
    if (got < 0)
    {
      got = 0;
    }
    if ((uint32_t)got < captureBytes)
    {
      memset((uint8_t *)local.captureBuffer + got, 0, captureBytes - (uint32_t)got);
    }

    for (uint32_t i = 0; i < frameLength; i++)
    {
      uint32_t srcBase = i * captureChannels;
      uint32_t dstBase = i * NUM_INPUT_CHANNELS;
      for (uint32_t ch = 0; ch < NUM_INPUT_CHANNELS; ch++)
      {
        int srcCh = inputChannelMap[ch];
        int sample = 0;
        if (srcCh >= 0 && (uint32_t)srcCh < captureChannels)
        {
          sample = local.captureBuffer[srcBase + (uint32_t)srcCh];
        }
        local.captureMapped[dstBase + ch] = sample;
      }
    }

    SspModulation_ingestInterleavedS32(local.captureMapped, frameLength,
                                       NUM_INPUT_CHANNELS);
    // int queuedAfter = SDL_GetQueuedAudioSize(local.capDev);
  }
  else
  {
    SspModulation_ingestInterleavedS32(NULL, frameLength, 0);
  }

  SspModulation_copyFrame(local.inFrame, frameLength);

  memset(local.outFrame, 0, sizeof(float) * frameLength * NUM_OUTPUT_CHANNELS);
  Pump_callback(local.inFrame, local.outFrame);


  memset(stream, 0, len);
  int *out = (int *)stream;
  if (local.playSpec.channels < 4)
  {
    for (uint32_t i = 0; i < frameLength; i++)
    {
      float x0 = local.outFrame[4*i + 0]; if (x0 > 1.0f) x0 = 1.0f; else if (x0 < -1.0f) x0 = -1.0f;
      float x1 = local.outFrame[4*i + 1]; if (x1 > 1.0f) x1 = 1.0f; else if (x1 < -1.0f) x1 = -1.0f;
      float x2 = local.outFrame[4*i + 2]; if (x2 > 1.0f) x2 = 1.0f; else if (x2 < -1.0f) x2 = -1.0f;
      float x3 = local.outFrame[4*i + 3]; if (x3 > 1.0f) x3 = 1.0f; else if (x3 < -1.0f) x3 = -1.0f;
      out[2*i]     = (int)((x0 + x2) * AUDIO_SAFE_MAX_OUTPUT_VALUE) << 7;
      out[2*i + 1] = (int)((x1 + x3) * AUDIO_SAFE_MAX_OUTPUT_VALUE) << 7;
    }
  }
  else
  {
    uint32_t outChannels = local.playSpec.channels;
    if (outChannels > MAX_PLAYBACK_CHANNELS)
      outChannels = MAX_PLAYBACK_CHANNELS;
    for (uint32_t i = 0; i < frameLength; i++)
    {
      for (uint32_t c = 0; c < NUM_OUTPUT_CHANNELS; c++)
      {
        int dstCh = outputChannelMap[c];
        if (dstCh >= 0 && (uint32_t)dstCh < outChannels)
        {
          float x = local.outFrame[4*i + c];
          if (x > 1.0f) x = 1.0f; else if (x < -1.0f) x = -1.0f;
          out[i * outChannels + (uint32_t)dstCh] = (int)(x * AUDIO_SAFE_MAX_OUTPUT_VALUE) << 8;
        }
      }
    }
  }
}

void Audio_init()
{
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  SDL_zero(local);

  for (int i = 0; i < SDL_GetNumAudioDrivers(); ++i)
  {
    logInfo("Audio driver %d: %s", i, SDL_GetAudioDriver(i));
  }

#if BUILDOPT_FORCE_ALSA
  if (SDL_AudioInit("alsa"))
  {
    logError("Failed to initialize alsa driver: %s", SDL_GetError());
    logInfo("Opening default driver...");
  }
#endif

  const char *driver_name = SDL_GetCurrentAudioDriver();

  if (driver_name)
  {
    logInfo("Audio subsystem initialized, driver = %s.", driver_name);
  }
  else
  {
    logError("Audio subsystem not initialized.");
  }
}

void Audio_restart(void)
{
  Audio_stop();
  SDL_Delay(200);
  Audio_start();
}

void Audio_start(void)
{
  int outputChannels = 2;
  int captureChannels = NUM_INPUT_CHANNELS;

  {
    bool isCapture = false;
    for (int i = 0; i < SDL_GetNumAudioDevices(isCapture); ++i)
    {
      SDL_AudioSpec spec;
      const char *name = SDL_GetAudioDeviceName(i, isCapture);
      if (name && !SDL_GetAudioDeviceSpec(i, isCapture, &spec))
      {
        logInfo("Audio Device %s : %d", name, spec.channels);
        if (strlen(local.playDevName) == 0 && spec.channels == 4)
        {
          outputChannels = 4;
          strncpy(local.playDevName, name, sizeof(local.playDevName));
          local.playDevName[sizeof(local.playDevName) - 1] = 0;
        }
      }
    }

    isCapture = true;
    for (int i = 0; i < SDL_GetNumAudioDevices(isCapture); ++i)
    {
      SDL_AudioSpec spec;
      const char *name = SDL_GetAudioDeviceName(i, isCapture);
      if (name && !SDL_GetAudioDeviceSpec(i, isCapture, &spec))
      {
        logInfo("Capture Device %s : %d", name, spec.channels);
        if (strlen(local.capDevName) == 0 && spec.channels >= captureChannels)
        {
          captureChannels = spec.channels;
          strncpy(local.capDevName, name, sizeof(local.capDevName));
          local.capDevName[sizeof(local.capDevName) - 1] = 0;
        }
      }
    }
  }

  if (strlen(local.playDevName) == 0)
  {
    SDL_AudioSpec spec;
    char *name = NULL;
    if (!SDL_GetDefaultAudioInfo(&name, &spec, false))
    {
      strncpy(local.playDevName, name, sizeof(local.playDevName));
      local.playDevName[sizeof(local.playDevName) - 1] = 0;
      outputChannels = spec.channels;
      logInfo("Default Audio Device %s : %d", local.playDevName, spec.channels);
    }
  }

  if (strlen(local.capDevName) == 0)
  {
    SDL_AudioSpec spec;
    char *name = NULL;
    if (!SDL_GetDefaultAudioInfo(&name, &spec, true))
    {
      strncpy(local.capDevName, name, sizeof(local.capDevName));
      local.capDevName[sizeof(local.capDevName) - 1] = 0;
      captureChannels = spec.channels;
      logInfo("Default Capture Device %s : %d", local.capDevName, spec.channels);
    }
  }

  if (strlen(local.playDevName))
  {
    logInfo("Using Audio Device %s", local.playDevName);
  }
  if (strlen(local.capDevName))
  {
    logInfo("Using Capture Device %s", local.capDevName);
  }

  #ifdef __APPLE__
  captureChannels = 8;
#endif 


  SDL_AudioSpec wantCap;
  SDL_zero(wantCap);
  wantCap.freq = globalConfig.sampleRate;
  wantCap.format = AUDIO_S32;
  wantCap.channels = captureChannels;
  wantCap.samples = globalConfig.frameLength;
  wantCap.callback = NULL;

  local.capDev = openCaptureDevice(strlen(local.capDevName) == 0 ? NULL : local.capDevName,
                                   globalConfig.sampleRate,
                                   globalConfig.frameLength,
                                   captureChannels,
                                   &local.capSpec);
  if (local.capDev == 0)
  {
    logError("Failed to open capture: %s", SDL_GetError());
  }
  else
  {
    logInfo("Capture Specs %dHz %dch", local.capSpec.freq, local.capSpec.channels);
    SDL_PauseAudioDevice(local.capDev, 0);
  }

  SDL_AudioSpec want;
  SDL_zero(want);
  want.freq = globalConfig.sampleRate;
  want.format = AUDIO_S32;
  want.channels = outputChannels;
  want.samples = globalConfig.frameLength;
  want.callback = playCallback;

  local.playDev = SDL_OpenAudioDevice(strlen(local.playDevName) == 0 ? NULL : local.playDevName,
                                      false,
                                      &want,
                                      &local.playSpec,
                                      0);
  if (local.playDev == 0)
  {
    logError("Failed to open audio: %s", SDL_GetError());
  }
  else
  {
    logInfo("Audio Specs %dHz %dch", local.playSpec.freq, local.playSpec.channels);
    SDL_PauseAudioDevice(local.playDev, 0);
  }
}

void Audio_stop(void)
{
  if (local.playDev)
  {
    SDL_CloseAudioDevice(local.playDev);
    local.playDev = 0;
  }
  if (local.capDev)
  {
    SDL_CloseAudioDevice(local.capDev);
    local.capDev = 0;
  }
}

uint32_t Audio_errorCount(void) { return 0; }

int Audio_getRate(void) { return globalConfig.sampleRate; }

void Audio_printErrorStatus(void)
{
  switch (SDL_GetAudioDeviceStatus(local.playDev))
  {
  case SDL_AUDIO_STOPPED:
    logInfo("audio stopped");
    break;
  case SDL_AUDIO_PLAYING:
    logInfo("audio playing");
    break;
  case SDL_AUDIO_PAUSED:
    logInfo("audio paused");
    break;
  default:
    logInfo("audio ???");
    break;
  }
}

int Audio_getLoad() { return 0; }

bool Audio_running() { return SDL_GetAudioDeviceStatus(local.playDev) == SDL_AUDIO_PLAYING; }
