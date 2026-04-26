#include <percussa/platform/HostSdlPlatform.h>

#include <percussa/input/Action.h>
#include <percussa/panel/Panel.h>
#include <percussa/platform/TargetDimensions.h>
#include <percussa/runtime/Runtime.h>
#include <percussa/ui/PanelRenderer.h>

#include <hal/display.h>
#include <hal/events.h>

#include <chrono>
#include <iostream>
#include <ostream>

#if defined(PERCUSSA_PLATFORM_HOST_SDL)
#include <SDL2/SDL.h>
#endif

namespace percussa
{
  namespace platform
  {
    namespace
    {
      const auto kDisplayTickPeriod = std::chrono::milliseconds(14);

#if defined(PERCUSSA_PLATFORM_HOST_SDL)
      bool mapScancodeToButton(SDL_Scancode scancode, input::HardwareButtonId &button)
      {
        switch (scancode)
        {
        case SDL_SCANCODE_1:
          button = input::HardwareButtonId::Button1;
          return true;
        case SDL_SCANCODE_2:
          button = input::HardwareButtonId::Button2;
          return true;
        case SDL_SCANCODE_3:
          button = input::HardwareButtonId::Button3;
          return true;
        case SDL_SCANCODE_4:
          button = input::HardwareButtonId::Button4;
          return true;
        case SDL_SCANCODE_Q:
          button = input::HardwareButtonId::Button5;
          return true;
        case SDL_SCANCODE_W:
          button = input::HardwareButtonId::Button6;
          return true;
        case SDL_SCANCODE_E:
          button = input::HardwareButtonId::Button7;
          return true;
        case SDL_SCANCODE_R:
          button = input::HardwareButtonId::Button8;
          return true;
        case SDL_SCANCODE_6:
        case SDL_SCANCODE_UP:
          button = input::HardwareButtonId::Up;
          return true;
        case SDL_SCANCODE_Y:
        case SDL_SCANCODE_DOWN:
          button = input::HardwareButtonId::Down;
          return true;
        case SDL_SCANCODE_5:
        case SDL_SCANCODE_LSHIFT:
          button = input::HardwareButtonId::ShiftL;
          return true;
        case SDL_SCANCODE_7:
        case SDL_SCANCODE_RSHIFT:
          button = input::HardwareButtonId::ShiftR;
          return true;
        case SDL_SCANCODE_T:
        case SDL_SCANCODE_LEFT:
          button = input::HardwareButtonId::Left;
          return true;
        case SDL_SCANCODE_U:
        case SDL_SCANCODE_RIGHT:
          button = input::HardwareButtonId::Right;
          return true;
        default:
          return false;
        }
      }

      bool mapScancodeToEncoderTurn(SDL_Scancode scancode, input::HardwareEncoderId &encoder, int &delta)
      {
        switch (scancode)
        {
        case SDL_SCANCODE_0:
          encoder = input::HardwareEncoderId::Encoder1;
          delta = -1;
          return true;
        case SDL_SCANCODE_EQUALS:
          encoder = input::HardwareEncoderId::Encoder1;
          delta = 1;
          return true;
        case SDL_SCANCODE_O:
          encoder = input::HardwareEncoderId::Encoder2;
          delta = -1;
          return true;
        case SDL_SCANCODE_LEFTBRACKET:
          encoder = input::HardwareEncoderId::Encoder2;
          delta = 1;
          return true;
        case SDL_SCANCODE_K:
          encoder = input::HardwareEncoderId::Encoder3;
          delta = -1;
          return true;
        case SDL_SCANCODE_SEMICOLON:
          encoder = input::HardwareEncoderId::Encoder3;
          delta = 1;
          return true;
        case SDL_SCANCODE_M:
          encoder = input::HardwareEncoderId::Encoder4;
          delta = -1;
          return true;
        case SDL_SCANCODE_PERIOD:
          encoder = input::HardwareEncoderId::Encoder4;
          delta = 1;
          return true;
        default:
          return false;
        }
      }

      bool mapScancodeToEncoderPress(SDL_Scancode scancode, input::HardwareEncoderId &encoder)
      {
        switch (scancode)
        {
        case SDL_SCANCODE_MINUS:
          encoder = input::HardwareEncoderId::Encoder1;
          return true;
        case SDL_SCANCODE_P:
          encoder = input::HardwareEncoderId::Encoder2;
          return true;
        case SDL_SCANCODE_L:
          encoder = input::HardwareEncoderId::Encoder3;
          return true;
        case SDL_SCANCODE_COMMA:
          encoder = input::HardwareEncoderId::Encoder4;
          return true;
        default:
          return false;
        }
      }

      bool mapKeyDownToAction(SDL_Scancode scancode, input::Action &action)
      {
        input::HardwareButtonId button = input::HardwareButtonId::Invalid;
        if (mapScancodeToButton(scancode, button))
        {
          action.type = input::ActionType::Button;
          action.hardwareButton = button;
          action.pressed = true;
          return true;
        }

        input::HardwareEncoderId encoder = input::HardwareEncoderId::Invalid;
        int delta = 0;
        if (mapScancodeToEncoderTurn(scancode, encoder, delta))
        {
          action.type = input::ActionType::EncoderTurn;
          action.hardwareEncoder = encoder;
          action.delta = delta;
          return true;
        }

        if (mapScancodeToEncoderPress(scancode, encoder))
        {
          action.type = input::ActionType::EncoderPress;
          action.hardwareEncoder = encoder;
          action.pressed = true;
          return true;
        }

        return false;
      }

