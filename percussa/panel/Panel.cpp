#include <percussa/panel/Panel.h>

#if defined(TARGET_SSP)
#include <percussa/panel/ssp/SspPanel.h>
#elif defined(TARGET_XMX)
#include <percussa/panel/xmx/XmxPanel.h>
#else
#error "No percussa panel selected at build time."
#endif

namespace percussa
{
  namespace panel
  {
    std::unique_ptr<Panel> createPanel()
    {
#if defined(TARGET_SSP)
      return std::unique_ptr<Panel>(new ssp::SspPanel());
#elif defined(TARGET_XMX)
      return std::unique_ptr<Panel>(new xmx::XmxPanel());
#endif
    }
  } // namespace panel
} // namespace percussa