# Build A Minimal XMX Panel And Controller

## Brief Description
Create a quick minimal `XmxPanel` and `XmxController` path that is good enough to bring up the main display and exercise the basic product selection path, without trying to make XMX fully functional yet.

## Why This Is Needed
The next practical milestone after SSP equivalence is to prove that the refactor can host a second product variant in a lightweight but honest way. The goal here is not full XMX parity or full hardware behavior. It is to get a minimal path that renders the main display, validates the panel/controller split, and proves that the current build/runtime structure can support XMX without forcing SSP assumptions everywhere.

This should stay deliberately small. A fast minimal bring-up is more useful than prematurely designing the full XMX interaction model.

## What The Work Entails
- Define the smallest panel/controller behavior needed to show the main display on the XMX path.
- Reuse shared runtime/platform pieces where they are already valid.
- Avoid pulling in SSP-specific UI behavior unless it is truly generic.
- Keep the XMX controller minimal and explicit about what is stubbed or provisional.
- Validate that the resulting target still builds cleanly and runs through the expected startup path.

## Notes
This task is intentionally not asking for a final XMX UI. It is a proof-of-shape task that should stay narrow and reversible.


## Additonal notes

Ive now got a 'minimal xmx' panel in place, which proves the main display works and , importantly, is readable (scale = 1).
also the audio is working.

alot more work needed as the SSP panel, that I copied from, is sized inappropriately.
intially impresions are, we might be able to get the sub display on same screen, which simplifies things a lot.
though the issue may be then placing of button labels, and toggle etc 
all this can be tested on mac thru the emulation.

also I need to confirm display dimensions. framebuffer is def 320x320, but Ive a feelng display is supposed to be 320x240?


note: I did not have core package built, so used testosc