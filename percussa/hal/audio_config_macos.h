#pragma once

/* macOS host: identity channel mapping for virtual CoreAudio device.
   kAudioInCh / kAudioOutCh must match NUM_INPUT_CHANNELS / NUM_OUTPUT_CHANNELS
   from hal/channels.h, which audio.c already includes before this header. */

#define kAudioInCh  20
#define kAudioOutCh 4

static int kInChMap[kAudioInCh]  = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
static int kOutChMap[kAudioOutCh] = {0, 1, 2, 3};

static const char *const kAudioOutputDevicePrefix = "Virtual-SSP-Out";
static const char *const kAudioInputDevicePrefix  = "Virtual-SSP-In";
