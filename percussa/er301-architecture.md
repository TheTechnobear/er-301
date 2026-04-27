# ER-301 Architecture — HAL, OD, and DSP Deep Dive

This document focuses on how the ER-301's audio processing engine works, how it is structured in `hal/` and `od/`, and how the Percussa port adapts it. It is intended as a reference for working on the Percussa SSP/XMX target.

---

## 0. The DSP Model — User View and Code Reality

This section covers how the ER-301's DSP is organised from the user's perspective, then maps that directly onto the code structures and execution model.

---

### 0.1 The User-Facing DSP Model

The ER-301 presents its DSP as a set of **chains**. Understanding chains and how they nest and connect is the key to understanding everything else.

#### Chains

A **chain** is an ordered sequence of units. Signal flows left-to-right through the units; each unit's audio output feeds the next unit's audio input. The chain as a whole has one audio input and one audio output (per channel).

Chains are either **mono** (one channel) or **stereo** (two channels, left and right processed together). A stereo chain processes both channels through every unit in lockstep.

#### Output chains and linking

There are four top-level **output chains**, one per hardware output: OUT1, OUT2, OUT3, OUT4. Each is mono by default. Linking OUT1 and OUT2 together makes them a stereo pair: they share a single stereo chain, and the ER-301 hardware receives left on OUT1 and right on OUT2.

#### Chain inputs

A chain's input is a signal source selected by the user. It can be:
- A **hardware input** — one of the audio inputs (IN1–IN4) or CV inputs (A1–D3) from the `InputTask` outlets
- The **output of another chain** — any top-level chain's output can feed into this chain's input, creating a signal routing dependency
- **Nothing** (silent) — the chain starts from an implicit zero signal

#### Global chains

In addition to the four output chains, the user can create **global chains** (also called "monitor" or "auxiliary" chains). These are independent chains that run alongside the output chains, not necessarily connected to a hardware output. Their output can be used as a signal source for other chains' inputs or for parameter modulation.

#### Units

