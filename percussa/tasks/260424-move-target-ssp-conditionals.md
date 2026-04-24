# Move TARGET_SSP Conditionals Out Of percussa/hal

## Brief Description
Refactor the remaining `TARGET_SSP` conditionals out of `percussa/hal` and move the hardware/product-specific decisions into cleaner runtime or platform-owned services.

## Why This Is Needed
The SSP equivalence phase appears complete enough to stop treating `percussa/hal` as a temporary landing zone for product-specific behavior. The remaining `TARGET_SSP` branches inside `percussa/hal` make the HAL layer carry policy that really belongs higher up in the runtime/platform boundary. This makes the code harder to reuse for XMX and makes it less obvious which behavior is truly generic versus SSP-specific.

This work is needed before the XMX path is expanded, because several of the current conditionals are implicitly assuming SSP hardware or SSP runtime behavior. Leaving those assumptions in the HAL layer will make the next platform/panel work messier than it needs to be.

## What The Work Entails
- Audit `percussa/hal` for remaining `TARGET_SSP` conditionals and classify each one.
- Separate generic HAL behavior from product-specific behavior.
- Move product/runtime decisions into the appropriate runtime or platform service instead of leaving them as preprocessor branches in HAL files.
- Preserve current SSP behavior while making the shape clean enough for XMX to reuse the same layers.
- Keep the scope local to `percussa` unless there is a concrete reason to widen it.

## Notes
The goal is not to remove conditionals blindly. The goal is to move ownership to the right layer so the HAL becomes a thinner hardware-facing surface and product/platform policy becomes explicit elsewhere.
