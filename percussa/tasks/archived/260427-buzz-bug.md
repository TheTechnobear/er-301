# 260427 Buzz Bug

## Current state

- User testing : FIXED, cannot reproduce with on hardware where before it was easily reproduced.
- Root cause is now believed to be a real race in `UnitChain::mute()` / `unmute()` that can let `Repeater` write through a muted outlet into the shared `ZeroOutput` buffer.
- The primary fix has been implemented in code by taking `mMutex` around the mute/unmute state flip so `process()` cannot observe the inconsistent intermediate state.
- This task stays open for follow-up monitoring and for secondary hardening work, especially around the broader `ZeroOutput` design hazard and any optional recovery/reset path.

## Original bug description

Reported during testing on both macOS host builds and XMX hardware builds:

- Sometimes a quiet but audible buzzing sound appears on a track when there should be silence.
- The issue does not appear to be unit-specific.
- Repro examples reported so far:
  - Insert a `SineOsc` on `OUT1`, bypass it, then re-enable it a few times, especially rapidly. While bypassed, a buzz can be heard even though there should be no sound.
  - Delete the unit after the buzz starts; the buzz can continue.
  - Sometimes inserting a single unit and then deleting it is enough to trigger the buzz.
- Actions reported to stop the buzz:
  - Assign any input source to the chain input. The input chosen does not matter. The buzz stops while the input is assigned, then returns when the input is removed.
  - Link and unlink channels.
- Additional observations from testing:
  - The buzz is visible on scopes, so it is present in the DSP graph rather than being an output-driver artifact.
  - The buzz can be modified by downstream units.
  - Applying an offset around `-0.6` can make it disappear.
  - The buzz is usually a similar tone each time and roughly around 10% level.
  - One confusing detail is that the buzz can affect the tone of a source unit like `SineOsc`, even though that unit should not be mixing chain audio input in the normal way.

## Test cases

### 1. Rapid bypass/unbypass on a source unit

1. Start with a clean root chain on `OUT1`.
2. Insert `SineOsc`.
3. Toggle bypass on and off repeatedly, including fast toggling.

**Expected:** When bypassed, the chain should be silent.  
**Actual:** A quiet buzz can sometimes appear and continue.

### 2. Delete-after-buzz persistence

1. Trigger the buzz with the bypass/unbypass repro above.
2. Delete the unit.

**Expected:** Output should return to silence.  
**Actual:** The buzz can remain after the unit is gone.

### 3. Insert/delete only

1. Start with an empty chain.
2. Insert a single unit.
3. Delete it.

**Expected:** Output should return to silence.  
**Actual:** Buzz can sometimes start even without the explicit bypass repro.

### 4. Chain input assignment suppression

1. Trigger the buzz on an otherwise empty or effectively silent chain.
2. Assign any input source to the chain input.
3. Remove that input source again.

**Expected:** If the bug source is unrelated to chain input fallback, input assignment should not matter.  
**Actual:** Assigning any input source suppresses the buzz; removing it allows the buzz path to return.

### 5. Channel link/unlink reset

1. Trigger the buzz on a root chain.
2. Link and unlink the adjacent output channels.

**Expected:** If the signal source remains present, link state changes should not clear it.  
**Actual:** Link/unlink clears the issue, consistent with a graph teardown/rebuild side effect.

### 6. Scope validation

1. Trigger the buzz.
2. Inspect the chain on scope and then process it with downstream units.

**Expected:** If this were a hardware/output-only issue, scopes would stay quiet.  
**Actual:** The buzz is present in the DSP path and is processed by later units.

## Analysis and investigation

### What was checked

- Root chain build/rebuild behavior in `xroot/Chain/*` and `od/tasks/UnitChain.cpp`
- Unit bypass flow in `xroot/Unit/init.lua`
- Input/output task plumbing in `od/tasks/InputTask.cpp` and `od/tasks/OutputTask.cpp`
- Inlet/outlet fallback behavior in `od/objects/Inlet.cpp` and `od/objects/Outlet.cpp`
- Root-chain link/unlink behavior in `xroot/Channels/*`
- A representative source unit path using `TestOscillator` and `Comparator`

### Key observations from code reading

#### 1. Empty or unpatched paths fall back to a shared zero outlet

- `Inlet::buffer()` returns `ZeroOutput.buffer()` when an inlet is disconnected.
- A root `UnitChain` with no active units and no chain input source falls through to its repeater path.
- The repeater input then reads the disconnected inlet fallback, which is the shared `ZeroOutput`.