A **unit** is an individual DSP processor within a chain: oscillator, filter, envelope follower, sample player, reverb, etc. Each unit exposes:
- An **audio input** (connected to the previous unit's output, or the chain input)
- An **audio output** (connected to the next unit, or the chain output)
- A set of **parameters** (gain, frequency, attack time, …)

Most parameters can be **modulated** by an audio-rate signal. This modulation source is itself a small chain attached to the parameter — a signal path that reads from any available outlet and scales/shapes it before applying it to the parameter value each frame.

#### Container units (Custom Units)

A **custom unit** is a unit that contains its own sub-chain. Internally it works identically to a top-level chain, but it appears as a single unit to the containing chain. This allows arbitrary nesting: a chain can contain a custom unit that itself contains sub-units:

```
OUT1 chain:
  → InputGain unit
  → CustomUnit "my reverb"
      → PreDelay unit
      → Diffuser unit
      → Tail unit
  → OutputLimiter unit
  → OUT1 hardware
```

#### Example: Simple mono output

```
IN1 (hardware)
  │
  ▼
OUT1 chain ──────────────────────────────────────► OUT1 hardware
  │
  ├─[1] VCA unit ◄── Gain param ◄── A1 (CV, modulation chain)
  │
  └─[2] Delay unit ◄── Time param ◄── B1 (CV, modulation chain)
```

#### Example: Stereo output with global chain send

```
IN1/IN2 (stereo hardware input)
  │
  ▼
GlobalChain "dry" (stereo)
  └─[1] VCA

GlobalChain "reverb" (stereo) ◄── input from "dry" chain output
  └─[1] Convolution reverb

OUT1+OUT2 chain (stereo, linked) ◄── input from "dry" chain output
  └─[1] Mixer ◄── also receives "reverb" chain output as side input

OUT3 ◄── input from "reverb" chain output (wet only monitor)
```

#### Example: Parameter modulation chain

```
OUT1 chain:
  └─[1] SineOscillator
           │
           ├─ Frequency parameter
           │     └── modulation chain:
           │           └─[1] Constant (base pitch)
           │           └─[2] Attenuator ◄── CV input A1
           │
           └─ Amplitude parameter
                 └── modulation chain:
                       └─[1] ADSR ◄── gate input G1  (ER-301 only; not on SSP/XMX)
```

---

### 0.2 Code Implementation

The user-facing model maps onto three nested layers of C++ objects:

```
TaskScheduler          ← one per AudioThread; holds all active Tasks
  └── UnitChain        ← one per top-level chain (output chains + global chains)
        └── Unit[]     ← ordered array of units in the chain
              └── Object[]  ← topologically sorted DSP graph inside the unit
                    ├── Inlet  ← audio input port (points to upstream Outlet buffer)
                    ├── Outlet ← audio output port (owns float[] sample buffer)
                    └── Parameter ← scalar control input with optional modulation Outlet
```

#### Key classes

| Class | File | Role |
|-------|------|------|
| `TaskScheduler` | `od/tasks/TaskScheduler.h/cpp` | Maintains priority-sorted `Task*` list; calls `task->process()` each frame |
| `Task` | `od/tasks/Task.h/cpp` | Base class; has `mPriority`, `mActive`, `process(float*, float*)` |
| `UnitChain` | `od/tasks/UnitChain.h/cpp` | Concrete `Task`; holds ordered `Unit*` vector; manages chain I/O via `Repeater` |
| `Unit` | `od/units/Unit.h/cpp` | Holds `Object*` vector + compiled `mProcessingOrder`; calls `object->process()` |
| `CustomUnit` | `od/units/CustomUnit.h` | A `Unit` that also holds a sub-`Unit*` vector — implements the container concept |
| `GraphCompiler` | `od/units/GraphCompiler.h/cpp` | Topological sort of `Object*` DAG; handles feedback (see §0.4) |
| `Object` | `od/objects/Object.h/cpp` | Base class for all DSP algorithms; has `Inlet[]`, `Outlet[]`, `Parameter[]` |
| `Inlet` | `od/objects/Inlet.h/cpp` | Audio input port; holds a pointer to an upstream `Outlet` |
| `Outlet` | `od/objects/Outlet.h/cpp` | Audio output port; owns a `float[]` buffer of `frameLength` samples |
| `Repeater` | `od/objects/Repeater.h/cpp` | Thin `{Inlet, Outlet}` pair; used as the chain boundary — copies input buffer to output |
| `InputTask` | `od/tasks/InputTask.h/cpp` | Highest-priority `Task`; writes hardware `inputs[]` into named Outlets (IN1–4, A1–D3, G1–G4) |
| `OutputTask` | `od/tasks/OutputTask.h/cpp` | Lowest-priority `Task`; copies Inlet buffers to hardware `outputs[]` |
| `ConnectionQueue` | `od/tasks/ConnectionQueue.h/cpp` | After-frame queue for atomic graph edits from the Lua thread |

#### How a chain's I/O is wired

`UnitChain` holds two `Repeater` objects (`mLeftOutput`, `mRightOutput`) as its audio outputs. `connectInternals()` wires:

```
mpLeftSource (Outlet*)                 mpRightSource (Outlet*)
     │                                       │
     ▼                                       ▼
Unit[0] ─── Unit[1] ─── ... ─── Unit[n]
                                    │
                          mLeftOutput.mInlet ─► mLeftOutput.mOutlet ──► downstream Inlets
                          mRightOutput.mInlet ─► mRightOutput.mOutlet ──► downstream Inlets
```

The chain's `setInput(i, Outlet*)` stores the source outlet in `mpLeftSource`/`mpRightSource`. This outlet can come from `InputTask` (hardware), from another `UnitChain::getOutput(i)`, or from any Object's outlet anywhere in the audio graph.

Units within a chain are connected sequentially by `connectInternals()`:
- The chain source feeds the first unit's input
- Each unit's output feeds the next unit's input via `Unit::connect(prev, next)`
- The last unit's output feeds the chain's `Repeater` input

#### Frame execution call chain

Each audio frame, the call chain is:

```
Pump_callback()
  └── AudioThread::process(inputs, outputs)
        └── TaskScheduler::process(inputs, outputs)
              ├── InputTask::process()           [priority INT_MAX-1]
              │     for each channel:
              │       copy inputs[ch][0..N] → Outlet buffer
              │
              ├── UnitChain::process()           [priority: user-defined order]
              │     for each Unit in mUnits:
              │       if unit->mEnabled:
              │         unit->process()
              │           for each Object in mProcessingOrder:
              │             object->updateParameters()
              │             object->process()
              │     mLeftOutput.copyInputToOutput()    ← copy last unit's output
              │     mRightOutput.copyInputToOutput()   ← to Repeater outlet buffers
              │
              ├── OutputTask::process()          [priority INT_MIN+2]
              │     for each output channel:
              │       copy Inlet buffer → outputs[ch][0..N]
              │
              └── ConnectionQueue::process()     [priority INT_MIN]
                    apply pending connect/disconnect edits atomically
```

#### ASCII DSP graph — simple example

Single mono output chain: IN1 → VCA → Delay → OUT1, with CV on VCA gain.

```
Frame N execution order (left = processed first):

[InputTask]         [UnitChain: OUT1]                        [OutputTask]
                    ┌─────────────────────────────────────┐
 IN1.Outlet ───────►│ VCA Unit                            │
                    │  ┌──────────┐                       │
 A1.Outlet ────────►│  │ Gain Obj │                       │
  (modulation)      │  │  Inlet◄──┼── A1.Outlet           │
                    │  │  Outlet──┼──►                    │
                    │  └──────────┘                       │
                    │  ┌──────────┐                       │
                    │  │ Mult Obj │ (audio × gain)        │
                    │  │  Inlet0◄─┼── IN1.Outlet          │
                    │  │  Inlet1◄─┼── Gain.Outlet         │
                    │  │  Outlet──┼──►                    │
                    │  └──────────┘                       │
                    │                                     │
                    │ Delay Unit                          │
                    │  ┌──────────┐                       │
                    │  │ Delay Obj│                       │
                    │  │  Inlet◄──┼── VCA.Outlet          │
                    │  │  Outlet──┼──►                    │
                    │  └──────────┘                       │
                    │                                     │
                    │ Repeater                            │
                    │  mInlet◄── Delay.Outlet             │
                    │  mOutlet──────────────────────────► │ OUT1.Inlet ──► outputs[0]
                    └─────────────────────────────────────┘
```

#### ASCII DSP graph — complex example

Two stereo global chains with a mixing output and parameter modulation:

```
TaskScheduler execution order (decreasing priority):

[InputTask] → [GlobalChain "synth" p=100] → [GlobalChain "fx" p=90] → [OUT1+2 chain p=80] → [OutputTask]

GlobalChain "synth" (stereo, p=100):
  input: none (source unit)
  ┌──────────────────────────────────────────────────┐
  │ OscUnit                                          │
  │   VCO_L.Outlet ──► left channel                 │
  │   VCO_R.Outlet ──► right channel                │
  │   Freq param ◄── modulation:                    │
  │     [Constant(440Hz)] ──► [Attenuverter] ◄── A1 │
  └──────────────────────────────────────────────────┘
  mLeftOutput.mOutlet, mRightOutput.mOutlet

GlobalChain "fx" (stereo, p=90):
  input: "synth" chain L+R outputs
  ┌────────────────────────────────────────────────────────┐
  │ CustomUnit "reverb"                                    │
  │   sub-chain:                                           │
  │     [PreDelay] ──► [Diffuser] ──► [Tail]              │
  └────────────────────────────────────────────────────────┘
  mLeftOutput.mOutlet, mRightOutput.mOutlet

OUT1+OUT2 chain (stereo linked, p=80):
  input: "synth" chain L+R outputs  (dry path)
  ┌──────────────────────────────────────────────┐
  │ MixerUnit                                    │
  │   ch0: ◄── "synth".mLeftOutput (dry)        │
  │   ch1: ◄── "fx".mLeftOutput    (wet)        │
  │   Mix param ◄── modulation: [Constant(0.3)] │
  └──────────────────────────────────────────────┘
  mLeftOutput ──► OutputTask.OUT1.Inlet
  mRightOutput ──► OutputTask.OUT2.Inlet
```

Key point from this example: `"fx"` chain uses `"synth"` as its input and has lower priority (p=90 < p=100). This means `"synth"` runs first and its Outlet buffers contain the current frame's data when `"fx"` processes. If the priorities were reversed, `"fx"` would read stale data from the previous frame — a **one-frame latency** artifact. The user/Lua layer is responsible for assigning correct priorities; the scheduler does not infer them from the signal graph.

---

### 0.3 Is a Global Graph Built? Is Processing Single-Threaded?

**There is no single global graph.** Optimisation is hierarchical:

| Level | Structure | How order is determined |
|-------|-----------|------------------------|
| `TaskScheduler` | Flat priority-sorted list | `mPriority` integer — set by Lua, not graph-derived |
| `UnitChain` | Ordered array of `Unit*` | User's chain order — strictly sequential |
| `Unit` | Topologically sorted `Object*` | `GraphCompiler` — derived from Outlet→Inlet connections |

`GraphCompiler` produces a proper topological order, but only within a single Unit's internal Object graph. Cross-unit and cross-chain ordering is purely priority/position based. If chain A feeds chain B but both have the same priority, or if priorities are assigned incorrectly, chain B sees chain A's output from the previous frame.

**All DSP runs on a single thread.** `TaskScheduler::process()` is called on the audio callback thread (core 1 on Percussa hardware). It iterates tasks sequentially, and within each UnitChain the units are also iterated sequentially. No parallelism exists within the audio frame.

**Could it be parallelised?** Theoretically yes, at the `UnitChain` level: independent chains with no cross-connections could run on separate cores simultaneously, merging their results before `OutputTask` runs. In practice this would require:
- A dependency graph across chains (to know which are truly independent)
- Lock-free buffer handoff between worker threads and the main audio callback
- A barrier synchronisation before `OutputTask` runs

This is not implemented. Given that the Percussa hardware already isolates core 1 for the audio callback and cores 0/2 serve OS and DMA, there is no spare isolated core to run a second audio worker without reintroducing scheduling jitter.

---

### 0.4 Feedback Loops

**Feedback is not rejected.** `GraphCompiler` handles it automatically within a Unit by breaking the cycle at the point that introduces the least disruption to signal flow.

The algorithm (`GraphCompiler::topographicalSort`):

1. Standard Kahn-style topological sort: iteratively place any Object whose upstream dependencies are all already placed.
2. If a full pass places nothing (**cycle detected**), call `orientGraph()` — assign each Object a `mMinDistance` from the graph's input objects via a forward BFS walk.
3. Call `breakCycle()` — find the Object that is furthest from inputs but whose output feeds an Object that is *close* to inputs. This is the "back edge" with the highest feedback score.
4. That Object's outputs are added to `processed` (treated as if already computed), and it is placed in the `delayed` list.
5. Resume the main sort; the cycle is now unblocked.
6. After all remaining Objects are sorted, recursively sort the `delayed` list and append it to the end.

The result: the cycle-breaking Object is scheduled **last in the frame**. Its Outlet buffer, when read by earlier Objects in the same frame, contains the output from the **previous frame** — introducing exactly one frame of latency in the feedback path. This is the theoretical minimum latency for any discrete-time feedback system.

```
Example: feedback from Obj C back to Obj A

Without feedback:   A ──► B ──► C        (normal DAG, processed A→B→C)

With feedback:      A ──► B ──► C
                    ▲           │
                    └───────────┘  (C feeds back to A)

GraphCompiler resolution:
  orientation: A(dist=0), B(dist=1), C(dist=2)
  feedbackScore for C→A edge: C.mMinDistance(2) - A.mMinDistance(0) = 2  (highest)
  C is chosen as the break point → delayed

Frame execution order:  A → B → [C reads A's CURRENT output]
                        then C → [A reads C's PREVIOUS frame output next frame]
```

At the cross-chain level (between `UnitChain` tasks), feedback is also possible if chain A depends on chain B and chain B also depends on chain A. The `TaskScheduler` does not detect this; it simply executes in priority order. Whichever chain runs second will read the other chain's previous-frame buffer — again one frame of latency. This is the expected behaviour for cross-chain feedback and must be managed by the patch author through careful priority assignment.

---

## 1. The HAL Contract

`hal/` contains only headers — no platform code. Every platform (firmware, emu, percussa) provides its own `.c/.cpp` implementations of these contracts.

### Core headers

| Header | Key symbols | Notes |
|--------|------------|-------|
| `audio.h` | `Audio_callback(int *samples)` | Raw I/O callback from hardware ADC/DAC |
| `pump.h` | `Pump_callback(float *inputs, float *outputs)` | **The real-time DSP entry point** |
| `events.h` | `Events_push/pop`, event type constants | 32-bit encoded: 16-bit type + 16-bit ID |
| `display.h` | `Display_*` functions | Dual framebuffer: main 256×64 4-bit, sub 128×64 1-bit |
| `channels.h` | `IN_CHANNEL_*`, `OUT_CHANNEL_*` macros | 20 inputs (4 audio, 12 CV, 4 gates), 4 outputs |
| `constants.h` | `MAX_AUDIO_FRAME_LENGTH=128`, `NUM_INPUT_CHANNELS=20` | Frame/channel sizing constants |
| `modulation.h` | ADC configuration constants | CV inputs at 60 kHz, ±10V |
| `gpio.h` | 47 GPIO IDs for LEDs, buttons, toggles | |
| `concurrency/` | `Thread.h`, `Mutex.h`, `EventFlags.h` | Platform-neutral threading primitives |

### The pump

`hal/pump/` contains the shared pump logic reused across all platforms. It owns the sample-rate conversion between the hardware ADC clock (60 kHz for CV) and the audio processing rate (48/96 kHz):

- `Resample4` — polyphase resampler for CV inputs
- SIMD gain application (ARM NEON on hardware, scalar fallback elsewhere)
- PID-based rate control to track ADC/DAC clock drift
- Mutex-protected FIFOs between the ADC ISR and `Pump_callback()`

`Pump_callback(float *inputs, float *outputs)` is called once per audio frame by the audio driver. On Percussa this is the RtAudio callback thread. `inputs` is a contiguous array of `NUM_INPUT_CHANNELS × frameLength` floats; `outputs` is `NUM_OUTPUT_CHANNELS × frameLength` floats.

---

## 2. The OD Layer — AudioThread and UIThread

The OD layer has two central singletons that own the entire runtime.

### AudioThread (`od/AudioThread.h/cpp`)

Owns everything that runs in the audio callback context.

Key members:
- `TaskScheduler mTasks` — the ordered list of Tasks executed each frame
- `InputTask` — first task run each frame; reads `inputs` array into Outlet buffers
- `OutputTask` — last task run; copies inlet buffers into the `outputs` array
- `BufferPool` — pre-allocated pool of 1024 float frames; allocation is constant-time and lock-free
- `ConnectionQueue` — batched graph modifications applied atomically between frames

Entry point: `Pump_callback()` calls `AudioThread::process(inputs, outputs)` which calls `mTasks.process()`.

### UIThread (`od/UIThread.h/cpp`)

Runs at 55 Hz (not real-time). Owns:
- `MainFrameBuffer` (256×64, 4-bit) and `SubFrameBuffer` (128×64, 1-bit)
- `GraphicContext` — scene graph of `Graphic` objects
- `JobQueue` — thread-safe queue for work dispatched from the audio thread
- PWM output updates (reads `InputTask::mLastInput` to drive front-panel LED meters)

---

## 3. The Task System (`od/tasks/`)

A **Task** is the unit of real-time audio work scheduled by the `AudioThread`. All DSP is expressed as a graph of Tasks.

### Task (`Task.h`)

```cpp
class Task : public ReferenceCounted {
public:
    virtual void process(float *inputs, float *outputs) = 0;
    int mPriority;      // higher value = earlier execution
    bool mActive;
    ExecutionTimer mExecutionTimer;
};
```

Tasks are not directly wired to each other via the `inputs/outputs` pointers — those are the raw hardware I/O arrays passed through from `Pump_callback`. Inter-task signal flow happens via **Outlets and Inlets** (see §4).

### TaskScheduler (`TaskScheduler.h/cpp`)

Maintains a sorted vector of `Task*` ordered by decreasing `mPriority`. On each call to `process()` it iterates the vector and calls `task->process(inputs, outputs)` on each active task.

Fixed priority anchors:
- `InputTask` runs at `INT_MAX - 1` (always first)
- `OutputTask` runs at `INT_MIN + 2` (always last)
- `ConnectionQueue` runs at `INT_MIN` (after outputs, applies graph edits)

User `UnitChain` tasks occupy the middle range and are sorted by the user's patch routing.

### InputTask (`InputTask.h/cpp`)

Reads the raw `inputs` array and writes into a set of named **Outlets**:

- `A1–A3, B1–B3, C1–C3, D1–D3` — 12 CV Outlets (from modulation inputs)
- `G1–G4` — 4 gate Outlets
- `IN1–IN4` — 4 audio Outlets

Also stores `float mLastInput[NUM_INPUT_CHANNELS]` for UI metering and PWM.

### OutputTask (`OutputTask.h/cpp`)

Has 4 named **Inlets** (`OUT1–OUT4`). On `process()` it copies each inlet's connected buffer into the corresponding slice of the `outputs` array.

Also has 4 monitor Outlets (`Monitor1–Monitor4`) that echo the output values for measurement objects.

### UnitChain (`UnitChain.h/cpp`)

The primary container for user DSP. Derives from `Task` and is added to the `TaskScheduler`.

A `UnitChain` holds an ordered list of `Unit` objects. On each frame it calls each Unit's internal object graph in compiled execution order. Key features:

- **Lock mechanism** — thread-safe insertion/removal of Units
- **LinearRamp fade** — smooth mute/unmute to avoid clicks
- **Repeater Outlets** — expose the chain's audio inputs as connectable sources within the chain

---

## 4. The Signal Graph — Objects, Inlets, Outlets

Within a `Unit` (and by extension a `UnitChain`), signal flow is expressed as a directed acyclic graph of **Objects** connected via **Inlets** and **Outlets**.

### Port / Inlet / Outlet (`od/objects/`)

```
Port (name, index, ReferenceCounted)
├── Inlet   — single inward connection to an upstream Outlet
└── Outlet  — vector of outward connections to downstream Inlets
```

An **Outlet** owns a `float*` buffer of `frameLength` samples. An **Inlet** holds a pointer to its connected Outlet's buffer; if unconnected it falls back to `ZeroOutput` or `OneOutput` (global constant buffers).

Connection and disconnection go through `ConnectionQueue` so they are applied atomically between frames, never mid-frame.

### Object (`od/objects/Object.h/cpp`)

```cpp
class Object : public ReferenceCounted {
    vector<Inlet*>        mInputs;
    vector<Outlet*>       mOutputs;
    vector<Parameter*>    mParameters;
    vector<Option*>       mOptions;
    vector<StateMachine*> mStateMachines;

    virtual void process() = 0;   // called once per frame in compiled order
    virtual void compile();       // dependency analysis
    ExecutionTimer mExecutionTimer;
};
```

Every concrete DSP algorithm (oscillator, filter, envelope, delay, …) derives from `Object` and implements `process()`. The `Inlet` buffers are read-only inputs; `Outlet` buffers are written outputs.

### Parameter (`od/objects/Parameter.h`)

Parameters are the automation/modulation inputs (not audio-rate signal inlets). They support:
- Hard set (immediate) vs. soft set (ramped over a frame)
- Hold during UI interaction to prevent parameter jumps
- Leader/follower tying via `Followable` base class
- Lua-readable current value

### Unit and GraphCompiler (`od/units/Unit.h`, `GraphCompiler.h`)

A **Unit** is a compiled execution schedule for a set of Objects.

`Unit::compile()` calls `GraphCompiler::compile()` which:
1. Traverses the DAG from output Inlets back to source Outlets
2. Detects feedback loops (invalid — would require a delay)
3. Assigns a topological execution order (longest-path analysis)
4. Stores the sorted `Object*` list

On each frame, `UnitChain` calls each Unit, which iterates its compiled object list and calls `object->process()` in order.

Object subtypes in `od/objects/`:

| Subdirectory | Examples |
|---|---|
| `control/` | Constant, Ramp, ADSR envelopes |
| `math/` | Mult, Add, Abs, Clip, scale operations |
| `mixing/` | Mixer, Crossfade, Panner, StereoToMono |
| `measurement/` | Probe, FifoProbe, LoudnessProbe, MinMax |
| `timing/` | Clock, Tapper, delay primitives |
| `heads/` | VCPlayback, GrainStretch — sample playback |
| `file/` | SampleRecorder, FilePlayer |
| `adapters/` | Format conversion utilities |

---

## 5. DSP Primitives (`od/audio/`)

Low-level DSP building blocks, used by Objects:

| File | Purpose |
|------|---------|
| `Sample.h/cpp` | In-memory audio sample buffer |
| `SampleFifo.h/cpp` | Lock-free ring buffer for audio streaming |
| `MonoResampler`, `StereoResampler` | Polyphase resamplers for pitch/rate shifting |
| `Resampler.h/cpp` | Base resampler |
| `OnsetDetector.h/cpp` | Transient / beat detection |
| `UniformlyPartitionedConvolution` | FFT-based convolution (reverb, IR) |
| `WavFileReader/Writer` | PCM WAV I/O |
| `SoundFileReader/Writer` | Generic sound file I/O |
| `Slice.h/cpp`, `Slices.h/cpp` | Segment markers within a Sample |
| `SampleLoader/Saver` | Async sample loading from card |

---

## 6. Full Audio Frame Path

```
RtAudio callback (Percussa audio thread)
  │
  └─► Pump_callback(float *inputs, float *outputs)
           │
           ├─► InputTask::process()
           │     Reads inputs[] → writes 20 Outlet buffers
           │     Stores mLastInput[] for UI/PWM
           │
           ├─► [UnitChain tasks, priority-sorted]
           │     For each UnitChain:
           │       For each Unit (in patch order):
           │         For each Object (in compiled DAG order):
           │           Object::process()
           │             Reads connected Inlet buffers
           │             Writes own Outlet buffers
           │
           ├─► [Measurement tasks: FifoProbe, LoudnessProbe, …]
           │
           ├─► OutputTask::process()
           │     Reads 4 Inlet buffers → writes outputs[]
           │
           └─► ConnectionQueue::process()
                 Applies pending connect/disconnect ops atomically
```

Frame parameters (configured in firmware/bootstrap):
- Frame length: 32, 64, or 128 samples
- Sample rate: 48 000 or 96 000 Hz
- Input channels: 20 (float, interleaved by channel then sample)
- Output channels: 4

---

## 7. Percussa HAL Implementation (`percussa/hal/`)

The Percussa port provides concrete implementations of all `hal/` contracts.

| File | Implements | Notes |
|------|-----------|-------|
| `audio.c` | `Audio_callback` | Delegates to RtAudio for cross-platform audio |
| `audio_config_ssp.h` | SSP channel routing + gain | 8 in / 8 out on SSP hardware |
| `audio_config_xmx.h` | XMX channel routing + gain | Expanded I/O on XMX |
| `audio_config_macos.h` | macOS routing (dev/testing) | |
| `display.cpp` | `Display_*` | Writes to Linux `/dev/fb0` or SDL2 texture |
| `events.cpp` | `Events_push/pop` | From keyboard, mouse, touchscreen |
| `gpio.c` | GPIO read/write | Software-emulated state |
| `encoder.cpp` | Knob rotation | Mouse wheel or physical encoders |
| `concurrency/` | Thread, Mutex, EventFlags | POSIX pthreads |
| `card.cpp` | SD card abstraction | Maps to filesystem paths (`~/.od/`) |
| `fft.c` | FFT | Delegates to FFTW (vs ARM NEON on hardware) |
| `simd.c` | SIMD helpers | Scalar fallback without ARM NEON |
| `timing.c` | Wallclock | `clock_gettime` |

### Platform abstraction (`percussa/platform/`)

```cpp
class Platform {
    virtual int run(runtime::Runtime &runtime) const = 0;
};
```

Implementations:
- `HostSdlPlatform` — SDL2 window, mouse/keyboard I/O (macOS/Linux desktop)
- `FbdevPlatform` — Linux framebuffer `/dev/fb0` + touchscreen (SSP/XMX hardware)
- `PluginPlatform` — VCV Rack plugin wrapper

### Runtime and Bootstrap (`percussa/runtime/`, `percussa/app/`)

`Bootstrap` runs once at startup:
1. Parses command-line arguments
2. Creates/loads `~/.od/` config files
3. Sets `XROOT` (Lua script path) and card root paths
4. Launches the Lua interpreter thread
5. Restores the last session (patch + UI state)

`Runtime` is the main loop coordinator, owns `Platform` and `Panel`, integrates audio + UI rendering.

---

## 8. Threading Model (Percussa)

| Thread | Owner | Runs | Communicates via |
|--------|-------|------|----------------|
| Audio callback | RtAudio | `Pump_callback()` each frame | Outlet/Inlet buffers (in-frame); `LockFreeQueue` / `ConnectionQueue` (cross-frame) |
| UI / main | `Platform::run()` | Event loop + display 55 Hz | `JobQueue` (audio→UI), `EventFlags` |
| Lua interpreter | `Bootstrap` | Patch logic, parameter changes | `Mutex`-protected queues into audio/UI |

The audio thread never blocks. Graph modifications (connect/disconnect) are queued via `ConnectionQueue` and applied atomically at the end of each frame.

---

## 9. Graphics System (`od/graphics/`)

Not directly related to DSP but relevant for UI work.

- `Graphic` — scene graph node with tween animation and attachment reference counting
- `GraphicContext` — owns the scene graph, manages draw order and overlays
- `FrameBuffer` — pixel buffer with primitives (line, rect, circle, text, waveform)
- `MainFrameBuffer` (256×64, 4-bit grayscale), `SubFrameBuffer` (128×64, 1-bit)
- Widget subdirectories: `charts/`, `controls/`, `meters/`, `lists/`, `text/`, `wayfinding/`, `sampling/`, `recording/`, `screensavers/`, etc.

---

## 10. Lua Integration (`od/glue/`)

The user-facing patch system runs in Lua. The `od/glue/` SWIG bindings expose:
- `Unit` creation and parameter access
- `Inlet`/`Outlet` connection
- `Sample` loading and slice management
- UI object construction

`AppInterpreter` wraps the Lua VM; `ExpressionInterpreter` handles math expressions in patch configs.

The Lua script tree lives in `xroot/` (pointed to by the `XROOT` environment variable). This is where unit definitions, UI screens, and system menus are defined.
