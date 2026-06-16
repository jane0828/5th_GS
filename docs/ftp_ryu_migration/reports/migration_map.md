# FTP Ryu Migration Map

## Executive result

Phase 1 analysis is complete without modifying source, headers, CMake, build scripts, old FTP, or Ryu originals.

The migration is not a call-name-only replacement. Upload/download have direct Ryu APIs, but the current integration also depends on six file-management APIs and a global cancellation gate that Ryu does not expose. Ryu additionally assumes a different GS FTP callback enum and chunk macro than this project. A hidden mixture of old and Ryu implementations would therefore be easy to create and must be rejected by source and link symbol gates.

## State, ownership, and cleanup comparison

| Concern | old 5th_GS client | Ryu client | Migration effect |
|---|---|---|---|
| transfer state | global `gs_ftp_state_t state` (`client.c:28-56`) | stack-local `ftpnew_state_t state` in each transfer (`ftpnew_client.c:72-88,591-614,722-728`) | Ryu removes cross-transfer global state and permits independent state objects, subject to CSP/thread safety. |
| cancellation | global `ftpavailable` defined in `force.c:26`; toggled by public `ftp_avail/ftp_abort`; polled in old data loops | no public cancel token/API and no `ftpavailable` | Agent 04 must define cancellation semantics; deleting calls is a behavior change. |
| cleanup entry | externally visible `ftp_done(remove_map)` operating on global state | private `ftpnew_done(&state, remove_map)` | Integration must not call cleanup after `ftpnew_*`; each API owns cleanup. |
| error cleanup | many branches call global `ftp_done`; connection-open failure in old upload returns without closing already-open file (`client.c:216-263`) | transfer branches normally call one private cleanup helper; upload connect failure closes file via helper (`ftpnew_client.c:638-645`) | Ryu is more self-contained. |
| download file | opens/writes final destination directly (`client.c:403-457`) | refuses existing final file, writes `.tmp`, resumes with `.map`, renames on success (`ftpnew_client.c:780-849`) | User-visible overwrite/resume behavior changes and must be accepted/tested. |
| map success cleanup | removes `.map` only (`client.c:1120-1128`) | removes `.map` and renames `.tmp` (`ftpnew_client.c:539-549`) | Ryu cleanup is transactional at file-name level. |
| connection close | sends `FTP_DONE`, sleeps, closes, zeros global state (`client.c:1130-1145`) | sends `FTP_DONE`, closes, nulls local connection (`ftpnew_client.c:551-558`) | No external post-call `ftp_abort`/`ftp_done` should remain. |
| CSP RX packet ownership | old download frees received packets after/error branches (`client.c:860-991`) | Ryu frees each `csp_read` packet on success and handled errors (`ftpnew_client.c:391-473`) | Ownership rule is caller-free after `csp_read`; do not retain `ftp_packet_t *` after free. |
| upload packet ownership | old modified client allocates with `csp_buffer_get` and passes to streaming send (`client.c:1082-1095`) | Ryu uses stack packet with `csp_transaction_persistent` (`ftpnew_client.c:252-312`) | Do not mix old streaming packet path into Ryu. |

Ryu cleanup caveat for Agent 03: `ftpnew_data` closes `state->conn` on send failure at `ftpnew_client.c:297-303` but still returns `GS_OK` at `:312`; the outer flow may then continue using a closed/non-null pointer. This is a **confirmed source defect candidate**, not fixed by Agent 01.

## Implementation phases

### P0 blockers

1. **Header vocabulary mismatch, confirmed.** Resolve `GS_FTP_MAX_CHUNK_SIZE` versus `GS_FTP_CHUNK_SIZE`, and generic versus direction-specific callback enum names, inside an isolated Ryu port/compatibility boundary.
2. **Missing Ryu operations, confirmed.** Decide the final product scope for list/move/remove/copy/mkdir/rmdir. Keeping old operations while switching only upload/download is a prohibited hidden mixture unless an explicit, reviewed dual-client architecture is approved.
3. **Cancellation mismatch, confirmed.** Replace `ftp_avail/ftp_abort` behavior with an explicit Ryu cancellation design or document that transfers are non-cancellable. Do not leave calls that control only old global state.
4. **Protocol compatibility, unverified.** Ryu wire return codes differ numerically from `gs_ftp_return_t` (`ftpnew_types.h:33-44` versus `gs/ftp/types.h:49-70`). Agent 05 must confirm the deployed server uses Ryu return values before old FTP removal.

### P1 build/port work

