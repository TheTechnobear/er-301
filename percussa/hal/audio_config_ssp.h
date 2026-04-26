#pragma once

/* SSP hardware channel mapping (ak4458 codec, armv7 Rockchip RK3188) */

#define kAudioInCh  16
#define kAudioOutCh 8

static int kInChMap[kAudioInCh]  = {11, 10, 9, 8, 15, 14, 13, 12, 3, 2, 1, 0, 7, 6, 5, 4};
static int kOutChMap[kAudioOutCh] = {3, 2, 1, 0, 7, 6, 5, 4};

static const char *const kAudioOutputDevicePrefix = "ak4458";
static const char *const kAudioInputDevicePrefix  = "ak4458";

static const char *const kUSBAudioOutputDevicePrefix = "UAC2_Gadget";
static const char *const kUSBAudioInputDevicePrefix  = "UAC2_Gadget";

static float kInGain = 0.2f / 0.18795f;
static float kOutGain = 5.0f / 5.248f;
static float kInOffset = 0.02300f;
static float kOutOffset = 0.f;
static float kNonAudioInGain = 0.5f;
