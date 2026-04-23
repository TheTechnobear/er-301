#include <percussa/ui/PanelRenderer.h>

#include <percussa/panel/Panel.h>
#include <percussa/ui/DisplayWidget.h>
#include <percussa/ui/PresentationState.h>
#include <percussa/ui/olive_bridge.h>

#include <algorithm>
#include <cctype>
#include <string>

namespace percussa
{
  namespace ui
  {
    namespace
    {
      const uint32_t kBackground = PERCUSSA_RGBA(31, 34, 40, 255);
      const uint32_t kPanelFace = PERCUSSA_RGBA(54, 58, 67, 255);
      const uint32_t kPanelEdge = PERCUSSA_RGBA(14, 16, 19, 255);
      const uint32_t kDisplayFace = PERCUSSA_RGBA(9, 10, 12, 255);
      const uint32_t kDisplayGlow = PERCUSSA_RGBA(24, 19, 8, 255);
      const uint32_t kButtonShell = PERCUSSA_RGBA(15, 16, 19, 255);
      const uint32_t kButtonFace = PERCUSSA_RGBA(35, 38, 46, 255);
      const uint32_t kButtonText = PERCUSSA_RGBA(245, 184, 40, 255);
      const uint32_t kStatusFace = PERCUSSA_RGBA(18, 20, 24, 255);
      const uint32_t kEncoderShell = PERCUSSA_RGBA(12, 13, 15, 255);
      const uint32_t kEncoderFace = PERCUSSA_RGBA(217, 221, 230, 255);
      const uint32_t kEncoderRim = PERCUSSA_RGBA(92, 98, 116, 255);
      const uint32_t kLedOff = PERCUSSA_RGBA(25, 20, 18, 255);
      const uint32_t kRedLed = PERCUSSA_RGBA(255, 62, 48, 255);
      const uint32_t kAmberLed = PERCUSSA_RGBA(255, 176, 36, 255);
      const uint32_t kToggleLine = PERCUSSA_RGBA(106, 112, 126, 255);

      std::string upper(const std::string &value)
      {
        std::string result = value;
        for (size_t i = 0; i < result.size(); ++i)
        {
          result[i] = (char)std::toupper((unsigned char)result[i]);
        }
        return result;
      }

      void drawCenteredText(Olivec_Canvas canvas, const std::string &text, const Rect &rect, int size, uint32_t color)
      {
        int textWidth = 0;
        int textHeight = 0;
        percussa_olive_text_metrics(text.c_str(), size, &textWidth, &textHeight);
        int x = rect.x + (rect.w - textWidth) / 2;
        int y = rect.y + (rect.h - textHeight) / 2;
        olivec_text(canvas, text.c_str(), x, y, percussa_olive_default_font(), (size_t)size, color);
      }

      void drawButton(Olivec_Canvas canvas, const ButtonWidget &button, bool active)
      {
        olivec_rect(canvas, button.bounds.x, button.bounds.y, button.bounds.w, button.bounds.h, kButtonShell);
        olivec_rect(canvas,
                    button.bounds.x + 3,
                    button.bounds.y + 3,
                    button.bounds.w - 6,
                    button.bounds.h - 6,
                    active ? kButtonText : kButtonFace);
        olivec_frame(canvas, button.bounds.x, button.bounds.y, button.bounds.w, button.bounds.h, 2, kPanelEdge);
        drawCenteredText(canvas, upper(button.label), button.bounds, 2, active ? kPanelEdge : kButtonText);
      }

      void drawDisplay(Olivec_Canvas canvas, const DisplayWidget &display, const DisplayState *state)
      {
        olivec_rect(canvas, display.bounds.x, display.bounds.y, display.bounds.w, display.bounds.h, kDisplayGlow);
        olivec_rect(canvas, display.bounds.x + 4, display.bounds.y + 4, display.bounds.w - 8, display.bounds.h - 8, kDisplayFace);
        olivec_frame(canvas, display.bounds.x, display.bounds.y, display.bounds.w, display.bounds.h, 2, kToggleLine);

        Rect labelRect(display.bounds.x + 8, display.bounds.y + 6, display.bounds.w - 16, 14);
        drawCenteredText(canvas, upper(display.label), labelRect, 1, kButtonText);

        if (state)
        {
          Rect titleRect(display.bounds.x + 8, display.bounds.y + 24, display.bounds.w - 16, 18);
          Rect line1Rect(display.bounds.x + 8, display.bounds.y + display.bounds.h / 2 - 14, display.bounds.w - 16, 18);
          Rect line2Rect(display.bounds.x + 8, display.bounds.y + display.bounds.h / 2 + 10, display.bounds.w - 16, 18);
          drawCenteredText(canvas, upper(state->title), titleRect, 1, kButtonText);
          drawCenteredText(canvas, upper(state->line1), line1Rect, 2, kButtonText);
          drawCenteredText(canvas, upper(state->line2), line2Rect, 1, kButtonText);
        }
      }

