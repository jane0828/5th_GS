# Phase 4 Targeted Re-review After Agent 04b

Run timestamp: `2026-06-16 15:00:49 KST (+0900)`

## Scope

This targeted re-review covers only the previous Phase 4 blocker: GUI/autopilot
Ryu FTP pthread argument ownership after Agent 04b.

Reviewed files:

- `src/miman_imgui.cpp`
- `src/miman_autopilot.cpp`
- `src/miman_ftprdp.cpp`
- `src/miman_ftprdp_integration.h`
- `docs/ftp_ryu_migration/reports/thread_arg_ownership_report.md`
- `docs/ftp_ryu_migration/reports/phase_4_review.md`
- `docs/ftp_ryu_migration/reports/ftprdp_integration_report.md`
- `docs/ftp_ryu_migration/state/decision_log.md`

No source, header, CMake, build, FTP_Ryu preserved source, legacy `client.c`, or
protocol core file was modified by this review.

## Agent 04b Diff Review

Agent 04b changed the GUI/autopilot FTP creation sites by adding a local
`start_ryu_ftp_thread()` helper to both:

- `src/miman_imgui.cpp`
- `src/miman_autopilot.cpp`

The helper allocates a `miman_ftp_worker_arg_t` with
`miman_ftp_create_worker_arg(State.ftplistup[NowFTP].local_path,
State.ftplistup[NowFTP].remote_path)` before calling `pthread_create()`.
`pthread_create()` receives the owned `arg` pointer. If thread creation fails,
the helper calls `miman_ftp_destroy_worker_arg(arg)` before returning.

## Creation-site Findings

`src/miman_imgui.cpp`:

- Upload path keeps `State.ftp_version == 1` on `ftp_uplink_force`.
- Ryu upload path is explicitly `else if(State.ftp_version == 3)` and calls
  `start_ryu_ftp_thread(ftp_ryu_uplink_onorbit)`.
- V2/default upload path still calls `ftp_uplink_onorbit` with
  `&State.ftplistup[NowFTP]`.
- Download path keeps `State.ftp_version == 1` on `ftp_downlink_force`.
- Ryu download path is explicitly `else if(State.ftp_version == 3)` and calls
  `start_ryu_ftp_thread(ftp_ryu_downlink_onorbit)`.
- V2/default download path still calls `ftp_downlink_onorbit` with
  `&State.ftplistup[NowFTP]`.

`src/miman_autopilot.cpp`:

- Download path keeps `State.ftp_version == 1` on `ftp_downlink_force`.
- Ryu download path is explicitly `else if(State.ftp_version == 3)` and calls
  `start_ryu_ftp_thread(ftp_ryu_downlink_onorbit)`.
- V2/default download path still calls `ftp_downlink_onorbit` with
  `&State.ftplistup[NowFTP]`.
- Upload path keeps `State.ftp_version == 1` on `ftp_uplink_force`.
- Ryu upload path is explicitly `else if(State.ftp_version == 3)` and calls
  `start_ryu_ftp_thread(ftp_ryu_uplink_onorbit)`.
- V2/default upload path still calls `ftp_uplink_onorbit` with
  `&State.ftplistup[NowFTP]`.

Targeted grep found no direct `pthread_create()` call passing
`&State.ftplistup[NowFTP]` to `ftp_ryu_uplink_onorbit` or
`ftp_ryu_downlink_onorbit`. The only Ryu worker dispatches in the reviewed
GUI/autopilot FTP transfer sites go through `start_ryu_ftp_thread()`.

## Worker-side Ownership

`src/miman_ftprdp_integration.h` declares:

- `miman_ftp_create_worker_arg()`
- `miman_ftp_destroy_worker_arg()`
- `ftp_ryu_uplink_onorbit()`
- `ftp_ryu_downlink_onorbit()`

`src/miman_ftprdp.cpp` defines the worker argument with a magic marker, copies
local/remote paths into the worker-owned job, and destroys the owned argument
after the worker-side copy. The legacy shared-`ftpinfo` fallback remains in the
worker for old or non-migrated callers, but the reviewed Ryu GUI/autopilot paths
no longer use that fallback.

## Rollback Path

Old FTP rollback behavior is preserved:

- `ftp_version == 1` still dispatches `ftp_uplink_force` /
  `ftp_downlink_force`.
- The V2/default path still dispatches `ftp_uplink_onorbit` /
  `ftp_downlink_onorbit`.
- Shared `&State.ftplistup[NowFTP]` remains only in those old paths, outside the
  explicit Ryu branch.

This is acceptable for the targeted Ryu blocker review. It does not certify the
old path as race-free; it confirms rollback behavior was not removed.

## Build Evidence

Agent 04b reported:

```sh
cmake -S . -B /tmp/5th_gs_agent04b_build \
  -DALLOW_OLD_FTP_BUILD_EXCLUSION=1 \
  -DALLOW_FINAL_REPLACEMENT=1
cmake --build /tmp/5th_gs_agent04b_build --target BEE-1000 -j2
```

Result in `thread_arg_ownership_report.md` and
`logs/20260616_143854_agent_04b_thread_arg_ownership.out`:
`Built target BEE-1000`.

This re-review did not rerun the build. Per the prompt, Agent 04b build evidence
was inspected and is sufficient for this targeted gate.

## Remaining Runtime Notes

No real OBC runtime smoke was executed by Agent 04b or by this re-review. Phase
5 should therefore validate, at runtime:

- `[FTP_RYU_TEST][id=...]` begin/progress/CRC/final/cleanup correlation.
- `miman_ftp_get_last_transfer_status()` after cleanup.
- Ryu cleanup behavior without old `ftp_abort()`.
- Callback context lifetime assumption under actual transfer behavior.
- Existing OBC protocol/parser/RDP risks carried forward from earlier phases.

## Decision

The specific Phase 4 blocking condition is resolved. GUI/autopilot Ryu runtime
paths are explicitly separated by `State.ftp_version == 3`, allocate/copy worker
arguments before `pthread_create()`, avoid directly passing shared
`State.ftplistup[NowFTP]` or shared `ftpinfo *` to Ryu workers, and clean up the
owned argument if `pthread_create()` fails. Old V1/V2 rollback paths remain.

Phase 5 runtime/diagnostic work may proceed with the runtime notes above. This
is not final deployment approval.

PHASE_DECISION: PHASE_GO_WITH_NOTES
PHASE_SUMMARY: Agent 04b resolved the Ryu GUI/autopilot pthread argument ownership blocker; Phase 5 may proceed for runtime diagnostics with OBC smoke and cleanup risks still open.
