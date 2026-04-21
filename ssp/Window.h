#pragma once

#include <ssp/Button.h>
#include <ssp/Encoder.h>
#include <ssp/Toggle.h>
#include <ssp/Led.h>
#include <ssp/constants.h>

#include <hal/channels.h>
#include <hal/events.h>

#include <SDL2/SDL.h>

#include <stdint.h>
#include <array>
#include <vector>

namespace ssp
{
  struct Window
  {
    Window();
    ~Window();

    void onResized(int w, int h);

    void renderMainFrame(uint8_t *frame);
    void renderSubFrame(uint8_t *frame);
    void setPosition(int x, int y, int correction = 0);
    void getPosition(int &x, int &y);
    int getTitleBarHeight();
    void update(uint8_t *mainFrame, uint8_t *subFrame);

    SDL_Window *window = 0;
    SDL_Renderer *renderer = 0;
    SDL_Texture *windowTexture = 0;
    std::vector<uint32_t> windowBuffer;

    SDL_Rect mainRect{ .x = MAIN_X, .y = MAIN_Y, .w = MAIN_W, .h = MAIN_H };
    SDL_Rect subRect{ .x = SUB_X, .y = SUB_Y, .w = SUB_W, .h = SUB_H };

    std::array<Button, 19> buttons{
      Button{ "M1(QS)", BUTTON_MAIN1 }, Button{ "M2", BUTTON_MAIN2 },     Button{ "M3", BUTTON_MAIN3 },
      Button{ "M4", BUTTON_MAIN4 },     Button{ "M5", BUTTON_MAIN5 },     Button{ "M6", BUTTON_MAIN6 },
      Button{ "", BUTTON_DIAL1 },       Button{ "CANCEL", BUTTON_DIAL2 }, Button{ "HOME", BUTTON_DIAL3 },
      Button{ "S1", BUTTON_SUB1 },      Button{ "S2", BUTTON_SUB2 },      Button{ "S3", BUTTON_SUB3 },
      Button{ "ENTER", BUTTON_ENTER },  Button{ "UP", BUTTON_UP },        Button{ "(SHIFT)", BUTTON_SHIFT },
      Button{ "", BUTTON_SELECT1 },     Button{ "", BUTTON_SELECT2 },     Button{ "", BUTTON_SELECT3 },
      Button{ "", BUTTON_SELECT4 }
    };

    std::array<Encoder, 4> encoders{
      Encoder{ "DATA", BUTTON_DIAL1 },
      Encoder{ "OUT", BUTTON_DIAL2 },
      Encoder{ "STORE", BUTTON_DIAL3 },
      Encoder{ "MODE", BUTTON_ENTER }
    };


    std::array<Toggle, 2> toggles{ Toggle{
                                     "STORAGE", TOGGLE_STORAGE_A, TOGGLE_STORAGE_B, "Z", "user", "admin", "eject" },
                                   Toggle{ "MODE", TOGGLE_MODE_A, TOGGLE_MODE_B, "X", "hold", "edit", "scope" } };
    static constexpr int TGL_STORE = 0;
    static constexpr int TGL_MODE = 1;


    int width = SCREEN_WIDTH;
    int height = SCREEN_HEIGHT;
    double pauseDisplayTime = 0;
  };
}; // namespace ssp