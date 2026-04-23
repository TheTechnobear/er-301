#pragma once

#include <percussa/panel/Controller.h>
#include <percussa/panel/Panel.h>

#include <memory>

namespace percussa
{
  namespace panel
  {
    struct Family
    {
      std::unique_ptr<Panel> panel;
      std::unique_ptr<Controller> controller;
    };

    Family createFamily();
  }
}