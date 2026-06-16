# Phase 4 Integration Review

Run ID: `20260616_132857`

## Reviewed phase

Phase 4 Integration only, after Agent 04 and Agent 02 retry outputs.

## Reviewed agents

- `agent_04_ftprdp_integration`
- `agent_02_build_cmake` for the Phase 4 CMake/link ownership retry only

## Reviewed outputs

- `docs/ftp_ryu_migration/09_phase_leader_checkpoints.md`
- `docs/ftp_ryu_migration/05_file_ownership.md`
- `docs/ftp_ryu_migration/prompts/agent_04_ftprdp_integration.md`
- `docs/ftp_ryu_migration/reports/ftprdp_integration_report.md`
- `docs/ftp_ryu_migration/reports/build_cmake_report.md`
- `docs/ftp_ryu_migration/logs/20260616_134746_agent_04_ftprdp_integration.out`
- `docs/ftp_ryu_migration/logs/20260616_135637_agent_02_build_cmake.out`
- `git diff -- CMakeLists.txt src/miman_ftprdp.cpp src/miman_ftprdp_integration.h src/miman_ftp.h`
- `rg` call-path, raw API, and pthread creation evidence
- `/tmp/5th_gs_agent02_retry_build` object, archive, and link-map symbol evidence

Agent 04 completed with `AGENT_STATUS: DONE`. Agent 02 completed the requested
CMake/link ownership retry with `AGENT_STATUS: DONE`.

## Missing outputs

- No real OBC runtime smoke was executed in Phase 4.
- No owning GUI/autopilot patch exists for the pre-`pthread_create()` argument
  boundary. Current creation sites still pass `&State.ftplistup[NowFTP]` to
  old workers in `src/miman_autopilot.cpp:634-662` and
  `src/miman_imgui.cpp:22192-22242`.
- No approved owner has changed those creation sites to use
  `miman_ftp_create_worker_arg()` before `pthread_create()`.

## File ownership violations

- The previous Agent 04 ownership violations were corrected or reassigned:
  `src/miman_ftp.h` has no current diff, and the public Ryu/status declarations
  are in Agent 04-owned `src/miman_ftprdp_integration.h`.
- The top-level `CMakeLists.txt` product link diff is now covered by Agent 02's
  retry report and validation.
- No preserved FTP_Ryu source or legacy `lib/gscsp/lib/libftp_client/src/client.c`
  modification was observed.

## Checkpoint review

### `miman_ftprdp.cpp` call path

Evidence passes for explicit old/Ryu paths:

- Ryu transfer branch in `src/miman_ftprdp.cpp:304-312` calls
  `ryu_ftp_upload()` / `ryu_ftp_download()` only.
- Old branch in `src/miman_ftprdp.cpp:317-324` calls `gs_ftp_upload()` /
  `gs_ftp_download()`.
- Ryu worker entry points exist at `src/miman_ftprdp.cpp:409-416`.
- `miman_ftprdp.cpp.o` has undefined references to `ryu_ftp_upload`,
  `ryu_ftp_download`, and old `gs_ftp_*`, with no raw `ftpnew_*` transfer
  reference from the integration object.

Checkpoint result: sufficient for call-path separation.

### Thread argument lifetime

Evidence is improved but still blocks runtime Ryu use:

- Agent 04 added `miman_ftp_create_worker_arg()` and
  `miman_ftp_destroy_worker_arg()` in the owned wrapper boundary.
- The worker copies local/remote paths into `miman_ftp_worker_copy_t` before the
  wait loop and transfer call.
- Existing GUI/autopilot creation sites still pass shared `State.ftplistup`
  storage directly to `pthread_create()`.
- Agent 04 correctly documents this as a formal blocker because a thread entry
  cannot eliminate a race that occurs before the worker starts.

Checkpoint result: blocked for Phase 5 runtime Ryu entry until the creation-site
owner adopts worker-owned arguments before `pthread_create()`.

