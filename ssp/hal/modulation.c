#include <hal/modulation.h>
#include <hal/channels.h>
#include <string.h>

// ./hal/modulation.h:  extern void Mod0_callback(int *samples);
// ./hal/pump/pump.cpp:void Mod0_callback(int *samples)

// ./testing/darwin/ssp/od/glue/app_swig.cpp:static int _wrap_Mod0_callback(lua_State* L) { { int SWIG_arg = 0; int *arg1 = 0 ; SWIG_check_num_args("Mod0_callback",1,1)
// ./testing/darwin/ssp/od/glue/app_swig.cpp:    if(!SWIG_isptrtype(L,1)) SWIG_fail_arg("Mod0_callback",1,"int *");
// ./testing/darwin/ssp/od/glue/app_swig.cpp:    if (!SWIG_IsOK(SWIG_ConvertPtr(L,1,(void**)&arg1,SWIGTYPE_p_int,0))){ SWIG_fail_ptr("Mod0_callback",1,SWIGTYPE_p_int); } 
// ./testing/darwin/ssp/od/glue/app_swig.cpp:    Mod0_callback(arg1); return SWIG_arg; fail: SWIGUNUSED; }  lua_error(L); return 0; }
// ./testing/darwin/ssp/od/glue/app_swig.cpp:    { "Mod0_callback", _wrap_Mod0_callback},


// ./arch/am335x/hal/spi/mod0.c:      Mod0_callback(self.ping);
// ./arch/am335x/hal/spi/mod0.c:      Mod0_callback(self.pong);


// ADS8688  external ADC (Analog-to-Digital Converter) connected via SPI to the am335x chipset, rather than a built-in ADC on the chip itself.
// SPI Communication: The file defines SPI command constants like ADS8688_CMD_AUTO_RST, ADS8688_CMD_MANUAL_CH_0 through ADS8688_CMD_MANUAL_CH_7, indicating channel-based ADC control via SPI.
// DMA Configuration: The Connection struct includes EDMA3 (Enhanced Direct Memory Access) configuration for efficient data transfer between the SPI peripheral and memory.
// Input Range Settings: Defines for bipolar and unipolar ranges (±2.5V, ±1.25V, ±0.625V, and unipolar variants), which are typical for precision ADC configurations.
// 8-Channel Support: The MOD_NUM_CHANNELS_PER_ADC suggests this is an 8-channel ADC module with ping-pong buffering for continuous data acquisition.
// The ADS8688 is a Texas Instruments 8-channel, 16-bit simultaneous-sampling ADC, commonly used in industrial and audio applications. In the ER-301 context, this appears to be part of the modulation input (MOD) system for analog signal acquisition.


// a bit 'misnamed' these are the 'hi-res' analog inputs, at 16bit @ 60kHz, ( adc, for soft gates are 12bit)
// two adc that callback on Mod0_callback(int *samples), Mod1_callback(int *samples);

static struct
{
  uint32_t range0[MOD_NUM_CHANNELS_PER_ADC];
  uint32_t range1[MOD_NUM_CHANNELS_PER_ADC];
} local;

void Modulation_init()
{
  memset(&local, 0, sizeof(local));
}

void Modulation_start(void)
{
}

void Modulation_restart(void)
{
}

void Modulation_setChannelRange(uint32_t channel, uint32_t range)
{
  if (channel >= MOD_NUM_CHANNELS)
    return;

  if (channel < MOD_NUM_CHANNELS_PER_ADC)
  {
    local.range0[channel] = range;
  }
  else
  {
    channel -= MOD_NUM_CHANNELS_PER_ADC;
    local.range1[channel] = range;
  }
}

uint32_t Modulation_getChannelRange(uint32_t channel)
{
  if (channel >= MOD_NUM_CHANNELS)
    return MOD_BIPOLAR_2500;

  if (channel < MOD_NUM_CHANNELS_PER_ADC)
  {
    return local.range0[channel];
  }
  else
  {
    channel -= MOD_NUM_CHANNELS_PER_ADC;
    return local.range1[channel];
  }
}
