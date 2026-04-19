# SSP UI and Integration, by the TheTechnobear
Getting the er-301 emulator ('emu') running on the SSP was a first step (phase 1), see '../README-SSP.md' for details on that.
The emulator has the audio hooks and a useable UI, but its not integrated with SSP hardware, nor uses its form factor.
this sub projects creates an SSP specific intefaace on to the underlying er301 firmware.

# Scope (initial)
Phase 1 is to integrate tightly with the SSP hardware, a dedicated UI, and ensure the hardware is fully utilised e.g. IO.
the basic premise is, the visualised buttons, jacks, encoder etc are a skeumorphic UI, which are not required on the SSP hardware platform, as it has a phyical form factor. also the main window and subwindow need to be large / more promient given the size of the SSP display, compared to a desktop.
the workflow etc, will initally be retained, buttons and encoders wil however be switched to using the SSP hardware instead (or additionally).


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