This matches the field observation that assigning any chain input source suppresses the bug. Once a real source is assigned, the chain no longer depends on the disconnected-input fallback.

#### 2. The same shared fallback is used by unpatched control inputs

This explains why a source unit can still be affected. Even if a source unit does not pass chain audio input through, it can still have unpatched control inlets reading from the same shared disconnected-input fallback.

For example, the test oscillator path uses:

- `TestOscillator` for the oscillator core
- `Comparator` for sync handling

If the default disconnected-input buffer is no longer truly silent, that signal can alter sync/control behavior and change the oscillator tone.

#### 3. Link/unlink likely fixes the symptom by forcing a rebuild

- Bypass toggling calls `self.chain:rebuildGraph()`.
- Channel link/unlink destroys and recreates root chain groups.
- Both behaviors cause disconnect/reconnect of chain internals and outputs.

This makes link/unlink useful as a reset mechanism, but not especially informative about the original source of the bad signal.

#### 4. The most suspicious code path is the mute-to-zero-outlet behavior

`Outlet::buffer()` returns `ZeroOutput.buffer()` when the outlet is muted:

- If an outlet is muted and later code still writes through the pointer returned by `buffer()`, it would write directly into the shared global zero buffer.
- If that happens once, every disconnected inlet that expects silence could start seeing the same contaminated signal.

That matches several reported symptoms unusually well:

- buzz appears where silence should be
- buzz survives unit deletion
- buzz disappears when a real source replaces the fallback path
- source units can change behavior through defaulted control inlets

### Alternative explanations considered

#### Uninitialized `InputTask` input buffers

This was an early suspicion from the field report, but it does not fit the empty/no-input case well. A chain with no assigned input source should be falling back to the disconnected-input zero outlet, not to `InputTask`.

#### Stale chain/output buffer after rapid graph changes

Still possible, but weaker than the shared-fallback explanation. The chain rebuild path explicitly disconnects and reconnects internal routing, so a plain missed disconnect is not the best fit.

#### Root-output reconnect ordering issue

Possible as a lower-confidence side issue, especially because root outputs are connected directly rather than through `ConnectionQueue`, but it does not explain the strong correlation with “assign any input source and the buzz stops”.

## Resulting conclusions

### Primary conclusion

The strongest current hypothesis is:

**A shared silent fallback buffer is becoming contaminated, and the main bug is therefore most likely in the disconnected-input / muted-output path rather than in any specific unit.**

More specifically:

- the field behavior points toward the path used when a chain or control inlet is disconnected
- the codebase uses a shared global `ZeroOutput` outlet for that fallback
- `Outlet::buffer()` can hand out the `ZeroOutput` buffer when an outlet is muted
- if any muted outlet is still written to, the global zero buffer can stop being zero

This is the best fit for the combined symptom set.

### Secondary conclusion

The bug is likely graph-wide once triggered, not local to the unit that triggered it. That explains why deleting the triggering unit does not necessarily stop the buzz.

## Next steps

1. Instrument the shared zero-buffer path first.
   - Confirm whether `ZeroOutput` ever becomes non-zero after graph edits, bypass toggles, or mute/unmute transitions.

2. Instrument `Outlet::buffer()` for muted outlets.
   - Identify which objects are requesting writable output buffers while muted.

3. Add a focused repro harness around:
   - rapid bypass toggling
   - insert/delete on a root chain
   - assign/remove chain input source

4. If the shared-zero hypothesis is confirmed, fix the design at the source rather than patching individual units.
   Candidate fixes to evaluate:
   - never return the shared zero buffer as a writable destination for muted outlets
   - make disconnected-input buffers effectively read-only at the API level where practical
   - add debug assertions to catch writes into shared constant buffers

5. Keep the unrelated side findings separate from this task so the main investigation stays focused.

---

## Root cause identified

### The exact mechanism

The contamination path is a race condition in `UnitChain::mute()` (`od/tasks/UnitChain.cpp:319`).

```cpp
void UnitChain::mute() {
    if (!mMuted) {
        mFade.reset(0.0f);
        while (mActive && Audio_running() && mFade.notFinished()) {
            Thread::yield();
        }
        mLeftOutput.mute();    // (A) sets mIsMuted = true on the Repeater outlet
        mRightOutput.mute();
        mMuted = true;         // (B) sets the chain-level guard flag
    }
}
```

`process()` takes `mMutex` to serialize itself with graph rebuilds (lock/unlock). But `mute()` does **not** hold `mMutex` when writing (A) and (B). Between these two lines, the audio callback can execute `process()`:

