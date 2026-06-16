# Agent 04b Thread Argument Ownership Report

Run timestamp: `2026-06-16 14:43:08 KST (+0900)`

## Scope

Agent 04b updated only the GUI/autopilot FTP thread creation boundary:

- `src/miman_imgui.cpp`
- `src/miman_autopilot.cpp`

No FTP_Ryu preserved source, legacy `lib/gscsp/lib/libftp_client/src/client.c`,
protocol core, old FTP source, or CMake/build file was modified.

## Implementation

The Ryu runtime path is now explicit as `State.ftp_version == 3`.

- GUI exposes a separate `Ryu` radio button.
- Autopilot and GUI keep existing `ftp_version == 1` and `ftp_version == 2`
  old FTP branches unchanged.
- Ryu upload/downlink branches call `start_ryu_ftp_thread()`.
- `start_ryu_ftp_thread()` creates a worker-owned argument with
  `miman_ftp_create_worker_arg()` before `pthread_create()`.
- `pthread_create()` receives the owned argument pointer, not
  `&State.ftplistup[NowFTP]`.
- If `pthread_create()` fails, `miman_ftp_destroy_worker_arg()` releases the
  owned argument before returning.
- Existing join policy is preserved: callers still `pthread_join(p_thread[8],
  NULL)` before creating the next FTP thread, and no new detach policy was
  introduced.

Worker-side lifetime remains as provided by Agent 04:

- `ftp_ryu_uplink_onorbit()` / `ftp_ryu_downlink_onorbit()` enter
  `ftp_transfer_onorbit(..., MIMAN_FTP_ENGINE_RYU, ...)`.
- `ftp_copy_worker_param()` copies the owned paths into the worker job and
  destroys the owned argument after the copy.
- Ryu transfer IDs and cleanup observations remain available through
  `[FTP_RYU_TEST][id=...]` logs and `miman_ftp_get_last_transfer_status()`.

## Static Evidence

Ryu creation helpers allocate before thread creation and clean up on create
failure:

```sh
rg -n "start_ryu_ftp_thread|miman_ftp_create_worker_arg|miman_ftp_destroy_worker_arg|ftp_ryu_(up|down)link_onorbit|pthread_create\\(&p_thread\\[8\\].*ftp_|State\\.ftp_version == 3|FTPVersionSelect3|State\\.ftplistup\\[NowFTP\\]" src/miman_imgui.cpp src/miman_autopilot.cpp
```

Key results:

- `src/miman_autopilot.cpp:25` and `src/miman_imgui.cpp:95` define
  `start_ryu_ftp_thread()`.
- `src/miman_autopilot.cpp:28-29` and `src/miman_imgui.cpp:98-99` call
  `miman_ftp_create_worker_arg(State.ftplistup[NowFTP].local_path,
  State.ftplistup[NowFTP].remote_path)` before `pthread_create()`.
- `src/miman_autopilot.cpp:37` and `src/miman_imgui.cpp:107` call
  `miman_ftp_destroy_worker_arg(arg)` in the `pthread_create()` failure path.
- `src/miman_autopilot.cpp:656` calls
  `start_ryu_ftp_thread(ftp_ryu_downlink_onorbit)`.
- `src/miman_autopilot.cpp:684` calls
  `start_ryu_ftp_thread(ftp_ryu_uplink_onorbit)`.
- `src/miman_imgui.cpp:22216` calls
  `start_ryu_ftp_thread(ftp_ryu_uplink_onorbit)`.
- `src/miman_imgui.cpp:22266` calls
  `start_ryu_ftp_thread(ftp_ryu_downlink_onorbit)`.

Old/Ryu separation evidence:

- Old force path remains:
  `pthread_create(..., ftp_downlink_force, &State.ftplistup[NowFTP])` and
  `pthread_create(..., ftp_uplink_force, &State.ftplistup[NowFTP])`.
- Old V2 path remains:
  `pthread_create(..., ftp_downlink_onorbit, &State.ftplistup[NowFTP])` and
  `pthread_create(..., ftp_uplink_onorbit, &State.ftplistup[NowFTP])`.
- Ryu path is only the explicit `State.ftp_version == 3` branch and does not
  pass `&State.ftplistup[NowFTP]` to `pthread_create()`.

Worker wrapper evidence:

```sh
rg -n "ftp_ryu_(up|down)link_onorbit|miman_ftp_create_worker_arg|miman_ftp_destroy_worker_arg|ftp_copy_worker_param|MIMAN_FTP_ENGINE_RYU|miman_ftp_get_last_transfer_status" src/miman_ftprdp.cpp src/miman_ftprdp_integration.h
```

Key results:

- `src/miman_ftprdp_integration.h:19-24` declares the worker-arg helper and
  Ryu worker entry points.
- `src/miman_ftprdp.cpp:129` defines `miman_ftp_create_worker_arg()`.
- `src/miman_ftprdp.cpp:152` defines `miman_ftp_destroy_worker_arg()`.
- `src/miman_ftprdp.cpp:236` destroys an owned worker arg after worker-side
  copy.
- `src/miman_ftprdp.cpp:409-416` defines the explicit Ryu worker wrappers.

## Build

Command:

```sh
cmake -S . -B /tmp/5th_gs_agent04b_build \
  -DALLOW_OLD_FTP_BUILD_EXCLUSION=1 \
  -DALLOW_FINAL_REPLACEMENT=1 &&
cmake --build /tmp/5th_gs_agent04b_build --target BEE-1000 -j2
```

Result: `Built target BEE-1000`.

The build emitted existing project warnings and CMake reported the two
manually-specified variables as unused. No compile or link error was observed.

## Runtime Status

No real OBC runtime smoke was executed by Agent 04b. This report is static and
build validation only.

## Handoff

Phase Leader targeted re-review should inspect:

- `src/miman_imgui.cpp` and `src/miman_autopilot.cpp` Ryu creation-site diff.
- `start_ryu_ftp_thread()` pre-`pthread_create()` allocation and failure
  cleanup.
- Explicit old/Ryu branch separation by `State.ftp_version`.

Runtime Agent should observe:

- `[FTP_RYU_TEST][id=...]` begin/progress/CRC/final/cleanup logs.
- `miman_ftp_get_last_transfer_status()` after cleanup.
- Ryu cleanup behavior without old `ftp_abort()`.

