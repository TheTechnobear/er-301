# Percussa Port — Architecture and Implementation

This document describes how the Percussa SSP/XMX port is structured, how it connects to the underlying ER-301 firmware, and where it deviates from the original hardware in ways that have functional or performance consequences.

For the broader ER-301 DSP engine internals (Tasks, Units, Objects, signal graph) see `er301-architecture.md`.

---

## 1. Overview

The Percussa port reuses the entire ER-301 application layer (`od/`) unchanged. It replaces only the hardware-facing layer: the HAL implementations, the audio driver, the display backend, and the bootstrap/init sequence.

The boundary is clean and narrow. The ER-301 firmware defines a set of C contracts in `hal/` (headers only, no implementations). The Percussa port provides all implementations. The `od/` layer above the HAL never knows it is not running on the original AM335x hardware.

The single most important seam is:

```
Pump_callback(float *inputs, float *outputs)
```

Everything above this function is shared ER-301 code. Everything below it is platform-specific.

---

## 2. Directory Structure

```
percussa/
├── hal/            HAL implementations (audio, display, GPIO, concurrency, …)
├── platform/       Platform interface + SDL/fbdev/plugin backends
├── panel/          Panel interface + SSP and XMX implementations
│   ├── ssp/        SSP encoder layout, controller, display scaling
│   └── xmx/        XMX encoder layout and controller
├── runtime/        Runtime coordinator (main loop)
├── app/            Bootstrap and session management
├── ui/             Shared UI component types
├── tasks/          Percussa-specific Task subclasses (if any)
├── od/             Local overrides to od/ (AppInterpreter, glue)
├── input/          Input abstraction layer
├── hw/             Low-level hardware helpers
└── support/        Utility code
```

Build-time selection:
- `PERCUSSA_PANEL=ssp|xmx` — selects panel and controller
- `PERCUSSA_PLATFORM=host-sdl|fbdev|plugin` — selects display/input backend

Defaults are inferred from the host and target:
- macOS → `host-sdl` + `ssp`
- Linux aarch64 → `fbdev` + `xmx`
- Other Linux → `fbdev` + `ssp`

---

## 3. Layered Architecture

The port is organised into four layers, each with a single well-defined responsibility:

### 3.1 Platform (`percussa/platform/`)

Owns the process entry point, display delivery, and hardware input transport.

```cpp
class Platform {
    virtual int run(runtime::Runtime &runtime) const = 0;
};
```

| Implementation | Target | Notes |
|---|---|---|
| `HostSdlPlatform` | macOS / Linux desktop | SDL2 window; keyboard and mouse map to panel actions |
| `FbdevPlatform` | SSP / XMX hardware | Writes to `/dev/fb0`; reads physical encoders and buttons |
| `PluginPlatform` | VCV Rack plugin | Embeds the ER-301 engine inside a plugin host |

The platform is responsible for the event loop and for driving the UI refresh. It does **not** own audio — audio is driven by RtAudio independently (see §5).

### 3.2 Panel (`percussa/panel/`)

Owns the hardware panel layout: how many encoders exist, their logical IDs, and the display geometry.

```cpp
class Panel {
    virtual void init() = 0;
    virtual void update() = 0;
};
```

SSP panel (`panel/ssp/`):
- 4 encoders
- ER-301 displays scaled 3× to fit the SSP screen
- Encoder 1 = data wheel; encoders 2–4 = toggle/mode controls

XMX panel (`panel/xmx/`):
- 4 encoders
- ER-301 displays rendered at native resolution
- Same encoder mapping as SSP

`createPanel()` is a build-time factory that returns the correct concrete panel for the `PERCUSSA_PANEL` build variable.

### 3.3 Runtime (`percussa/runtime/`)

`Runtime` is the main loop coordinator. It owns the `Platform` and `Panel` instances and integrates UI rendering with the audio subsystem. It does not contain product policy — it delegates to platform and panel.

### 3.4 HAL (`percussa/hal/`)

Concrete implementations of all `hal/` contracts. See §4 for details.

---

## 4. HAL Implementation

Each file in `percussa/hal/` corresponds to a contract defined in `hal/`. The table below covers the significant ones.

