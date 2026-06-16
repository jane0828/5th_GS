# Phase 5 Hardening/Diagnostics Review

- Reviewed phase: `Phase 5 Hardening/Diagnostics`
- Reviewed agents: `agent_06_runtime_connection_diag`, `agent_07_parser_hardening`
- Run ID: `20260616_154913`
- Reviewer: `agent_08_phase_leader`
- Review timestamp: `2026-06-16 16:06:39 KST (+0900)`

## Reviewed Outputs

- `docs/ftp_ryu_migration/reports/runtime_connection_report.md`
- `docs/ftp_ryu_migration/reports/20260616_154913_agent06_diagnostic.diff`
- `docs/ftp_ryu_migration/logs/20260616_154913_agent06_build_status.txt`
- `docs/ftp_ryu_migration/logs/20260616_154913_agent06_runtime_table_dumps.md`
- `docs/ftp_ryu_migration/logs/20260616_155213_agent_06_runtime_connection_diag.out`
- `docs/ftp_ryu_migration/reports/parser_hardening_report.md`
- `docs/ftp_ryu_migration/reports/parser_hardening_optional.patch`
- `docs/ftp_ryu_migration/reports/parser_hardening_status_tests.c`
- `docs/ftp_ryu_migration/logs/20260616_155213_agent_07_parser_hardening.out`
- Phase 5 checkpoint in `docs/ftp_ryu_migration/09_phase_leader_checkpoints.md`
- Current source/diff/symbol-search evidence gathered by Phase Leader

## Checkpoint Review

| Checkpoint | Evidence | Review result |
|---|---|---|
| Connection table baseline/peak/0/5/10/30s | Agent 06 added a default-off `FTP_RYU_CONN_DIAG` path and documented labels `baseline`, `post-connect`, `post-close`, `complete+0s`, `complete+5s`, `complete+10s`, `complete+30s`. `20260616_154913_agent06_runtime_table_dumps.md` states runtime execution was not performed and no measured baseline, peak, final count, or pool-exhaustion sequence was captured. | Missing required runtime evidence |
| `csp_close` logging by transfer ID | Agent 06 wrapped Ryu `csp_connect`/`csp_close` sites and logs transfer ID, pointer/index, state, return value, and CSP identifiers when diagnostics are enabled. No live transfer log was produced. | Mechanism present; transfer evidence missing |
| RDP delayed close timeline | Agent 06 statically identified `CSP_CONN_MAX=10` and `csp_rdp_set_opt(6, 30000, 16000, 1, 8000, 3)`, and the diagnostic wrapper can report `RDP_CLOSE_WAIT` residency. No measured `RDP_CLOSE_WAIT` timeline exists. | Missing required runtime timeline |
| Parser validation | Agent 07 produced a validation matrix and standalone malformed harness covering truncated payload, oversized/endian-crafted entries, overflow, range, zero-count, duplicate/overlap, and contradictory progress cases. Report says 15 standalone cases passed. | Sufficient for optional hardening review |
| Literal principle | Agent 07 kept hardening as `parser_hardening_optional.patch`; report states current Ryu source still has the unbounded status call and no hardening helper. | Satisfied, with checksum caveat |
| Patch state / binary reflection | Agent 07 reports optional patch not applied, `git apply --check` success, and product build success without source hardening. Agent 06 reports diagnostics-off and diagnostics-on builds both link `BEE-1000`; default diagnostics are off. | Patch states clear; parser hardening not in product binary |

## Missing Outputs

- Timestamped runtime table dumps for at least one actual Ryu upload and one actual Ryu download at `baseline`, `complete+0s`, `complete+5s`, `complete+10s`, and `complete+30s`.
- Transfer-correlated `csp_connect_return` and `csp_close_return` logs from a live Ryu transfer, including connection index/pointer and close return values.
- Measured `RDP_CLOSE_WAIT` residency timeline showing whether slots recover, remain delayed until about 30 seconds, or leak.
- Baseline, peak, final open-count table and pool-exhaustion sequence from repeated upload/download runs.
- Normal/timeout/remote-error/abort comparison results, or an explicit environment-backed retry result for each unavailable case.

