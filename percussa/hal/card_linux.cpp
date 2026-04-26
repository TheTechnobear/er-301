#include <hal/card.h>
#include <hal/log.h>
#include <od/config.h>

#include <cctype>
#include <fstream>
#include <string>
#include <vector>

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

  static std::vector<std::string> discoverFrontUSBCandidates()
  {
    std::vector<std::string> devices;
    std::ifstream stream("/proc/partitions");
    if (stream)
    {
      std::string major;
      std::string minor;
      std::string blocks;
      std::string name;
      while (stream >> major >> minor >> blocks >> name)
      {
        if (name.size() < 4)
        {
          continue;
        }

        // Match sdXn partitions, e.g. sda1, sdb1, sdc2.
        if (name[0] == 's' && name[1] == 'd' && std::isalpha(name[2]) &&
            std::isdigit(name[name.size() - 1]))
        {
          devices.push_back("/dev/" + name);
        }
      }
    }

    // Keep legacy behavior as explicit fallback.
    devices.push_back("/dev/sda1");
    return devices;
  }
}

bool shouldUseFrontUSBMount(uint32_t drv)
{
  bool useFrontUSB = percussa::app::usbStarted() && percussa::app::usbMassStorageMode();
  return drv == CARD_FRONT && useFrontUSB;
}

bool mountFrontUSBCard()
{
  const char *mountPoint = globalConfig.frontRoot;

  auto candidates = discoverFrontUSBCandidates();
  for (const auto &device : candidates)
  {
    std::string mountCmd = "mount ";
    mountCmd += shellQuote(device.c_str());
    mountCmd += " ";
    mountCmd += shellQuote(mountPoint);
    if (runShellCommand(mountCmd))
    {
      logInfo("Mounted USB drive %s at %s.", device.c_str(), mountPoint);
      return true;
    }
  }

  if (isMountedAtPath(mountPoint))
  {
    logInfo("USB drive already mounted at %s.", mountPoint);
    return true;
  }

  logWarn("Failed to mount USB drive at %s (tried %zu candidates).", mountPoint,
          candidates.size());
  return false;
}

void unmountFrontUSBCard()
{
  const char *mountPoint = globalConfig.frontRoot;
  std::string unmountCmd = "umount ";
  unmountCmd += shellQuote(mountPoint);
  if (!runShellCommand(unmountCmd))
  {
    logWarn("Failed to unmount USB drive at %s.", mountPoint);
    return;
  }
  logInfo("Unmounted USB drive at %s.", mountPoint);
}
