#include <hal/channels.h>
#include <rtaudio_c.h>
#include <hal/audio.h>
#include <hal/log.h>
#include <hal/constants.h>
#include <hal/pump.h>
#include <od/config.h>
#include <string.h>

#if defined(__linux__)
#include <pthread.h>
#include <sched.h>
#endif

#define MAX_CAPTURE_CHANNELS 32
#define MAX_PLAYBACK_CHANNELS 32

void SspModulation_ingestInterleavedS32(const int *samples, uint32_t frames, uint32_t channels);
void SspModulation_copyFrame(float *dst, uint32_t frames);

static struct AudioLocals {
  int captureMapped[MAX_AUDIO_FRAME_LENGTH * NUM_INPUT_CHANNELS] __attribute__((aligned(CACHELINE_SIZE_MAX)));
  float inFrame[MAX_AUDIO_FRAME_LENGTH * NUM_INPUT_CHANNELS] __attribute__((aligned(CACHELINE_SIZE_MAX)));
  float outFrame[MAX_AUDIO_FRAME_LENGTH * NUM_OUTPUT_CHANNELS] __attribute__((aligned(CACHELINE_SIZE_MAX)));
  int inputRoutingMap[MAX_CAPTURE_CHANNELS] __attribute__((aligned(CACHELINE_SIZE_MAX)));

  rtaudio_t audio;
  unsigned int outputDevId;
  unsigned int inputDevId;
  unsigned int outputChannels;
  unsigned int inputChannels;
  uint32_t debugFrameCounter;
} local;

static unsigned int currentSampleRate(void)
{
  return (unsigned int)globalConfig.sampleRate;
}

static unsigned int currentFrameLength(void)
{
  return (unsigned int)globalConfig.frameLength;
}

#if defined(__linux__)
static void Audio_pinCurrentThreadToCore(int core, const char *label)
{
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);
  int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
  if (rc != 0)
  {
    logError("Failed to pin %s thread to core %d: %d", label, core, rc);
  }
  else
  {
    logInfo("Pinned %s thread to core %d.", label, core);
  }
}
#endif

#if defined(TARGET_SSP)
#include <percussa/hal/audio_config_ssp.h>
#elif defined(TARGET_XMX)
#include <percussa/hal/audio_config_xmx.h>
#elif defined(__APPLE__)
#include <percussa/hal/audio_config_macos.h>
#else
#error "No audio hardware configuration selected."
#endif

// logical mapping
static int inputChannelMap[NUM_INPUT_CHANNELS] = {
    INPUT_IN1, INPUT_IN2, INPUT_IN3, INPUT_IN4, INPUT_G1, INPUT_G2, INPUT_G3, INPUT_G4, INPUT_A1, INPUT_B1, INPUT_C1, INPUT_D1,
    INPUT_A2, INPUT_B2, INPUT_C2, INPUT_D2, INPUT_A3, INPUT_B3, INPUT_C3, INPUT_D3};
static int outputChannelMap[NUM_OUTPUT_CHANNELS] = {0, 1, 2, 3};

static int Audio_mapOutputChannel(uint32_t logicalChannel, uint32_t playbackChannels)
{
  if (logicalChannel >= (uint32_t)NUM_OUTPUT_CHANNELS)
  {
    return -1;
  }

  int routedChannel = outputChannelMap[logicalChannel];
  if (routedChannel < 0)
  {
    return -1;
  }

  if ((uint32_t)routedChannel >= (uint32_t)kAudioOutCh)
  {
    return -1;
  }

  int hwChannel = kOutChMap[routedChannel];
  if (hwChannel < 0 || (uint32_t)hwChannel >= playbackChannels)
  {
    return -1;
  }

  return hwChannel;
}

static void Audio_buildInputRoutingMap(void)
{
  for (uint32_t i = 0; i < MAX_CAPTURE_CHANNELS; i++)
  {
    local.inputRoutingMap[i] = -1;
  }

  uint32_t logicalCount = kAudioInCh;
  if (logicalCount > (uint32_t)NUM_INPUT_CHANNELS)
  {
    logicalCount = (uint32_t)NUM_INPUT_CHANNELS;
  }

  for (uint32_t logicalJack = 0; logicalJack < logicalCount; logicalJack++)
  {
    int hwCh = kInChMap[logicalJack];
    int destCh = inputChannelMap[logicalJack];
    if (hwCh >= 0 && (uint32_t)hwCh < MAX_CAPTURE_CHANNELS &&
        destCh >= 0 && (uint32_t)destCh < NUM_INPUT_CHANNELS)
    {
      local.inputRoutingMap[(uint32_t)hwCh] = destCh;
    }
  }
}

static bool hasSubstring(const char *text, const char *needle)
{
  if (!text || !needle)
  {
    return false;
  }
  return strstr(text, needle) != NULL;
}

