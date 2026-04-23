#include <percussa/runtime/ssp/CardState.h>

namespace percussa
{
  namespace runtime
  {
    namespace ssp
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
}