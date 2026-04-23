# Percussa Scaffold

This directory is a clean refactor target for the current `ssp` subproject.
It is intentionally separate so the new architecture can be exercised without
forcing early changes into `ssp`.

## Design

The new design separates the product into four main layers:

1. `runtime`
   Owns shared application coordination, boot policy, and the stable handoff
   into ER-301-facing processing.

2. `panel`
   Owns panel-specific UI layout and control inventory.
   `SSP` and `XMX` are separate panel implementations that reuse shared UI
   component types.

3. `platform`
   Owns entrypoints and backend-specific integration such as display delivery,
   audio callback hookup, and hardware or host input transport.

4. `controller`
  Owns product-specific mapping from hardware actions into presentation state.

## Intent

- Keep `ssp` unchanged while the new ownership model is proven.
- Avoid spreading platform conditionals through shared runtime code.
- Treat `SSP`, `XMX`, and `plugin` as peer targets.
- Keep Olive as the shared drawing implementation.
- Keep the effective ER-301 audio seam centered on frame exchange around
  `Pump_callback(...)`.

## Initial Structure

- `runtime/Runtime.*`
  Shared coordinator.
- `panel/Panel.h`
  Shared panel interface.
- `panel/Controller.h`
  Shared controller interface.
- `panel/Family.*`
  Product-family assembly for matching panel and controller pairs.
- `panel/ssp/SspPanel.*`
  SSP panel layout.
- `panel/ssp/SspController.*`
  SSP interaction policy.
- `panel/xmx/XmxPanel.*`
  XMX panel layout.
- `panel/xmx/XmxController.*`
  XMX interaction policy.
- `platform/*.h|*.cpp`
  Early platform adapter scaffolding.
- `ui/*.h`
  Generic renderable component definitions.

## Brief Plan

1. Land a compileable scaffold with explicit runtime, panel, and platform
   boundaries.
2. Move platform selection to build-time so each build compiles only the
  relevant platform implementation and dependencies.
3. Model `SSP` and `XMX` as separate panel families using shared UI components.
4. Add host SDL, fbdev, and plugin-oriented platform adapters behind a common
  platform interface.
5. Port behavior from `ssp` gradually, starting with layout ownership,
   presentation ownership, and input-action boundaries.
6. Keep build selection in `percussa.mk` rather than pushing more target
   branches into core classes.
7. Keep product panel/controller pairing out of `main.cpp` by assembling it in
  the panel layer.

## Near-Term Migration Targets

- Move layout ownership out of widget classes and into panel classes.
- Keep platform and panel selection at build-time, not runtime.
- Shrink the future runtime coordinator so it does not absorb backend policy.
- Hide SDL inside a host platform adapter.
- Treat fbdev and plugin frame presentation as alternate display targets for
  the same shared rendering output.

## Build Selection

Panel and platform are selected at build time.

- `PERCUSSA_PANEL=ssp|xmx`
- `PERCUSSA_PLATFORM=host-sdl|fbdev|plugin`

Cross builds should go through the existing buildroot flow, for example:

- `BUILDROOT=$SSP_BUILDROOT make percussa PERCUSSA_PANEL=ssp`

## SSP Bootstrap Migration

`PERCUSSA_PANEL=ssp` now includes a first real `SSPCore.cpp` bootstrap/config migration instead of relying only on panel/controller scaffold.

Current migrated pieces:

- SSP command-line parsing and key/value config helpers
- local HAL support for logging, timing, UART, and fileops
- migrated EventFlags, Encoder, Events, PWM, and ADC support used by the next `SSPCore::run()` init batch
- migrated Card, Config, Rng, USB, and Modulation support used by the remaining small pre-audio `SSPCore::run()` init calls
- existing arch heap support is now wired into SSP builds and `Heap_init()` is restored in the bootstrap order
- `percussa/hal/audio.c` is now based on `ssp/hal/audio.c`, using RtAudio plus SSP channel mapping at the HAL boundary
- `percussa/hal/pump/*` now mirrors the HAL pump file layout and provides the current strong `Audio_callback(...)` path
- SSP config/session restore and save via `percussa/runtime/ssp/SspBootstrap.*`
- a narrow copied `od/glue/Interpreter` that initializes Lua and seeds `package.path` and `app.roots`
- an SSP-only runtime bridge that feeds shared hardware actions into the migrated legacy GPIO/event/encoder surface

Current behavior:

- host builds use `~/.ssp`
- fbdev builds use `/media/BOOT/er301`
- missing host config files are created automatically as `~/.ssp/ssp.config`
- host SSP runs now read firmware config from `~/.ssp/rear/firmware.cfg`
- SSP toggle state is restored from and saved to the session file
- SSP action input now also drives the migrated legacy encoder and event queue surface
- SSP bootstrap now initializes the SSP-derived HAL audio path and pump init path; the deeper `Pump_callback(...)` side is still pending

This is intentionally not the full `AppInterpreter` migration yet. It wires in the smallest useful `SSPCore` bootstrap slice first.

## Host SDL Input

The host SDL keyboard path is intended to emulate hardware-facing panel input,
not UI widgets directly. It should only emit shared hardware actions such as
`button(id, pressed)`, `encoder(id, delta)`, and encoder-press actions.

Current host key map:

- `1 2 3 4 q w e r` -> `Button1..Button8`
- arrow keys -> `Left/Right/Up/Down`
- left shift / right shift -> `ShiftL / ShiftR`
- `u i o p` -> `P1..P4`
- `a/z`, `s/x`, `d/c`, `f/v` -> encoder `1..4` delta `-1/+1`
- `5 6 7 8` -> encoder press `1..4`
- `Esc` -> quit