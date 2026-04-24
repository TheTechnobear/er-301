#pragma once

#include <hal/gpio.h>

inline bool percussa_state_read(uint32_t id)
{
  return Gpio_read(id);
}

inline void percussa_state_write(uint32_t id, bool value)
{
  Gpio_write(id, value);
}