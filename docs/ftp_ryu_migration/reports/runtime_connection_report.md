# Runtime/Connection Diagnostic Report

- Agent: Runtime/Connection Diagnostic Agent
- Run ID: `20260616_154913`
- Phase: Phase 5 execution/update
- Status: diagnostic patch applied behind opt-in CMake flag; runtime transfer execution not performed

## Scope

This pass instrumented only the Ryu FTP/CSP lifecycle boundary. It did not
change protocol behavior, packet layout, transfer algorithms, RDP timeout
values, or the legacy FTP `client.c` implementation.

The diagnostic code is disabled by default. Production/default builds compile
the Ryu client to direct `csp_connect()` and `csp_close()` calls with no table
sampling and no post-transfer sleeps.

## Applied Diagnostic Patch

| File | Purpose | Production behavior |
|---|---|---|
| `lib/libftp_client_ryu/CMakeLists.txt` | Adds `FTP_RYU_CONN_DIAG` option, default `OFF` | No change unless explicitly enabled |
| `lib/libftp_client_ryu/adapter/include/ryu_ftp_connection_diag.h` | Public no-op/diagnostic API for transfer ID and table dumps | Inline no-ops when disabled |
| `lib/libftp_client_ryu/adapter/src/ryu_ftp_connection_diag.c` | Logs connect/close/table rows with CSP/RDP state | Not compiled when disabled |
| `lib/libftp_client_ryu/port/client/src/ftpnew_client.c` | Wraps Ryu `csp_connect` and `csp_close` call sites | Macros expand to direct CSP calls when disabled |
| `src/miman_ftprdp.cpp` | Supplies transfer ID and captures baseline/+0/+5/+10/+30 dumps in diagnostic builds | No-op calls only when disabled |

Diagnostic diff artifact:
`docs/ftp_ryu_migration/reports/20260616_154913_agent06_diagnostic.diff`

Important caveat: `lib/libftp_client_ryu/` is still an untracked migration
subtree in this worktree, so a repository-level `git diff` cannot represent
those files as normal tracked hunks. The diff artifact is therefore a local
diagnostic artifact, not a clean production patch ready for commit.

Rollback surface for this agent's patch:

```sh
rm -f lib/libftp_client_ryu/adapter/include/ryu_ftp_connection_diag.h
rm -f lib/libftp_client_ryu/adapter/src/ryu_ftp_connection_diag.c
```

Then remove the `FTP_RYU_CONN_DIAG` option/source additions from
`lib/libftp_client_ryu/CMakeLists.txt`, the `ryu_ftp_connection_diag.h`
include and diagnostic calls from `src/miman_ftprdp.cpp`, and the
`FTPNEW_CSP_CONNECT`/`FTPNEW_CSP_CLOSE` macros from
`lib/libftp_client_ryu/port/client/src/ftpnew_client.c`.

## What Is Logged

With `-DFTP_RYU_CONN_DIAG=ON`, each Ryu upload/download emits lines prefixed
with `[FTP_RYU_CONN_DIAG]`.

Connect/close events include:

| Field | Meaning |
|---|---|
| `t` | `csp_get_ms()` timestamp |
| `transfer` | Miman transfer ID from `src/miman_ftprdp.cpp` |
| `engine`, `direction` | `RYU` and `UPLOAD`/`DOWNLOAD` |
| `event` | `csp_connect_call`, `csp_connect_return`, `csp_close_call`, `csp_close_return` |
| `site` | Ryu call-site label such as `ftpnew_upload`, `ftpnew_download`, `ftpnew_done` |
| `conn`, `idx` | connection pointer and index in `csp_conn_get_array()` |
| `ret` | wrapper return value, with `-1` for NULL connect |
| `state`, `rdp` | outer CSP connection state and RDP state |
| `src`, `dst`, `dport`, `sport`, `flags` | CSP identifiers/flags |
| `age_ms`, `close_wait_ms` | age since `conn->timestamp`, with close-wait age only for `RDP_CLOSE_WAIT` |

Table dumps include the same per-slot fields and an aggregate row:
`csp_conn_max=<N> open=<count>`.

## Required Runtime Dumps

The diagnostic wrapper now emits these phases for every Ryu transfer:

