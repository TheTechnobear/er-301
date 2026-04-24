# Percussa Refactor Plan

## Purpose

This is a living document for the `percussa` effort.

It should answer four questions at any point in time:

- what `percussa` is trying to become
- what design decisions are already settled
- what is actually integrated today
- what still needs to be done next

This document is intentionally not a session log. Historical details belong in commit history and short handoff notes. This file should stay biased toward current position and remaining work.

## Scope And Constraints

Current working assumptions:

- `ssp` remains unchanged while the new architecture is proven in `percussa`
- changes should stay inside `percussa`, `scripts/percussa.mk`, and the top-level `Makefile` unless explicitly expanded
- platform selection is build-time, not runtime
- panel selection is build-time, not runtime
- unused platform dependencies should not be required by unrelated targets
- panel layout ownership belongs in panel-specific code, not generic widgets
- the deeper ER-301 migration seam is around `Pump_callback(...)`, `od/AudioThread.cpp`, and the broader runtime that currently lives behind `SSPCore`

## Current Build Model

Top-level targets:

- `make percussa-ssp`
- `make percussa-xmx`
- `make percussa-ssp-clean`
- `make percussa-xmx-clean`

Output directories:

- `testing/darwin/percussa-ssp`
- `testing/darwin/percussa-xmx`
- `testing/linux/percussa-ssp`
- `testing/linux/percussa-xmx`

Current platform selection:

- on macOS, `PERCUSSA_PLATFORM` defaults to `host-sdl`
- on linux, `PERCUSSA_PLATFORM` defaults to `fbdev`
- linux SDL is still available, but only by explicitly setting `PERCUSSA_PLATFORM=host-sdl`

Important settled design change:

- `PERCUSSA_PLATFORM` is now the single source of truth for platform choice
- the earlier separate SDL toggle has been removed because SDL and fbdev are mutually exclusive platform choices
- cross toolchain selection is explicit via `PERCUSSA_TOOLCHAIN_FILE`; no toolchain file means native build

## Settled Design Decisions

These are no longer open questions.

### 1. Product variants build separately

`SSP` and `XMX` are separate targets with separate output directories. They are not one binary with runtime panel switching.

### 2. Platform is chosen at build time

`host-sdl`, `fbdev`, and later `plugin` are separate platform selections. Non-selected platform code should not drag in its dependencies.

### 3. Panel owns layout

`SspPanel` and `XmxPanel` own geometry and panel-specific visual composition. Generic UI code should stay dumb.

### 4. Platform owns backend integration

SDL, framebuffer presentation, Linux input polling, and future plugin host callbacks belong in `platform` and nearby transport code, not in shared runtime code.

### 5. Olive is not the abstraction target

The real boundary is the rendered buffer handoff and the runtime/audio frame handoff, not Olive itself.

### 6. Audio migration should follow HAL boundaries

`percussa/hal/audio.c` and `percussa/hal/pump/*` are the correct outer file locations. The remaining problem is the deeper callback owner and broader ER-301 runtime behind them.

## What Exists Today

### Architecture skeleton

The project already has concrete `runtime`, `panel`, `platform`, `ui`, `input`, and `hal` areas under `percussa`.

Implemented structure includes:

- product-specific panels and controllers for `ssp` and `xmx`
- a shared renderer based on Olive
- host SDL presentation
- fbdev presentation and Linux hardware input transport
- a plugin platform stub
- SSP bootstrap/config/session helpers and a growing set of mirrored HAL/runtime files

### Real SSP-side integration already in place

The SSP path is no longer just scaffold code. It now includes real migrated slices for:

- panel geometry and front-panel state
- bootstrap/config/session startup
- GPIO-backed front-panel behavior
- encoder/events/PWM/ADC init chain
- card/config/rng/usb/modulation init chain
- heap init
- HAL-shaped audio shell
- mirrored `hal/pump` support code
- card state and legacy hardware bridges

