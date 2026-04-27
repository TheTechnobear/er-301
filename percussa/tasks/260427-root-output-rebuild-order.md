# 260427 Root Output Rebuild Order

## Summary

During the buzz investigation, a lower-confidence concern came up around how root chain outputs are rebuilt.

`UnitChain` internal wiring is torn down and rebuilt under `lock()` / `unlock()`, but root output connections are managed directly from the chain layer rather than through `ConnectionQueue`.

## Why it looked suspicious

- Rapid graph changes are involved in the field repro.
- Link/unlink operations rebuild root chains and clear the symptom.
- Direct output reconnect order can sometimes hide stale or transient graph-state problems.

## Why it was discounted for the current buzz investigation

- It is a weaker fit than the shared disconnected-input fallback theory.
- It does not directly explain why assigning **any** chain input source suppresses the buzz.
- It also does not naturally explain why source-unit control behavior changes when the buzz is present.

## Suggested next step

Only revisit this if the shared-zero / muted-output investigation does not reproduce or explain the bug.