| Phase | Trigger |
|---|---|
| `baseline` | immediately before Ryu upload/download call |
| `post-connect` | immediately after wrapped `csp_connect()` returns |
| `post-close` | immediately after wrapped `csp_close()` returns |
| `complete+0s` | immediately after transfer function returns |
| `complete+5s` | diagnostic build only, after 5 seconds |
| `complete+10s` | diagnostic build only, after 10 seconds |
| `complete+30s` | diagnostic build only, after 30 seconds |

The +5/+10/+30 sampling intentionally delays only diagnostic-enabled workers.
Default builds do not sleep.

## Static Lifecycle Finding

`CSP_CONN_MAX` is 10 in the current gscsp build:

| Source | Value |
|---|---:|
| `lib/gscsp/build/lib/libgscsp/lib/libcsp/include/csp/csp_autoconfig.h` | `#define CSP_CONN_MAX 10` |
| `lib/gscsp/build/config.log` | `-DCSP_CONN_MAX=10` |

The runtime RDP options are set in `src/miman_csp.cpp`:

```text
csp_rdp_set_opt(6, 30000, 16000, 1, 8000, 3)
```

Therefore `RDP_CLOSE_WAIT` slot release can be delayed up to the configured
30,000 ms connection timeout unless a subsequent close/remote event releases
the slot earlier.

## Capacity Table

Runtime execution was not possible in this environment, so measured baseline,
peak, and final counts remain unresolved. The table below is the current
evidence state.

| Case | `CSP_CONN_MAX` | Baseline | Peak | Final | Pool exhaustion sequence |
|---|---:|---:|---:|---:|---|
| Static model | 10 | unknown | up to 10 inferred | unknown | each RDP active close may retain one `CONN_OPEN/RDP_CLOSE_WAIT` slot until timeout |
| Local runtime | 10 | not measured | not measured | not measured | no CSP endpoint/integration harness available |

Safe diagnostic repeat interval remains 35 seconds: 30 seconds configured RDP
close-wait timeout plus scheduling margin. This interval is a measurement
guard, not a production fix.

## Runtime Test Plan

Run with:

```sh
cmake -S . -B /tmp/gs5_agent06_diag_build -DFTP_RYU_CONN_DIAG=ON
cmake --build /tmp/gs5_agent06_diag_build -j2
/tmp/gs5_agent06_diag_build/BEE-1000 2> docs/ftp_ryu_migration/logs/20260616_154913_runtime_diag.stderr
```

Then exercise:

| Case | Direction | Outcome | Repetitions | Start interval |
|---|---|---|---:|---:|
| N-UP | upload | normal | 12 | 35 s |
| N-DL | download | normal | 12 | 35 s |
| T-UP | upload | timeout | 3 | 35 s |
| T-DL | download | timeout | 3 | 35 s |
| R-UP | upload | remote error | 3 | 35 s |
| R-DL | download | remote error | 3 | 35 s |
| A-UP | upload | abort | 3 | 35 s |
| A-DL | download | abort | 3 | 35 s |
| B-UP | upload | normal burst | 12 | 1 s until first exhaustion |
| B-DL | download | normal burst | 12 | 1 s until first exhaustion |

Stop immediately on `csp_connect_return conn=NULL`, `No more free
connections`, process crash, or an open slot still present after 35 seconds.

## Build Status

| Command | Result |
|---|---|
| `cmake --build build -j2` | failed before compilation; stale `build/CMakeCache.txt` points at `/home/hyvrid/Desktop/0609/BEE_GS` |
| `cmake -S . -B /tmp/gs5_agent06_build && cmake --build /tmp/gs5_agent06_build -j2` | success; default diagnostics-off build linked `BEE-1000` |
| `cmake -S . -B /tmp/gs5_agent06_diag_build -DFTP_RYU_CONN_DIAG=ON && cmake --build /tmp/gs5_agent06_diag_build -j2` | success; diagnostic-enabled build linked `BEE-1000` |

Both successful builds emitted many pre-existing warnings in UI/config/comms
code. No diagnostic compile/link failure was observed.

## Runtime Status

No live transfer was executed. Blockers:

- no CSP hardware endpoint or simulator was exposed in this environment
- no safe headless integration runner was identified
- running the GUI ground-station binary without an endpoint would not exercise
  upload/download lifecycle paths

