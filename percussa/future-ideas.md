# Future Ideas

Speculative and exploratory notes on potential improvements to the ER-301/Percussa architecture.

---

## Multi-Core DSP

### Question

> The current ER-301 hardware has a single core, so it uses preemptive multitasking with one audio thread. Is there any evidence that the original author placed in hooks for multiple DSP cores? Are there any natural points where multiple cores could be added — e.g. running certain chains (global? different outputs?) on different cores?

### Analysis

#### No intentional multi-core hooks

The author did not leave any explicit multi-core hooks. The DSP path is strictly sequential by design.

The execution path from top to bottom:

```
Pump_callback()
  └── TaskScheduler::process()        ← mutex held for entire loop
        └── for each Task in order:
              Task::process()
                └── UnitChain → Unit::process()
                      └── for each Object in mProcessingOrder:
                            Object::process()
```

Everything is a flat sequential loop. No worker threads, no thread affinity, no `#ifdef` for SMP, no core pinning anywhere in the DSP path.

#### Natural parallelism boundaries that exist

Even without hooks, the architecture has several natural split points:

**Coarse-grained (lowest risk, highest value):**

- **Independent UnitChains** — each chain is already a self-contained task with its own mutex. Multiple chains with no data dependency between them could run on separate cores with minimal coupling. This is the most practical target. It matches how patches are typically structured (global chain + per-output chains).
- **Per-output-pair processing** — if output channels don't share state, they could be split across cores.

**Medium-grained:**

- **TaskScheduler priority list** — tasks are already sorted by priority. A work-stealing thread pool could respect that order while parallelizing independent tasks.

**Fine-grained (most invasive):**

- **Within a Unit's Object DAG** — `GraphCompiler` does a full topological sort and tracks dependency order (`od/units/GraphCompiler.cpp`). That structure *could* support parallel execution of objects at the same DAG depth (objects with no mutual dependency). However, the sorted order is flattened into a plain `std::vector<Object*>` and all dependency metadata is discarded after compilation. Exploiting this would require `GraphCompiler` to emit level-sets rather than a flat order.

#### Key structural limitations

- `TaskScheduler::process()` (`od/tasks/TaskScheduler.cpp`) holds the mutex for the **entire execution loop**, not just add/remove. Any parallelization at the chain/task level requires this mutex to be refactored.
- `mProcessingOrder` in `Unit` is a flat vector with no depth/level information retained. Parallelizing within a unit requires extending or rerunning `GraphCompiler` to emit dependency level-sets.
- All lock-free queues (`LockFreeQueue`, `ProducerConsumerQueue`) are **single-producer / single-consumer**. N-core fan-out would require replacement or augmentation.

#### Recommended approach for Percussa

The lowest-risk, highest-reward approach is **one core per independent UnitChain**, with a barrier after all chains complete before `OutputTask` runs:

