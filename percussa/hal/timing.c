#include <hal/timing.h>
#include <hal/log.h>

#include <sys/time.h>
#include <time.h>

typedef struct
{
  tick_t offset;
} Local;

static Local local;

static tick_t monotonicTicks(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (tick_t)ts.tv_sec * 1000000000LL + (tick_t)ts.tv_nsec;
}

tick_t ticks(void)
{
  return monotonicTicks();
}

double ticks2secsD(tick_t nticks)
{
  return (double)nticks / 1000000000.0;
}

float ticks2secs(tick_t nticks)
{
  return (float)nticks / 1000000000.0f;
}

float wallclock(void)
{
  tick_t t = monotonicTicks() - local.offset;
  return (float)t / 1000000000.0f;
}

void wallclock_reset(void)
{
  local.offset = monotonicTicks();
}

unsigned long micros(void)
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return (unsigned long)tv.tv_sec * 1000000UL + (unsigned long)tv.tv_usec;
}

void Timing_init(void)
{
  logInfo("--------Timing Init--------------");
  wallclock_reset();
  logInfo("Tick Frequency: %llu Hz", 1000000000ULL);
  logInfo("Tick Period: %d ns", (int)(ticks2secsD(1) * 1000000000));
  logInfo("---------------------------------");
}