#include <hal/adc.h>
#include <hal/log.h>

// eqiv of ../../arch/am335x/hal/adc.c
// 4 'soft gates' , so analog, 12bit @96khz
// see channels for how they later get assigned, basically they are put after mods
// callback on  Adc_callback(int *samples)

void Adc_init() {}

void Adc_start(void) {}

void Adc_stop(void) {}
