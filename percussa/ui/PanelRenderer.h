#pragma once

#include <percussa/ui/PresentationState.h>

#include <string>
#include <stdint.h>

#include <vector>

namespace percussa
{
  namespace panel
  {
    class Panel;
  }

  namespace ui
  {
    struct RenderedPanel
    {
      int width = 0;
      int height = 0;
      std::vector<uint32_t> pixels;
    };

    class PanelRenderer
    {
    public:
      RenderedPanel render(const panel::Panel &panel, const PresentationState &state) const;
    };
  }
}