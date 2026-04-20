#pragma once

#include <ssp/constants.h>
#include <stdint.h>
#include <string>
#include <hal/gpio.h>

namespace ssp
{
  class Window;
  struct Led
  {
    Led(const char *label, uint32_t id) : label(label), id(id) {};

    int state()
    {
      return Gpio_read(id);
    }

    std::string label;
    uint32_t id;
  };

  struct RedLed : public Led
  {
    RedLed(const char *label, uint32_t id) : Led(label, id)
    {
    }
  };

  struct OrangeLed : public Led
  {
    OrangeLed(uint32_t id) : Led("", id)
    {
    }
  };

} // namespace ssp