static unsigned int findDeviceByNameMatch(rtaudio_t audio,
                                          const char *needle,
                                          bool requireInput,
                                          bool requireOutput)
{
  int n = rtaudio_device_count(audio);
  if (n <= 0)
  {
    logError("Audio: no devices available for match '%s' (api=%s).",
             needle ? needle : "<null>",
             rtaudio_api_display_name(rtaudio_current_api(audio)));
    return 0;
  }
  for (int i = 0; i < n; i++)
  {
    unsigned int id = rtaudio_get_device_id(audio, i);
    rtaudio_device_info_t info = rtaudio_get_device_info(audio, id);
    if (!hasSubstring(info.name, needle))
    {
      continue;
    }
    if (requireInput && info.input_channels == 0)
    {
      continue;
    }
    if (requireOutput && info.output_channels == 0)
    {
      continue;
    }
    return id;
  }

  return 0;
}

static int audioCallback(void *outputBuffer, void *inputBuffer,
                         unsigned int nFrames, double streamTime,
                         rtaudio_stream_status_t status, void *userdata)
{
  (void)streamTime;
  (void)status;
  (void)userdata;

#if defined(__linux__)
  if (local.debugFrameCounter == 0)
  {
    Audio_pinCurrentThreadToCore(1, "audio");
  }
#endif
  local.debugFrameCounter++;

  uint32_t frameLength = nFrames;
  if (frameLength > MAX_AUDIO_FRAME_LENGTH)
  {
    frameLength = MAX_AUDIO_FRAME_LENGTH;
  }

  if (inputBuffer)
  {
    const int *src = (const int *)inputBuffer;
    uint32_t captureChannels = local.inputChannels;
    if (captureChannels > MAX_CAPTURE_CHANNELS)
    {
      captureChannels = MAX_CAPTURE_CHANNELS;
    }

    uint32_t mappedChannels = captureChannels;
    memset(local.captureMapped, 0, sizeof(int) * frameLength * NUM_INPUT_CHANNELS);
    for (uint32_t i = 0; i < frameLength; i++)
    {
      uint32_t srcBase = i * captureChannels;
      uint32_t dstBase = i * NUM_INPUT_CHANNELS;
      for (uint32_t ch = 0; ch < mappedChannels; ch++)
      {
        int destCh = local.inputRoutingMap[ch];
        if (destCh >= 0 && (uint32_t)destCh < NUM_INPUT_CHANNELS)
        {
          local.captureMapped[dstBase + (uint32_t)destCh] = src[srcBase + ch];
        }
      }
    }
    SspModulation_ingestInterleavedS32(local.captureMapped, frameLength, NUM_INPUT_CHANNELS);
  }
  else
  {
    SspModulation_ingestInterleavedS32(NULL, frameLength, 0);
  }

  SspModulation_copyFrame(local.inFrame, frameLength);

  memset(local.outFrame, 0, sizeof(float) * frameLength * NUM_OUTPUT_CHANNELS);
  Pump_callback(local.inFrame, local.outFrame);

  if (outputBuffer)
  {
    int *out = (int *)outputBuffer;
    uint32_t outChannels = local.outputChannels;
    if (outChannels > MAX_PLAYBACK_CHANNELS)
    {
      outChannels = MAX_PLAYBACK_CHANNELS;
    }

    memset(out, 0, frameLength * outChannels * sizeof(int));
    for (uint32_t i = 0; i < frameLength; i++)
    {
      for (uint32_t c = 0; c < NUM_OUTPUT_CHANNELS; c++)
      {
        int dstCh = Audio_mapOutputChannel(c, outChannels);
        if (dstCh >= 0)
        {
          float x = local.outFrame[NUM_OUTPUT_CHANNELS * i + c];
          if (x > 1.0f) x = 1.0f; else if (x < -1.0f) x = -1.0f;
          out[i * outChannels + (uint32_t)dstCh] = (int)(x * AUDIO_SAFE_MAX_OUTPUT_VALUE) << 8;
        }
      }
    }
  }

  return 0;
}

void Audio_init()
{
  memset(&local, 0, sizeof(local));
  local.audio = rtaudio_create(RTAUDIO_API_UNSPECIFIED);
  if (!local.audio)
  {
    logError("Audio: failed to create RtAudio instance.");
    return;
  }

  if (rtaudio_current_api(local.audio) == RTAUDIO_API_DUMMY)
  {
    logError("Audio: RtAudio selected Dummy API; no real audio backend available.");
    rtaudio_destroy(local.audio);
    local.audio = NULL;
    return;
  }

  int n = rtaudio_device_count(local.audio);
  logInfo("Audio: %d device(s) found via %s.", n,
          rtaudio_api_display_name(rtaudio_current_api(local.audio)));
  for (int i = 0; i < n; i++)
  {
    unsigned int id = rtaudio_get_device_id(local.audio, i);
    rtaudio_device_info_t info = rtaudio_get_device_info(local.audio, id);
    logInfo("  [%d] %s  out=%d in=%d", i, info.name,
            info.output_channels, info.input_channels);
  }
}

