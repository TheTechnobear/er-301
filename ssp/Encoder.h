#pragma once

#include <array>
#include <stdint.h>
#include <string>

#include <hal/gpio.h>
#include <ssp/olive_bridge.h>

namespace ssp
{
  struct Encoder
  {
    Encoder(const char *label, uint32_t id) : label(label), id(id)
    {
    }

    bool isPressed() const
    {
      return id != 0 && Button_pressed(id);
    }

    void setGeometry(int cx_, int cy_, int r_)
    {
      cx = cx_;
      cy = cy_;
      r = r_;
    }

    bool hasGeometry() const
    {
      return r > 0;
    }

    void render(Olivec_Canvas canvas) const;

    static void applyDefaultLayout(std::array<Encoder, 4> &encoders);

    std::string key;
    std::string label;
    uint32_t id;
    int cx = 0;
    int cy = 0;
    int r = 0;
  };
}
