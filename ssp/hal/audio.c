#include <SDL2/SDL.h>
#include <hal/audio.h>
#include <hal/log.h>
#include <od/config.h>

// eqiv of ../../arch/am335x/hal/audio.c

// Called on each frame in the ADC thread.
//  extern void Adc_callback(int *samples);

// ./hal/adc.h:  extern void Adc_callback(int *samples);
// ./hal/pump/pump.cpp:void Adc_callback(int *samples)
// ./testing/darwin/ssp/od/glue/app_swig.cpp:static int _wrap_Adc_callback(lua_State* L) { { int SWIG_arg = 0; int *arg1 = 0 ; SWIG_check_num_args("Adc_callback",1,1)
// ./testing/darwin/ssp/od/glue/app_swig.cpp:    if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("Adc_callback",1,"int *");
// ./testing/darwin/ssp/od/glue/app_swig.cpp:    if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_int,0))){ SWIG_fail_ptr("Adc_callback",1,SWIGTYPE_p_int); } 
// ./testing/darwin/ssp/od/glue/app_swig.cpp:    Adc_callback(arg1); return SWIG_arg; fail: SWIGUNUSED; }  lua_error(L); return 0; }
// ./testing/darwin/ssp/od/glue/app_swig.cpp:    { "Adc_callback", _wrap_Adc_callback},
// ./arch/am335x/hal/adc.c:        Adc_callback(self.ping);
// ./arch/am335x/hal/adc.c:        Adc_callback(self.pong);


#define MAX_AUDIO_BUFFER_BYTES (AUDIO_NUM_CHANNELS * MAX_AUDIO_FRAME_LENGTH * sizeof(int))

static struct AudioLocals {
  int buffer[CACHE_ALIGNED_SIZE(MAX_AUDIO_BUFFER_BYTES) / sizeof(int)] __attribute__((aligned(CACHELINE_SIZE_MAX)));
  SDL_AudioSpec playSpec;
  SDL_AudioDeviceID playDev;
  char devName[32];
} local;

void playCallback(void *userdata, Uint8 *stream, int len) {
  // Generate audio.
  Audio_callback(local.buffer);

  // Convert 24-bit to 32-bit
  int *out = (int *)stream;
  if (local.playSpec.channels < 4) {
    // mix down to stereo.
    for (int i = 0; i < FRAMELENGTH; i++) {
      // left channel = OUT1 + OUT3
      out[2 * i] = (local.buffer[4 * i] + local.buffer[4 * i + 2]) << 7;
      // right channel = OUT2 + OUT4
      out[2 * i + 1] = (local.buffer[4 * i + 1] + local.buffer[4 * i + 3]) << 7;
    }
  } else {
    for (int i = 0; i < FRAMELENGTH; i++) {
      for (int c = 0; c < 4; c++) {
        out[2 * i + c] = local.buffer[4 * i + c] << 7;
      }
    }
  }
}

void Audio_init() {
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  SDL_zero(local);

  for (int i = 0; i < SDL_GetNumAudioDrivers(); ++i) {
    logInfo("Audio driver %d: %s", i, SDL_GetAudioDriver(i));
  }

#if BUILDOPT_FORCE_ALSA
  if (SDL_AudioInit("alsa")) {
    logError("Failed to initialize alsa driver: %s", SDL_GetError());
    logInfo("Opening default driver...");
  }
#endif

  const char *driver_name = SDL_GetCurrentAudioDriver();

  if (driver_name) {
    logInfo("Audio subsystem initialized, driver = %s.", driver_name);
  } else {
    logError("Audio subsystem not initialized.");
  }
}

void Audio_restart(void) {
  Audio_stop();
  SDL_Delay(200);
  Audio_start();
}

void Audio_start(void) {
  bool isCapture = false;
  int numChannels = 2;

#if __APPLE__
  // testing only
  {
    for (int i = 0; i < SDL_GetNumAudioDevices(isCapture); ++i) {
      SDL_AudioSpec spec;
      const char *name = NULL;
      name = SDL_GetAudioDeviceName(i, isCapture);
      if (name && !SDL_GetAudioDeviceSpec(i, isCapture, &spec)) {
        logInfo("Audio Device %s : %d ", name, spec.channels);
        if (strlen(local.devName) == 0 && spec.channels == 4) {
          numChannels = 4;
          strncpy(local.devName, name, 32);
        }
      }
      // if(strcmp(name,"VirtualIn")==0) {

      // }
    }
  }

  if (strlen(local.devName) == 0) {
    // default
    SDL_AudioSpec spec;
    char *name = NULL;
    if (!SDL_GetDefaultAudioInfo(&name, &spec, isCapture)) {
      strncpy(local.devName, name, 32);
      logInfo("Default Audio Device %s : %d ", local.devName, spec.channels);
    }
  }
  logInfo("Using Audio Device %s", local.devName);

#endif // testing apple

  SDL_AudioSpec want;
  SDL_zero(want);
  want.freq = globalConfig.sampleRate;
  want.format = AUDIO_S32;
  want.channels = numChannels;
  want.samples = globalConfig.frameLength;
  want.callback = playCallback;

  local.playDev =
    SDL_OpenAudioDevice(strlen(local.devName) == 0 ? NULL : local.devName, isCapture, &want, &local.playSpec, 0);
  if (local.playDev == 0) {
    logError("Failed to open audio: %s", SDL_GetError());
  } else {
    // note: device Id return is NOT the same as index, 0 = fail, 1 = legacy, after than?
    logInfo("Audio Specs %dHz %dch", local.playSpec.freq, local.playSpec.channels);
    SDL_PauseAudioDevice(local.playDev, 0); /* start audio playing. */
  }
}

void Audio_stop(void) { SDL_CloseAudioDevice(local.playDev); }

uint32_t Audio_errorCount(void) { return 0; }

int Audio_getRate(void) { return globalConfig.sampleRate; }

void Audio_printErrorStatus(void) {
  switch (SDL_GetAudioDeviceStatus(local.playDev)) {
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