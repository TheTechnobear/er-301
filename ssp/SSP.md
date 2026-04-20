# SSP UI and Integration, by the TheTechnobear
Getting the er-301 emulator ('emu') running on the SSP was a first step (phase 1), see '../README-SSP.md' for details on that.
The emulator has the audio hooks and a useable UI, but its not integrated with SSP hardware, nor uses its form factor.
this sub projects creates an SSP specific intefaace on to the underlying er301 firmware.

# Scope (initial)
Phase 1 is to integrate tightly with the SSP hardware, a dedicated UI, and ensure the hardware is fully utilised e.g. IO.
the basic premise is, the visualised buttons, jacks, encoder etc are a skeumorphic UI, which are not required on the SSP hardware platform, as it has a phyical form factor. also the main window and subwindow need to be large / more promient given the size of the SSP display, compared to a desktop.
the workflow etc, will initally be retained, buttons and encoders wil however be switched to using the SSP hardware instead (or additionally).


# Status
- stripped unnecessary UI
- added inputs (have map but not used yet)



# To do
- UI for SSP , labels
- encoder mapping, see design ideas
- button mapping see design ideas


# Limitatons
- SDL2 is limited to max 8 channels in and out! , need to consider alternative.

alternatives? 
Juce - too heavy, many dependancies
rt_audio  - like traxhost, rtaudio for audio + SDL for display (only use SDL on mac, also for keyinput)
rt_audio + somthing else, to ditch SDL

think rt_audio + SDL for initial test

using 
https://github.com/tsoding/olive.c




# Out of scope (phase 1.)
things out of scope, see below for more details.
- XMX (vs SSP)
- Changes to the er-301 firmware, see below
- Synthor plugin

# XMX implementation
not technically hard, but the I need to see how the UI fits onto the larger SSP, then decide if it makes sense for XMX.
less buttons, smaller screen spcace, less IO make it questioable.

# Deeper integegation (phase 2+ ?)
the emulator is built on emualating the hardware thru a HAL. this integration is pretty low level, and requires no (?) changes in the underlying firmware thats used on the er-301 hardware. this SSP version will do the same.
this means there are quite a lot of restrictions on how the SSP / emulator thru the HAL can interact with the firmware, due to the firmware being 'hardcoded' (?) to the hardware form factor. 
In theory, (I'd need to check the codebase) we could go deeper and try to change the UI / form factor, and just use the original software more as components, and as 'starting points' for a custom implementation of the 'firmware'.
this way we could have a new workflow, and open a lot of new possibltiies,e.g. more tracks.
HOWEVER, whilst modules would remain compatible, the firmware would no longer be compatible with the er301 hardware (or emu).
Its a bit step, and something, Id only be interested in IF I find the projects is something I want to dig much deeper into, and has longer term futur


# Standalone vs Synthor plug
I make this a synthor plugin, which has some benefits (eg. multiple instances), but its non-trivial as, Id potentialy have to drag in a LOT of dependancies. I could avoid this by making it a 'native' SSP pluging, but thats creates more effort, and also makes it more difficult to test.
ALSO, Im keen to give lighter, more focused experience - less is more. 
Synthor and ER301 are two complex beasts already, combining is possibly a bit of overkill. 



# Observations

## display
we can change the screen display size, but not the size of the windows, in hal/display.h we have 
```c++
#define MAIN_HORIZONTAL_PIXELS 256
#define MAIN_VERTICAL_PIXELS 64
```

changing these e.g. double dosn't work, you just get the display twice.
I suspect this is due to graphics/MainFrameBuffer.cpp encoding a particular size framebuffer...
which is then rendered on to the surface.


Fonts are made available via Windows.cpp:54, these are only used for labels, they are not used in the main display.
(these labels are obviously on the hardware panel, so the only 'fonts' used in the hardware are for the windows)

The Windows are drawn via the framebuffer as bitmaps from selection of fonts and sizes. od/graphics/fonts,
limited sizes as not truetype, and not something we could easily override

## IO handling

hardware is quite different to how emu works, so lets dig in...

arch/am335x/hal 
the hardware had 3 inputs sources and 1 output.
IN
MOD0/1  - spi/mod*.c/h - Mod0/1 come from an TWO external (via SPI) adc : ADS8688 , each 8 channels,  16 bit 60khz
this gives ABCD 1-3, 12 channels + IN 1-4 
ADC adc.c - interal ADC gives 4 x 12 bit 'gate inputs , we get IN G1-G4, 96kHz


OUT 
audio.c - Audio output is 4 channels - and is 4 channels 32bit ot 24 depending on SR

all the callbacks are hal/pump.cpp

these run at different rates/clocks! 

so its stores frame from the callbacks, and then the audio_callback(), it combines then.
(resampling due to different timings and format)
then it calls  Pump_callback(self.in_frame, self.out_frame), with these combined input frames.

i.e. the callback sees all the sources as 'one' set of channels.
which is where we see things get combined in channels.h

we find in AudioThread.cpp
```
extern "C"
{
  void Pump_callback(float *inputs, float *outputs)
  {
    od::local->audioTimer.start();
    od::local->tasks.process(inputs, outputs);
    od::local->audioTimer.stop();
  }
}

ok, so back to the SSP/EMU

audio.C calls, Audio_callback() thats implmented in hal/pump/pump.cpp, and this is how it ties into the er301 infra,
basicallu calling Pump_callback().