Timestamped runtime table dumps are therefore not available from this run.
The diagnostic patch now provides the required dump mechanism for the next
hardware/simulator run.

## Runtime-Limited Status

Retry check timestamp: `2026-06-16 19:29:15 KST (+0900)`.

Live Ryu upload/download execution is not possible in this Codex environment.
The workspace contains buildable GUI binaries, but no reachable real OBC,
confirmed CSP network, simulator, or headless command path that can start a Ryu
FTP upload/download and return CSP/RDP connection-table evidence.

Evidence from the retry readiness probe:

| Check | Result | Evidence |
|---|---|---|
| Diagnostic hooks present | pass | `ryu_ftp_diag_set_transfer()`, `ryu_ftp_diag_dump()`, wrapped `FTPNEW_CSP_CONNECT`, wrapped `FTPNEW_CSP_CLOSE` are present |
| Diagnostic build | pass | `cmake -S . -B /tmp/gs5_agent06_runtime_limited_diag_build -DFTP_RYU_CONN_DIAG=ON` and `cmake --build /tmp/gs5_agent06_runtime_limited_diag_build -j2` linked `BEE-1000` |
| Local runtime probe | blocked | `timeout 5s /tmp/gs5_agent06_runtime_limited_diag_build/BEE-1000` exited `255` with `Glfw Error 65544: X11: Failed to open display :0` |
| Live transfer evidence | unavailable | no OBC/simulator transfer was run; no baseline/0/5/10/30 table dump was produced |

Readiness log:
`docs/ftp_ryu_migration/logs/20260616_192915_agent06_runtime_limited_readiness.log`

Runtime-limited table-dump placeholder:
`docs/ftp_ryu_migration/logs/20260616_192915_agent06_runtime_limited_table_dumps.md`

This retry does not claim OBC smoke, live upload/download success, connection
slot recovery, pool exhaustion reproduction, or `RDP_CLOSE_WAIT` residency
observation.

## Manual Live Test Procedure

Build the diagnostic binary on the real GS/OBC setup:

```sh
cmake -S . -B /tmp/gs5_agent06_live_diag_build -DFTP_RYU_CONN_DIAG=ON
cmake --build /tmp/gs5_agent06_live_diag_build -j2
/tmp/gs5_agent06_live_diag_build/BEE-1000 \
  2> docs/ftp_ryu_migration/logs/manual_ryu_runtime_diag.stderr
```

Use the GUI FTP panel and select `Ryu` in the FTP version radio group, then use
the existing `Upload` and `Download` buttons. The Ryu GUI call path is:

| Direction | GUI path evidence | Worker path |
|---|---|---|
| Upload | `src/miman_imgui.cpp` Ryu radio `State.ftp_version == 3`, `Upload` button | `start_ryu_ftp_thread(ftp_ryu_uplink_onorbit)` -> `ryu_ftp_upload()` |
| Download | `src/miman_imgui.cpp` Ryu radio `State.ftp_version == 3`, `Download` button | `start_ryu_ftp_thread(ftp_ryu_downlink_onorbit)` -> `ryu_ftp_download()` |
| Autopilot upload | `State.ftp_version == 3` branch | `start_ryu_ftp_thread(ftp_ryu_uplink_onorbit)` |
| Autopilot download | `State.ftp_version == 3` branch | `start_ryu_ftp_thread(ftp_ryu_downlink_onorbit)` |

Required capture schedule for every transfer:

| Snapshot | Required marker |
|---|---|
| Baseline before transfer | `[FTP_RYU_CONN_DIAG] ... dump=baseline csp_conn_max=... open=...` |
| Connect call/return | `event=csp_connect_call`, then `event=csp_connect_return conn=... idx=...` |
| Post-connect table | `dump=post-connect` |
| Close call/return | `event=csp_close_call`, then `event=csp_close_return ret=...` |
| Post-close table | `dump=post-close` |
| Immediate completion | `dump=complete+0s` |
| +5 seconds | `dump=complete+5s` |
| +10 seconds | `dump=complete+10s` |
| +30 seconds | `dump=complete+30s` |

Required fields to preserve in the log:

