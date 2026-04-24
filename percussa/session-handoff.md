# Percussa Session Handoff

## Current Position

`percussa` is no longer just a UI scaffold. It now has:

- separate `SSP` and `XMX` build targets
- build-time platform selection
- panel-specific output directories
- a growing real SSP migration surface through `hal`, `runtime/ssp`, and `panel/ssp`
- an honest full-surface build that compiles and links through the broader ER-301 surface instead of a shallow percussa-only subset
- explicit SSP and XMX linux cross-builds with separated staged dependencies

The work is still in the proving-ground phase. `ssp` remains unchanged.

## Current Design Shape

The active design assumptions are:

- `panel` owns product-specific layout
- `platform` owns SDL/fbdev/plugin entry and backend integration
- `runtime` should become the shared coordination and processing boundary
- `hal` and `od/glue` style boundaries should still be respected conceptually
- platform and panel selection happen at build time, not runtime

Important recent design cleanup:

- `PERCUSSA_PLATFORM` is now the single platform selector
- there is no separate SDL toggle anymore
- SDL and fbdev are treated as mutually exclusive platform choices
- cross toolchain selection is explicit via `PERCUSSA_TOOLCHAIN_FILE`; no toolchain file means native build

## Active Build Targets

Top-level targets:

- `make percussa-ssp`
- `make percussa-xmx`
- `make percussa-ssp-clean`
- `make percussa-xmx-clean`

Output roots:

- `testing/darwin/percussa-ssp`
- `testing/darwin/percussa-xmx`
- `testing/linux/percussa-ssp`
- `testing/linux/percussa-xmx`

Platform defaults:

- macOS defaults to `PERCUSSA_PLATFORM=host-sdl`
- linux defaults to `PERCUSSA_PLATFORM=fbdev`
- linux SDL must be requested explicitly with `PERCUSSA_PLATFORM=host-sdl`

## What Is Integrated Today

### Product/panel side

- `SspPanel`, `SspController`, and `SspFrontPanelState` exist and contain real migrated SSP layout/state slices
- `XmxPanel` and `XmxController` exist, but XMX remains provisional
- panel-family assembly is product-specific and build-time selected

### Platform/input/presentation side

- `HostSdlPlatform` is real and used on macOS host builds
- `FbdevPlatform` plus Linux input/framebuffer transport is present
- `PluginPlatform` is still a stub
- shared input actions exist and are used by the current host/Linux transports

### SSP-side runtime/HAL migration

The following SSP-oriented slices are already present in `percussa`:

- bootstrap/config/session helpers
- command line and key/value store helpers
- TLS
- logging, timing, UART, fileops
- EventFlags, Encoder, Events, PWM, ADC
- Card, Rng, Modulation, USB, Config
- heap init via the arch-specific heap implementation
- `percussa/hal/audio.c`
- mirrored `percussa/hal/pump/*`
- `LegacyHardware` and `CardState` bridges

This is enough to make the build and startup path substantially more truthful than the original scaffold.

## Main Remaining Seam

The key unresolved migration boundary is still:

- `Pump_callback(...)`
- `od/AudioThread.cpp`
- the broader ER-301 runtime/process ownership that should sit behind that callback path

Current state:

- outer HAL audio shell is real
- mirrored pump-side support is real
- `Audio_callback(...)` exists in the right place
- the real owner servicing `Pump_callback(...)` is not yet migrated

If you need one sentence for where the project is blocked architecturally, it is this: the build is now honest enough that the next real job is migrating the deeper callback/runtime owner, not adding more outer scaffolding.

## Current Gaps

### 1. Runtime is still too thin

It coordinates startup and some SSP-only bridges, but it is not yet the broader shared runtime layer that should own lifecycle and processing boundaries.

### 2. XMX is still provisional

The XMX build target is real, but the layout and behavior are still based on current assumptions rather than a fully verified hardware definition.

### 3. Plugin is still a stub

The platform slot exists, but real plugin framebuffer/audio host integration is still out of scope.

### 4. `scripts/percussa.mk` needs cleanup

It works, but it still relies on broad recursive source lists plus `filter-out`. Now that correctness is better, readability and maintainability should be improved later.

### 5. `od/glue` integration is still narrow

`Interpreter.*` exists, but the broader `AppInterpreter`/`luaopen_app(...)` surface is still not migrated.

### 6. Build-side SSP/XMX separation is no longer the blocker

The explicit toolchain split is now in place and validated.

Current validated shapes:

- `SSP_BUILDROOT=... make percussa-ssp PERCUSSA_TOOLCHAIN_FILE=scripts/toolchains/ssp.mk`
- `XMX_BUILDROOT=... make percussa-xmx PERCUSSA_TOOLCHAIN_FILE=scripts/toolchains/xmx.mk`

Separated staged dependency outputs are also in place:

- support libraries: `testing/linux/libs-ssp` and `testing/linux/libs-xmx`
- FFTW staging: `testing/linux/fftw3-ssp/usr` and `testing/linux/fftw3-xmx/usr`

That means the remaining work is no longer buildroot/toolchain plumbing. The next blocker is still the runtime/audio ownership seam.



## Recommended Next Step

Work the next step around the real callback owner behind `Pump_callback(...)`.

The right bias is:

- move one level deeper into the real ER-301 ownership path
- avoid inventing a fresh abstraction if a real existing owner already exists nearby
- keep the change local enough that the same build loop still exposes the next concrete missing seam

In practice that means focusing on `od/AudioThread.cpp` and the runtime/process graph around it, not on adding more outer HAL shells.

The build-side SSP and XMX split is now good enough that it should not be the focus of the next session unless a concrete new regression appears.

## Validation State

Confirmed recently:

- `make percussa-ssp`
- `make percussa-xmx`
- `SSP_BUILDROOT=... make percussa-ssp PERCUSSA_TOOLCHAIN_FILE=scripts/toolchains/ssp.mk`
- `XMX_BUILDROOT=... make percussa-xmx PERCUSSA_TOOLCHAIN_FILE=scripts/toolchains/xmx.mk`
- panel-specific output directories and clean targets
- linux defaulting to `fbdev`
- explicit linux SDL path via `PERCUSSA_PLATFORM=host-sdl`

Useful smoke commands:

- `make percussa-ssp`
- `make percussa-xmx`
- `./testing/darwin/percussa-ssp/percussa-ssp.elf --once`
- `./testing/darwin/percussa-xmx/percussa-xmx.elf --once`
- `SSP_BUILDROOT=... make percussa-ssp PERCUSSA_TOOLCHAIN_FILE=scripts/toolchains/ssp.mk`
- `XMX_BUILDROOT=... make percussa-xmx PERCUSSA_TOOLCHAIN_FILE=scripts/toolchains/xmx.mk`
- `TOOLCHAIN_FLAVOR=ssp ./scripts/build-fftw-cross.sh`
- `TOOLCHAIN_FLAVOR=xmx ./scripts/build-fftw-cross.sh`

## Do And Do Not

Do:

- keep changes inside `percussa`, `scripts/percussa.mk`, and the top-level `Makefile` unless there is a concrete reason to expand scope
- prefer real migrated code over new scaffolding
- use the build to expose the next missing boundary
- preserve build-time platform and panel selection

Do not:

- modify `ssp` unless explicitly requested
- reintroduce runtime platform or panel switching
- reintroduce duplicated platform knobs like the removed SDL toggle
- paper over the audio/runtime seam with another shallow placeholder if a nearby real owner exists
