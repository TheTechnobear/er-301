# Clean Up scripts/percussa.mk

## Brief Description
Simplify `scripts/percussa.mk` so the target/source grouping is easier to read and maintain now that the current SSP and XMX build split is already working.

## Why This Is Needed
The current makefile shape is functional, but it still leans on broad recursive source lists and `filter-out` behavior. That was acceptable while the main goal was getting the build honest enough to expose the real missing seams. Now that the current target/platform split is working, readability and maintainability matter more.

This is not a correctness blocker, but it will reduce friction for the next rounds of runtime/platform and XMX work.

## What The Work Entails
- Revisit how `scripts/percussa.mk` groups common, platform-specific, and panel-specific sources.
- Reduce reliance on large recursive source lists plus exclusions where a clearer grouping can replace them.
- Preserve the current target behavior and output structure.
- Keep the file easy to review so it is obvious what each target actually compiles.
- Avoid mixing this cleanup with broader changes unless they are directly required.

## Notes
The goal is clarity, not cleverness. Prefer explicit source grouping over dynamic behavior that hides what is really being built.
