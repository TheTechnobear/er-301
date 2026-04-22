#pragma once

#include <od/extras/LockFreeQueue.h>
#include <ssp/Window.h>
#include <ssp/hw/SSPButEnc.h>
#include <hal/display.h>
#include <map>

#ifndef SSP_USE_SDL
#if defined(__APPLE__)
#define SSP_USE_SDL 1
#else
#define SSP_USE_SDL 0
#endif
#endif

#if SSP_USE_SDL
#include <SDL2/SDL.h>
#endif

namespace ssp
{
  struct SSPCore
  {
    SSPCore();
    int run(int argc, char **argv);
    void putDisplayBuffer(DisplayBuffer *buffer);
    DisplayBuffer *getDisplayBuffer();
    int getEncoderValue();
    bool isRearCardPresent();
    bool isFrontCardPresent();

  private:
    void loop();
#if SSP_USE_SDL
    void handleKeyUp(SDL_Keysym sym);
    void handleKeyDown(SDL_Keysym sym);
    void handleMouseButton(SDL_MouseButtonEvent &e);
#endif
    void mapButtonToKey(uint32_t id, const std::string &key);

    bool writeDefaultConfiguration(const std::string &filename);
    void loadDefaultConfiguration();
    bool loadConfiguration(const std::string &filename);
    std::string xRoot;
    std::string rearRoot;
    std::string frontRoot;
    std::string configRoot;
    std::string sessionFilename;
    std::string configFilename;
    double mouseWheelToKnobFactor;
    double leftRightToKnobFactor;
    double upDownToKnobFactor;
    bool rearCardPresent = true;
    bool frontCardPresent = true;

    // Persist state between sessions.
    void saveState();
    void restoreState();

    Window *window = 0;
    DisplayBuffer ping, pong;
    od::LockFreeQueue<DisplayBuffer *, 4> readyQ, renderQ;
  #if SSP_USE_SDL
    uint customEventType = SDL_USEREVENT;
  #endif
    double encoderValue = 0;
    bool quit = false;
    bool storageToggleFocused = false;
    bool modeToggleFocused = false;
    SSPButEnc hardwareInput;
    bool hardwareInputEnabled = false;

    // Keyboard Mapping
    std::map<std::string, uint32_t> keyGpioMap;
    std::map<uint32_t, std::string> gpioKeyMap;
    std::string storageToggleFocusKey;
    std::string modeToggleFocusKey;
    std::string zoomInKey;
    std::string zoomOutKey;
    std::string quitKey; // Must be modified with CTRL.

  };
}