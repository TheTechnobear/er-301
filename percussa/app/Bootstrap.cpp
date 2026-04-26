#include <percussa/app/Bootstrap.h>

#include <percussa/app/CardState.h>
#include <percussa/od/glue/AppInterpreter.h>
#include <percussa/support/CommandLine.h>
#include <percussa/support/KeyValueStore.h>
#include <percussa/support/tls.h>

#include <od/config.h>
#include <od/graphics/Cursor.h>

#include <hal/card.h>
#include <hal/fileops.h>
#include <hal/gpio.h>
#include <hal/heap.h>
#include <hal/log.h>
#include <hal/timing.h>
#include <hal/uart.h>
#include <hal/events.h>
#include <hal/encoder.h>
#include <hal/pwm.h>
#include <hal/adc.h>
#include <hal/audio.h>
#include <hal/display.h>
#include <hal/rng.h>
#include <hal/usb.h>
#include <hal/modulation.h>
#include <hal/pump.h>

#include <climits>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <thread>

namespace percussa
{
  namespace app
  {
    namespace
    {
      void defaultPaths(std::string &prefix, std::string &xroot)
      {
#if defined(PERCUSSA_PLATFORM_FBDEV)
        prefix = "/media/BOOT/er301";
        xroot = prefix + "/xroot";
#else
        prefix = "~/.ssp";
        xroot = "./xroot";
#endif
      }

      char *realpathEx(const char *path, char *buff)
      {
        char expanded[PATH_MAX];
        const char *target = path;
        if (*path == '~')
        {
          const char *home = getenv("HOME");
          if (home)
          {
            std::snprintf(expanded, sizeof(expanded), "%s%s", home, path + 1);
            target = expanded;
          }
        }

        char *resolved = realpath(target, buff);
        if (!resolved)
        {
          std::strncpy(buff, target, PATH_MAX - 1);
          buff[PATH_MAX - 1] = 0;
        }
        return buff;
      }
    }

    Bootstrap::Bootstrap(int argc, char **argv) :
      mArgc(argc),
      mArgv(argv)
    {
    }

    Bootstrap::~Bootstrap()
    {
    }

    bool Bootstrap::initialize()
    {
      percussa::support::CommandLine cmdLine(mArgc, mArgv);

      TLS_setName("main");
      loadDefaultConfiguration();

      if (cmdLine.optionExists("-c"))
      {
        mConfigFilename = cmdLine.getOption("-c");
      }
      if (cmdLine.optionExists("--config"))
      {
        mConfigFilename = cmdLine.getOption("--config");
      }

      Uart_init();
      Uart_enable();
      Log_init();
      Heap_init();
      Timing_init();
      percussa::app::setCardPresence(mRearCardPresent, mFrontCardPresent);
      Card_init();
      Gpio_init();
      std::string firmwareCfg = mRearRoot + "/firmware.cfg";
      Config_init(firmwareCfg.c_str(), mXRoot.c_str(), mRearRoot.c_str(), mFrontRoot.c_str());
      Encoder_init();
      Events_init();
      Rng_init();
      USB_init();
      Pwm_init();
      Adc_init();
      Modulation_init();
      Audio_init();
      Display_init();

      if (!pathExists(mConfigFilename.c_str()))
      {
        logWarn("%s does not exist, creating a default one.", mConfigFilename.c_str());
        writeDefaultConfiguration(mConfigFilename);
      }
      else if (!loadConfiguration(mConfigFilename))
      {
        logWarn("There was a problem loading %s. Using default SSP configuration.", mConfigFilename.c_str());
      }

      restoreState();
      startInterpreterBootstrap();
      return true;
    }

    void Bootstrap::finalize()
    {
      Audio_stop();
      Events_push(EVENT_QUIT);
      if (mInterpreterThread.joinable())
      {
        mInterpreterThread.join();
      }
      saveState();
    }

    bool Bootstrap::writeDefaultConfiguration(const std::string &filename)
    {
      std::string prefix;
      std::string xroot;
      defaultPaths(prefix, xroot);

      std::ofstream f(filename);
      if (!f.is_open())
      {
        return false;
      }

      f << "## ER-301 SSP Configuration\n\n";
      f << "## Uncomment lines below to set your own values.\n\n";
      f << "## Root for the Lua interpreter\n";
      f << "# XROOT " << xroot << "\n\n";
      f << "## Session state file\n";
      f << "# SESSION " << prefix << "/ssp.session\n\n";
      f << "## Use this root for the rear SD card.\n";
      f << "# REAR_ROOT  " << prefix << "/rear\n";
      f << "# REAR_PRESENT true\n\n";
      f << "## Use this root for the front SD card.\n";
      f << "# FRONT_ROOT " << prefix << "/front\n";
      f << "# FRONT_PRESENT true\n\n";
      f << "## Knob mapping\n\n";
      f << "##  Scale factor for mouse wheel. Negate to invert.\n";
      f << "# MOUSE_WHEEL_FACTOR " << mMouseWheelToKnobFactor << "\n\n";
      f << "## Cursor animation tuning. Higher tween is faster cursor travel.\n";
      f << "## Breath controls the small idle bob of the cursor. Set to 0 to disable.\n";
      f << "# CURSOR_TWEEN_STEP " << od::Cursor::defaultTweenStep() << "\n";
      f << "# CURSOR_BREATH_STEP " << od::Cursor::defaultBreathStep() << "\n\n";
      return true;
    }

