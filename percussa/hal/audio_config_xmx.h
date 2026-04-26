#pragma once

/* XMX hardware channel mapping (Rockchip RK3568, aarch64) */

#define kAudioInCh  8
#define kAudioOutCh 2

static int kInChMap[kAudioInCh]  = {0, 1, 2, 3, 4, 5, 6, 7};
static int kOutChMap[kAudioOutCh] = {0, 1};

static const char *const kAudioOutputDevicePrefix = "rockchip";
static const char *const kAudioInputDevicePrefix  = "rockchip";