| File | HAL contract | Implementation |
|------|-------------|----------------|
| `audio.c` | `audio.h` | RtAudio for cross-platform audio I/O |
| `display.cpp` | `display.h` | SDL2 texture (host) or `/dev/fb0` (fbdev) |
| `events.cpp` | `events.h` | Keyboard/mouse (SDL) or physical inputs (fbdev) |
| `encoder.cpp` | `encoder.h` | Mouse wheel (SDL) or hardware rotary encoders |
| `gpio.c` | `gpio.h` | Software-emulated state (no physical GPIO on Linux) |
| `concurrency/` | `concurrency/` | POSIX pthreads (vs TI-RTOS on firmware) |
| `card.cpp` | `card.h` | Filesystem paths (`~/.ssp/` or `/media/BOOT/er301/`) |
| `timing.c` | `timing.h` | `clock_gettime(CLOCK_MONOTONIC)` |
| `fft.c` | `fft.h` | FFTW3 (vs ARM NEON + custom FFT on firmware) |
| `simd.c` | `simd.h` | Scalar fallback (NEON intrinsics only on actual ARM hardware) |
| `pump/` | `pump.h` | Shared pump logic; audio path is mirrored from SSP HAL |

### Audio configuration

Channel routing and gain values differ between SSP and XMX and are defined in:
- `audio_config_ssp.h` — 8 in / 8 out; CV scaled from ±5V to ±10V
- `audio_config_xmx.h` — reduced channel count; same CV scaling
- `audio_config_macos.h` — development defaults for desktop

The audio config header is selected at compile time based on `PERCUSSA_PANEL`.

### Pump

`percussa/hal/pump/` mirrors the layout of `hal/pump/` from the original firmware. The strong `Audio_callback()` implementation sets up the SSP-specific channel demux and gain application before handing off to `Pump_callback()`. This keeps all channel-count and voltage-scaling knowledge inside the HAL, invisible to `od/`.

---

## 5. Bootstrap and Startup Sequence

`percussa/app/Bootstrap` runs once at process start:

1. Parse command-line arguments (config file path, overrides)
2. Load or create `ssp.config` / `xmx.config` (key-value text file)
3. Resolve paths: `XROOT`, `REAR_ROOT`, `FRONT_ROOT`, `SESSION`
4. Call `Heap_init()` (buffer pool allocation)
5. Initialise HAL subsystems: card, RNG, modulation, USB
6. Start the Lua interpreter thread (runs `xroot/boot/start.lua`)
7. Restore session state from the session file (patch, UI position)
8. Hand off to `Runtime::run()` → `Platform::run()`

The Lua interpreter runs on its own thread and communicates with the audio and UI threads through the same lock-free queues used on ER-301 firmware.

Config file location:
- Host builds: `~/.ssp/ssp.config`
- fbdev builds: `/media/BOOT/er301/ssp.config`

Missing config files are created with defaults on first run.

---

## 6. Audio Path in Detail

```
RtAudio callback thread
  │
  └─► Audio_callback(int *rawSamples)          [percussa/hal/audio.c]
          │
          ├── Channel demux using audio_config_ssp.h routing table
          ├── Apply input gain (CV ×2 scaling: ±5V → ±10V)
          │
          └─► Pump_callback(float *inputs, float *outputs)   [hal/pump/]
                   │
                   ├─► InputTask::process()         [od/tasks/]
                   │     Maps inputs[] → 20 named Outlet buffers
                   │
                   ├─► TaskScheduler::process()     [od/tasks/]
                   │     Priority-sorted Tasks:
                   │       UnitChain → Units → Objects (DSP graph)
                   │
                   ├─► OutputTask::process()        [od/tasks/]
                   │     Maps 4 Inlet buffers → outputs[]
                   │
                   └─► ConnectionQueue::process()   [od/tasks/]
                         Applies pending graph edits atomically
          │
          └── Apply output gain and write to RtAudio output buffer
```

On ER-301 hardware, `Audio_callback` is triggered by a hardware timer interrupt on the AM335x SoC at a fixed frame rate. On Percussa, it is the RtAudio callback — functionally equivalent but scheduled by the OS audio subsystem rather than bare-metal hardware.

---

## 7. Threading Model

The Percussa port uses three threads. Understanding their roles and how they communicate is important for any work touching audio or UI.

### Audio thread (RtAudio callback)

