# 260427 OUT4 Disconnect Zeroing

## Summary

While investigating the buzz bug, an unrelated output-path issue was found in `od/tasks/OutputTask.cpp`.

When `OUT4` is disconnected, the fallback zeroing path calls:

`zeroOutput(outputs, 4)`

The valid channel indices are `0..3`, so this is off by one.

## Why this looks like a bug

- `OUT1`, `OUT2`, and `OUT3` use channel indices `0`, `1`, and `2`.
- `OUT4` should therefore use channel index `3`.
- The current code does not explicitly zero the fourth output channel when disconnected.

## Why it was discounted for the current buzz investigation

- The field report centers on `OUT1`.
- This bug is channel-specific to `OUT4`.
- It does not explain the strong relationship between the buzz and disconnected chain inputs.

## Suggested next step

Fix the off-by-one call and verify disconnected `OUT4` really goes silent in both host and hardware builds.
