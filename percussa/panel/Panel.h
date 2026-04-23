#pragma once

#include <percussa/ui/DisplayWidget.h>
#include <percussa/ui/ButtonWidget.h>
#include <percussa/ui/EncoderWidget.h>
#include <percussa/ui/LedWidget.h>
#include <percussa/ui/ToggleWidget.h>

#include <vector>

namespace percussa
{
  namespace panel
  {
    class Panel
    {
    public:
      virtual ~Panel()
      {
      }

      virtual const char *name() const = 0;
      virtual int width() const = 0;
      virtual int height() const = 0;

      virtual const std::vector<ui::DisplayWidget> &displays() const = 0;
      virtual const std::vector<ui::ButtonWidget> &buttons() const = 0;
      virtual const std::vector<ui::EncoderWidget> &encoders() const = 0;
      virtual const std::vector<ui::LedWidget> &leds() const = 0;
      virtual const std::vector<ui::ToggleWidget> &toggles() const = 0;
    };
  }
}