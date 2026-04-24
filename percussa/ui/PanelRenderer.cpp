#include <percussa/ui/PanelRenderer.h>

#include <percussa/panel/Panel.h>

namespace percussa
{
  namespace ui
  {
    RenderedPanel PanelRenderer::render(const panel::Panel &panel) const
    {
      RenderedPanel rendered;
      rendered.width = panel.width();
      rendered.height = panel.height();
      rendered.pixels.resize((size_t)rendered.width * (size_t)rendered.height);

      Olivec_Canvas canvas = olivec_canvas(&rendered.pixels[0], (size_t)rendered.width, (size_t)rendered.height, (size_t)rendered.width);

      panel.render(canvas);

      return rendered;
    }
  }
}