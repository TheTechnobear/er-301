#pragma once

namespace percussa
{
  namespace runtime
  {
    namespace ssp
    {
      void setCardPresence(bool rearPresent, bool frontPresent);
      bool isRearCardPresent();
      bool isFrontCardPresent();
    }
  }
}