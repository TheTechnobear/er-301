#pragma once

#include <string>
#include <thread>

namespace percussa
{
  namespace app
  {
    class Bootstrap
    {
    public:
      Bootstrap(int argc, char **argv);
      ~Bootstrap();

      bool initialize();
      void finalize();

    private:
      int mArgc;
      char **mArgv;
      std::string mXRoot;
      std::string mRearRoot;
      std::string mFrontRoot;
      std::string mConfigRoot;
      std::string mSessionFilename;
      std::string mConfigFilename;
      double mMouseWheelToKnobFactor = 0.5;
      bool mRearCardPresent = true;
      bool mFrontCardPresent = true;
      std::thread mInterpreterThread;

      bool writeDefaultConfiguration(const std::string &filename);
      void loadDefaultConfiguration();
      bool loadConfiguration(const std::string &filename);
      void restoreState();
      void saveState();
      void startInterpreterBootstrap();
    };
  }
}