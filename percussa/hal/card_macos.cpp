#include <hal/card.h>

bool shouldUseFrontUSBMount(uint32_t drv)
{
  (void)drv;
  return false;
}

bool mountFrontUSBCard()
{
  return true;
}

void unmountFrontUSBCard()
{
}
