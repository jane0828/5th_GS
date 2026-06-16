# Agent 04 FTP Ryu Integration Report

Run ID: `20260616_132857`

Retry context: Phase 4 review required Agent 04 to remove the unauthorized
`src/miman_ftp.h` API change, make callback lifetime assumptions explicit, and
resolve or formally block the thread-argument lifetime boundary.

## Scope and ownership

Agent 04 retry-owned changes:

- `src/miman_ftprdp.cpp`
- `src/miman_ftprdp_integration.h`
- `docs/ftp_ryu_migration/reports/ftprdp_integration_report.md`

Ownership cleanup:

- `src/miman_ftp.h` no longer contains Agent 04 Ryu/status declarations.
- Public Ryu smoke/status declarations moved to
  `src/miman_ftprdp_integration.h`, an Agent 04-owned wrapper header.
- This retry did not modify top-level `CMakeLists.txt`. The existing product
  link wiring for `FTP_Ryu::client` remains an Agent 02 Build/CMake ownership
  item.

No FTP_Ryu preserved source, existing `libftp_client/src/client.c`, or protocol
definition core file was modified.

## Selector and call paths

The old and Ryu paths remain explicit and separately named:

- Old downlink: `ftp_downlink_onorbit()` -> `MIMAN_FTP_ENGINE_OLD`
- Old uplink: `ftp_uplink_onorbit()` -> `MIMAN_FTP_ENGINE_OLD`
- Ryu downlink smoke path: `ftp_ryu_downlink_onorbit()` -> `MIMAN_FTP_ENGINE_RYU`
- Ryu uplink smoke path: `ftp_ryu_uplink_onorbit()` -> `MIMAN_FTP_ENGINE_RYU`

Evidence in `src/miman_ftprdp.cpp`:

- Ryu branch calls only `ryu_ftp_upload()` and `ryu_ftp_download()` at lines
  304-312.
- Old branch keeps `gs_ftp_upload()` and `gs_ftp_download()` at lines 317-324.
- Old-only `ftp_avail()` and `ftp_abort()` are gated to
  `MIMAN_FTP_ENGINE_OLD` at lines 332 and 366.
- Ryu smoke worker entry points are at lines 409-416.

Log selector:

- All transfer logs use `[FTP_%s_TEST][id=%u][%s]`, producing
  `[FTP_OLD_TEST]` or `[FTP_RYU_TEST]` with transfer ID and direction.
- Begin, progress, CRC, final return code, and cleanup logs carry the same
  transfer ID.

## Lifetime review

Worker-owned copy:

- `ftp_copy_worker_param()` copies local/remote paths into
  worker-owned `miman_ftp_worker_copy_t::paths` before the wait loop or
  transfer call.
- `gs_ftp_settings_t` is snapshotted into the worker-owned job before transfer.
- `miman_ftp_create_worker_arg()` provides the safe creation-side contract:
  callers allocate/copy paths before `pthread_create()`, pass the returned
  pointer to an old or Ryu worker, and the worker frees it immediately after
  copying into its own job.

Legacy boundary and formal block:

- Current GUI/autopilot creation sites still pass `&State.ftplistup[NowFTP]`
  directly into `pthread_create()` in files outside Agent 04 ownership:
  `src/miman_autopilot.cpp:634-662` and
  `src/miman_imgui.cpp:22192-22242`.
- The worker logs a warning when it receives that legacy shared `ftpinfo`
  pointer, because the pre-start race cannot be eliminated inside a thread
  entry after `pthread_create()` has already received a shared address.
- Phase 5/runtime use of the Ryu path is blocked until the GUI/autopilot owner
  changes those creation sites to:

```cpp
miman_ftp_worker_arg_t * arg =
    miman_ftp_create_worker_arg(State.ftplistup[NowFTP].local_path,
                                State.ftplistup[NowFTP].remote_path);
pthread_create(&p_thread[8], NULL, ftp_ryu_downlink_onorbit, arg);
```

If `pthread_create()` fails, the caller must call
`miman_ftp_destroy_worker_arg(arg)`.

Callback context:

- `ftp_transfer_callback()` now guards `info == NULL` before reading
  `info->user_data` or delegating to `ftp_callback()`.
- `miman_ftp_callback_context_t` is stack-owned by `ftp_run_transfer()` and is
  valid for the synchronous old/Ryu transfer APIs used here.
- No evidence was found that either API retains callbacks after returning. If
  runtime testing proves async callback retention, callback context must become
  heap-owned and freed after an explicit terminal event.

Thread-safety:

