#pragma once

#include <SDL2/SDL.h>
#include <stdint.h>
#include <string>
#include <hal/gpio.h>

namespace ssp
{

  class Window;
  struct Button
  {
    Button(const char *label, uint32_t id) : label(label), id(id)
    {
    }

    bool isPressed()
    {
      return Button_pressed(id);
    }

    std::string key;
    std::string label;
    uint32_t id;
  };


} // namespace ssp