      bool mapKeyUpToAction(SDL_Scancode scancode, input::Action &action)
      {
        if (!mapKeyDownToAction(scancode, action))
        {
          return false;
        }
        if (action.type == input::ActionType::Button || action.type == input::ActionType::EncoderPress)
        {
          action.pressed = false;
          action.delta = 0;
          return true;
        }
        return false;
      }

      void presentTexture(SDL_Renderer *rendererHandle, SDL_Texture *texture)
      {
        SDL_SetRenderDrawColor(rendererHandle, 0, 0, 0, 255);
        SDL_RenderClear(rendererHandle);
        SDL_RenderCopy(rendererHandle, texture, 0, 0);
        SDL_RenderPresent(rendererHandle);
      }

#endif
    } // namespace

    int HostSdlPlatform::run(runtime::Runtime &runtime) const
    {
#if defined(PERCUSSA_PLATFORM_HOST_SDL)
      const panel::Panel &panel = runtime.panel();
      Size expected = target::panelSize();
      if (panel.width() != expected.width || panel.height() != expected.height)
      {
        std::cerr << "HostSdlPlatform: panel size mismatch. Expected "
                  << expected.width << "x" << expected.height
                  << ", got " << panel.width() << "x" << panel.height() << std::endl;
        return 1;
      }

      ui::PanelRenderer renderer;
      ui::RenderedPanel rendered = renderer.render(panel);

      if (SDL_Init(SDL_INIT_VIDEO) != 0)
      {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
      }

      int scale = rendered.width() > 1000 ? 1 : 2;
      SDL_Window *window = SDL_CreateWindow("Percussa Host",
                                            SDL_WINDOWPOS_CENTERED,
                                            SDL_WINDOWPOS_CENTERED,
                    rendered.width() * scale,
                    rendered.height() * scale,
                                            SDL_WINDOW_SHOWN);
      if (!window)
      {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
      }

      SDL_Renderer *rendererHandle =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
      if (!rendererHandle)
      {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
      }

      SDL_Texture *texture = SDL_CreateTexture(
        rendererHandle, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, rendered.width(), rendered.height());
      if (!texture)
      {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(rendererHandle);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
      }

      SDL_UpdateTexture(texture, 0, rendered.data(), rendered.width() * (int)sizeof(uint32_t));

      bool quit = mOnce;
      bool firstFrame = true;
      DisplayBuffer *lastPresentedBuffer = Display_getLastPutBuffer();
      std::chrono::steady_clock::time_point lastDisplayTick = std::chrono::steady_clock::now() - kDisplayTickPeriod;
      while (true)
      {
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        if (now - lastDisplayTick >= kDisplayTickPeriod)
        {
          Events_push(EVENT_DISPLAY_READY);
          lastDisplayTick = now;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
          if (event.type == SDL_QUIT)
          {
            quit = true;
          }
          if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
          {
            quit = true;
          }
          if (event.type == SDL_KEYDOWN)
          {
            input::Action action;
            if (mapKeyDownToAction(event.key.keysym.scancode, action))
            {
              if (event.key.repeat != 0 && action.type != input::ActionType::EncoderTurn)
              {
                continue;
              }
              runtime.handleAction(action);
              rendered = renderer.render(panel);
              SDL_UpdateTexture(texture, 0, rendered.data(), rendered.width() * (int)sizeof(uint32_t));
            }
          }
          if (event.type == SDL_KEYUP)
          {
            input::Action action;
            if (mapKeyUpToAction(event.key.keysym.scancode, action))
            {
              runtime.handleAction(action);
              rendered = renderer.render(panel);
              SDL_UpdateTexture(texture, 0, rendered.data(), rendered.width() * (int)sizeof(uint32_t));
            }
          }
        }

        DisplayBuffer *currentBuffer = Display_getLastPutBuffer();
        if (currentBuffer != lastPresentedBuffer)
        {
          rendered = renderer.render(panel);
          SDL_UpdateTexture(texture, 0, rendered.data(), rendered.width() * (int)sizeof(uint32_t));
          lastPresentedBuffer = currentBuffer;
        }

        presentTexture(rendererHandle, texture);

        if (quit)
        {
          if (mOnce && firstFrame)
          {
            SDL_PumpEvents();
          }
          break;
        }
        firstFrame = false;
      }

      SDL_DestroyTexture(texture);
      SDL_DestroyRenderer(rendererHandle);
      SDL_DestroyWindow(window);
      SDL_Quit();
      return 0;
#else
      (void)runtime;
      std::cerr << "Host SDL platform is unavailable in this build." << std::endl;
      return 1;
#endif
    }
  } // namespace platform
} // namespace percussa