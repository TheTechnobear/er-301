#include <percussa/panel/Family.h>

#if defined(PERCUSSA_PANEL_SSP)
#include <percussa/panel/ssp/SspController.h>
#include <percussa/panel/ssp/SspPanel.h>
#elif defined(PERCUSSA_PANEL_XMX)
#include <percussa/panel/xmx/XmxController.h>
#include <percussa/panel/xmx/XmxPanel.h>
#else
#error "No percussa panel selected at build time."
#endif

namespace percussa
{
  namespace panel
  {
    Family createFamily()
    {
      Family family;

#if defined(PERCUSSA_PANEL_SSP)
      std::unique_ptr<ssp::SspPanel> sspPanel(new ssp::SspPanel());
      family.controller.reset(new ssp::SspController(*sspPanel));
      family.panel = std::move(sspPanel);
#elif defined(PERCUSSA_PANEL_XMX)
      std::unique_ptr<xmx::XmxPanel> xmxPanel(new xmx::XmxPanel());
      family.controller.reset(new xmx::XmxController(*xmxPanel));
      family.panel = std::move(xmxPanel);
#endif

      return family;
    }
  }
}