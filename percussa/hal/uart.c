#include <hal/uart.h>

#include <stdbool.h>
#include <stdio.h>

typedef struct
{
  bool enabled;
  bool initialized;
} Local;

static Local self = {
  false,
  false,
};

void Uart_init(void)
{
  self.initialized = true;
}

void Uart_write(const char *buffer, int size)
{
  if (!self.enabled)
  {
    return;
  }
  fwrite(buffer, 1, (size_t)size, stdout);
  fflush(stdout);
}

void Uart_puts(const char *buffer)
{
  if (!self.enabled)
  {
    return;
  }
  fputs(buffer, stdout);
  fflush(stdout);
}

void Uart_enable()
{
  self.enabled = self.initialized;
}

void Uart_disable()
{
  self.enabled = false;
}

bool Uart_isEnabled()
{
  return self.enabled;
}