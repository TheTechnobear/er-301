#pragma once

#include <percussa/ui/Geometry.h>
#include <percussa/ui/olive_bridge.h>

#include <string>

namespace percussa
{
  namespace ui
  {
    class DisplayWidget
    {
    public:
      DisplayWidget(const std::string &text, const Rect &rect, const std::string &roleName) :
        label(text),
        bounds(rect),
        role(roleName)
      {
      }

      virtual ~DisplayWidget()
      {
      }

      virtual void render(Olivec_Canvas canvas, const uint8_t *frame) const;

      std::string label;
      Rect bounds;
      std::string role;

    protected:
      void renderGeneric(Olivec_Canvas canvas) const;
      void renderFrame(Olivec_Canvas canvas, const uint8_t *frame) const;

      virtual int sourceWidth() const;
      virtual int sourceHeight() const;
      virtual int pixelBrightness(const uint16_t *src, int srcX, int srcY) const;
    };
  }
}