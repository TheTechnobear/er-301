# To do / Improve 

a grab bag of things that could possibly be improved.
this will included tasks Ive archived, as they are 'complete', but have possible follow ups

I do not include active tasks here, nor 'future work' that I've not commited too see [future ideas](percussa/future-ideas.md)

that, said this list, nor tasks are a commitment ;) 

# 1   handing out the address of the shared `ZeroOutput` buffer  - Fragile
from : [buzz bug](tasks/archived/260427-buzz-bug.md)
### Longer-term design note

Even with the race fixed, `Outlet::buffer()` handing out the address of the shared `ZeroOutput` buffer when an outlet is muted is a latent hazard. Any future call site that receives and writes to that pointer will corrupt global state silently. A more defensive design would give each `Outlet` a private discard buffer used only when muted, keeping `ZeroOutput` read-only. This is a broader refactor and not required to fix the immediate bug.

### Recovery-path note

Link/unlink is **not** a direct re-zero path for `ZeroOutput` / `OneOutput`; those global buffers are initialized in `AudioThread::init()`. If a recovery/reset action is added later, it should be an explicit synchronized operation rather than relying on link/unlink side effects.

note: more problematic outside of er301, as er301 is single core, albeit premptive... rather than true multi-thread/core.

