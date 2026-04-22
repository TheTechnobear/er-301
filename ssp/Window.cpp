#include "constants.h"
#include <ssp/Window.h>
#include <ssp/olive_bridge.h>
#include <hal/log.h>
#include <hal/timing.h>

#ifndef SSP_USE_SDL
#if defined(__APPLE__)
#define SSP_USE_SDL 1
#else
#define SSP_USE_SDL 0
#endif
#endif

namespace ssp
{
  static inline uint32_t grayColor(int value)
  {
    return SSP_RGBA(value, (int)(value * SCREEN_TINT), 0, 255);
  }

  static inline void renderPanelOverlay(Window &window, Olivec_Canvas canvas)
  {
    // SSP panel.
    // olivec_rect(canvas, 0, 0, SCREEN_WIDTH - 150, SCREEN_HEIGHT - 180 , SSP_RGBA(5, 5, 5, 255));

    for (const Encoder &encoder : window.encoders)
    {
      encoder.render(canvas);
    }

    for (const Button &button : window.buttons)
    {
      button.render(canvas);
    }
  }

  Window::Window()
  {
#if SSP_USE_SDL
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

    window = SDL_CreateWindow(
      "ER-301 on SSP", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
      logFatal("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
      logFatal("Renderer could not be created! SDL Error: %s", SDL_GetError());
    }

    windowTexture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (windowTexture == NULL)
    {
      logFatal("Failed to create window texture: %s", SDL_GetError());
    }
#else
    framebuffer.reset(new HardwareFramebuffer(SCREEN_WIDTH, SCREEN_HEIGHT));
    if (!framebuffer->init())
    {
      logFatal("Failed to initialize SSP hardware framebuffer.");
    }
#endif

    windowBuffer.resize(SCREEN_WIDTH * SCREEN_HEIGHT, SSP_RGBA(P_BACKGROUND, P_BACKGROUND, P_BACKGROUND, 255));

    Button::applyDefaultRectLayout(buttons);
    Encoder::applyDefaultLayout(encoders);
  }

  Window::~Window()
  {
#if SSP_USE_SDL
    SDL_DestroyTexture(windowTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
#endif
  }

  void Window::renderMainFrame(uint8_t *frame)
  {
    uint16_t *src = (uint16_t *)frame;
    for (int y = 0; y < MAIN_VERTICAL_PIXELS; y++)
    {
      uint32_t *row = &windowBuffer[(MAIN_Y + y) * SCREEN_WIDTH + MAIN_X];
      int yy = MAIN_VERTICAL_PIXELS - y - 1;
      for (int x = 0; x < MAIN_HORIZONTAL_PIXELS; x++)
      {
        int xx = MAIN_HORIZONTAL_PIXELS - x - 1;
        uint16_t cell = *(src + (yy << 7) + (xx >> 1));
        int shift = (((~xx) & 0b1) << 2);
        int value = (cell >> shift) & 0xF;
        value *= SCREEN_BRIGHTNESS;
        row[x] = grayColor(value);
      }
    }
  }

  void Window::renderSubFrame(uint8_t *frame)
  {
    uint16_t *src = (uint16_t *)frame;
    for (int y = 0; y < SUB_VERTICAL_PIXELS; y++)
    {
      uint32_t *row = &windowBuffer[(SUB_Y + y) * SCREEN_WIDTH + SUB_X];
      int yy = SUB_VERTICAL_PIXELS - y - 1;
      int shift = yy & 0b111;
      for (int x = 0; x < SUB_HORIZONTAL_PIXELS; x++)
      {
        int xx = SUB_HORIZONTAL_PIXELS - x - 1;
        uint16_t cell = *(src + ((yy >> 3) << 7) + xx);
        int value = (cell >> shift) & 0b1;
        value *= 0xF * SCREEN_BRIGHTNESS;
        row[x] = grayColor(value);
      }
    }
  }

  void Window::onResized(int w, int h)
  {
#if SSP_USE_SDL
    width = w;
    height = h;
    pauseDisplayTime = 1;
#else
    (void)w;
    (void)h;
#endif
  }

  void Window::update(uint8_t *mainFrame, uint8_t *subFrame)
  {
    static tick_t timestamp = ticks();

    tick_t now = ticks();
    double t = ticks2secsD(now - timestamp);

    if (t < pauseDisplayTime)
    {
#if SSP_USE_SDL
      SDL_RenderPresent(renderer);
#endif
      return;
    }
    pauseDisplayTime = 0;

    Olivec_Canvas canvas = olivec_canvas(windowBuffer.data(), SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH);
    olivec_fill(canvas, SSP_RGBA(P_BACKGROUND, P_BACKGROUND, P_BACKGROUND, 255));

    renderPanelOverlay(*this, canvas);

    renderMainFrame(mainFrame);
    renderSubFrame(subFrame);

  #if SSP_USE_SDL
    SDL_UpdateTexture(windowTexture, NULL, windowBuffer.data(), SCREEN_WIDTH * (int)sizeof(uint32_t));
    SDL_RenderCopy(renderer, windowTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
  #else
    framebuffer->present(windowBuffer.data());
  #endif
  }

  void Window::setPosition(int x, int y, int correction)
  {
#if SSP_USE_SDL
    // If the requested position is out of bounds then position in the center.
    SDL_Rect rect;
    SDL_GetDisplayBounds(0, &rect);
    if (x > rect.w)
    {
      x = rect.w / 2;
    }
    if (y > rect.h)
    {
      y = rect.h / 2;
    }
#define HACK_WINDOW_POSITION
#ifdef HACK_WINDOW_POSITION
    y -= correction;
    SDL_SetWindowPosition(window, x, y);
    SDL_GetWindowPosition(window, &x, &y);
    y -= correction;
    SDL_SetWindowPosition(window, x, y);
#else
    SDL_SetWindowPosition(window, x, y);
#endif
#else
    (void)x;
    (void)y;
    (void)correction;
#endif
  }

  void Window::getPosition(int &x, int &y)
  {
#if SSP_USE_SDL
    SDL_GetWindowPosition(window, &x, &y);
#else
    x = 0;
    y = 0;
#endif
  }

  int Window::getTitleBarHeight()
  {
#if SSP_USE_SDL
    int top;
    SDL_GetWindowBordersSize(window, &top, 0, 0, 0);
    return top;
#else
    return 0;
#endif
  }

} // namespace ssp