## File Ownership Violations

- No Phase Leader source/CMake edits were made during this review.
- No Worker ownership violation was found in Agent 07 outputs: hardening remains in reports/test/optional patch artifacts and old `client.c`, integration source, OBC server, and CSP/libcsp core were not modified by Agent 07.
- Agent 06 changed Ryu diagnostic wrapper files, Ryu target CMake, Ryu client connect/close call wrappers, and `src/miman_ftprdp.cpp` diagnostic no-op hooks. This is within the Phase 5 prompt's approved diagnostic-only patch/logging wrapper scope as long as `FTP_RYU_CONN_DIAG` remains default-off and production behavior is unchanged.
- No hidden old/new mixture was found in legacy `lib/gscsp/lib/libftp_client/src/client.c`; current Ryu integration path continues through `ryu_ftp_upload` / `ryu_ftp_download` and old FTP paths remain explicit rollback paths.

## Technical Risks

- Phase 5 runtime success criteria are not met because connection recovery versus leak is still unmeasured.
- `RDP_CLOSE_WAIT` delayed close remains a plausible explanation, but it is not proven by live timeline evidence.
- The parser hardening patch is optional and not reflected in the product binary; uploads still carry the unbounded status-reply risk until the deviation is approved and applied.
- `lib/libftp_client_ryu/checksums.sha256` hash for `port/client/src/ftpnew_client.c` does not match the current file hash per Agent 07 report. This predates parser hardening but weakens literal-baseline evidence until disposed by the port/checksum owner.
- No real OBC or simulator runtime smoke was executed. OBC wire compatibility, cleanup behavior without old `ftp_abort()`, and callback synchronous-lifetime assumptions remain carried-forward risks.

## Decision

`PHASE_RETRY_REQUIRED`

Phase 5 cannot advance to Supervisor because the required runtime connection table, close-return, and RDP delayed-close evidence is missing. Parser hardening outputs are sufficient as an optional, unapplied hardening package, but that does not satisfy the runtime diagnostics checkpoint.

## Required Retry Items

Retry `agent_06_runtime_connection_diag` only.

Required items:

- Run the diagnostics-enabled binary on a real CSP endpoint, simulator, or explicitly safe transfer harness that exercises the `State.ftp_version == 3` Ryu path.
- Capture timestamped table dumps at `baseline`, `post-connect`, `post-close`, `complete+0s`, `complete+5s`, `complete+10s`, and `complete+30s`.
- Correlate every dump with transfer ID, direction, `csp_connect_return`, connection pointer/index, `csp_close_return`, outer connection state, RDP state, ports, flags, age, and `RDP_CLOSE_WAIT` residency.
- Provide baseline, peak, final open counts and the pool-exhaustion sequence for repeated upload/download attempts.
- Execute or explicitly mark unavailable with environment evidence: normal, timeout, remote-error, abort, and burst upload/download cases.
- Do not change protocol behavior, packet layout, transfer algorithms, CSP/libcsp core, old `client.c`, or parser hardening state during the retry.

## Handoff To Orchestrator

- Keep Phase 5 in retry state and dispatch only `agent_06_runtime_connection_diag`.
- Do not dispatch Supervisor until a new Phase 5 review confirms live runtime table/close/RDP evidence, or the workflow is explicitly blocked due to unavailable endpoint/harness after retry evidence.
- Preserve Agent 07 outputs as accepted optional hardening artifacts; no Agent 07 retry is required for this Phase 5 review.

## Handoff To Supervisor If Needed

- Parser hardening is optional and unapplied; product binary remains exposed to the unbounded status-reply risk.
- Runtime leak/delayed-close remains unresolved because no live transfer was executed.
- Checksum mismatch for `ftpnew_client.c` requires disposition before final literal-baseline confidence.
- OBC protocol compatibility and real OBC smoke remain unresolved.