    void Bootstrap::loadDefaultConfiguration()
    {
      char tmp[PATH_MAX];
      std::string prefix;
      std::string xroot;
      defaultPaths(prefix, xroot);

      od::Cursor::resetAnimationSteps();

      realpathEx(prefix.c_str(), tmp);
      mConfigRoot = tmp;
      if (!pathExists(mConfigRoot.c_str()))
      {
        createDirectory(mConfigRoot.c_str());
      }
      mRearRoot = mConfigRoot + "/rear";
      mRearCardPresent = true;
      mFrontRoot = mConfigRoot + "/front";
      mFrontCardPresent = true;
      mSessionFilename = mConfigRoot + "/ssp.session";
      mConfigFilename = mConfigRoot + "/ssp.config";
      realpathEx(xroot.c_str(), tmp);
      mXRoot = tmp;
      mMouseWheelToKnobFactor = 0.5;
    }

    bool Bootstrap::loadConfiguration(const std::string &filename)
    {
      percussa::support::KeyValueStore db;
      if (!db.load(filename))
      {
        return false;
      }

      float cursorTweenStep = db.getFloat("CURSOR_TWEEN_STEP", od::Cursor::defaultTweenStep());
      if (cursorTweenStep <= 0.0f)
      {
        cursorTweenStep = od::Cursor::defaultTweenStep();
      }

      float cursorBreathStep = db.getFloat("CURSOR_BREATH_STEP", od::Cursor::defaultBreathStep());
      if (cursorBreathStep < 0.0f)
      {
        cursorBreathStep = od::Cursor::defaultBreathStep();
      }

      od::Cursor::setAnimationSteps(cursorTweenStep, cursorBreathStep);

      mMouseWheelToKnobFactor = db.getFloat("MOUSE_WHEEL_FACTOR", (float)mMouseWheelToKnobFactor);

      char tmp[PATH_MAX];
      realpathEx(db.get("XROOT", mXRoot).c_str(), tmp);
      mXRoot = tmp;

      realpathEx(db.get("SESSION", mSessionFilename).c_str(), tmp);
      mSessionFilename = tmp;

      realpathEx(db.get("REAR_ROOT", mRearRoot).c_str(), tmp);
      mRearRoot = tmp;
      mRearCardPresent = db.get("REAR_PRESENT", "true") != "false";

      realpathEx(db.get("FRONT_ROOT", mFrontRoot).c_str(), tmp);
      mFrontRoot = tmp;
      mFrontCardPresent = db.get("FRONT_PRESENT", "true") != "false";
      return true;
    }

    void Bootstrap::restoreState()
    {
      if (!pathExists(mSessionFilename.c_str()))
      {
        return;
      }

      percussa::support::KeyValueStore db;
      if (!db.load(mSessionFilename))
      {
        logWarn("Failed to load from %s.", mSessionFilename.c_str());
        return;
      }

      Gpio_write(TOGGLE_STORAGE_A, db["TOGGLE_STORAGE_A"] == "1");
      Gpio_write(TOGGLE_STORAGE_B, db["TOGGLE_STORAGE_B"] == "1");
      Gpio_write(TOGGLE_MODE_A, db["TOGGLE_MODE_A"] == "1");
      Gpio_write(TOGGLE_MODE_B, db["TOGGLE_MODE_B"] == "1");
      logInfo("Restored SSP state from %s.", mSessionFilename.c_str());
    }

    void Bootstrap::saveState()
    {
      percussa::support::KeyValueStore db;
      db["TOGGLE_STORAGE_A"] = Gpio_read(TOGGLE_STORAGE_A) ? "1" : "0";
      db["TOGGLE_STORAGE_B"] = Gpio_read(TOGGLE_STORAGE_B) ? "1" : "0";
      db["TOGGLE_MODE_A"] = Gpio_read(TOGGLE_MODE_A) ? "1" : "0";
      db["TOGGLE_MODE_B"] = Gpio_read(TOGGLE_MODE_B) ? "1" : "0";

      if (db.save(mSessionFilename))
      {
        logInfo("Saved SSP state to %s.", mSessionFilename.c_str());
      }
      else
      {
        logWarn("Failed to save to %s.", mSessionFilename.c_str());
      }
    }

    void Bootstrap::startInterpreterBootstrap()
    {
      const std::string xRoot = mXRoot;
      const std::string rearRoot = mRearRoot;
      const std::string frontRoot = mFrontRoot;
      mInterpreterThread = std::thread(
        [xRoot, rearRoot, frontRoot]()
        {
          TLS_setName("lua");
          od::AppInterpreter interp;
          interp.init();
          interp.execute("package.path = '%s/?.lua;%s/?/init.lua'", xRoot.c_str(), xRoot.c_str());
          interp.execute("app.EMULATION = true");
          interp.execute("app.roots = {x='%s',rear='%s',front='%s'}", xRoot.c_str(), rearRoot.c_str(), frontRoot.c_str());
          interp.execute("dofile('%s/boot/logging.lua')", xRoot.c_str());
          interp.execute("dofile('%s/boot/start.lua')", xRoot.c_str());
        });
    }
  }
}