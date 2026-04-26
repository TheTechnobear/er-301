#include <percussa/ui/PanelRenderer.h>

#include <percussa/panel/Panel.h>

namespace percussa
{
  namespace ui
  {
    RenderedPanel PanelRenderer::render(const panel::Panel &panel) const
    {
      RenderedPanel rendered(panel.width(), panel.height());

      Olivec_Canvas canvas =
        olivec_canvas(rendered.data(), (size_t)rendered.width(), (size_t)rendered.height(), (size_t)rendered.width());

      panel.render(canvas);

      return rendered;
    }
  } // namespace ui
} // namespace percussa