# Phase 5 Runtime-Limited Targeted Re-review

- Reviewed phase: `Phase 5 Runtime/Connection Diagnostic retry`
- Reviewed agents: `agent_06_runtime_connection_diag`, `agent_07_parser_hardening`
- Reviewer: `agent_08_phase_leader`
- Review timestamp: `2026-06-16 19:38:43 KST (+0900)`
- Decision: `PHASE_GO_WITH_NOTES`

## Reviewed Outputs

- `docs/ftp_ryu_migration/reports/runtime_connection_report.md`
- `docs/ftp_ryu_migration/reports/phase_5_review.md`
- `docs/ftp_ryu_migration/reports/parser_hardening_report.md`
- `docs/ftp_ryu_migration/reports/thread_arg_ownership_report.md`
- `docs/ftp_ryu_migration/logs/20260616_192754_agent_06_runtime_connection_diag_runtime_limited.out`
- `docs/ftp_ryu_migration/logs/20260616_192915_agent06_runtime_limited_readiness.log`
- `docs/ftp_ryu_migration/logs/20260616_192915_agent06_runtime_limited_table_dumps.md`
- `docs/ftp_ryu_migration/state/decision_log.md`

## Targeted Findings

| Review item | Finding | Result |
|---|---|---|
| Live runtime evidence honesty | Agent 06 states that no live Ryu upload/download, OBC smoke, connection-table dump, RDP close-wait observation, or pool-exhaustion reproduction was run. The local probe failed before transfer startup with `Glfw Error 65544: X11: Failed to open display :0`, and no OBC/simulator/headless runner is available in this Codex environment. | Honest runtime-limited handoff |
| Diagnostic readiness | The report confirms diagnostic hooks for transfer ID, `csp_connect` call/return, `csp_close` call/return, connection pointer/index, CSP endpoint fields, RDP state, close-wait age, and aggregate table dumps. A fresh `FTP_RYU_CONN_DIAG=ON` build linked `BEE-1000`. | Sufficient |
| Connection table schedule | Required markers are documented for `baseline`, `post-connect`, `post-close`, `complete+0s`, `complete+5s`, `complete+10s`, and `complete+30s`. | Sufficient |
| Close-return correlation | The expected schema requires `[FTP_RYU_TEST][id=...]` correlation with `transfer=<id>`, `csp_connect_return`, `conn`, `idx`, `csp_close_return`, and `ret`. | Sufficient |
| RDP close-wait observation | The expected schema requires `state`, `rdp`, `close_wait_ms`, and explicit observation of `rdp=RDP_CLOSE_WAIT`; pass/fail criteria distinguish delayed close from leak by +30 second recovery. | Sufficient |
| `CSP_CONN_MAX` and pool exhaustion | `CSP_CONN_MAX=10` is carried into the capacity table, and burst upload/download criteria require the final successful connect sequence before `conn=NULL` or `No more free connections`. | Sufficient |
| Manual live test procedure | The procedure identifies diagnostic build commands, GUI Ryu selection, upload/download entry points, normal/timeout/remote-error/abort/burst cases, intervals, stop criteria, required log fields, and pass/fail criteria. | Sufficient |
| Parser hardening | Agent 07 remains accepted as optional/unapplied hardening. The product binary still does not include parser hardening; the prior Phase 5 review position is unchanged. | Accepted with notes |

## Decision Rationale

The previous Phase 5 `PHASE_RETRY_REQUIRED` decision was correct because live
runtime evidence was missing. The retry did not produce live evidence either,
but it also did not claim to. Instead, Agent 06 documented a concrete runtime
blocker, verified the diagnostic-enabled build, preserved a manual live test
procedure, defined the required log schema and pass/fail criteria, and kept all
unmeasured runtime behavior as open risk.

Under the runtime-limited re-review criteria, hardware/runtime unavailability
does not by itself block the migration when diagnostic readiness, manual
procedure, and open-risk documentation are sufficient. Those conditions are met.

## Open Risks For Supervisor

- Live runtime validation is still pending.
- No real OBC or simulator Ryu FTP smoke has been executed.
- No live connection table dumps exist for `baseline`, `post-connect`,
  `post-close`, `complete+0s`, `complete+5s`, `complete+10s`, or
  `complete+30s`.
- No measured close-return-to-slot-recovery correlation exists.
- No measured `RDP_CLOSE_WAIT` residency timeline exists.
- No measured baseline, peak, final open-count table or pool-exhaustion
  sequence exists.
- Cleanup behavior without old `ftp_abort()` remains unverified on real
  transfers.
- Parser hardening remains optional and unapplied; the product binary still
  carries the unbounded status-reply parser risk until a later approved
  deviation applies it.
- The `ftpnew_client.c` checksum manifest mismatch noted by Agent 07 remains a
  literal-baseline confidence issue for final disposition.

## Handoff

Phase 6 Supervisor can proceed with notes. The Supervisor must treat live Ryu
runtime validation as pending and must not treat Phase 5 as live OBC validated.
The next real GS/OBC or simulator window must run the documented manual
procedure and attach transfer-correlated diagnostic logs before any final
replacement or deployment decision.

PHASE_DECISION: PHASE_GO_WITH_NOTES
PHASE_SUMMARY: Diagnostic readiness and manual runtime procedure are sufficient for Supervisor handoff, but live Ryu/OBC runtime validation remains pending as an open risk.