| Field group | Required evidence |
|---|---|
| Transfer correlation | `[FTP_RYU_TEST][id=...]` and `transfer=<id>` in `[FTP_RYU_CONN_DIAG]` lines |
| Connect evidence | `csp_connect_call`, `csp_connect_return`, `conn`, `idx`, `ret` |
| Close evidence | `csp_close_call`, `csp_close_return`, `ret` |
| Endpoint evidence | `src`, `dst`, `dport`, `sport`, `flags` |
| RDP evidence | `state`, `rdp`, `close_wait_ms`, especially `rdp=RDP_CLOSE_WAIT` |
| Capacity evidence | `csp_conn_max`, active `open`, baseline count, peak count, final count |
| Exhaustion evidence | final successful connect sequence before `conn=NULL` or `No more free connections` |

Manual test matrix:

| Case | Direction | Outcome to force | Repetitions | Start interval | Pass criteria |
|---|---|---|---:|---:|---|
| N-UP | upload | normal success | 12 | 35 s | every connect has matching close and final open returns to baseline by +30 s |
| N-DL | download | normal success | 12 | 35 s | every connect has matching close and final open returns to baseline by +30 s |
| T-UP | upload | timeout | 3 | 35 s | close path logged; retained slot clears by +30 s or is reported |
| T-DL | download | timeout | 3 | 35 s | close path logged; retained slot clears by +30 s or is reported |
| R-UP | upload | remote error | 3 | 35 s | close path logged; no unrecovered slot after +30 s |
| R-DL | download | remote error | 3 | 35 s | close path logged; no unrecovered slot after +30 s |
| A-UP | upload | abort | 3 | 35 s | abort cleanup logs close/return or explicit missing-close path |
| A-DL | download | abort | 3 | 35 s | abort cleanup logs close/return or explicit missing-close path |
| B-UP | upload | burst normal | 12 | 1 s until exhaustion | capture sequence immediately before `conn=NULL`/pool exhaustion |
| B-DL | download | burst normal | 12 | 1 s until exhaustion | capture sequence immediately before `conn=NULL`/pool exhaustion |

Manual pass/fail criteria:

| Criterion | Pass | Fail |
|---|---|---|
| Connect/close pairing | every transfer ID has `csp_connect_return conn!=NULL` and matching `csp_close_return` for that pointer/index | missing close, close returns error, or close uses a different pointer without explanation |
| Slot recovery | final `open` count equals baseline by `complete+30s` | any extra open slot persists beyond +30 s |
| Delayed close verdict | `RDP_CLOSE_WAIT` appears after close but clears by +30 s with count recovery | `RDP_CLOSE_WAIT` persists past +30 s or prevents new connect |
| Leak verdict | no persistent slot after all outcomes | persistent `CONN_OPEN` slot with no timeout recovery |
| Pool exhaustion sequence | reproduced only under burst; sequence shows whether close-wait slots consume all 10 slots | normal 35 s interval still reaches `No more free connections` |

## Open Runtime Risks

| Risk | Status | Required disposition |
|---|---|---|
| Live OBC/Ryu transfer evidence | open | Run the manual procedure on real GS/OBC or simulator |
| RDP delayed close vs leak verdict | open | Compare baseline, peak, final count and `RDP_CLOSE_WAIT` residency |
| Safe repeat interval | provisional 35 s | Confirm slot recovery by +30 s before using shorter intervals |
| `CSP_CONN_MAX` sizing | keep at 10 for diagnosis | Do not increase until leak/delayed-close sequence is measured |
| Cleanup without old `ftp_abort()` | open | Verify normal/error/abort cases produce close-return evidence |
| Pool exhaustion sequence | open | Capture burst upload/download failure sequence if reproduced |

## Agent 08 Handoff

- Verdict: delayed close remains the leading explanation. The new diagnostic
  wrapper can prove whether each transfer's close maps to slot recovery or a
  retained `RDP_CLOSE_WAIT` slot.
- Safe repeat interval: 35 seconds between transfer starts for non-burst
  diagnostic cases.
- `CSP_CONN_MAX`: keep at 10 for measurement; increasing it now would mask the
  lifecycle sequence.
- Unresolved path: live normal/timeout/remote-error/abort runs with
  `FTP_RYU_CONN_DIAG=ON`.
- Expected exhaustion signature: open count approaches 10; each recent transfer
  shows `csp_close_return` while its slot remains `CONN_OPEN/RDP_CLOSE_WAIT`;
  next `csp_connect_return` is NULL and libcsp logs `No more free connections`.

