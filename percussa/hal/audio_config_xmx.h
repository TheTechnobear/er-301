#pragma once

/* XMX hardware channel mapping (Rockchip RK3568, aarch64) */

#define kAudioInCh  8
#define kAudioOutCh 2

static int kInChMap[kAudioInCh]  = {0, 1, 2, 3, 4, 5, 6, 7};
static int kOutChMap[kAudioOutCh] = {0, 1};

static const char *const kAudioOutputDevicePrefix = "rockchip";
static const char *const kAudioInputDevicePrefix  = "rockchip";

static const char *const kUSBAudioOutputDevicePrefix = "UAC2_Gadget";
static const char *const kUSBAudioInputDevicePrefix  = "UAC2_Gadget";

static float kInGain = (0.2f/(0.9574f-0.7650f)) * -1.0f;
static float kOutGain = -1.0f/1.02f;
static float kInOffset = -0.00014f;
// static float kOutOffset = 0.088f*0.2f;
static float kOutOffset = 0.f;
static float kNonAudioInGain = 0.5f;
