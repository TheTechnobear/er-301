# Migrate The Real Pump_callback Owner

## Brief Description
Move deeper into the real ER-301 ownership path behind `Pump_callback(...)` so the percussa audio/runtime boundary is backed by the actual runtime owner rather than only the outer HAL-shaped shell.

## Why This Is Needed
This is the main remaining architectural seam in the refactor. The outer audio files are in the right place and the build is honest enough to compile through the broader ER-301 surface, but the real owner servicing `Pump_callback(...)` has not been migrated yet. That means the audio/runtime shape is still incomplete from a refactor-architecture standpoint even if SSP behavior is already close enough for the current equivalence goal.

In the current percussa shape, `percussa/hal/audio.c` is the outer audio-thread adapter: it gathers input, prepares the frame buffers, and then calls `Pump_callback(inputs, outputs)`. That part is already the natural entry point into the ER-301 processing side.

The reason `od/AudioThread.cpp` matters is that, in the legacy ER-301 codebase, `Pump_callback(...)` is implemented there. `AudioThread` is not the hardware callback itself; it is the engine-side owner behind that callback. It owns the audio task scheduler, the input/output tasks, the frame pool, the connection queue, and the actual `tasks.process(inputs, outputs)` call that turns one audio frame into the next.

So this task is really about answering a concrete ownership question: should percussa continue calling directly into the legacy `od::AudioThread` implementation, or should that ownership be migrated/extracted into a clearer percussa-side runtime boundary while preserving behavior?

This task matters because it is the path toward a more truthful runtime boundary and a cleaner long-term architecture for additional products.

## What The Work Entails
- Trace the real owner behind `Pump_callback(...)` in the existing ER-301 path.
- Identify the smallest credible migration step that moves one level deeper without inventing a new shallow wrapper.
- Use `od/AudioThread.cpp` and the surrounding processing/runtime graph as the primary reference, because that file currently defines what `Pump_callback(...)` actually does in the legacy system.
- Make the current call chain explicit: hardware/backend callback -> `percussa/hal/audio.c` -> `Pump_callback(...)` -> legacy `od::AudioThread` task processing.
- Decide whether the next step is to depend on that legacy owner more explicitly, or to extract/migrate the ownership that file currently holds.
- If this exposes additional `od/glue` or interpreter-surface work, treat that as part of this migration only where it is concretely required by the ownership change.
- Keep the change local enough that the build continues to expose the next missing seam.
- Preserve current behavior while improving ownership clarity.

## Notes
This is not primarily a UI-equivalence task. It is a refactor-depth and runtime-ownership task. Any `od/glue` expansion should be driven by this concrete migration, not tracked as a vague standalone backlog item.

If handed to a developer as a standalone task, the expected first step is to inspect `percussa/hal/audio.c`, `hal/pump.h`, and `od/AudioThread.cpp`, then propose the smallest change that makes the processing owner behind `Pump_callback(...)` more explicit and less accidental.
