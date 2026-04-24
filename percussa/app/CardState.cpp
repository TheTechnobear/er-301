#include <percussa/app/CardState.h>

namespace percussa
{
  namespace app
  {
    namespace
    {
      bool gRearCardPresent = true;
      bool gFrontCardPresent = true;
    }

    void setCardPresence(bool rearPresent, bool frontPresent)
    {
      gRearCardPresent = rearPresent;
      gFrontCardPresent = frontPresent;
    }

    bool isRearCardPresent()
    {
      return gRearCardPresent;
    }

    bool isFrontCardPresent()
    {
      return gFrontCardPresent;
    }
  }
}