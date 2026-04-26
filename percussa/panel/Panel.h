#pragma once

#include <percussa/ui/DisplayWidget.h>
#include <percussa/ui/ButtonWidget.h>
#include <percussa/ui/EncoderWidget.h>
#include <percussa/ui/LedWidget.h>
#include <percussa/ui/ToggleWidget.h>
#include <percussa/ui/olive_bridge.h>

#include <memory>
#include <vector>

namespace percussa
{
  namespace panel
  {
    class Controller;

    class Panel
    {
    public:
      virtual ~Panel()
      {
      }

      virtual int width() const = 0;
      virtual int height() const = 0;
      virtual void render(Olivec_Canvas canvas) const = 0;
      virtual std::unique_ptr<Controller> createController() = 0;
    };

    std::unique_ptr<Panel> createPanel();
  }
}