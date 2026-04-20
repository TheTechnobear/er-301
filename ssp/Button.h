#pragma once

#include <stdint.h>
#include <string>
#include <array>
#include <hal/gpio.h>
#include <ssp/olive_bridge.h>

namespace ssp
{

  class Window;
  struct Button
  {
    Button(const char *label, uint32_t id) : label(label), id(id)
    {
    }

    bool isPressed() const
    {
      return Button_pressed(id);
    }

    void setGeometry(int x_, int y_, int w_, int h_)
    {
      x = x_;
      y = y_;
      w = w_;
      h = h_;
    }

    bool hasGeometry() const
    {
      return w > 0 && h > 0;
    }

    void render(Olivec_Canvas canvas) const;

    static void applyDefaultRectLayout(std::array<Button, 19> &buttons);

    std::string key;
    std::string label;
    uint32_t id;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
  };


} // namespace ssp