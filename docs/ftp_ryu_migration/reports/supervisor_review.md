# Supervisor Review

- Run ID: `20260616_194256`
- Agent: `agent_09_supervisor`
- Scope: independent audit of orchestration state, phase reviews, git/source/link/symbol evidence, wire/parser/runtime evidence, rollback, and final decision.

## Decision

`NO_GO`

This is a NO-GO for final replacement/deployment. The migration has useful build and diagnostic progress, but the final gate is not satisfied because:

1. Supplied OBC V2.1 wire behavior is explicitly incompatible, and no actual flight OBC build/capture has cleared that risk.
2. No live Ryu upload/download, OBC/simulator smoke, connection-table recovery, `RDP_CLOSE_WAIT` residency, or pool-exhaustion evidence exists.
3. The final `BEE-1000` link still contains legacy FTP `client.c.2.o` / `force.c.2.o` and old symbols, so the final build/exclusion criterion is not met. This appears to be explicit rollback/test coexistence, not a hidden source mixture, but it is not a final replacement state.
4. Ryu literal-source provenance is no longer clean: `sha256sum -c lib/libftp_client_ryu/checksums.sha256` fails for `port/client/src/ftpnew_client.c` and `CMakeLists.txt`.
5. Parser hardening remains optional/unapplied, leaving the product binary exposed to the unbounded status-reply parser risk documented by Phase 5.

## Orchestration Audit

Orchestrator state and decision log are consistent with the observed artifact trail. Phase 1 through Phase 5 have Phase Leader reviews, and retries/blockers are recorded:

- Phase 4 initially hit ownership and thread-argument lifetime blockers, then Agent 04/02 retry and Agent 04b remediation were reviewed.
- Phase 5 initially received `PHASE_RETRY_REQUIRED` because runtime evidence was missing, then a runtime-limited re-review allowed Supervisor handoff with explicit open risk.
- Latest Orchestrator handoff correctly states that Phase 5 is diagnostic-ready, not live OBC validated, and unlocks only Agent 09.

I found no evidence that Supervisor prerequisites were fabricated. The issue is not orchestration honesty; it is that the final acceptance criteria still contain unresolved NO-GO items.

## Build, Link, And Symbol Audit

Independent command:

```sh
cmake -S . -B /tmp/gs5_supervisor_build -DFTP_RYU_CONN_DIAG=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXE_LINKER_FLAGS='-Wl,-Map,/tmp/gs5_supervisor_build/BEE-1000.map'
cmake --build /tmp/gs5_supervisor_build --target BEE-1000 -j2
```

Result: build and link succeeded. Warnings are numerous and mostly pre-existing.

Ryu archive symbols are isolated from old FTP symbols:

- `libftp_client_ryu.a` defines `ftpnew_*`, `ryu_ftp_upload`, `ryu_ftp_download`, and diagnostic symbols.
- It does not define `gs_ftp_*`, `ftp_done`, `ftpavailable`, `ftp_abort`, or `ftp_avail`.

Final executable is not old-free:

- `nm -g /tmp/gs5_supervisor_build/BEE-1000` shows `ftpnew_*` and `ryu_ftp_*`, but also `gs_ftp_upload`, `gs_ftp_download`, `ftp_done`, `ftpavailable`, `ftp_abort`, and `ftp_avail`.
- Link map selects `lib/gscsp/build/libmimancsp.a(client.c.2.o)` for `gs_ftp_upload` and `force.c.2.o` for `ftpavailable`.

This is acceptable as a test/rollback build only. It fails the acceptance criterion that final `BEE-1000` exclude the existing `client.c` object/library.

## Hidden Mixture Audit

No hidden old/new source mixture was found in the reviewed transfer path:

- Product Ryu transfers in `src/miman_ftprdp.cpp` call `ryu_ftp_upload()` and `ryu_ftp_download()`.
- Raw `ftpnew_upload()` / `ftpnew_download()` calls are confined to the Ryu adapter/port.
- GUI/autopilot Ryu branches select `State.ftp_version == 3` and use `miman_ftp_create_worker_arg()` before `pthread_create()`.
- Old V1/V2 FTP paths remain explicit rollback/default branches.

However, old non-transfer operations in `miman_ftprdp.cpp` still use `gs_ftp_list/move/remove/copy/mkdir/rmdir`, and the final binary still links old FTP implementation objects. That is not hidden mixture, but it is not a complete final migration.

## Ryu Source And Checksum Audit