void Audio_restart(void)
{
  Audio_stop();
  Audio_start();
}

void Audio_start(void)
{
  if (!local.audio)
  {
    return;
  }

  unsigned int outputDevId = rtaudio_get_default_output_device(local.audio);
  unsigned int inputDevId = rtaudio_get_default_input_device(local.audio);

  const char *outputPrefix = NULL;
  const char *inputPrefix = NULL;

  outputPrefix = kAudioOutputDevicePrefix;
  inputPrefix = kAudioInputDevicePrefix;

  if (outputPrefix)
  {
    unsigned int selected = findDeviceByNameMatch(local.audio, outputPrefix, false, true);
    if (selected)
    {
      outputDevId = selected;
    }
    else
    {
      logInfo("Audio: output match '%s' not found, using default device.", outputPrefix);
    }
  }

  if (inputPrefix)
  {
    unsigned int selected = findDeviceByNameMatch(local.audio, inputPrefix, true, false);
    if (selected)
    {
      inputDevId = selected;
    }
    else
    {
      logInfo("Audio: input match '%s' not found, using default device.", inputPrefix);
    }
  }

  local.outputDevId = outputDevId;
  local.inputDevId = inputDevId;

  rtaudio_device_info_t outInfo = rtaudio_get_device_info(local.audio, outputDevId);
  rtaudio_device_info_t inInfo = rtaudio_get_device_info(local.audio, inputDevId);

  local.outputChannels = outInfo.output_channels;
  if (local.outputChannels > MAX_PLAYBACK_CHANNELS)
  {
    local.outputChannels = MAX_PLAYBACK_CHANNELS;
  }
  local.inputChannels = inInfo.input_channels;
  if (local.inputChannels > MAX_CAPTURE_CHANNELS)
  {
    local.inputChannels = MAX_CAPTURE_CHANNELS;
  }

  Audio_buildInputRoutingMap();

  logInfo("Audio: output device '%s' (%d ch)", outInfo.name, local.outputChannels);
  logInfo("Audio: input  device '%s' (%d ch)", inInfo.name, local.inputChannels);

  rtaudio_stream_parameters_t outParams;
  memset(&outParams, 0, sizeof(outParams));
  outParams.device_id = local.outputDevId;
  outParams.num_channels = local.outputChannels;
  outParams.first_channel = 0;

  rtaudio_stream_parameters_t inParams;
  memset(&inParams, 0, sizeof(inParams));
  inParams.device_id = local.inputDevId;
  inParams.num_channels = local.inputChannels;
  inParams.first_channel = 0;

  unsigned int bufferFrames = currentFrameLength();

  rtaudio_error_t err = rtaudio_open_stream(
      local.audio,
      &outParams,
      local.inputChannels > 0 ? &inParams : NULL,
      RTAUDIO_FORMAT_SINT32,
      currentSampleRate(),
      &bufferFrames,
      audioCallback,
      NULL,
      NULL,
      NULL);

  if (err != RTAUDIO_ERROR_NONE)
  {
    logError("Audio: failed to open stream: %s", rtaudio_error(local.audio));
    return;
  }

  err = rtaudio_start_stream(local.audio);
  if (err != RTAUDIO_ERROR_NONE)
  {
    logError("Audio: failed to start stream: %s", rtaudio_error(local.audio));
  }
  else
  {
    logInfo("Audio: stream started %dHz %d-frame %d-out %d-in",
          currentSampleRate(), bufferFrames,
            local.outputChannels, local.inputChannels);
  }
}

void Audio_stop(void)
{
  if (!local.audio)
  {
    return;
  }
  if (rtaudio_is_stream_running(local.audio))
  {
    rtaudio_stop_stream(local.audio);
  }
  if (rtaudio_is_stream_open(local.audio))
  {
    rtaudio_close_stream(local.audio);
  }
}

int Audio_getRate(void)
{
  return (int)currentSampleRate();
}

void Audio_printErrorStatus(void)
{
  if (!local.audio)
  {
    logInfo("audio: no instance");
    return;
  }
  if (rtaudio_is_stream_running(local.audio))
  {
    logInfo("audio playing");
  }
  else if (rtaudio_is_stream_open(local.audio))
  {
    logInfo("audio stopped");
  }
  else
  {
    logInfo("audio closed");
  }
}

int Audio_getLoad()
{
  return 0;
}

bool Audio_running()
{
  return local.audio && rtaudio_is_stream_running(local.audio);
}