#pragma once

#include <string>
#include <vector>

namespace percussa
{
  namespace ui
  {
    struct DisplayState
    {
      std::string title;
      std::string line1;
      std::string line2;
    };

    struct PresentationState
    {
      std::vector<DisplayState> displays;
      std::vector<bool> buttonActive;
      std::vector<bool> ledActive;
      std::vector<int> togglePositions;
      std::string statusText;
    };
  }
}