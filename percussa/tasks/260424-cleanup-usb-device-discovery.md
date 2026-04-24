# Remove Hard-Coded USB Device Assumptions

## Brief Description
Replace the legacy hard-coded front USB block-device assumption with a more robust device discovery or mounting strategy in the percussa SSP path.

## Why This Is Needed
The SSP-equivalence goal no longer depends on this if real behavior already matches legacy `ssp`, but the current implementation still assumes the front USB storage device appears as `/dev/sda1`. That is fragile and depends on kernel/device enumeration details rather than explicit discovery.

This is therefore a robustness task, not an equivalence blocker. It should make the front USB mount behavior safer and more portable without changing the higher-level behavior the user already sees.

## What The Work Entails
- Review the current mount path in `percussa/hal/card.cpp`.
- Replace the hard-coded `/dev/sda1` assumption with a more reliable discovery strategy or a safer mounting approach.
- Preserve the existing USB mass-storage mode gating behavior.
- Keep failure logging clear enough that hardware troubleshooting remains straightforward.
- Avoid widening the task into unrelated storage or session work.

## Notes
The goal is not to redesign the full storage stack. The goal is to remove the most brittle legacy assumption from the current front USB mount flow.
