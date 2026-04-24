//#define BUILDOPT_VERBOSE
//#define BUILDOPT_DEBUG_LEVEL 10
#include <hal/card.h>

#include <od/config.h>
#include <hal/events.h>
#include <hal/log.h>

#include <percussa/app/CardState.h>

#include <fstream>
#include <stdlib.h>
#include <string>

typedef struct sd
{
  int mode;
  bool mountedBySystem;
} sd_t;

static sd_t sd[2];

#if defined(TARGET_SSP)
namespace percussa
{
  namespace app
  {
    bool usbStarted();
    bool usbMassStorageMode();
  }
}

namespace
{
  static std::string shellQuote(const char *text)
  {
    std::string out = "'";
    for (const char *p = text; *p; ++p)
    {
      if (*p == '\'')
      {
        out += "'\\''";
      }
      else
      {
        out += *p;
      }
    }
    out += "'";
    return out;
  }

  static bool runShellCommand(const std::string &command)
  {
    int rc = system(command.c_str());
    return rc == 0;
  }

  static bool isMountedAtPath(const char *mountPoint)
  {
    std::ifstream stream("/proc/mounts");
    if (!stream)
    {
      return false;
    }

    std::string device;
    std::string mountedPath;
    std::string fsType;
    std::string options;
    int dump = 0;
    int pass = 0;
    while (stream >> device >> mountedPath >> fsType >> options >> dump >> pass)
    {
      if (mountedPath == mountPoint)
      {
        return true;
      }
    }

    return false;
  }
}
#endif

static bool shouldUseFrontUSBMount(uint32_t drv)
{
#if defined(TARGET_SSP)
  bool useFrontUSB = percussa::app::usbStarted() && percussa::app::usbMassStorageMode();
  return drv == CARD_FRONT && useFrontUSB;
#else
  (void)drv;
  return false;
#endif
}

static bool mountFrontUSBCard()
{
#if defined(TARGET_SSP)
  const char *device = "/dev/sda1";
  const char *mountPoint = globalConfig.frontRoot;
  std::string mountCmd = "mount ";
  mountCmd += shellQuote(device);
  mountCmd += " ";
  mountCmd += shellQuote(mountPoint);
  if (!runShellCommand(mountCmd))
  {
    if (isMountedAtPath(mountPoint))
    {
      logInfo("USB drive already mounted at %s.", mountPoint);
      return true;
    }
    logWarn("Failed to mount USB drive %s at %s.", device, mountPoint);
    return false;
  }
  logInfo("Mounted USB drive %s at %s.", device, mountPoint);
  return true;
#else
  return true;
#endif
}

static void unmountFrontUSBCard()
{
#if defined(TARGET_SSP)
  const char *mountPoint = globalConfig.frontRoot;
  std::string unmountCmd = "umount ";
  unmountCmd += shellQuote(mountPoint);
  if (!runShellCommand(unmountCmd))
  {
    logWarn("Failed to unmount USB drive at %s.", mountPoint);
    return;
  }
  logInfo("Unmounted USB drive at %s.", mountPoint);
#endif
}