### GUI pointer safety

Evidence is partial:

- The safe helper path deep-copies local and remote path buffers before transfer.
- The current GUI/autopilot path has not adopted that helper and still passes
  mutable shared GUI state to pthread creation.

Checkpoint result: blocked for runtime use; no additional Agent 04 retry is
useful unless Orchestrator grants or routes GUI/autopilot ownership.

### Hidden mixture

Evidence passes for the Phase 4 test/rollback model:

- No raw `ftpnew_upload` / `ftpnew_download` call appears in Agent 04 integration
  files.
- Ryu archive defines expected `ryu_ftp_*` and `ftpnew_*` symbols and no
  `gs_ftp_*`, `ftp_done`, or `ftpavailable`.
- Link map shows old FTP objects selected from the existing product
  `libmimancsp.a` for rollback call sites, while Ryu transfer objects are
  selected from `libftp_client_ryu.a`.
- Agent 02 validated the duplicate `libmimancsp.a` scan as redundant but not
  link-conflicting in the captured build.

Checkpoint result: no hidden old/new transfer mixture found. Explicit old and
Ryu symbol coexistence remains acceptable only for this test/rollback phase.

### Transition/rollback

Evidence is sufficient with a blocker note:

- Old product workers remain as rollback/default entry points.
- Ryu smoke worker entry points are present but are not wired into GUI/autopilot
  production creation sites.
- Integration report includes rollback and final-switch gates.

Checkpoint result: rollback boundary is clear; final/runtime transition is
blocked on GUI/autopilot argument ownership and later protocol/runtime gates.

## Technical risks

- Runtime Ryu testing is blocked until thread creation uses worker-owned
  arguments before `pthread_create()`.
- Existing old GUI/autopilot call sites still carry the legacy shared-state
  pointer race; this review does not authorize Phase 5 to exercise Ryu through
  those call sites.
- `ftp_transfer_callback()` now guards `info == NULL`, but callback context is
  still stack-owned and depends on synchronous old/Ryu transfer APIs. Runtime
  testing must flag any async callback retention.
- Ryu path intentionally does not call old `ftp_avail()` / `ftp_abort()`; Runtime
  Agent must later observe cleanup and connection state when runtime is unblocked.
- Actual OBC runtime smoke, protocol compatibility, parser validation, and RDP
  close behavior remain unresolved.

## Decision

`PHASE_BLOCKED`

## Required retry items

No same-scope Agent 04 or Agent 02 retry is requested from this review. Their
retry outputs resolved the prior ownership and static link evidence issues.

The blocking item is outside the reviewed Worker scope:

1. Orchestrator must route ownership for `src/miman_autopilot.cpp` and
   `src/miman_imgui.cpp` pthread creation sites, or explicitly define a separate
   runtime harness that calls `ftp_ryu_uplink_onorbit()` /
   `ftp_ryu_downlink_onorbit()` with `miman_ftp_create_worker_arg()` before
   Phase 5 runtime work starts.
2. The owner of those call sites must allocate/copy paths before
   `pthread_create()`, pass the owned argument to the Ryu worker, and destroy it
   on `pthread_create()` failure.
3. After that ownership work, Phase 4 needs a targeted re-review of the
   creation-site diff and argument lifetime evidence before Phase 5 unlocks.

## Handoff to Orchestrator

Keep Phase 5 blocked. Do not dispatch Agent 06 or Agent 07 against the Ryu path
until the pre-`pthread_create()` argument ownership boundary is implemented or an
approved safe runtime harness is provided. Route the GUI/autopilot call-site
ownership explicitly; Agent 08 must not patch those files.

## Handoff to Supervisor if needed

Carry forward: no real OBC runtime smoke, unresolved OBC wire incompatibility,
Ryu cleanup without old `ftp_abort()` pending observation, explicit old/Ryu
symbol coexistence acceptable only as a test/rollback state, and the stack-owned
callback context assumption pending runtime validation.
