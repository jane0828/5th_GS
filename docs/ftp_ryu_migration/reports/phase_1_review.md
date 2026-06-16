# Phase 1 Analysis Review

- Run ID: `20260615_224121`
- Reviewed phase: `Phase 1 Analysis`
- Reviewed agents: `agent_01_structure_analysis`, `agent_05_protocol_wire_check`, `agent_06_runtime_connection_diag`

## Reviewed Outputs

- Agent 01: `migration_map.md`, `api_mapping.md`, `dependency_report.md`
- Agent 05: `protocol_wire_report.md`, `protocol_wire_layout.c`
- Agent 06: `runtime_connection_report.md`, static evidence log, non-applied diagnostic proposal
- Runner evidence: all three Worker outputs exited `0` with `AGENT_STATUS: DONE`
- Repository evidence: current source search, initial git status/diff capture, reported clean build/syntax checks, symbol commands, and independent wire-layout diagnostic

## Checkpoint Review

| Checkpoint | Result | Evidence / condition |
|---|---|---|
| Structure analysis completeness | pass | Old/Ryu API, state, cleanup, symbol, and behavior differences are mapped in all three Agent 01 reports. |
| API mapping sufficiency | pass with blockers | Upload/download and callbacks/settings are traced. Six file-management APIs and cancellation have no Ryu equivalent and are explicitly classified as blockers, not silently mixed. |
| Dependency completeness | pass with Phase 2 proof required | Two Ryu C sources, required headers/config, scoped include roots, external symbols, and forbidden test include root are listed. Exact final CSP/GS link granularity remains for Agent 02 archive/link-map proof. |
| Protocol risk | pass | Type/layout/offset/endian/status-length matrix, raw decode procedure, `16777216` cause, OBC verdict, and capture requirements are present. Supplied OBC V2.1 is explicitly `비호환`; actual flight build remains unverified. |
| Separate target input | pass with notes | A dedicated static target allowlist is defined: `ftpnew_client.c`, `ftpnew_client_crc32.c`, Ryu client/config headers, project GS FTP types, CSP and GS utility providers. Global use of Ryu test replacement headers is forbidden. |
| Runtime plan | pass | `CSP_CONN_MAX=10`, 30-second close wait, 35-second diagnostic interval, transfer IDs, close returns, and baseline/+0/+5/+10/+30 table capture with repetition/stop criteria are specified. Runtime leak is not claimed without measurement. |

## Missing Outputs

- No mandatory Phase 1 report is missing.
- Actual flight OBC source/build identity and raw capture are unavailable; this is a protocol deployment blocker, not a missing Phase 1 analysis artifact.
- Runtime connection table results are intentionally deferred to Phase 5.
- Workflow state was not normalized after Worker completion: `agent_status.md` still marks Agents 01/05/06 `READY` and Agent 08 `BLOCKED`, despite successful Worker logs and the runner entering Phase 1 review.

## File Ownership Violations

None found for the reviewed Worker outputs. Agent 01 changed mapping reports, Agent 05 changed protocol report/diagnostic files, and Agent 06 changed runtime evidence/report/proposal files. No reviewed source, header, CMake, or build script change was attributed to these Workers.

The Phase Leader did not modify Worker outputs or dispatch state.

## Hidden Old/New Mixture Review

No integrated `ftpnew_*` implementation or call path exists yet. Current product source still uses old `gs_ftp_*`, `ftpavailable`, and `ftp_done`; therefore there is no new/old link mixture introduced by Phase 1. Phase 2 must reject any archive or executable that obtains `ftpnew_*` while accidentally pulling old FTP implementation objects.

## Technical Risks

1. Supplied OBC V2.1 is incompatible due to status `entries` conversion order, incorrect reply payload type placement, and unbounded status receive length.
2. Legacy and Ryu failure return-code values differ; success-only compatibility is insufficient for deployment approval.
3. Ryu source does not compile against current project headers without an isolated chunk-macro/callback-enum compatibility boundary.
4. Ryu lacks list/move/remove/copy/mkdir/rmdir and cancellation APIs used by current integration. Keeping those operations through an undeclared old-client path is prohibited hidden mixture.
5. First RDP close may leave a slot in `RDP_CLOSE_WAIT` for up to 30 seconds while returning success. Permanent leakage is unresolved pending Phase 5 measurement.
6. Ryu `ftpnew_data` has a reported send-failure cleanup/return defect candidate that Agent 03 must verify before integration.

## Decision

`PHASE_GO_WITH_NOTES`

Phase 1 required reports, blocker classifications, separate-target inputs, protocol incompatibility statement, and runtime plan are sufficient for Phase 2 Build/CMake work. This is not a final migration `GO`/`NO-GO` decision and does not approve deployment against the supplied OBC.

## Required Retry Items

None for Phase 1 Workers.

## Handoff to Orchestrator

1. Normalize Agents 01/05/06 to completed/reviewed state and Agent 08 Phase 1 review status before unlocking Agent 02.
2. Dispatch only Agent 02 for Phase 2; do not dispatch Agent 03 or integration work.
3. Require Agent 02 to produce a dedicated archive, exact source/include/link inputs, verbose clean build, `ar` listing, defined/undefined symbols, executable link evidence, and proof that existing `client.c` was unchanged.
4. Reject global inclusion of `FTP_Ryu/test/external/include` and reject any hidden old/new FTP object mixture.

## Handoff to Supervisor if Needed

Carry forward the supplied OBC incompatibility, unverified flight build/capture, return-code mismatch, missing non-transfer/cancellation APIs, RDP delayed release, and Ryu send-failure defect candidate. None is resolved by this Phase 1 review.