```
Lua thread (core 0)                  Audio thread (core 1)

mLeftOutput.mute()   ← (A) mIsMuted=true
                                      process() fires
                                        mMutex.enter()
                                        if (mMuted)          ← false: not yet set
                                        else if (fade done)  ← true: fade completed
                                        else:
                                          copyInputToOutput()
                                            out = mOutlet.buffer()
                                                mIsMuted=true → ZeroOutput.buffer()!
                                            in  = last unit's output buffer
                                            memcpy(ZeroOutput, in, FRAMELENGTH*4)
                                            ZeroOutput permanently contaminated
                                        mMutex.leave()
mMuted = true        ← (B)
```

`ZeroOutput.mBuffer` now contains one frame of audio from the fading chain. `initializeGlobalOutlets()` is called only once at startup and never re-zeros the buffer. Every disconnected `Inlet` in the entire audio graph falls back to `ZeroOutput.buffer()` — they all now return this frozen frame.

### How the bypass toggle reaches mute

`Unit:toggleBypass()` (`xroot/Unit/init.lua:319`) calls:

```lua
local wasMuted = chain:muteIfNeeded()   -- calls pChain:mute() if not already muted
...enableBypass() / disableBypass()...
chain:unmuteIfNeeded(wasMuted)          -- calls pChain:unmute()
```

`muteIfNeeded` maps directly to `UnitChain::mute()`. Every bypass toggle drives the full mute/unmute cycle against an actively-processing chain, repeatedly opening the race window. Rapid toggling multiplies the exposure.

### Why every symptom follows from this one bug

| Symptom | Explanation |
|---------|-------------|
| Buzz persists after unit deleted | `ZeroOutput.mBuffer` is never re-zeroed; deleting units has no effect on it |
| Buzz affects source units like SineOsc | Control inlets (sync, modulation) on internal Objects fall back to `ZeroOutput`; contaminated control values alter the oscillator |
| Any chain input assignment suppresses it | The Repeater inlet is connected to a real Outlet; the main signal path no longer reads from `ZeroOutput` |
| Removing the input restores the buzz | Repeater inlet falls back to `ZeroOutput` again |
| Link/unlink suppresses it | Observationally, link/unlink resets enough root-chain state to clear or mask the symptom, but it does **not** directly call `initializeGlobalOutlets()` or explicitly re-zero `ZeroOutput`. The exact cleanup path is therefore weaker than the mute-race proof and should be treated as a reset correlation rather than a fully proven mechanism |
| Buzz is a consistent tone (~10% level) | `ZeroOutput` holds exactly one frame of audio captured at the moment the race fired; that frame is replayed each frame from every disconnected inlet |
| Downstream units modify the buzz | The contaminated signal passes through the DSP graph as any other signal would |

### Secondary race in unmute

```cpp
void UnitChain::unmute() {
    if (mMuted) {
        mLeftOutput.unmute();   // (A)
        mRightOutput.unmute();
        mMuted = false;         // (B)
        mFade.reset(1.0f);      // (C)
    }
}
```

Between (B) and (C) the audio callback can fire with `mMuted=false` and the ramp already at zero (finished). It skips the fade-in and calls `copyInputToOutput()` immediately. The outlet is unmuted so it writes to the real buffer — no `ZeroOutput` contamination — but the fade-in is bypassed, causing a click/pop on unmute. This is a separate issue from the primary contamination bug.

---

## Recommended fix

Take `mMutex` for the state-flip in both `mute()` and `unmute()`, making the outlet flag and chain flag changes atomic with respect to `process()`.

**Status:** Implemented as the primary fix. Follow-up work remains open.

**`od/tasks/UnitChain.cpp` — `mute()`:**

```cpp
void UnitChain::mute() {
    if (!mMuted) {
        mFade.reset(0.0f);
        while (mActive && Audio_running() && mFade.notFinished()) {
            Thread::yield();
        }
        // Hold the mutex so process() cannot run between the outlet mute and
        // mMuted=true, preventing a write through the muted outlet into ZeroOutput.
        mMutex.enter();
        mLeftOutput.mute();
        mRightOutput.mute();
        mMuted = true;
        mMutex.leave();
    }
}
```

**`od/tasks/UnitChain.cpp` — `unmute()`:**

```cpp
void UnitChain::unmute() {
    if (mMuted) {
        // Hold the mutex so the fade-in starts atomically with clearing mMuted,
        // preventing a zero-latency volume jump if process() fires between the two.
        mMutex.enter();
        mLeftOutput.unmute();
        mRightOutput.unmute();
        mMuted = false;
        mFade.reset(1.0f);
        mMutex.leave();
    }
}
```

