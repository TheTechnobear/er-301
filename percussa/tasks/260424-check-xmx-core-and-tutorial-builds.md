# Check XMX core And tutorial Builds

## Brief Description
Verify the `core` and `tutorial` build paths for XMX and make any small build-script or output-location adjustments needed so those targets build in a clean and predictable way.

## Why This Is Needed
Once the minimal XMX path exists, the next likely friction point is build-system shape rather than runtime behavior. The current percussa build structure already handles SSP and XMX target separation, but the supporting build outputs for `core` and `tutorial` may still carry assumptions that were only exercised in the SSP flow.

This is mainly a build hygiene task. It is needed so XMX work does not stall on avoidable script/output-location issues.

## What The Work Entails
- Check whether `core` and `tutorial` targets build correctly for the XMX configuration.
- Verify output locations and staging paths are consistent with the current target split.
- Make small script or output-path adjustments where needed.
- Avoid broad build-system redesign unless a concrete blocker requires it.
- Keep the resulting build shape easy to understand for both SSP and XMX targets.

## Notes
This task should stay focused on practical build usability, not on a wholesale rewrite of the build scripts.
