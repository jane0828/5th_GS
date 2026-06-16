# Final Manager NO-GO Report

- Report timestamp: `2026-06-16 20:19:49 KST (+0900)`
- Agent: `agent_10_final_manager`
- Scope: final user-submittable summary after Supervisor audit
- Supervisor decision preserved: `NO_GO`

## 1. Executive Summary

The FTP_Ryu migration completed useful code-port, build, integration, diagnostic-readiness, and static review work across Phases 1 through 5. The current repository can build `BEE-1000` with the Ryu FTP client archive linked, and the Ryu runtime path is selectable for test/rollback coexistence.

This is not a deployment approval. The final Supervisor audit issued `NO_GO`, and this Final Manager report preserves that result unchanged. Code migration preparation, build validation, and static diagnostics progressed, but final replacement/deployment remains blocked by unresolved OBC wire compatibility, absent live Ryu/OBC runtime evidence, old FTP implementation objects still linked into the final executable, stale Ryu checksum provenance, and parser hardening that remains optional/unapplied.

## 2. Final Verdict

`NO_GO`

Deployment, final replacement, and any claim that the migration is fully validated are blocked.

Permitted current interpretation:

- Ryu FTP code is ported into an in-tree adapter/archive structure.
- Product build and static linkage evidence exist.
- Ryu runtime diagnostics are ready to run in a real GS/OBC or simulator environment.
- Old FTP remains available as an explicit rollback/default path.

Not permitted:

- Claiming migration success.
- Claiming deployment `GO`.
- Claiming actual flight OBC wire compatibility.
- Claiming live Ryu upload/download validation.
- Claiming final old-FTP-free replacement.

## 3. What Was Completed

- Phase 1 analysis produced migration mapping, API/dependency reports, protocol/wire analysis, and runtime diagnostic planning.
- Phase 2 created and validated an isolated `ftp_client_ryu` static target with alias `FTP_Ryu::client`.
- Phase 3 ported FTP_Ryu into `lib/libftp_client_ryu/` with adapter/wrapper boundaries and checksum/diff reporting.
- Phase 4 integrated explicit old/Ryu transfer selection through `src/miman_ftprdp.cpp`, then remediated Ryu thread argument ownership in GUI/autopilot creation sites.
- Phase 5 added default-off Ryu connection diagnostics behind `FTP_RYU_CONN_DIAG=ON` and documented manual live runtime procedure.
- Optional parser hardening artifacts were prepared, syntax-checked, and standalone-tested, but not applied to product source.
- Multiple clean/product builds succeeded, including diagnostic-enabled builds.
- Supervisor audit was completed and produced a final `NO_GO` decision.

## 4. What Was Not Completed

- Actual flight OBC compatibility was not proven.
- No live Ryu upload/download was executed against a reachable OBC, simulator, or confirmed CSP endpoint.
- No transfer-correlated connection table dumps were produced from runtime execution.
- No measured `RDP_CLOSE_WAIT` recovery/leak verdict exists.
- No pool-exhaustion sequence was measured.
- Final `BEE-1000` still links old FTP implementation objects and symbols.
- Ryu checksum provenance is stale after diagnostic/source deviations.
- Parser hardening remains optional and unapplied to the product binary.
- Final build profile that excludes old FTP implementation objects has not been produced.

## 5. Modified Files

Current Final Manager changes are limited to allowed files:

- `docs/ftp_ryu_migration/reports/final_manager_nogo_report.md`
- `docs/ftp_ryu_migration/state/decision_log.md`

Migration workflow files reported as modified or added by prior agents include:

- `CMakeLists.txt`
- `lib/libftp_client_ryu/CMakeLists.txt`
- `lib/libftp_client_ryu/port/client/src/ftpnew_client.c`
- `lib/libftp_client_ryu/port/client/src/ftpnew_client_crc32.c`
- `lib/libftp_client_ryu/port/config/*`
- `lib/libftp_client_ryu/adapter/include/*`
- `lib/libftp_client_ryu/adapter/src/*`
- `src/miman_ftprdp.cpp`
- `src/miman_ftprdp_integration.h`
- `src/miman_imgui.cpp`
- `src/miman_autopilot.cpp`
- migration reports, logs, optional patches, and state files under `docs/ftp_ryu_migration/`

This report does not modify source/header/CMake/build files, FTP_Ryu preserved source, legacy `client.c`, or protocol core.