### XMX state

`XmxPanel` and `XmxController` exist and build, but the layout remains provisional. The current assumption set is:

- one `320x240` main display
- four encoders
- soft buttons `1` to `8`

That is enough to prove the architecture, but not enough to call the XMX definition complete.

XMX cross-build validation now goes through an explicit toolchain include.

Valid shape:

- `XMX_BUILDROOT=... make percussa-xmx PERCUSSA_TOOLCHAIN_FILE=scripts/toolchains/xmx.mk`

The matching SSP form is:

- `SSP_BUILDROOT=... make percussa-ssp PERCUSSA_TOOLCHAIN_FILE=scripts/toolchains/ssp.mk`

Cross-built support libraries are also separated by toolchain now, so SSP and XMX no longer reuse the same `testing/linux/libs` archive outputs.

Cross-built FFTW staging is also separated by toolchain now:

- SSP uses `testing/linux/fftw3-ssp/usr`
- XMX uses `testing/linux/fftw3-xmx/usr`

Invalid combinations remain invalid:

- XMX toolchain with `percussa-ssp`
- SSP toolchain with `percussa-xmx`
- raw `BUILDROOT=... make percussa-*` without `PERCUSSA_TOOLCHAIN_FILE`



## What Is Actually Working

Validated working paths at the moment:

- `make percussa-ssp` on macOS
- `make percussa-xmx` on macOS
- `SSP_BUILDROOT=... make percussa-ssp PERCUSSA_TOOLCHAIN_FILE=scripts/toolchains/ssp.mk` for the linux cross build
- `XMX_BUILDROOT=... make percussa-xmx PERCUSSA_TOOLCHAIN_FILE=scripts/toolchains/xmx.mk` for the linux cross build
- panel-specific output directories and clean targets
- separated SSP and XMX staged dependency outputs for support libraries and FFTW
- linux defaulting to `fbdev`
- explicit linux override with `PERCUSSA_PLATFORM=host-sdl`



What that means in practice:

- the honest full-surface percussa build now links through the broader ER-301 surface instead of the earlier shallow percussa-only build
- the current build system shape is close enough to use the compiler and linker to expose real missing migration seams

## What Is Missing

This is the important section.

### 1. The real audio/runtime callback owner is still missing

Current state:

- `percussa/hal/audio.c` is SSP-derived and real
- mirrored `percussa/hal/pump/*` is in place
- `Audio_callback(...)` exists in the right place
- `Pump_callback(...)` is still not backed by the real migrated owner

What is missing:

- the `od/AudioThread.cpp` side of the system
- the broader runtime/process graph that should actually service `Pump_callback(...)`

This is the main technical seam left in the SSP integration path.

### 2. `runtime` is still too thin

Current state:

- `runtime` coordinates startup and platform delegation
- some SSP-only bridging exists through `LegacyHardware` and `CardState`

What is missing:

- a real shared runtime layer that owns application lifecycle, processing boundaries, and buffer handoff in a way that can host more of the ER-301 core

### 3. `scripts/percussa.mk` still needs cleanup

Current state:

- the build is now honest and working across the current variants
- platform and panel selection are build-time only
- platform choice is no longer duplicated by an extra SDL flag

What is missing:

- clearer source grouping
- less reliance on large recursive source lists plus `filter-out`
- easier-to-read separation between common sources, panel-specific sources, and platform-specific sources

This is now a maintainability task, not a correctness blocker.

### 4. XMX is still an architectural placeholder

Current state:

- XMX builds cleanly
- XMX has separate panel and controller classes
- XMX now also cross-builds cleanly with the explicit XMX toolchain and separated staged dependencies

What is missing:

- a real hardware-backed panel definition
- any deeper XMX-specific runtime behavior beyond the current scaffold level

the build-side separation is now in place; the remaining XMX work is runtime and hardware-definition work, not cross-toolchain plumbing.

