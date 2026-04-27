# Copilot Instructions for ER-301

## Build commands

- Main active target: Percussa host/hardware port.
  - `make percussa`
  - `make percussa PERCUSSA_PANEL=xmx`
  - `make percussa TOOLCHAIN_FILE=scripts/toolchains/ssp.mk`
  - `make percussa TOOLCHAIN_FILE=scripts/toolchains/xmx.mk`
- Desktop emulator:
  - `make emu`
  - Run from the repo root as `testing/<arch>/emu/emu.elf` unless `XROOT` is configured explicitly.
- Legacy TI firmware/mod targets still exist:
  - `make app`
  - `make core`
  - `make teletype`
- Clean targets:
  - `make percussa-clean`
  - `make emu-clean`

Build outputs land under `PROFILE/ARCH/...`, where `PROFILE` defaults to `testing` and `ARCH` defaults from the host OS (`linux` or `darwin`) unless overridden. Percussa outputs are panel-specific, for example `testing/darwin/percussa-ssp/percussa-ssp.elf`.

There is no first-party top-level `make test`, single-test runner, or lint target in this repository. The checked-in GitHub Actions workflow currently validates `make emu` on Ubuntu and macOS.

## High-level architecture

- `hal/` is the platform contract layer. It is header-only; concrete implementations live in `app/`, `emu/`, and `percussa/hal/`.
- `od/` is the shared engine and UI stack. The real-time path is `Pump_callback()` -> `AudioThread` -> `TaskScheduler` -> `InputTask` / `UnitChain` tasks / `OutputTask`.
- `od/UIThread` is separate from audio and owns display refresh, graphics contexts, and the realtime-to-UI job queue.
- `xroot/` is the Lua application tree. Boot scripts, unit definitions, and UI behavior live there, with SWIG bindings exposed from `od/glue/` and `percussa/od/glue/`.
- `percussa/` is the active development target. It wraps the shared OD engine with build-time-selected panel and platform layers (`ssp` vs `xmx`, `host-sdl` vs `fbdev` vs `plugin`), plus startup/config handling in `percussa/app/Bootstrap.*` and runtime coordination in `percussa/runtime/`.

## Key conventions

- Treat the audio thread as hard real-time. Do not introduce blocking I/O, heap-heavy work, or ad hoc cross-thread mutations in code that runs under `Pump_callback()` / `AudioThread`. Existing patterns use preallocated frame buffers, lock-free queues, and `ConnectionQueue`.
- Graph wiring changes are staged, not applied immediately. Use the existing `AudioThread` / `ConnectionQueue` path for connect-disconnect behavior rather than mutating object graphs in place.
- Keep panel/platform selection at build time. In Percussa code, prefer `PERCUSSA_PANEL` and `PERCUSSA_PLATFORM`-driven composition over adding more runtime branching inside shared core classes.
- Preserve the layering: platform-neutral DSP and UI logic belongs in `od/`; hardware and OS integration belongs in `percussa/hal/`, `percussa/platform/`, `emu/hal/`, or `arch/*`. The Percussa port intentionally prefers percussa-local replacements where migration exists instead of pushing more target-specific branches into core code.
- Be aware of config-root differences when changing startup behavior:
  - emulator host config defaults to `~/.od/emu.config`
  - Percussa host config defaults to `~/.ssp/ssp.config`
  - Percussa fbdev defaults use `/media/BOOT/er301`
- If a change affects app boot, unit definitions, or UI behavior, check whether matching updates are needed in `xroot/` and in the SWIG glue layer, not just in C++.