- Transfer ID allocation and last-status storage are protected by
  `ftp_transfer_status_lock`.
- Existing `console.AddLog`, `printftp`, and `State.ftp_mode` behavior follows
  the prior worker threading model; this phase does not certify those globals
  as generally thread-safe.

## Return code, transfer ID, and cleanup observability

`miman_ftp_transfer_status_t` in `src/miman_ftprdp_integration.h` records:

- `transfer_id`
- `return_code`
- `cleanup_done`
- `engine`
- `direction`
- `local_path`
- `remote_path`

`miman_ftp_get_last_transfer_status()` exposes the last transfer state for
runtime diagnostics. Status is initialized after argument copy and updated
after cleanup, allowing Runtime Agent to correlate logs and state by transfer
ID.

## Build and smoke evidence

Agent 04 build after retry was attempted with the existing worktree, including
the current top-level `CMakeLists.txt` link state pending Agent 02 ownership.

Configure:

```sh
cmake -S . -B /tmp/5th_gs_agent04_retry_build \
  -DALLOW_OLD_FTP_BUILD_EXCLUSION=1 \
  -DALLOW_FINAL_REPLACEMENT=1
```

Build:

```sh
cmake --build /tmp/5th_gs_agent04_retry_build --target BEE-1000 -j2
```

Result: `Built target BEE-1000`. The build emitted pre-existing warnings in
unrelated source/header areas. No new integration compile or link error was
observed.

Smoke checks to refresh:

```sh
rg -n "ftpnew_upload|ftpnew_download" \
  src/miman_ftprdp.cpp src/miman_ftprdp_integration.h
```

Result: no raw transfer API calls in Agent 04 integration files.

```sh
nm -u /tmp/5th_gs_agent04_retry_build/CMakeFiles/BEE-1000.dir/src/miman_ftprdp.cpp.o |
  rg "ryu_ftp_|ftpnew_|gs_ftp_"
```

Result: `miman_ftprdp.cpp.o` references `ryu_ftp_upload`,
`ryu_ftp_download`, old `gs_ftp_*`, and no raw `ftpnew_upload` /
`ftpnew_download`.

```sh
nm -g --defined-only /tmp/5th_gs_agent04_retry_build/lib/libftp_client_ryu/libftp_client_ryu.a |
  rg "ryu_ftp_|ftpnew_|gs_ftp_|ftp_done|ftpavailable"
```

Result: archive defines `ryu_ftp_upload`, `ryu_ftp_download`, and expected
literal `ftpnew_*` symbols. It defines no old `gs_ftp_*`, `ftp_done`, or
`ftpavailable` symbols.

Runtime smoke against an OBC was not executed in this phase. Final switchover
remains blocked until protocol/runtime gates complete.

## Rollback and final-switch plan

Rollback:

1. Continue calling existing `ftp_uplink_onorbit()` and
   `ftp_downlink_onorbit()` from GUI/autopilot paths.
2. Leave `ftp_ryu_uplink_onorbit()` and `ftp_ryu_downlink_onorbit()` unused.
3. Agent 02 owns any top-level link rollback or validation for
   `FTP_Ryu::client`.

Final switch after gates:

1. Agent 02 owns product link validation and duplicate CSP-link disposition.
2. Agent 05 confirms packet/wire compatibility against the actual flight OBC
   build and capture point.
3. Agent 06 confirms cleanup behavior, return-code handling, and transfer
   ID/log correlation under runtime tests.
4. GUI/thread owner changes creation sites to allocate/copy worker arguments
   before `pthread_create()`.
5. Only after those gates, replace final product transfer call sites in a
   separate diff and keep old path available behind an explicit rollback
   selector.

## Handoff

Agent 05:

- Capture old and Ryu paths separately.
- Ryu transfer packets originate only from `ryu_ftp_upload()` /
  `ryu_ftp_download()` with `use_rdp=true`.
- Use `[FTP_RYU_TEST][id=...]` logs to correlate capture windows.

Agent 06:

- Observe `miman_ftp_get_last_transfer_status()` after transfer cleanup.
- Correlate `transfer_id`, return code, and `[FTP_RYU_TEST] cleanup_done=1`.
- Verify Ryu cleanup does not depend on old `ftp_avail()` / `ftp_abort()`.

Agent 08:

- Confirm `src/miman_ftp.h` has no Agent 04 API diff.
- Treat legacy GUI/autopilot `&State.ftplistup[NowFTP]` creation sites as a
  formal blocker for runtime Ryu use until their owner adopts
  `miman_ftp_create_worker_arg()`.
- Route top-level CMake/link validation to Agent 02.