## 6. Old FTP vs Ryu FTP Current State

Old FTP remains present, linked, and selectable. It is still the explicit rollback/default path for V1/V2 behavior and for file-management operations that were not replaced by Ryu transfer wrappers.

Ryu FTP is integrated as a separate transfer path. The product Ryu transfer branch calls `ryu_ftp_upload()` and `ryu_ftp_download()` through the adapter/wrapper boundary, while raw `ftpnew_upload()` and `ftpnew_download()` calls remain confined to the Ryu client/archive implementation.

The current state is coexistence for test and rollback, not final replacement. Supervisor confirmed that this is not a hidden source mixture in the Ryu archive, but the final executable still contains old and Ryu FTP namespaces together. That fails final old-FTP-free replacement criteria.

## 7. Build and Static Validation

Build evidence from Agent 02, Agent 04/04b, Agent 06, Agent 07, and Supervisor shows:

- `ftp_client_ryu` static archive builds.
- `BEE-1000` product target links with `FTP_Ryu::client`.
- Diagnostic-enabled build with `-DFTP_RYU_CONN_DIAG=ON` links.
- Ryu archive defines `ryu_ftp_upload`, `ryu_ftp_download`, and expected `ftpnew_*` symbols.
- Ryu archive does not define old `gs_ftp_*`, `ftp_done`, or `ftpavailable` symbols.
- Product link still selects old FTP objects from `libmimancsp.a`, including `client.c.2.o` and `force.c.2.o`.

Therefore build/static validation progressed, but final build acceptance is not met because `BEE-1000` is not old-FTP-free.

## 8. Protocol/Wire Status

Protocol/wire status remains blocking.

The report verdict is: supplied OBC V2.1 source is incompatible, and actual flight build behavior is unverified. The legacy packet numbers, packed field widths, and status offsets match across old GS, FTP_Ryu, and supplied OBC headers, but the supplied OBC implementation has unsafe/incompatible behavior:

- OBC converts `entries` before using it as a host loop bound.
- OBC reply handlers write reply type through the wrong pointer base instead of payload data.
- Old/Ryu status receive paths use unbounded fixed-stack receive behavior.
- Ryu and legacy failure return-code semantics differ.

Ryu against a correctly implemented legacy server is only conditionally compatible. Real flight OBC build/capture evidence is still required.

## 9. Runtime/Connection Diagnostic Status

Runtime diagnostic readiness is complete enough for handoff, but live validation does not exist.

Diagnostic hooks are documented for:

- transfer IDs
- `csp_connect` call/return
- `csp_close` call/return
- connection pointer/index
- CSP endpoint fields
- RDP state
- `RDP_CLOSE_WAIT` age
- connection table dumps
- `CSP_CONN_MAX`/open-count aggregation

Required dump phases are documented as `baseline`, `post-connect`, `post-close`, `complete+0s`, `complete+5s`, `complete+10s`, and `complete+30s`.

The environment could not produce live proof. The local probe failed before transfer startup with `Glfw Error 65544: X11: Failed to open display :0`, and no reachable OBC, simulator, confirmed CSP endpoint, or safe headless Ryu transfer runner was available.

## 10. Parser Hardening Status

Parser hardening is optional and unapplied.

Agent 07 prepared:

- `docs/ftp_ryu_migration/reports/parser_hardening_optional.patch`
- `docs/ftp_ryu_migration/reports/parser_hardening_status_tests.c`
- malformed status parser test results
- `git apply --check` evidence

The hardening was not applied to the product Ryu source or product binary. The current Ryu status request path still contains the unbounded status receive risk documented in Phase 5. This remains a Supervisor `NO_GO` reason.

## 11. Supervisor NO_GO Reasons

| Blocker | Current status | Deployment impact |
|---|---|---|
| OBC wire incompatibility | Supplied OBC V2.1 is incompatible; actual flight build/capture unavailable | Blocks final wire approval |
| No live Ryu/OBC runtime validation | No live upload/download, OBC smoke, table dumps, or RDP recovery evidence | Blocks runtime approval |
| Final executable still links old FTP objects | `BEE-1000` includes old `client.c.2.o`, `force.c.2.o`, and old symbols | Blocks final replacement criteria |
| Ryu checksum provenance stale | `sha256sum -c` fails for `ftpnew_client.c` and `CMakeLists.txt` | Blocks clean source provenance |
| Parser hardening unapplied | Optional patch exists but product binary is not hardened | Leaves known parser risk active |