With this change the race window is closed. Either `process()` runs before the mutex is acquired by `mute()` (outlet not yet muted, writes to real buffer — safe), or `mute()` acquires the mutex first and all three flags are set before `process()` can execute (it sees `mMuted=true` → `// do nothing` — safe).

### Longer-term design note

Even with the race fixed, `Outlet::buffer()` handing out the address of the shared `ZeroOutput` buffer when an outlet is muted is a latent hazard. Any future call site that receives and writes to that pointer will corrupt global state silently. A more defensive design would give each `Outlet` a private discard buffer used only when muted, keeping `ZeroOutput` read-only. This is a broader refactor and not required to fix the immediate bug.

### Recovery-path note

Link/unlink is **not** a direct re-zero path for `ZeroOutput` / `OneOutput`; those global buffers are initialized in `AudioThread::init()`. If a recovery/reset action is added later, it should be an explicit synchronized operation rather than relying on link/unlink side effects.

---

## Is Percussa more likely to trigger this than ER-301 hardware?

**Yes — substantially more likely.** There are two compounding reasons.

### 1. True multi-core parallelism

On the **ER-301 hardware** (TI AM335x, single Cortex-A8 core, TI-RTOS/SysBIOS), the audio ISR and the Lua task share one CPU. The ISR can only preempt the Lua task at a hardware interrupt boundary. The race window between (A) `mLeftOutput.mute()` and (B) `mMuted = true` is two consecutive store instructions. The compiler will typically emit them back-to-back; the probability of an ISR firing in that specific inter-instruction gap is very low in practice.

On **Percussa SSP/XMX** (quad-core Cortex-A17, Linux), the Lua/UI thread runs on core 0 and the audio callback thread runs on core 1 **simultaneously**. The two cores execute truly in parallel. The race window is not a matter of interrupt timing — core 1 is continuously executing. Any moment during `mute()` after (A) but before (B), even if that lasts microseconds, is a window where core 1 can execute `process()` and hit the bad path. The probability per bypass toggle is orders of magnitude higher.

### 2. Cache coherency / memory ordering

On multi-core ARM without explicit memory barriers (`dmb`/`dsb`), writes from one core are not guaranteed to be observed by another core in program order. `mIsMuted = true` (written on core 0) and `mMuted = true` (also written on core 0) may be observed by core 1 in any order, or with stale cached values. It is theoretically possible for core 1 to observe `mIsMuted=true` while still seeing the old `mMuted=false` — precisely the bad state — even if no OS preemption occurs.

The original code was written for a single-core system where memory ordering between "threads" is governed only by ISR priority, not cache coherency. On Percussa's multi-core system, the write-ordering assumption does not hold without barriers. The mutex fix addresses this: `Mutex::enter()` and `Mutex::leave()` are implemented using POSIX pthreads, which include the necessary memory barriers as part of their contract.

### Summary

The bug exists in both platforms — the race is real on single-core too — but on ER-301 hardware the window is narrow enough that it rarely manifests. On Percussa's dual-core setup with truly concurrent threads and no memory barriers protecting the flag writes, the bug is reliably reproducible with rapid bypass toggling.

---

## Git history — changes since v0.6.16

Checked all commits from `v0.6.16` (`3c08845`) to `HEAD` touching `od/tasks/`, `od/objects/`, `od/units/`, `hal/pump/`, `percussa/hal/`, and the xroot chain/unit Lua files.

**None of the commits since v0.6.16 touch the race-affected code.** The mute/unmute logic in `UnitChain.cpp`, `Outlet::buffer()` in `Outlet.cpp`, and the `Repeater` copy path are all unchanged from the last release. The bug has been present since at least v0.6.16.

Commits since v0.6.16 that touch DSP-adjacent areas (none affect the bug):

| Commit | Summary | Relevance |
|--------|---------|-----------|
| `d6b5f44` | Improve comments on frame pool methods in `AudioThread.h` | Comment-only, no functional change |
| `31c8751` | Fix monitor output — scope view Lua corrections | UI/scope only |
| `86c449e` | Add `hardSet`/`softSet` to `Followable` interface | Parameter interface refactor, unrelated |
| `048f070` | Separate `hardSet`/`softSet` into `Settable` interface | Same refactor, unrelated |
| `2c0ceaf` | Fix move-to-mixer crash (out-of-bounds in `SpottedStrip`) | UI/graphics only |
| `5a57544` | Unit section background graphic support | UI only |
| Percussa commits | Port work: XMX panel, toolchain, platform refactor | Platform layer only |
