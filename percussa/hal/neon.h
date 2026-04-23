#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __ARM_NEON
#include <arm_neon.h>
#else
#error "Percussa pump SIMD currently requires NEON-compatible targets."
#endif

#ifdef __cplusplus
}
#endif