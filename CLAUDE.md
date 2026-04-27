# ER-301 Sound Computer — Codebase Guide

## What This Project Is

Firmware and software for the **ER-301 Sound Computer**, a eurorack DSP module by Orthogonal Devices. The repo contains:

- The core DSP engine and UI (`od/`, `hal/`)
- The original TI AM335x firmware application (`app/`)
- An SDL2-based desktop development emulator (`emu/`)
- A port to **Percussa SSP and XMX** hardware (`percussa/`) — the active development target

The Percussa port (`percussa/`) is what is currently being worked on. It replaces the TI-RTOS/AM335x layer with Linux + RtAudio running on Percussa hardware, while reusing the full `od/` DSP engine.

## Directory Map

| Directory | Role |
|-----------|------|
| `hal/` | Hardware Abstraction Layer — platform-neutral headers only |
| `od/` | Core application: DSP engine, UI infrastructure, Lua bindings |
| `percussa/` | Percussa SSP/XMX platform implementation |
| `emu/` | SDL2 desktop emulator (macOS/Linux) |
| `app/` | Original TI AM335x firmware application (SysBIOS) |
| `xroot/` | Lua script tree — UI logic and patch definitions |
| `libs/` | Third-party libraries (rtaudio, lua, lodepng, SDL_FontCache) |
| `scripts/` | Make-based build orchestration |
| `mods/tutorial/` | Mod SDK and example units |

## Key Concepts

### HAL (`hal/`)

Pure-header contracts. Every platform provides its own implementation. Key files:

- `audio.h` — `Audio_callback()` drives raw audio I/O
- `pump.h` — `Pump_callback(float *inputs, float *outputs)` is the real-time DSP entry point
- `events.h` — hardware events (buttons, encoders, USB) as 32-bit encoded values
- `display.h` — dual framebuffer (main 256×64 4-bit, sub 128×64 1-bit)
- `channels.h` — input channel routing macros (20 in, 4 out)
- `constants.h` — frame sizes (MAX_AUDIO_FRAME_LENGTH=128), channel counts

### OD Layer (`od/`)

The platform-independent DSP engine and application.

- `AudioThread` — singleton managing the real-time audio graph via `TaskScheduler`
- `UIThread` — singleton managing display refresh (55 Hz) and the graphics scene graph
- `od/tasks/` — `Task` base class, `TaskScheduler`, `InputTask`, `OutputTask`, `UnitChain`
- `od/units/` — `Unit` (DAG container with compiled execution order), `GraphCompiler`
- `od/objects/` — `Object`/`Inlet`/`Outlet`/`Parameter` — the DSP graph nodes
- `od/audio/` — DSP primitives: resampling, sample I/O, convolution, onset detection
- `od/graphics/` — framebuffer, scene graph (`Graphic`, `GraphicContext`), display widgets
- `od/glue/` — SWIG bindings exposing C++ objects to Lua (`app.i`)

### Percussa Port (`percussa/`)

Implements all `hal/` contracts for Linux + Percussa hardware:

- `percussa/hal/` — concrete hal implementations (RtAudio, POSIX pthreads, fbdev/SDL2 display)
- `percussa/platform/` — `Platform` interface with `HostSdlPlatform`, `FbdevPlatform`, `PluginPlatform`
- `percussa/app/` — `Bootstrap` (init, config, Lua thread)
- `percussa/panel/` — hardware panel abstraction (SSP vs XMX encoders/displays)
- `percussa/runtime/` — `Runtime` main loop coordinator
- `percussa/ui/` — SSP/XMX UI widgets

## Audio Processing Flow

```
Hardware → Pump_callback(inputs, outputs)
              ├── InputTask  (fills 20 Outlet buffers)
              ├── TaskScheduler (priority-ordered Tasks)
              │    └── UnitChain → Unit → Object::process() chain
              └── OutputTask (copies 4 inlet buffers to DAC)
```

Frame length: 32/64/128 samples. Sample rate: 48 or 96 kHz.

## Threading

- **Audio thread** — `Pump_callback()` called by RtAudio; runs entire DSP graph per frame
- **UI thread** — display refresh at 55 Hz, reads from audio thread via lock-free queues
- **Lua thread** — Lua interpreter for patch logic, launched by Bootstrap

Cross-thread communication uses lock-free queues (`LockFreeQueue`, `ConnectionQueue`) and pre-allocated buffer pools.

## Build

```sh
make percussa    # Percussa SSP/XMX
make emu         # SDL2 desktop emulator
make app         # TI AM335x firmware
```

Build scripts are in `scripts/*.mk`.

## Architecture Deep Dive

See `percussa/er301-architecture.md` for detailed documentation of the DSP/Task system and Percussa adaptation.