1. Replace the sequential task loop in `TaskScheduler::process()` with a thread-pool dispatch.
2. Identify which tasks have no inter-task data dependencies (inputs/outputs don't overlap) and dispatch those concurrently.
3. Join all worker threads before `OutputTask` collects results.

This requires no changes to the DSP graph internals (`Unit`, `Object`, `GraphCompiler`) and no changes to individual units or objects. The parallelism is entirely at the chain boundary.

---

## New UI / Reimplementation

### Question

> The Percussa port emulates the ER-301 hardware UI — rendering its display buffer and simulating GPIO read/writes — meaning the UI is fixed to follow the original hardware. What would be required to create a completely new UI? Is enough exposed in the firmware layer? Could compatibility be maintained?
>
> An alternative (and possibly preferable) approach would be to copy the codebase and strip out everything not required, reusing only the DSP graphs, chains, etc., and implementing a fully new UI. This would also simplify the codebase considerably. Since the firmware is no longer in active development, divergence is not a concern.

### Analysis

#### The seam is real and reasonably clean

The DSP engine and the UI are coupled at **one well-defined interface**: the `Parameter` / `Inlet` / `Outlet` objects on `Object` subclasses. The UI reads and writes parameters via method calls (`Parameter::value()`, `Parameter::hardSet()`, `Parameter::softSet()`, `Parameter::tie()`). DSP objects have no reverse dependencies on the UI — they do not call UI callbacks, write to display buffers, or know about widgets. The seam is almost entirely one-directional.

Two minor exceptions:

- `od/objects/file/FileSource.cpp` uses `UIThread::getRealtimeJobQueue()` for async sample loading. This is a generic background work queue, not a UI concern — it could be renamed and decoupled trivially.
- `od/objects/file/FileSinkThread.h` has a `friend RecordingGraphic` declaration for a UI widget to monitor recording state. This is an optional read-only monitoring hook.

Neither constitutes real coupling to the UI rendering stack.

#### What to keep (DSP engine — ~20,000 lines of C++)

```
od/objects/       — Object, Inlet, Outlet, Parameter, Option; all math/mixing/timing/measurement objects
od/units/         — Unit, CustomUnit, EffectUnit, GraphCompiler
od/tasks/         — UnitChain, TaskScheduler, InputTask, OutputTask, ObjectCache, ConnectionQueue
od/audio/         — Sample, SampleLoader, WavFileReader, resampling, convolution
od/AudioThread.h  — real-time audio loop driver
od/extras/        — ReferenceCounted, Lockable, LockFreeQueue, Random, etc.
od/ui/JobQueue.h  — generic background I/O queue (keep, rename if desired)
hal/              — audio.h, pump.h, channels.h, constants.h (platform contracts)
```

#### What to remove (UI stack — ~57,000 lines of C++ + Lua)

```
od/graphics/      — ALL: FrameBuffer, GraphicContext, Graphic, Drawing, and all 60+ widget subclasses
                    (19,279 lines — entirely ER-301 hardware UI, 256×64 + 128×64 displays, encoder widgets, etc.)
od/ui/            — ChannelLEDs, DialMap, EncoderHysteresis, Busy (ER-301 hardware-specific)
od/UIThread.cpp   — display update loop, event polling, screensaver, LED control
xroot/            — ALL 207 Lua files (34,813 lines): application flow, menus, unit wrappers,
                    views, controls, context managers — the entire ER-301 application layer
percussa/ui/      — DisplayWidget, PanelRenderer (rendering bridge to Percussa display — no longer needed)
percussa/panel/   — SSP/XMX encoder/button adaptation (replace with your own input handling)
```

#### The Lua layer

Lua sits in the middle. It is **tightly coupled to the ER-301 UI** (207 files, 34,813 lines of `app.UIThread`, `app.GraphicContext`, `app.Graphic`, etc.) but **cleanly coupled to the DSP engine** (uses `app.AudioThread.addTask()`, `app.Unit()`, `app.UnitChain()`, `app.ObjectList()`). The DSP-side Lua calls are a thin, straightforward API.

Options:

- **Keep Lua, replace graphics bindings** — remove all `od/graphics` SWIG exports from `od/glue/app.cpp.swig`, add new UI framework exports, rewrite xroot/ against the new UI. Lowest mechanical disruption to the DSP scripting layer.
- **Replace Lua entirely** — expose the DSP engine via a C API or Python/Rust bindings and drive it from a different runtime. More work upfront, cleaner result long-term.

Either way, the DSP orchestration logic (create units, load objects, compile graphs, route I/O) needs to be reproduced. The existing Lua code is the best reference for what that looks like.

#### The SWIG binding file is the explicit contract

`od/glue/app.cpp.swig` lists every C++ class exposed to Lua. It is the natural place to draw the line:

- Keep: ~60 DSP engine headers (`Inlet`, `Outlet`, `Parameter`, `Object`, `Unit`, `UnitChain`, `AudioThread`, etc.)
- Remove: ~100+ graphics headers (`FrameBuffer`, `Graphic`, `GraphicContext`, all widget subclasses)
- Regenerate the SWIG output after the cut

#### Recommended path: clean reimplementation (strip and rebuild)

Given that the firmware is no longer in active upstream development, the pragmatic approach is a **fork-and-strip**:

1. **Copy the repo.** Start from the current Percussa branch.
2. **Delete the UI stack.** Remove `od/graphics/`, the graphics-heavy parts of `od/ui/`, `od/UIThread.cpp`, and all of `xroot/`.
3. **Fix the two DSP/UI coupling points.** Decouple `FileSource` from `UIThread::getRealtimeJobQueue()` (pass the queue in via constructor or a platform interface). Remove the `friend RecordingGraphic` declaration.
4. **Verify the DSP engine compiles standalone.** Adjust `od/glue/app.cpp.swig` to remove graphics exports. Build `od/` + `od/tasks/` + `od/audio/` against the HAL stubs.
5. **Implement your new UI layer.** Drive `AudioThread`, construct `UnitChain`s, read/write `Parameter`s. The existing xroot/ Lua is the reference for what DSP orchestration must do.
6. **Implement new input handling.** Replace `percussa/panel/` with whatever control surface your UI needs.

#### What you get

- A codebase roughly one-third the size (~20,000 lines of battle-tested DSP C++ vs ~77,000 lines total).
- The full DSP engine: topological graph compilation, priority task scheduling, real-time audio I/O, sample loading.
- No ER-301 hardware assumptions anywhere in the DSP path.
- Complete freedom to design the UI for Percussa's actual capabilities (larger display, different controls, network/OSC, whatever).

#### What you lose

- The ER-301 unit library as Lua scripts (xroot/). These would need to be reproduced in whatever scripting layer you choose — but the C++ Object implementations (the actual DSP) are all in `od/objects/` and are kept.
- The existing patch save/load format (which is Lua-serialised state). A new format would be needed.
- Lua-defined custom units. These would need porting to whatever scripting environment replaces xroot/.

---

### UI layer detail: Lua vs C++

The split is approximately **95% Lua, 5% C++**.

**C++ (`od/graphics/`, `od/ui/`)** provides:
- A low-level rendering primitive library: `FrameBuffer` with pixel/line/text/box drawing, a base `Graphic` class with a `draw(FrameBuffer &fb)` virtual method, and concrete widget subclasses (`ListBox`, `MondrianList`, `TextPanel`, etc.).
- Minimal hardware glue: encoder hysteresis, dial curve mapping, LED control, the `JobQueue` async worker.
- No application-level UI logic whatsoever.

**Lua (`xroot/`)** provides everything visible to the user:
- `Application.lua` — the main event loop: `waitForEvent()` / `pullEvent()` / `updateDisplay()`, mode dispatch, global event handlers.
- `UserMode.lua`, `AdminMode.lua` — application modes.
- `Unit/Editor.lua`, `Unit/Menu.lua` — parameter editor and menus.
- `Unit/ViewControl/` (17 files) — per-parameter view controls: GainBias, Gate, Pitch, Clock, Fader, etc.
- All patch navigation, SD card browser, settings UI, slicing views, package manager.

**Lua owns the main UI loop.** C++ starts the Lua interpreter, and Lua calls back into C++ for event polling and display rendering. All behaviour is driven from Lua.

---

### Third-party / mod unit compatibility

Mod units follow a strict two-part structure, making DSP/UI separation clean:

**`onLoadGraph()`** — DSP wiring. Creates C++ Objects (via SWIG bindings), adds them to the Unit's DAG, connects Inlets/Outlets. This code uses only `app.AudioThread`, `app.Unit`, `app.Object` subclasses, `app.Inlet`, `app.Outlet`, `app.Parameter`. **Zero UI coupling. Survives a UI replacement entirely unchanged.**

**`onLoadViews()`** — UI wiring. Creates Lua-side ViewControl instances (`app.ViewControl.GainBias()`, `app.ViewControl.Gate()`, etc.) and adds them to the unit editor. This code is specific to the current UI framework and **must be rewritten** for a new UI. However, it is completely isolated to this one method and has no effect on the DSP graph.

Mod units never write directly to `FrameBuffer` from Lua — that's not exposed. If a mod unit needs custom display output, it ships a C++ `Graphic` subclass compiled into a shared library, implementing `draw(FrameBuffer &fb)`. This is isolated and well-structured, though the rendering target would need to adapt to a new display model.

Unit registration is Lua-side: a `toc.lua` file lists the unit names; Lua loads them via `require()` at runtime. There is no C++ registry. A new UI would need an equivalent discovery mechanism, but it's trivial to replicate.

**Bottom line for mods**: any mod that follows the standard pattern is compatible at the DSP level with no changes. The `onLoadViews()` method in each unit file is the only thing that needs rewriting per unit.

---

### mods/core compatibility

`mods/core` has a clean three-layer structure:

| Layer | Location | Lines | UI coupling | Verdict |
|-------|----------|-------|-------------|---------|
| DSP Objects | `objects/` | ~8,000 | None | **Keep as-is** |
| Graphics subclasses | `graphics/` | ~600 | Isolated (`draw(FrameBuffer&)`) | **Adapt rendering target** |
| Lua unit wrappers | `assets/*.lua` | ~6,000 | `onLoadViews()` only | **Keep DSP half, rewrite views** |

The DSP objects (`Counter`, `Clipper`, `Rectify`, `Fold`, `Limiter`, `GridQuantizer`, `Spread`, `BandedUnit`, etc.) are pure signal processing C++ with no UI includes or dependencies. They can be compiled and used directly.

The graphics subclasses (`RecordHeadDisplay`, `GranularHeadDisplay`, `PitchCircle`) implement `draw(FrameBuffer &fb)` and write into the ER-301 display buffer. They would need adaptation to whatever display model the new UI uses, but the code is isolated in one directory and follows a consistent pattern.

The Lua unit wrappers follow the standard `onLoadGraph()` / `onLoadViews()` split. The `onLoadGraph()` DSP wiring is fully reusable; the `onLoadViews()` UI code needs mechanical rewriting per unit.

**mods/core is a keep target with bounded, mechanical adaptation work.** The most valuable part — the DSP object library — requires no changes at all.

---

### Parameter model and a form-factor-independent UI

#### How the current parameter/UI pipeline works

`Parameter` (C++, `od/objects/Parameter.h`) is a pure float value holder. It knows only its name, current value, target value, and ramp step. No display units, no range, no format string — completely UI-agnostic.

`ViewControl` classes (Lua, `xroot/Unit/ViewControl/`) own everything else: display metadata, pixel layout, encoder binding, DialMap range, and the call to `Parameter::softSet()`. They are hardcoded for the ER-301's 256×64 strip display and 6-button layout — literal pixel constants (`BUTTON1_CENTER = 20`, `BUTTON2_CENTER = 63`, etc.) are baked throughout.

Units wire these together in `onLoadViews()`. A unit does not draw anything — it selects a ViewControl *type* (GainBias, Fader, Gate, etc.), passes in the C++ Object, and the ViewControl extracts the Parameter from it and takes ownership of rendering and input. The `onLoadGraph()` DSP wiring is entirely separate and untouched.

The encoder-to-value path (`DialMap` → `Readout` → `Parameter::softSet()`) is shared infrastructure, already abstracted from specific hardware layout. It can be reused in a new UI.

**Conclusion**: Parameter is a clean abstract concept. The ViewControl framework is display-specific and replaceable. Replacing it does not touch the DSP graph layer.

---

#### The form factor problem

**ER-301**: 1 data encoder + 6 M-buttons. Interaction model: press M1–M6 to *select* a parameter, then turn the one encoder to change it. The UI is naturally a horizontal strip of 6 slots. Navigation (scrolling to more parameters/units) also uses the single encoder — so the encoder serves double duty: navigate vs. edit.

**Percussa SSP/XMX**: 4 data encoders + 2 rows of 4 buttons aligned directly below the encoders. The natural interaction model is: 4 parameters visible and directly editable simultaneously, no selection step required. The 4 buttons below each encoder serve as confirm/toggle/page actions.

The current Percussa port emulates the ER-301 interaction model as faithfully as possible — it does not attempt to exploit the 4-encoder form factor. The goal here is a new UI layer that is native to 4-encoder operation while keeping the DSP/parameter model identical.

---

#### Design sketch: QuadPage UI model

The core idea is a **page-based, 4-encoder-per-page layout** where encoders are always directly bound — no focus/select step.

**Structural concepts:**

- **Page**: a group of up to 4 parameters, one per encoder. The visible state at any moment.
- **PageSet**: all the pages for one unit. Units with more than 4 exposed parameters get multiple pages.
- **Encoder direct binding**: at all times, encoder N directly controls parameter N of the current page. Turning it immediately calls `Parameter::softSet()`. No button press required first.
- **Row-of-4 buttons**: the buttons below the encoders act as page-local context actions (fine/coarse toggle, reset to default, mute, confirm) rather than parameter selectors.

**Navigation model:**

```
[ Unit A | Page 1 of 2 ]           ← header: unit name, page indicator
[ param1 ] [ param2 ] [ param3 ] [ param4 ]   ← current page parameters
[  fine  ] [ reset  ] [  ---   ] [  ---   ]   ← bottom row: context actions

Encoder row (physical): ← → to move between pages within a unit
                        ↑ ↓ to move between units in the chain
```

The second row of 4 buttons could handle: previous page / next page / previous unit / next unit, with the remaining 4 acting as per-page context (fine mode, reset param, open sub-menu).

**Unit `onLoadViews()` changes:**

Units would declare parameter groups (pages of 4) rather than individual ViewControls with strip positions:

```lua
-- current ER-301 style:
controls.gain = GainBias { button = "gain", gainbias = objects.gain, ... }

-- new quad-page style:
pages[1] = QuadPage {
  { label = "Gain",  param = objects.gain:getParameter("Gain"),  map = Maps.dB },
  { label = "Bias",  param = objects.gain:getParameter("Bias"),  map = Maps.volt },
  { label = "Start", param = objects.start:getParameter("Bias"), map = Maps.int(0,256) },
  { label = "Finish",param = objects.finish:getParameter("Bias"),map = Maps.int(0,256) },
}
```

The `QuadPage` ViewControl type handles all rendering and encoder binding. The C++ `Parameter` objects and `DialMap` infrastructure are unchanged — just wired up differently from Lua.

**What changes and what doesn't:**

| Layer | Change |
|-------|--------|
| `Parameter` / `Inlet` / `Outlet` (C++) | None |
| `DialMap` / `Readout` (C++) | None (reuse directly) |
| DSP Object implementations (C++) | None |
| `onLoadGraph()` in all units | None |
| `onLoadViews()` in all units | Rewritten to use `QuadPage` instead of strip ViewControls |
| `ViewControl` Lua framework | Replaced with new `QuadPage` / `QuadControl` types |
| Navigation / event dispatch (Lua) | Replaced (page/unit navigation vs. spot/select model) |
| Display rendering (C++ or Lua) | Replaced (4-column page layout vs. 6-slot strip) |

**Compatibility path for existing units (mods/core etc.):**

A compatibility shim is feasible. A `toQuadPages(controls)` converter function could inspect a unit's declared ViewControls, extract the Parameter objects from each, and auto-pack them into QuadPages of 4. This would give existing units a usable UI on the new form factor without any manual porting. Custom ViewControls with C++ graphics would still need manual adaptation, but the standard GainBias/Fader/Gate/OptionControl cases could be automated.

**Longer term**, units could opt into native `QuadPage` declarations in `onLoadViews()` for a better-grouped parameter layout — but the shim makes unported units functional immediately.

---

#### Summary

The parameter model is already abstract enough to support this. `Parameter` needs no changes. The work is:
1. Define `QuadPage` / `QuadControl` ViewControl types (Lua + C++ rendering).
2. Implement new navigation event dispatch (page/unit navigation instead of spot/select).
3. Rewrite `onLoadViews()` for units you want to optimise — or write an auto-shim for the rest.
4. Adapt `mods/core` Lua assets to use `QuadPage` declarations (mechanical, ~6k lines, consistent pattern).

The DSP engine, all Object implementations, and all graph wiring are entirely untouched.

---

### Phased implementation: layout-first, rendering-later

#### How the graphics coordinate system works

`Graphic` stores `mLeft`/`mBottom` as parent-relative offsets. On every `draw(FrameBuffer &fb)` call, `mWorldLeft`/`mWorldBottom` are computed by summing down the parent chain (`Graphic.cpp`, `updateWorldCoordinates()`). All actual FrameBuffer drawing methods (`fill`, `hline`, `vline`, `text`, etc.) use these world coordinates. `setPosition()` triggers a world-coordinate update that cascades to all children.

There is no `blit()` or buffer-to-buffer paste operation in the current codebase. `FrameBuffer` is a drawing surface only.

#### Phase 1: change the layout, not the rendering

This is the key insight for phasing: **you don't need to change how any ViewControl renders internally**. You only need to change *where it is placed* in the scene graph.

The current ER-301 layout code places ViewControls at hardcoded horizontal positions (`BUTTON1_CENTER = 20`, `BUTTON2_CENTER = 63`, ...) one per button slot. Swapping that layout code to a 4-column quad arrangement — one column per encoder — changes the final display position without touching a single pixel of ViewControl rendering logic. `setPosition()` cascades the change through the child hierarchy automatically.

Phase 1 work is therefore:

1. Replace the strip layout manager (the code that calls `setPosition()` per ViewControl) with a quad-column layout manager. The 4 columns map to the 4 encoders. Each column has a fixed x position and full display height.
2. Implement quad-page navigation: left/right moves between pages of 4 parameters, up/down moves between units. This replaces the spot/select/scroll encoder navigation.
3. Map encoder input directly to the focused column's ViewControl. No M-button press needed — encoder N drives column N's parameter at all times.
4. The bottom row of 4 buttons becomes page/unit navigation or context actions rather than parameter selectors.

At the end of Phase 1 the UI is native to 4-encoder operation. Every existing ViewControl renders exactly as before — same graphics code, same DialMap, same Parameter binding — just at a different position on a different display layout.

#### Phase 2: true offscreen compositing (optional, later)

If you later want each ViewControl to render into its own independent buffer (so you can scale, fade, overlay, or freely reposition components at runtime), you need one small addition to the C++ layer: a `blit()` method on `FrameBuffer`.

```cpp
// Add to FrameBuffer:
void blit(const FrameBuffer &src, int srcX, int srcY,
          int w, int h, int dstX, int dstY);
```

With that in place you can:
- Create a `FrameBuffer` sized to the ViewControl's bounding box.
- Set the ViewControl's parent world origin to `(0, 0)` so it draws into local coordinates.
- Call `graphic->draw(offscreenBuffer)`.
- Blit the offscreen buffer onto the real display at whatever position you choose.

This enables true compositing: slide-in animations, per-control opacity, freely-overlapping panels, or rendering the same ViewControl into multiple display regions. It is not needed for Phase 1 — the scene graph position mechanism already handles static placement — but it opens the door to a fully composited UI in Phase 2 without changing any ViewControl rendering code.

#### Phase 3: replace individual ViewControl renderers

Once the layout framework and navigation model are solid, individual ViewControls can be replaced one at a time with native quad-layout renderers — smaller, higher information density, designed for the Percussa display size. Since each ViewControl is isolated and the Parameter binding interface is unchanged, these replacements are independent of each other and of the navigation layer.

#### Summary of phases

| Phase | What changes | What stays the same |
|-------|-------------|---------------------|
| 1 — Layout & navigation | Layout manager (strip → quad-column), input routing (M-button → direct encoder), page/unit navigation | All ViewControl rendering, all Parameter binding, all DSP |
| 2 — Compositing (optional) | Add `FrameBuffer::blit()`, switch layout to offscreen-then-paste | All ViewControl rendering, all Parameter binding, all DSP |
| 3 — New renderers | Individual ViewControl Lua/C++ implementations, one at a time | Parameter model, DialMap, DSP, navigation |

Each phase is independently shippable and doesn't break previous work.

---

## Changing I/O channel counts (outputs 4→8, more mod inputs)

### AC/DC coupling — not a firmware concern

The ER-301's AC-coupled outputs are purely a hardware property (output capacitors on the analog stage). There is no DC blocking filter, highpass filter, or any coupling enforcement anywhere in the firmware audio path. `Sample::removeDC()` (`od/audio/Sample.cpp`) exists only for processing sample files loaded from disk; `OnsetDetector` skips the DC FFT bin for analysis purposes only. The live audio output path passes DC freely. Percussa's DC-coupled outputs already work correctly with the existing firmware — no firmware change is needed or would help.

### How channel counts are currently encoded

The counts live in two places:

```
hal/constants.h:  NUM_OUTPUT_CHANNELS = 4
                  NUM_INPUT_CHANNELS  = 20
```

These constants drive the interleaved buffer layout in `Pump_callback` — the audio buffer is `float[NUM_INPUT_CHANNELS * frameLength]` for inputs and `float[NUM_OUTPUT_CHANNELS * frameLength]` for outputs. Changing them adjusts the stride used in `InputTask` and `OutputTask` copy loops automatically.

However, `OutputTask` and `InputTask` use **hardcoded named struct members**, not arrays:

```cpp
// OutputTask.h
Inlet mOut1{"OUT1"}, mOut2{"OUT2"}, mOut3{"OUT3"}, mOut4{"OUT4"};
Outlet mMonitor1{"Monitor1"}, ...mMonitor4{"Monitor4"};

// InputTask.h
Outlet mA1{"A1"}, mA2{"A2"}, mA3{"A3"};
Outlet mB1{"B1"}, ...
Outlet mIN1{"IN1"}, ...mIN4{"IN4"};
```

And the Lua boot wiring explicitly names every channel:

```lua
-- xroot/boot/app-setup.lua
externalSources["OUT1"] = Source("OUT1", outputTask.mMonitor1)
...
externalDestinations["OUT4"] = outputTask.mOut4
```

### What changing outputs 4→8 requires

**C++ changes (mechanical):**

1. `hal/constants.h` — change `NUM_OUTPUT_CHANNELS` from 4 to 8.
2. `od/tasks/OutputTask.h` — add `mOut5`–`mOut8` Inlets and `mMonitor5`–`mMonitor8` Outlets.
3. `od/tasks/OutputTask.cpp` — add four more `if (mOutN.isConnected()) { copyToOutputs(..., N-1); }` blocks. The existing `copyToOutputs` loop already uses `NUM_OUTPUT_CHANNELS` as the interleave stride, so it adapts automatically.

**Lua changes (mechanical):**

4. `xroot/boot/app-setup.lua` — add OUT5–OUT8 to `externalSources` and `externalDestinations`.
5. `xroot/Channels/Group.lua` — add "OUT5"–"OUT8" to the channel name list (currently a hardcoded 4-element table).
6. `xroot/Channels/init.lua` — add link/unlink operations for new adjacent pairs (OUT5+6, OUT6+7, OUT7+8).

That is the complete set of changes. Nothing in the DSP graph engine (`Unit`, `UnitChain`, `TaskScheduler`, `Object`) knows or cares about the number of physical outputs.

### What changing mod inputs requires

The 20 inputs split into two groups in `hal/channels.h`:

- **Modulation inputs** (A1–D3, G1–G4): routed via the ADC hardware, each mapped to a physical channel index via `INPUT_xx` defines.
- **Audio inputs** (IN1–IN4): routed via the audio codec.

Adding more modulation inputs (e.g. passing more channels from Percussa's `Pump_callback`):

1. `hal/constants.h` — increase `NUM_INPUT_CHANNELS`.
2. `hal/channels.h` — add new `INPUT_Xx` defines for the new channels, mapping to their buffer index positions.
3. `od/tasks/InputTask.h` — add new `Outlet` members for the new inputs.
4. `od/tasks/InputTask.cpp` — add `INPUTCOPY(Xx)` calls and extend `mLastInput` cache (it is sized `NUM_INPUT_CHANNELS`).
5. `xroot/boot/app-setup.lua` — add new named sources to `externalSources`.

Adding more audio inputs (IN5–IN8 style) is identical, just using the IN-group naming convention.

### On the Percussa side

Since Percussa's `Pump_callback` is just called with `float *inputs, float *outputs` and the channel counts are defined at compile time in `hal/constants.h`, changing the channel count is a compile-time decision. The Percussa RtAudio setup would need to open the corresponding number of hardware channels, and the constants updated to match. No runtime detection or dynamic allocation is involved.

### Summary

| Change | C++ files | Lua files | DSP engine impact |
|--------|-----------|-----------|-------------------|
| Outputs 4→8 | `constants.h`, `OutputTask.h/.cpp` | `app-setup.lua`, `Channels/Group.lua`, `Channels/init.lua` | None |
| Add mod inputs | `constants.h`, `channels.h`, `InputTask.h/.cpp` | `app-setup.lua` | None |
| AC/DC coupling | None | None | Not a firmware concern |

The DSP graph engine is completely insulated from physical I/O counts. `Unit`, `UnitChain`, `TaskScheduler`, and all `Object` subclasses are unchanged regardless of how many physical channels exist.

---

## Plan Outline

An ordered implementation plan across all the ideas above. Each phase produces something buildable and usable on Percussa hardware. Some deliberate rework is accepted (Phase 2 navigation is later refined in Phase 3 rendering), but the ordering is chosen to minimise it: UI framework before channel expansion; channel expansion before multi-core; multi-core last as it is the most disruptive to the DSP layer.

---

### Phase 1 — Fork and establish a clean baseline

**Goal:** stable, buildable starting point with known deletion targets.

- Fork from the current `ssp` Percussa branch
- Verify clean build and successful boot on SSP/XMX hardware
- Audit and document exactly what will be stripped in later phases (do not delete yet)
- Confirm `mods/core` DSP objects all compile and units are reachable

**Testable:** identical behaviour to current Percussa port — audio works, existing UI navigable.

---

### Phase 2 — Native 4-encoder UI navigation

**Goal:** break away from the ER-301 one-encoder interaction model, without touching any rendering or DSP code.

- Replace the strip layout manager with a quad-column layout (one column per encoder, positioned via `setPosition()` — existing ViewControl rendering unchanged)
- Implement direct encoder binding: encoder N always controls column N parameter, no M-button focus step
- Implement page/unit navigation: left/right pages within a unit, up/down between units in chain
- Reassign the 2×4 button rows to navigation and context actions (fine/coarse, reset, page nav)
- Write a compatibility shim that auto-maps existing ViewControl declarations to quad pages so all units and `mods/core` work without modification

**Testable:** all existing units controllable via native 4-encoder workflow on Percussa hardware. `mods/core` units fully accessible.

*Dependency: Phase 1 complete.*

---

### Phase 3 — Codebase strip and new rendering layer

**Goal:** remove the ER-301 UI stack, replace with a Percussa-native implementation. The navigation model from Phase 2 is kept; only rendering changes.

- Strip `xroot/` Lua application layer; replace with a minimal new boot + application framework built around the QuadPage model
- Strip `od/graphics/` C++ widget library; replace with a thin rendering layer sized for the Percussa display
- Implement new `QuadPage`/`QuadControl` ViewControl types with Percussa-native rendering
- Port `mods/core` `onLoadViews()` from old strip-style declarations to `QuadPage` declarations
- Remove the Phase 2 compatibility shim once all target units are ported
- Decouple `FileSource` from `UIThread::getRealtimeJobQueue()` (pass queue via constructor)

**Testable:** clean, stripped codebase; all `mods/core` units have native quad-layout UI; audio and patch graph unchanged.

*Dependency: Phase 2 complete. This is the longest phase.*

---

### Phase 4 — Channel count expansion

**Goal:** increase physical I/O to match Percussa hardware capabilities.

- Increase `NUM_OUTPUT_CHANNELS` (4→8) in `hal/constants.h`
- Extend `OutputTask` with new named Inlets/Outlets for OUT5–OUT8
- Increase `NUM_INPUT_CHANNELS` and extend `InputTask` for additional mod inputs
- Update `hal/channels.h` with new `INPUT_xx` index defines
- Wire new channels into the new UI framework (app-setup, channel group declarations)
- Expose new outputs and inputs in the patch routing UI

**Testable:** 8 physical outputs routable in patches; additional mod inputs available as signal sources.

*Dependency: Phase 3 complete (avoids duplicating Lua wiring work in both old and new UI).*

---

### Phase 5 — Multi-core DSP

**Goal:** exploit Percussa's multi-core CPU for DSP throughput. Kept last because it is the most invasive change to the DSP layer and is easiest to reason about on a clean, well-understood codebase.

- Refactor `TaskScheduler::process()` to release the mutex before execution (currently held for the full loop)
- Implement a thread pool dispatcher that runs independent `UnitChain` tasks concurrently on separate cores
- Add a barrier before `OutputTask` so all chains complete before outputs are collected
- Validate there are no latent shared-state races exposed by true parallelism (see architecture notes on single-core vs multi-core race conditions)

**Testable:** measurable CPU load reduction under heavy patches; audio output bit-identical to single-threaded execution for patches with no inter-chain dependencies.

*Dependency: Phase 3 or 4 complete. No UI work required.*

---

### Phase 6 — Reimagined UI (post-strip, open-ended)

**Goal:** rethink the UI more broadly now that there are no inherited ER-301 constraints.

Phase 2 intentionally does the minimum to get 4-encoder navigation working — it is a direct translation of the strip model into a wider column layout, not a final UI vision. Once Phase 3 is complete, the composable component layer exists (`QuadPage`/`QuadControl`, `FrameBuffer::blit()` for offscreen compositing) and the codebase carries no legacy layout assumptions. This is the point to re-imagine navigation and layout from scratch if desired.

Possibilities that become natural at this stage:
- A patch overview / signal flow view showing chains and connections
- Per-unit full-screen editors with more information density
- Context-sensitive layouts that change shape depending on unit type
- Animated transitions between views using the blit compositing layer

Phase 2's navigation model (page/unit movement, direct encoder binding) would likely survive mostly intact as the interaction idiom — what changes is the visual structure and information hierarchy around it.

**Testable:** user-defined milestone. This phase has no fixed scope.

*Dependency: Phase 3 complete.*

---

### Dependency summary

```
Phase 1 (baseline)
    └── Phase 2 (navigation)
            └── Phase 3 (strip + rendering)   ← longest
                    ├── Phase 4 (channels)
                    ├── Phase 5 (multi-core)
                    └── Phase 6 (reimagined UI)
```

Phases 4, 5, and 6 are all independent of each other once Phase 3 is done and can be tackled in any order.