- Runs `Pump_callback()` once per audio frame at the configured frame rate
- Highest effective priority; on hardware the audio core is isolated from the OS scheduler
- **Must never block** — no mutexes, no dynamic allocation, no system calls
- Communicates outward via lock-free structures only:
  - **Outlet/Inlet buffers** — in-frame signal routing (no synchronisation needed; single writer, single reader per buffer)
  - **LockFreeQueue** — measurement data pushed to UI (FifoProbe, LoudnessProbe)
  - **ConnectionQueue** — receives graph edits from Lua thread, applied at end-of-frame

### UI / main thread

- Runs `Platform::run()` — the SDL2 or fbdev event loop
- Refreshes displays at 55 Hz (`GRAPHICS_REFRESH_RATE`)
- Reads encoder/button events from the `Events` queue (populated by platform input handlers)
- Drives Lua callbacks for UI interactions (these are synchronous on the Lua thread, not the UI thread)
- Communicates with audio thread via `JobQueue` (UI → audio for parameter changes) and `LockFreeQueue` (audio → UI for metering data)

### Lua interpreter thread

- Launched by `Bootstrap` as a detached background thread
- Runs `xroot/boot/start.lua` and then enters the Lua event loop
- Owns patch loading, unit instantiation, and parameter binding
- Submits graph modifications (connect/disconnect) through `ConnectionQueue` — these are queued and applied atomically by the audio thread at the end of a frame, so Lua never needs to synchronise directly with the audio callback

### Comparison with ER-301 firmware threading

**Single-core ER-301:** The AM335x has one CPU core. TI-RTOS/SysBIOS runs the audio callback as a high-priority ISR that *preempts* the Lua/UI task. At any instant, only one of them is executing — the ISR physically stops the Lua task, runs to completion, then returns control. The audio and Lua threads are **mutually exclusive in time**. They never overlap.

This has an important consequence for shared state: any data accessed by both the audio callback and the Lua thread is protected from true concurrent access by definition. Locking is still needed to prevent logical races (e.g. the ISR reading a partially-updated structure), but the window is narrow — measured in CPU instructions between hardware interrupt boundaries — and the lack of true concurrency means many such races are statistically unlikely to manifest.

**Multi-core Percussa:** Core 0 runs Lua/UI and core 1 runs the audio callback simultaneously. The audio thread does **not** preempt the Lua thread — they run in parallel, independently, at the same time. There is no mutual exclusion by construction. The audio callback is not interrupting anything on core 0; it simply runs continuously on its own core.

The practical consequence for correctness: **any shared state must be explicitly and correctly protected by locking on Percussa**, whereas on the single-core ER-301 incomplete or absent locking often goes undetected because the race window is too narrow to be hit in practice. Code that has survived for years on ER-301 hardware without incident can expose latent race conditions immediately on Percussa. This is not a flaw in the port — it is the port correctly revealing bugs that always existed in the shared code.

There is also a second dimension on multi-core that does not exist on single-core: **cache coherency**. On ARM multi-core, without explicit memory barriers (`dmb`/`dsb`), writes from one core are not guaranteed to be observed by another core in program order. A pair of flag writes on core 0 can appear to core 1 in a different order, or with stale cached values, even when no OS preemption occurs. POSIX mutexes include the required barriers as part of their specification, so correctly-locked code handles this automatically. Unlocked or weakly-locked shared state is vulnerable to both the timing race *and* the ordering hazard.

On Percussa, Linux provides preemptive scheduling. In principle this is less deterministic than a bare-metal ISR, but the hardware configuration closes most of that gap:

**Custom buildroot:** The Percussa Linux image is a minimal custom buildroot with only the software needed to run the application. This eliminates most background kernel work that would otherwise compete for CPU time.

**CPU core isolation:** Linux processes, kernel threads, and the UI all run on core 0. The RtAudio audio callback thread runs on core 1, which is isolated from the Linux scheduler so the OS does not migrate other work onto it.

**IRQ affinity pinning:** Hardware interrupts are pinned to specific cores so that no interrupt handler fires on the audio core unexpectedly:

```bash
echo 1 > /proc/irq/34/smp_affinity   # SD card (dw-mci)         → core 0
echo 2 > /proc/irq/35/smp_affinity   # Audio SDIO (dw-mci)      → core 1
echo 1 > /proc/irq/41/smp_affinity   # USB                      → core 0
echo 1 > /proc/irq/42/smp_affinity   # USB                      → core 0
echo 1 > /proc/irq/43/smp_affinity   # USB                      → core 0
echo 4 > /proc/irq/58/smp_affinity   # DMA controller (SDIO)    → core 2
echo 4 > /proc/irq/59/smp_affinity   # DMA controller (SDIO)    → core 2
```