`sha256sum -c lib/libftp_client_ryu/checksums.sha256` from the Ryu target directory fails:

- `port/client/src/ftpnew_client.c`: `FAILED`
- `CMakeLists.txt`: `FAILED`

The `ftpnew_client.c` diff against `../../../FTP_Ryu/client/src/ftpnew_client.c` is diagnostic-related: it wraps `csp_connect` / `csp_close` through `FTPNEW_CSP_CONNECT` / `FTPNEW_CSP_CLOSE` when `FTP_RYU_CONN_DIAG` is enabled. This is explainable, but the manifest/source-diff evidence has not been re-owned and refreshed by the port/checksum owner. Literal baseline confidence is therefore degraded.

## Wire And Parser Audit

Protocol/wire report verdict remains: supplied OBC V2.1 is incompatible; actual flight build is unverified.

Key unresolved wire issues:

- OBC converts `entries` before using it as a loop bound.
- OBC writes reply type through the wrong pointer base instead of payload `reply->data`.
- Old/Ryu status receive path uses unbounded fixed-stack receive behavior.
- Ryu/legacy failure return-code semantics differ.

The optional parser hardening package has standalone malformed tests and `git apply --check` evidence, but it is not applied to the product binary. This is not enough for final GO against the documented parser risk.

## Runtime Audit

Runtime evidence is diagnostic-ready but not live-validated:

- Diagnostic-enabled build links.
- Runtime report documents transfer IDs, connect/close return logging, connection table dumps, `RDP_CLOSE_WAIT` fields, and manual live procedure.
- Local execution was blocked before transfer startup by `Glfw Error 65544: X11: Failed to open display :0`.
- No real OBC, simulator, or headless transfer harness evidence exists.

Required runtime acceptance items are missing: successful upload/download, file size/CRC, baseline/peak/final connection counts, 0/5/10/30 second table dumps, `RDP_CLOSE_WAIT` recovery/leak verdict, and repeated normal/error/abort/burst behavior.

## Rollback And Ownership

Rollback instructions exist for the migration runs and explicitly warn against `git reset --hard`. Old FTP paths remain linked and selectable, so operational rollback is possible in the current test build.

No current unauthorized source/header/CMake edits by Agent 09 were made. Earlier ownership violations were recorded and routed through retries. Current source changes outside reports are from prior workers.

## Findings

1. **NO-GO: OBC wire compatibility is not cleared.** Supplied OBC V2.1 is incompatible and the actual flight build/capture is unavailable. This directly matches the NO-GO rule for wire mismatch.
2. **NO-GO: final executable still links old FTP implementation.** `BEE-1000.map` selects `client.c.2.o` and `force.c.2.o`; `nm` shows old and Ryu symbols together. This is explicit rollback coexistence, not hidden mixture, but it fails final replacement criteria.
3. **NO-GO: no live runtime validation.** Connection recovery, `RDP_CLOSE_WAIT`, pool exhaustion, cleanup without old `ftp_abort()`, and transfer success remain unmeasured.
4. **High risk: Ryu source checksum manifest is stale.** Diagnostic changes to `ftpnew_client.c` and target CMake are explainable but not re-baselined, weakening provenance.
5. **High risk: parser hardening is not in the product binary.** The optional patch is tested but unapplied.

## Final Manager Handoff

Report the migration as buildable and diagnostically prepared, but not approved for final replacement/deployment. Preserve the old FTP rollback path until:

- Agent 05 obtains/validates actual flight OBC compatibility or documents required OBC-side fix acceptance.
- Agent 06 runs live GS/OBC or simulator Ryu upload/download diagnostics with the required connection table and repeat evidence.
- Agent 03 re-owns the Ryu source/CMake checksum deviation or restores literal baseline plus an approved diagnostic wrapper approach.
- Agent 07 hardening is either approved/applied with refreshed checksum evidence or explicitly accepted as an operational risk.
- Agent 02/04 produce a final build profile that excludes old FTP implementation objects once rollback strategy is formally switched from linked coexistence to documented recovery.

SUPERVISOR_DECISION: NO_GO
SUPERVISOR_SUMMARY: Final deployment is blocked by unresolved OBC wire incompatibility, no live runtime validation, old FTP objects still linked in the final executable, stale Ryu checksum provenance, and unapplied parser hardening.
REMEDIATION_AGENTS: agent_05_protocol_wire_check agent_06_runtime_connection_diag agent_03_ryu_client_port agent_07_parser_hardening agent_02_build_cmake agent_04_ftprdp_integration