## 12. Required Manual GS/OBC Validation

Run only on a real GS/OBC setup, confirmed simulator, or safe headless harness that can exercise Ryu transfer paths.

1. Build diagnostic binary:

```sh
cmake -S . -B /tmp/gs5_agent06_live_diag_build -DFTP_RYU_CONN_DIAG=ON
cmake --build /tmp/gs5_agent06_live_diag_build -j2
```

2. Run `BEE-1000` with diagnostic stderr captured under `docs/ftp_ryu_migration/logs/`.
3. Select FTP version `Ryu` in the GUI or equivalent safe runner.
4. Execute upload and download normal cases.
5. Execute timeout, remote-error, abort, and burst cases where operationally safe.
6. Capture file size and CRC/progress success evidence for normal transfers.
7. Capture transfer-correlated `[FTP_RYU_TEST][id=...]` and `[FTP_RYU_CONN_DIAG]` lines.
8. Preserve connection table dumps at `baseline`, `post-connect`, `post-close`, `complete+0s`, `complete+5s`, `complete+10s`, and `complete+30s`.
9. Record baseline, peak, and final open connection counts.
10. Record `conn`, `idx`, `ret`, `state`, `rdp`, `close_wait_ms`, source/destination, ports, and flags.
11. Stop on `csp_connect_return conn=NULL`, `No more free connections`, crash, CRC mismatch, protocol error, or a connection slot still open after the expected recovery window.
12. Attach raw payload capture around status replies before endian conversion for both GS and OBC sides.

Acceptance requires successful transfer evidence plus connection cleanup/recovery evidence. Diagnostic readiness alone is not acceptance.

## 13. Rollback Plan

Current rollback posture is linked coexistence.

- Keep old FTP V1/V2 paths as default/rollback.
- Do not remove old FTP implementation objects until final wire/runtime/build gates are cleared.
- Avoid `git reset --hard` or destructive rollback commands.
- To disable Ryu use operationally, keep GUI/autopilot selection on old FTP branches and avoid invoking `ftp_ryu_uplink_onorbit()` / `ftp_ryu_downlink_onorbit()`.
- To remove diagnostic-only runtime instrumentation later, remove the `FTP_RYU_CONN_DIAG` CMake/source additions and restore direct `csp_connect()` / `csp_close()` call sites as documented in `runtime_connection_report.md`.
- If parser hardening is applied later and must be rolled back, revert only the approved hardening patch and refresh checksum/diff evidence.

## 14. Recommended Next Steps

Priority 1: Resolve wire compatibility.

- Validate the actual flight OBC build, not only supplied source.
- Capture raw status payloads at GS and OBC boundaries.
- Obtain or document OBC-side fixes for endian loop order, reply payload pointer use, length bounds, and chunk validation.

Priority 2: Produce live runtime evidence.

- Run the Agent 06 diagnostic procedure on real GS/OBC or simulator.
- Collect upload/download success, CRC/file-size evidence, connection table dumps, close-return correlation, and pool-exhaustion behavior.

Priority 3: Re-own Ryu source provenance.

- Refresh or restore checksum evidence for `ftpnew_client.c` and Ryu CMake changes.
- Distinguish literal preserved source from approved diagnostic/hardening deviations.

Priority 4: Decide parser hardening.

- Apply the optional hardening with Phase Leader/Supervisor-approved checksum updates, or explicitly accept the operational risk in writing.

Priority 5: Build final replacement profile.

- Produce a final `BEE-1000` profile that excludes old FTP implementation objects only after rollback strategy is formally changed from linked coexistence to documented recovery.
- Re-run link map and symbol checks to prove old implementation exclusion.

## 15. Final Manager Statement

The final Supervisor decision is preserved as `NO_GO`. The migration is buildable and diagnostically prepared, and the old/Ryu coexistence state is useful for controlled testing and rollback. It is not approved for final replacement or deployment.

Before any deployment `GO`, the project must clear OBC wire compatibility with the actual flight target, run live Ryu upload/download diagnostics with connection lifecycle proof, resolve final old FTP object linkage, refresh Ryu checksum provenance, and either apply parser hardening or formally accept the remaining parser risk.

FINAL_MANAGER_STATUS: FINAL_REPORTED
FINAL_SUMMARY: Supervisor NO_GO preserved; final deployment remains blocked pending OBC wire/runtime validation and remaining hardening/removal work.