      void drawEncoder(Olivec_Canvas canvas, const EncoderWidget &encoder)
      {
        int cx = encoder.bounds.x + encoder.bounds.w / 2;
        int cy = encoder.bounds.y + encoder.bounds.h / 2;
        int r = std::min(encoder.bounds.w, encoder.bounds.h) / 2;
        olivec_circle(canvas, cx, cy, r, kEncoderShell);
        olivec_circle(canvas, cx, cy, r - 4, kEncoderFace);
        olivec_circle(canvas, cx, cy, r - 10, kEncoderRim);

        Rect labelRect(encoder.bounds.x - 10, encoder.bounds.y + encoder.bounds.h + 6, encoder.bounds.w + 20, 16);
        drawCenteredText(canvas, upper(encoder.label), labelRect, 1, kButtonText);
      }

      void drawLed(Olivec_Canvas canvas, const LedWidget &led, bool active)
      {
        int radius = std::max(3, std::min(led.bounds.w, led.bounds.h) / 2);
        int cx = led.bounds.x + radius;
        int cy = led.bounds.y + led.bounds.h / 2;
        uint32_t color = led.color == "amber" ? kAmberLed : kRedLed;
        olivec_circle(canvas, cx, cy, radius, kLedOff);
        olivec_circle(canvas, cx, cy, std::max(1, radius - 2), active ? color : kButtonFace);
        Rect textRect(led.bounds.x + radius * 2 + 6, led.bounds.y, led.bounds.w + 50, led.bounds.h);
        drawCenteredText(canvas, upper(led.label), textRect, 1, kButtonText);
      }

      void drawToggle(Olivec_Canvas canvas, const ToggleWidget &toggle, int position)
      {
        olivec_frame(canvas, toggle.bounds.x, toggle.bounds.y, toggle.bounds.w, toggle.bounds.h, 1, kToggleLine);

        Rect titleRect(toggle.bounds.x, toggle.bounds.y + 2, toggle.bounds.w, 12);
        drawCenteredText(canvas, upper(toggle.label), titleRect, 1, kButtonText);

        const char *rows[3] = { toggle.low.c_str(), toggle.mid.c_str(), toggle.high.c_str() };
        for (int i = 0; i < 3; ++i)
        {
          int y = toggle.bounds.y + 18 + i * 12;
          olivec_line(canvas, toggle.bounds.x + 10, y + 5, toggle.bounds.x + toggle.bounds.w - 10, y + 5, kToggleLine);
          Rect rowRect(toggle.bounds.x + 20, y, toggle.bounds.w - 24, 10);
          if (i == position)
          {
            olivec_rect(canvas, rowRect.x - 4, rowRect.y - 1, rowRect.w + 8, rowRect.h + 2, kStatusFace);
          }
          drawCenteredText(canvas, upper(rows[i]), rowRect, 1, kButtonText);
        }
      }

      void drawStatus(Olivec_Canvas canvas, int width, int height, const std::string &statusText)
      {
        Rect statusRect(16, height - 32, width - 32, 18);
        olivec_rect(canvas, statusRect.x, statusRect.y, statusRect.w, statusRect.h, kStatusFace);
        olivec_frame(canvas, statusRect.x, statusRect.y, statusRect.w, statusRect.h, 1, kToggleLine);
        drawCenteredText(canvas, upper(statusText), statusRect, 1, kButtonText);
      }
    }

    RenderedPanel PanelRenderer::render(const panel::Panel &panel, const PresentationState &state) const
    {
      RenderedPanel rendered;
      rendered.width = panel.width();
      rendered.height = panel.height();
      rendered.pixels.resize((size_t)rendered.width * (size_t)rendered.height);

      Olivec_Canvas canvas = olivec_canvas(&rendered.pixels[0], (size_t)rendered.width, (size_t)rendered.height, (size_t)rendered.width);
      olivec_fill(canvas, kBackground);
      olivec_rect(canvas, 8, 8, rendered.width - 16, rendered.height - 16, kPanelFace);
      olivec_frame(canvas, 8, 8, rendered.width - 16, rendered.height - 16, 2, kPanelEdge);

      const std::vector<DisplayWidget> &displays = panel.displays();
      for (size_t i = 0; i < displays.size(); ++i)
      {
        const DisplayState *displayState = i < state.displays.size() ? &state.displays[i] : 0;
        drawDisplay(canvas, displays[i], displayState);
      }

      const std::vector<ButtonWidget> &buttons = panel.buttons();
      for (size_t i = 0; i < buttons.size(); ++i)
      {
        if (!buttons[i].visible)
        {
          continue;
        }
        bool active = i < state.buttonActive.size() ? state.buttonActive[i] : false;
        drawButton(canvas, buttons[i], active);
      }

      const std::vector<EncoderWidget> &encoders = panel.encoders();
      for (size_t i = 0; i < encoders.size(); ++i)
      {
        if (!encoders[i].visible)
        {
          continue;
        }
        drawEncoder(canvas, encoders[i]);
      }

      const std::vector<LedWidget> &leds = panel.leds();
      for (size_t i = 0; i < leds.size(); ++i)
      {
        bool active = i < state.ledActive.size() ? state.ledActive[i] : false;
        drawLed(canvas, leds[i], active);
      }

      const std::vector<ToggleWidget> &toggles = panel.toggles();
      for (size_t i = 0; i < toggles.size(); ++i)
      {
        int position = i < state.togglePositions.size() ? state.togglePositions[i] : 1;
        drawToggle(canvas, toggles[i], position);
      }

      drawStatus(canvas, rendered.width, rendered.height, state.statusText);

      return rendered;
    }
  }
}