## Supervisor NO_GO Remediation Retry

Retry timestamp: `2026-06-16 20:04:49 KST (+0900)`.

This Agent 06 retry addressed only the runtime/connection diagnostic scope from
the Supervisor `NO_GO`. It did not modify protocol behavior, packet layout,
transfer algorithms, CSP/RDP core, legacy FTP `client.c`, or production close
semantics.

| Item | Result | Evidence |
|---|---|---|
| Diagnostic source presence | pass | `FTP_RYU_CONN_DIAG`, `ryu_ftp_diag_set_transfer()`, `ryu_ftp_diag_dump()`, `FTPNEW_CSP_CONNECT`, and `FTPNEW_CSP_CLOSE` are present in the Ryu path |
| Connection table API | pass | diagnostic implementation uses `csp_conn_get_array(&count)` and logs `csp_conn_max`, slot index, state, RDP state, endpoints, flags, age, and `close_wait_ms` |
| Current diagnostic build | pass | `cmake -S . -B /tmp/gs5_agent06_retry_diag_build -DFTP_RYU_CONN_DIAG=ON && cmake --build /tmp/gs5_agent06_retry_diag_build -j2` linked `/tmp/gs5_agent06_retry_diag_build/BEE-1000` |
| Live transfer execution | blocked | no reachable real OBC, simulator, confirmed CSP endpoint, or headless transfer runner is available in this Codex environment |
| Baseline/+0/+5/+10/+30 live dumps | unavailable | dump hooks exist, but no Ryu upload/download ran |
| Pool exhaustion verdict | unresolved | no live burst upload/download sequence could be produced |

Timestamped retry log/table placeholder:
`docs/ftp_ryu_migration/logs/20260616_200449_agent06_runtime_connection_retry.md`.

Current Agent 06 verdict for Agent 08:

| Question | Answer |
|---|---|
| Leak vs delayed close | unresolved by live evidence; static evidence still points to possible 30 s RDP delayed close |
| Safe repeat interval | keep 35 s for non-burst diagnostics until live +30 s slot recovery is shown |
| `CSP_CONN_MAX` | observed configured value remains 10; do not increase as a diagnostic substitute |
| Required next path | run diagnostic build on a real GS/OBC setup or simulator and collect normal/timeout/remote-error/abort plus burst upload/download logs |

This retry therefore remains `RETRY_REQUIRED`, not `DONE`, because the
Supervisor-requested runtime proof depends on external runtime infrastructure
that is not exposed in this workspace.

## Supervisor NO_GO Remediation Retry 2

Retry timestamp: `2026-06-16 20:09:48 KST (+0900)`.

This pass preserved the prior diagnostic-only implementation and performed a
fresh verification build without changing source behavior.

| Item | Result | Evidence |
|---|---|---|
| Diagnostic source presence | pass | `FTP_RYU_CONN_DIAG`, `ryu_ftp_diag_set_transfer()`, `ryu_ftp_diag_dump()`, `FTPNEW_CSP_CONNECT`, and `FTPNEW_CSP_CLOSE` remain present in the Ryu path |
| Current diagnostic build | pass | `cmake -S . -B /tmp/gs5_agent06_current_diag_build -DFTP_RYU_CONN_DIAG=ON` succeeded |
| Current diagnostic link | pass | `cmake --build /tmp/gs5_agent06_current_diag_build -j2` linked `/tmp/gs5_agent06_current_diag_build/BEE-1000` with pre-existing warnings |
| Live transfer execution | blocked | no reachable real OBC, simulator, confirmed CSP endpoint, or headless transfer runner is available in this Codex environment |
| Baseline/+0/+5/+10/+30 live dumps | unavailable | diagnostic hooks exist, but no Ryu upload/download ran |
| Leak/delayed-close verdict | unresolved | no live connection table sequence was produced |

Timestamped retry log/table placeholder:
`docs/ftp_ryu_migration/logs/20260616_200948_agent06_runtime_connection_retry.md`.

Agent 08 handoff remains unchanged: keep `CSP_CONN_MAX=10`, use a 35 s safe
repeat interval for non-burst diagnostic runs, and collect live normal,
timeout, remote-error, abort, and burst upload/download logs before claiming
slot recovery, delayed-close residency, or a leak verdict.