1. Agent 02 creates a distinct `ftpnew` static target containing both Ryu C sources and scoped include paths.
2. Agent 03 ports the Ryu client literally into its owned location and records only compatibility diffs. Old `client.c` remains untouched.
3. Agent 02 verifies the archive exports `ftpnew_*`, no old implementation symbols, and links against the intended CSP/GS providers.
4. Agent 03 addresses the closed-connection/error-return defect candidate and validates every error path reaches cleanup exactly once.

### P2 integration work

1. Agent 04 changes RDP upload/download call sites to `ftpnew_upload_rdp` and `ftpnew_download_rdp`.
2. Agent 04 removes old post-call `ftp_abort` behavior from the Ryu path only after cancellation semantics are resolved.
3. Agent 04 preserves callback lifetime: `ftp_config` and `ftpinfo *` are currently used synchronously inside the worker, while callback `gs_ftp_info_t` objects are stack-local and valid only during callback invocation.
4. Agent 04 classifies or migrates every other `gs_ftp_*` call. Final source audit must show no accidental mixed path.

## Existing integration risks

- `ftp_list_callback` returns `int` but has no return statement (`miman_ftprdp.cpp:59-76`).
- It calls `fprintf(fd, flistbuf)` with data as the format string (`:73`), which is a format-string risk.
- The callback opens the output with `"wb"` for every entry (`:66`), so each callback truncates the previous result.
- Upload/download workers busy-wait on `State.uplink_mode` (`:85,128`) and use shared GUI/state objects. These are Agent 04 concerns; no Phase 1 edits were made.
- `miman_ftpfcd.cpp` still depends on force-mode old APIs (`:243,284`) and the same `ftp_avail/ftp_abort` gate (`:220-294`). Final removal of old FTP from the executable therefore requires explicit force-mode scope, not just `miman_ftprdp.cpp` edits.

## Build status

| Check | Result |
|---|---|
| existing `build/` | **failed** before compile: stale cache points to `/home/hyvrid/Desktop/0609/BEE_GS` |
| clean configure in `/tmp/5th_gs_agent01_build` | **succeeded** |
| clean current 5th_GS build | **succeeded** at 100% in `/tmp/5th_gs_agent01_build`; warnings remain, no build input was modified |
| Ryu syntax against current project headers | **failed**, confirmed missing macro/enums |
| current archive symbols | old FTP present; no `ftpnew_*` |

Commands:

```sh
cmake --build build --target BEE-1000 -j2
cmake -S . -B /tmp/5th_gs_agent01_build
cmake --build /tmp/5th_gs_agent01_build --target BEE-1000 -j2
cc -std=gnu11 -fsyntax-only <project and Ryu include roots> \
  FTP_Ryu/client/src/ftpnew_client.c FTP_Ryu/client/src/ftpnew_client_crc32.c
nm -g --defined-only lib/gscsp/build/libmimancsp.a | \
  rg "gs_ftp_|ftpavailable|ftp_done|ftpnew_"
```

## Prioritized open questions

1. **P0, Agent 05/Phase Leader, unverified:** Is the deployed server's base transfer protocol using Ryu `ftp_ret_t` numeric values? If not, Ryu client error handling is wire-incompatible.
2. **P0, Product/Agent 04, unverified:** Must list/move/remove/copy/mkdir/rmdir remain available in the final executable?
3. **P0, Agent 04, unverified:** Is user cancellation during an active transfer required? If yes, Ryu needs a transfer-scoped cancellation contract.
4. **P1, Agent 03, inferred:** Should compatibility emit project direction-specific callback events directly, or should a small adapter translate a private Ryu event enum? Direct emission is smaller; a private enum better preserves upstream source shape.
5. **P1, Agent 02, unverified:** Can old FTP objects be excluded from `libmimancsp.a` while force-mode and command registration remain buildable?
6. **P2, Agent 04, unverified:** Is Ryu's refusal to overwrite an existing destination and `.tmp` behavior acceptable to the GUI workflow?

## Handoff summary (확정/추정/미확인)

- **확정:** Upload/download map to Ryu RDP wrappers; callback function signature and payload layout are reusable.
- **확정:** Current Ryu source does not compile against 5th_GS FTP headers without isolated compatibility changes.
- **확정:** Ryu has local transfer state and integrated private cleanup; old global `state`, `ftpavailable`, and `ftp_done` must not be imported.
- **확정:** Six currently used file-management APIs and cancellation have no Ryu public equivalent.
- **확정:** Current `libmimancsp.a` exports old FTP symbols and no Ryu symbols.
- **확정:** The clean current executable builds successfully and exports old FTP/global cleanup symbols but no `ftpnew_*`.
- **추정:** A separate static library plus a narrowly scoped compatibility layer is the implementable migration boundary.
- **미확인:** Deployed server wire compatibility, final non-transfer feature scope, cancellation requirement, and complete old-object exclusion.
