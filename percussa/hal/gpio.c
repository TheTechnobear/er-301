#include <hal/gpio.h>

#include <hal/events.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
  bool value;
  uint32_t upEvent;
  uint32_t downEvent;
  bool hasEvents;
} gpio_t;

typedef struct
{
  gpio_t info[NUM_GPIO_IDS];
} Local;

static Local local;

static void configureOutput(uint32_t id)
{
  local.info[id].value = false;
}

static void configureSwitch(uint32_t id)
{
  local.info[id].value = false;
}

static void configureButton(uint32_t id)
{
  local.info[id].value = true;
}

void Gpio_setEvents(uint32_t id, uint32_t up, uint32_t dn)
{
  local.info[id].upEvent = up;
  local.info[id].downEvent = dn;
  local.info[id].hasEvents = true;
}

bool Gpio_read(uint32_t id)
{
  return local.info[id].value;
}

void Gpio_write(uint32_t id, bool value)
{
  bool changed = local.info[id].value != value;
  local.info[id].value = value;
  if (changed && local.info[id].hasEvents)
  {
    Events_push(value ? local.info[id].upEvent : local.info[id].downEvent);
  }
}

void Gpio_toggle(uint32_t id)
{
  Gpio_write(id, Gpio_read(id) ? false : true);
}

void Gpio_init(void)
{
  int i;
  for (i = 0; i < NUM_GPIO_IDS; i++)
  {
    local.info[i].value = false;
  }

  configureOutput(LED_SAFE);
  configureOutput(LED_IO);
  configureOutput(LED_DIAL1);
  configureOutput(LED_DIAL2);
  configureOutput(LED_OUT1);
  configureOutput(LED_OUT2);
  configureOutput(LED_OUT3);
  configureOutput(LED_OUT4);
  configureOutput(LED_LINK12);
  configureOutput(LED_LINK34);
  configureOutput(LED_LINK23);

  configureOutput(PWM_SIN);
  configureOutput(PWM_SCLK);
  configureOutput(PWM_XLAT);
  configureOutput(PWM_BLANK);

  configureOutput(MAIN_OLED_RESET);
  configureOutput(SUB_OLED_RESET);
  configureOutput(OLED_POWER);

  configureOutput(PCM4104_nRESET);
  configureOutput(PCM4104_MUTE);
  configureOutput(PCM4104_FS0);
  configureOutput(PCM4104_FS1);
  configureOutput(AUDIO_EXTERNAL_CLOCK_ENABLE);
  configureOutput(ADS8688_RESET);

  configureButton(BUTTON_MAIN1);
  configureButton(BUTTON_MAIN2);
  configureButton(BUTTON_MAIN3);
  configureButton(BUTTON_MAIN4);
  configureButton(BUTTON_MAIN5);
  configureButton(BUTTON_MAIN6);
  configureButton(BUTTON_SUB1);
  configureButton(BUTTON_SUB2);
  configureButton(BUTTON_SUB3);
  configureButton(BUTTON_ENTER);
  configureButton(BUTTON_UP);
  configureButton(BUTTON_SHIFT);
  configureButton(BUTTON_DIAL1);
  configureButton(BUTTON_DIAL2);
  configureButton(BUTTON_DIAL3);
  configureButton(BUTTON_SELECT1);
  configureButton(BUTTON_SELECT2);
  configureButton(BUTTON_SELECT3);
  configureButton(BUTTON_SELECT4);

  configureSwitch(TOGGLE_STORAGE_A);
  configureSwitch(TOGGLE_STORAGE_B);
  configureSwitch(TOGGLE_MODE_A);
  configureSwitch(TOGGLE_MODE_B);
}