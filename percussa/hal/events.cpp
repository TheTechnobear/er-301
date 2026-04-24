#include <hal/constants.h>
#include <hal/concurrency/EventFlags.h>
#include <hal/encoder.h>
#include <hal/events.h>
#include <hal/fifo.h>
#include <hal/gpio.h>
#include <hal/log.h>

#define REPEAT_PERIOD 3
#define REPEAT_DELAY 25

struct Local
{
  fifo_t q;
  od::EventFlags events;
#define onPost od::EventFlags::flag00
  int buttonTimer[19];

  Local() :
    q(),
    events(),
    buttonTimer{0}
  {
  }
};

static Local local;

static void checkButtonRepeat(uint32_t id)
{
  int i = (int)(id - BUTTON_MAIN1);
  if (Button_pressed(id))
  {
    if (local.buttonTimer[i] > REPEAT_PERIOD + REPEAT_DELAY)
    {
      local.buttonTimer[i] = REPEAT_DELAY;
      Events_push(EVENT(EVENT_REPEAT, id));
    }
    else
    {
      local.buttonTimer[i]++;
    }
  }
  else
  {
    local.buttonTimer[i] = 0;
  }
}

static void checkEncoder(void)
{
  (void)Encoder_getValue();
}

static void configureButtonEvents(uint32_t id)
{
  Gpio_setEvents(id, EVENT(EVENT_RELEASE, id), EVENT(EVENT_PRESS, id));
}

extern "C"
{
  void Events_init(void)
  {
    fifo_init(&local.q);

    configureButtonEvents(BUTTON_MAIN1);
    configureButtonEvents(BUTTON_MAIN2);
    configureButtonEvents(BUTTON_MAIN3);
    configureButtonEvents(BUTTON_MAIN4);
    configureButtonEvents(BUTTON_MAIN5);
    configureButtonEvents(BUTTON_MAIN6);
    configureButtonEvents(BUTTON_SUB1);
    configureButtonEvents(BUTTON_SUB2);
    configureButtonEvents(BUTTON_SUB3);
    configureButtonEvents(BUTTON_ENTER);
    configureButtonEvents(BUTTON_UP);
    configureButtonEvents(BUTTON_SHIFT);
    configureButtonEvents(BUTTON_DIAL1);
    configureButtonEvents(BUTTON_DIAL2);
    configureButtonEvents(BUTTON_DIAL3);
    configureButtonEvents(BUTTON_SELECT1);
    configureButtonEvents(BUTTON_SELECT2);
    configureButtonEvents(BUTTON_SELECT3);
    configureButtonEvents(BUTTON_SELECT4);
    Gpio_setEvents(TOGGLE_MODE_A, EVENT_MODE, EVENT_MODE);
    Gpio_setEvents(TOGGLE_MODE_B, EVENT_MODE, EVENT_MODE);
    Gpio_setEvents(TOGGLE_STORAGE_A, EVENT_STORAGE, EVENT_STORAGE);
    Gpio_setEvents(TOGGLE_STORAGE_B, EVENT_STORAGE, EVENT_STORAGE);
  }

  void Events_push(uint32_t e)
  {
    logDebug(1, "push type=%d id=%d", EVENT_TYPE(e), EVENT_ID(e));
    fifo_push(&local.q, e);
    local.events.post(onPost);
  }

  void Events_clear(void)
  {
    fifo_init(&local.q);
  }

  static void check(void)
  {
    checkButtonRepeat(BUTTON_MAIN1);
    checkButtonRepeat(BUTTON_MAIN2);
    checkButtonRepeat(BUTTON_MAIN3);
    checkButtonRepeat(BUTTON_MAIN4);
    checkButtonRepeat(BUTTON_MAIN5);
    checkButtonRepeat(BUTTON_MAIN6);
    checkButtonRepeat(BUTTON_SUB1);
    checkButtonRepeat(BUTTON_SUB2);
    checkButtonRepeat(BUTTON_SUB3);
    checkButtonRepeat(BUTTON_ENTER);
    checkButtonRepeat(BUTTON_UP);
    checkButtonRepeat(BUTTON_SHIFT);
    checkButtonRepeat(BUTTON_DIAL1);
    checkButtonRepeat(BUTTON_DIAL2);
    checkButtonRepeat(BUTTON_DIAL3);
    checkButtonRepeat(BUTTON_SELECT1);
    checkButtonRepeat(BUTTON_SELECT2);
    checkButtonRepeat(BUTTON_SELECT3);
    checkButtonRepeat(BUTTON_SELECT4);
    checkEncoder();
  }

  bool Events_waitWithTimeout(uint32_t timeout)
  {
    check();
    return local.events.waitForAll(onPost, timeout) & onPost;
  }

  void Events_wait(void)
  {
    check();
    local.events.waitForAll(onPost);
  }

  uint32_t Events_pull(void)
  {
    uint32_t value = EVENT_NONE;
    fifo_pop(&local.q, &value);
    return value;
  }
}