The audio SDIO interrupt (IRQ 35) fires on core 1 alongside the audio callback — this is intentional, keeping audio data delivery and audio processing on the same core and avoiding cross-core cache invalidation on the audio path. SD card, USB, and unrelated DMA interrupts are all kept off the audio core. The SDIO DMA controller (IRQs 58–59) is pinned to core 2, further reducing interrupt load on both core 0 (UI) and core 1 (audio).

The combined effect — minimal OS image, core isolation, and IRQ pinning — gives latency characteristics close to a bare-metal ISR, substantially better than a general-purpose Linux RT configuration.

---

## 8. Display Rendering

The ER-301 defines two logical displays:
- Main display: 256×64 pixels, 4-bit grayscale
- Sub display: 128×64 pixels, 1-bit monochrome

The `od/` layer renders into these two framebuffers using `MainFrameBuffer` and `SubFrameBuffer`. The HAL display implementation then presents them on the physical screen.

**SSP:** The physical display is larger than the ER-301's logical displays, so `percussa/hal/display.cpp` scales both framebuffers up 3× before blitting to the SDL2 texture or framebuffer device. The 3× scaling is nearest-neighbour; pixel art from the original UI is preserved faithfully but appears larger.

**XMX:** The physical display matches the ER-301's native resolution; no scaling is applied.

---

## 9. Deviations from ER-301 Hardware and Their Effects

This section summarises every known deviation, who it affects (user / developer / both), and the practical consequence.

### 9.1 Missing Gate Inputs (User-facing)

**ER-301:** 4 dedicated gate inputs G1–G4, sampled at 96 kHz, 12-bit.
**SSP / XMX:** No gate inputs.

**Effect:** Any patch that uses G1–G4 will receive a constant zero signal on those inlets. The `InputTask` still creates the G1–G4 Outlets; they just never receive any data. Patches that use gates for triggering (e.g., sample playback, envelope triggers) will not respond. A CV input (A1–D3 on SSP, A1–D1 on XMX) can substitute for gates if the Lua patch is edited accordingly.

### 9.2 Reduced CV Inputs on XMX (User-facing)

**ER-301 / SSP:** 12 CV inputs A1–D3.
**XMX:** 4 CV inputs A1–D1 only.

**Effect:** Inlets mapped to A2–D3 receive zero. Patches designed for the ER-301 that use more than 4 CV channels will need remapping on XMX.

### 9.3 Reduced Audio Outputs on XMX (User-facing)

**ER-301 / SSP:** 4 audio outputs OUT1–4.
**XMX:** 2 audio outputs OUT1–2.

**Effect:** Signals routed to OUT3–OUT4 are computed by the DSP engine but discarded — there is no physical output. The headphone jack on XMX is a hardware mirror of OUT1–2 and cannot be remapped.

### 9.4 CV Voltage Range (User-facing)

**ER-301:** CV inputs accept ±10V.
**SSP / XMX:** Physical CV range is ±5V; the HAL scales this to ±10V before passing to `Pump_callback`.

**Effect:** Electrically, the port expects ±5V modulation signals at the hardware jacks. A 1V/oct keyboard CV will track correctly. A signal intended to drive the full ±10V range of the ER-301 will only use half its modulation depth unless the source is attenuated/scaled.

### 9.5 Sample Rate Fixed at 48 kHz (User-facing)

**ER-301:** Configurable 48 or 96 kHz via `firmware.cfg`.
**SSP / XMX:** Fixed at 48 kHz (RtAudio configuration; `audio_config_ssp.h`).

**Effect:** Patches that rely on 96 kHz operation for extended high-frequency response will run at 48 kHz. Maximum audio bandwidth is 24 kHz. CPU headroom is larger at 48 kHz than 96 kHz.

### 9.6 AC vs DC Coupling (User-facing)

**ER-301:** Audio inputs/outputs are AC coupled; CV/gate inputs are DC coupled.
**SSP / XMX:** All inputs and outputs are DC coupled.

**Effect:** DC-offset signals passing through audio outputs on SSP/XMX will reach the output jacks. This is generally not a problem in practice but may cause unexpected DC offset in downstream modules if the patch produces a biased signal.