extern "C"
{
  void Card_init()
  {
    sd[0].mode = CARD_MODE_NOT_CONNECTED;
    sd[1].mode = CARD_MODE_NOT_CONNECTED;
    sd[0].mountedBySystem = false;
    sd[1].mountedBySystem = false;
  }

  bool Card_mount(uint32_t drv)
  {
    if (drv < 2)
    {
      if (sd[drv].mode == CARD_MODE_NOT_CONNECTED)
      {
        if (shouldUseFrontUSBMount(drv))
        {
          if (!mountFrontUSBCard())
          {
            logWarn("Card_mount: USB front card mount failed");
            return false;
          }
          sd[drv].mountedBySystem = true;
        }

        if (Card_connect(drv, CARD_MODE_FATFS))
        {
          return true;
        }

        if (sd[drv].mountedBySystem)
        {
          logInfo("Card_mount: Card_connect failed, unmounting USB drive");
          unmountFrontUSBCard();
          sd[drv].mountedBySystem = false;
        }
      }
    }

    return false;
  }

  void Card_unmount(uint32_t drv)
  {
    if (Card_isMounted(drv))
    {
      Card_disconnect(drv);
      if (sd[drv].mountedBySystem)
      {
        unmountFrontUSBCard();
        sd[drv].mountedBySystem = false;
      }
    }
  }

  bool Card_isMounted(uint32_t drv)
  {
    if (drv < 2)
    {
      return sd[drv].mode == CARD_MODE_FATFS;
    }
    return false;
  }

  bool Card_format(uint32_t drv)
  {
    (void)drv;
    return false;
  }

  uint32_t Card_sizeInBlocks(uint32_t drv)
  {
    (void)drv;
    return 0;
  }

  bool Card_readBlocks(uint32_t drv, uint8_t *buffer, uint32_t sector, uint32_t count)
  {
    (void)drv;
    (void)buffer;
    (void)sector;
    (void)count;
    return false;
  }

  bool Card_readAlignedBlocks(uint32_t drv, uint8_t *buffer, uint32_t sector, uint32_t count)
  {
    (void)drv;
    (void)buffer;
    (void)sector;
    (void)count;
    return false;
  }

  bool Card_writeBlocks(uint32_t drv, uint8_t *buffer, uint32_t sector, uint32_t count)
  {
    (void)drv;
    (void)buffer;
    (void)sector;
    (void)count;
    return false;
  }

  void Card_printErrorStatus(void)
  {
    logInfo("card 0: not implemented");
    logInfo("card 1: not implemented");
  }

  bool Card_isHighCapacity(uint32_t drv)
  {
    (void)drv;
    return false;
  }

  uint32_t Card_getVersion(uint32_t drv)
  {
    (void)drv;
    return 0;
  }

  uint32_t Card_getBusWidth(uint32_t drv)
  {
    (void)drv;
    return 0;
  }

  uint32_t Card_getTransferSpeed(uint32_t drv)
  {
    (void)drv;
    return 0;
  }

  bool Card_supportsCMD23(uint32_t drv)
  {
    (void)drv;
    return false;
  }

  bool Card_isConnected(uint32_t drv)
  {
    return sd[drv].mode != CARD_MODE_NOT_CONNECTED;
  }

  uint32_t Card_getMode(uint32_t drv)
  {
    return sd[drv].mode;
  }

  bool Card_connect(uint32_t drv, uint32_t requestedMode)
  {
    if (sd[drv].mode == (int)requestedMode)
    {
      return true;
    }

    Card_disconnect(drv);

    if (requestedMode == CARD_MODE_NOT_CONNECTED)
    {
      return true;
    }
    if (Card_isPresent(drv))
    {
      sd[drv].mode = requestedMode;
      switch (requestedMode)
      {
      case CARD_MODE_RAW:
        logDebug(1, "drv=%d connected in raw mode", drv);
        if (drv == CARD_REAR)
        {
          Events_push(EVENT_USB_REAR_CARD_MOUNT);
        }
        else
        {
          Events_push(EVENT_USB_FRONT_CARD_MOUNT);
        }
        break;
      case CARD_MODE_FATFS:
        logDebug(1, "drv=%d connected in fatfs mode", drv);
        break;
      default:
        break;
      }
      return true;
    }
    else
    {
      logWarn("Card(%d) is not present.  Cannot connect.", drv);
      return false;
    }
  }

  void Card_disconnect(uint32_t drv)
  {
    if (sd[drv].mode == CARD_MODE_RAW)
    {
      if (drv == CARD_REAR)
      {
        Events_push(EVENT_USB_REAR_CARD_UNMOUNT);
      }
      else
      {
        Events_push(EVENT_USB_FRONT_CARD_UNMOUNT);
      }
    }
    sd[drv].mode = CARD_MODE_NOT_CONNECTED;
  }

  bool Card_isPresent(uint32_t drv)
  {
    if (drv == CARD_REAR)
    {
      return percussa::app::isRearCardPresent();
    }
    if (drv == CARD_FRONT)
    {
      return percussa::app::isFrontCardPresent();
    }
    return false;
  }

  void Card_test(void)
  {
  }
}