### 5. `plugin` remains intentionally incomplete

Current state:

- the plugin platform exists only as a stub

What is missing:

- host-provided framebuffer handoff
- host-provided audio callback integration
- dependency decisions around whether plugin needs any of the current standalone platform/audio machinery

Plugin remains out of scope for the current step.

### 6. The copied Lua/od glue integration is still narrow

Current state:

- `percussa/od/glue/Interpreter.*` exists

What is missing:

- the broader `AppInterpreter` surface
- `luaopen_app(...)`
- whichever additional glue/runtime pieces are required once the real audio/runtime ownership is migrated

## Immediate Priorities

The next work should stay ordered and narrow.

### Priority 1. Migrate the real callback owner behind `Pump_callback(...)`

This is the most important remaining SSP-side seam.

Success looks like:

- `Pump_callback(...)` is no longer a placeholder boundary
- the migrated code path is driven by the real `od/AudioThread.cpp` ownership model, or by a very explicit extracted equivalent

### Priority 2. Strengthen `runtime` around the processing boundary

Once the callback owner is clearer, reshape `runtime` so it is more than startup glue and controller dispatch.

Success looks like:

- clearer ownership of lifecycle and processing boundaries
- less SSP-specific bridging logic leaking outward from the old `SSPCore` model

### Priority 3. Clean up `scripts/percussa.mk`

Now that the target split and platform selection are behaving correctly, simplify the file structure.

Success looks like:

- explicit source groups
- fewer broad recursive lists with exclusions
- easier review of what each target actually compiles

### Priority 4. Replace provisional XMX assumptions with a real panel definition

This should happen after the SSP-side runtime/audio seam is better understood.

Build-side SSP/XMX separation is already validated and should not be treated as the next blocker.

## Not In Scope Right Now

- rewriting `ssp`
- adding plugin integration beyond keeping the platform slot available
- broad stylistic cleanup outside the active percussa migration path
- solving unrelated warning cleanup across `od`

## Current Validation Commands

Use these as the current smoke tests.

- `make percussa-ssp`
- `make percussa-xmx`
- `./testing/darwin/percussa-ssp/percussa-ssp.elf --once`
- `./testing/darwin/percussa-xmx/percussa-xmx.elf --once`
- `SSP_BUILDROOT=... make percussa-ssp PERCUSSA_TOOLCHAIN_FILE=scripts/toolchains/ssp.mk`
- `SSP_BUILDROOT=... make percussa-ssp PERCUSSA_TOOLCHAIN_FILE=scripts/toolchains/ssp.mk PERCUSSA_PLATFORM=host-sdl` (low priority)
- `XMX_BUILDROOT=... make percussa-xmx PERCUSSA_TOOLCHAIN_FILE=scripts/toolchains/xmx.mk`

If the staged FFTW dependencies need rebuilding:

- `TOOLCHAIN_FLAVOR=ssp ./scripts/build-fftw-cross.sh`
- `TOOLCHAIN_FLAVOR=xmx ./scripts/build-fftw-cross.sh`

Invalid combinations:

- `XMX_BUILDROOT=... make percussa-ssp PERCUSSA_TOOLCHAIN_FILE=scripts/toolchains/xmx.mk`
- `SSP_BUILDROOT=... make percussa-xmx PERCUSSA_TOOLCHAIN_FILE=scripts/toolchains/ssp.mk`
- raw `BUILDROOT=... make percussa-*` without `PERCUSSA_TOOLCHAIN_FILE`


## Definition Of Progress

The project is moving in the right direction if each step does at least one of these:

- replaces a scaffold-only path with a real migrated ownership slice
- removes a fake or duplicated build/runtime switch
- increases the amount of honest ER-301 code compiled and linked by `percussa`
- shrinks the remaining gap around `Pump_callback(...)`, `od/AudioThread.cpp`, and the shared runtime/process boundary