### 9.7 Scheduling: OS vs Bare Metal (Developer / performance / correctness)

**ER-301:** Single core. Audio callback is a hardware timer ISR that preempts the Lua/UI task. Audio and Lua/UI are mutually exclusive in time — they never execute simultaneously.
**SSP / XMX:** Dual-core. Audio callback runs on core 1 continuously while Lua/UI runs on core 0. They execute truly in parallel.

**Effect on latency:** Worst-case audio callback jitter is higher on SSP/XMX than on bare metal. In practice, with the audio core isolated and IRQs pinned, jitter is imperceptible.

**Effect on correctness (more important):** Any shared state between the audio thread and the Lua/UI thread must be correctly protected by locking on Percussa. On the single-core ER-301, incomplete locking often goes unnoticed because the race window (between ISR preemption points) is too narrow to be hit in practice. The same code running on Percussa's truly-concurrent cores can expose those latent races immediately. Additionally, ARM multi-core requires explicit memory barriers for write ordering between cores; POSIX mutexes provide these automatically, but any unprotected shared state is vulnerable to both timing races and cache-coherency hazards. See §7 for the full discussion.

### 9.8 FFT Implementation (Developer / performance)

**ER-301:** Custom FFT tuned for ARM NEON on Cortex-A8.
**SSP / XMX:** FFTW3 (static link). On ARM hardware with NEON, FFTW uses NEON-optimised kernels; on macOS desktop it uses the host SIMD.

**Effect:** FFTW performance is generally good, but first-use planning overhead may cause a latency spike the first time a convolution unit is instantiated. This is a known FFTW characteristic; pre-planning at startup would mitigate it but is not currently done.

### 9.9 SIMD / NEON (Developer / performance)

**ER-301:** NEON intrinsics used throughout pump and DSP code, mandatory on Cortex-A8.
**SSP / XMX (cross-build):** Compiled with `-mcpu=cortex-a17 -mfpu=neon-vfpv4`; NEON is available and used.
**SSP / XMX (macOS host build):** SIMD falls back to scalar in `percussa/hal/simd.c`. Performance on the desktop build is lower, but this target is for development only.

### 9.10 USB Audio (User-facing)

**ER-301:** Not applicable (no USB audio in hardware).
**SSP / XMX:** Not yet implemented. USB audio would expose additional I/O channels, including those currently missing (gates, extra CV, extra outputs). This is a planned feature.

### 9.11 GPIO and Physical Controls (Developer)

**ER-301:** Physical LEDs, buttons, toggles driven via the GPIO HAL to real hardware registers.
**SSP / XMX:** GPIO state is software-emulated (`percussa/hal/gpio.c`). LEDs and toggles exist as in-memory state updated by the panel layer; the panel maps them back to the physical SSP/XMX display and indicator LEDs.

**Effect:** No functional difference for patch users. Developers adding new GPIO-dependent features must ensure the panel layer handles any new GPIO IDs.

---

## 10. Summary Table

| Deviation | User effect | Developer effect |
|-----------|------------|-----------------|
| No gate inputs | G1–G4 always zero; gate-driven patches non-functional | `InputTask` still emits G1–G4 Outlets (zero) |
| XMX: 4 CV only | A2–D3 always zero | Same as above |
| XMX: 2 outputs only | OUT3–OUT4 computed but not audible | OutputTask writes to hardware buffer; channels 3–4 discarded |
| CV ±5V scaled ×2 | Correct V/oct; half modulation depth for ±10V sources | Scaling in HAL; transparent to `od/` |
| 48 kHz fixed | No 96 kHz mode | `firmware.cfg` sample rate setting ignored |
| DC coupled I/O | Possible DC offset at outputs | None |
| Multi-core parallelism | Near-ISR jitter via core isolation + IRQ pinning | Audio and Lua/UI run simultaneously — all shared state needs correct locking. Single-core ER-301 got away with weak locking; Percussa exposes those latent races |
| FFTW instead of custom FFT | Possible first-use spike for convolution | Pre-planning at startup would help |
| SIMD (desktop build) | N/A (development target) | Scalar fallback; profiling not representative |
| No USB audio | Missing gates/CV/outputs cannot be recovered via USB yet | Planned feature |
| GPIO emulated | None | New GPIO IDs need panel layer support |
