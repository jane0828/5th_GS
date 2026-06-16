# Task Queue

| Task | Prerequisite | Ready Condition | Completion Condition | Review Required | Reviewer | Next Task Unlock Rule |
|---|---|---|---|---|---|---|
| Phase 1 / Agent 01 Structure | 없음 | prompt/reports 경로 사용 가능 | 3개 분석 보고서와 handoff 완료 | Yes | Agent 08 | Agent 01/05/06 모두 완료 시 Phase 1 Review |
| Phase 1 / Agent 05 Protocol | 없음 | old/Ryu/OBC 자료 사용 가능 | wire report, compatibility verdict, handoff 완료 | Yes | Agent 08 | Agent 01/05/06 모두 완료 시 Phase 1 Review |
| Phase 1 / Agent 06 Runtime Plan | 없음 | CSP/RDP 자료 사용 가능 | diagnostic plan과 반복 test 설계 완료 | Yes | Agent 08 | Agent 01/05/06 모두 완료 시 Phase 1 Review |
| Phase 1 Review / Agent 08 | Phase 1 Workers 완료 | status `WAITING_FOR_PHASE_LEADER` | PHASE_* 판정과 review note | N/A | Phase Leader | `PHASE_GO*`면 Agent 02 READY, retry면 대상 Worker READY |
| Phase 2 / Agent 02 Build/CMake | Phase 1 승인 | Phase 1 `APPROVED_BY_PHASE_LEADER` | 별도 target build와 symbol 증거 완료 | Yes | Agent 08 | Agent 02 완료 시 Phase 2 Review |
| Phase 2 Review / Agent 08 | Agent 02 완료 | Phase 2 `WAITING_FOR_PHASE_LEADER` | PHASE_* 판정과 review note | N/A | Phase Leader | `PHASE_GO*`면 Agent 03 READY |
| Phase 3 / Agent 03 Ryu Port | Phase 2 승인 | Phase 2 `APPROVED_BY_PHASE_LEADER` | port/checksum/diff/adapter/build 완료 | Yes | Agent 08 | Agent 03 완료 시 Phase 3 Review |
| Phase 3 Review / Agent 08 | Agent 03 완료 | Phase 3 `WAITING_FOR_PHASE_LEADER` | PHASE_* 판정과 review note | N/A | Phase Leader | `PHASE_GO*`면 Agent 04 READY |
| Phase 4 / Agent 04 Integration | Phase 3 승인 | Phase 3 `APPROVED_BY_PHASE_LEADER` | call path/lifetime/build/smoke/rollback 증거 완료 | Yes | Agent 08 | Agent 04 완료 시 Phase 4 Review |
| Phase 4 Review / Agent 08 | Agent 04 완료 | Phase 4 `WAITING_FOR_PHASE_LEADER` | PHASE_* 판정과 review note | N/A | Phase Leader | `PHASE_GO*`면 Phase 5 Workers READY |
| Phase 5 / Agent 06 Runtime Update | Phase 4 승인 | executable test path와 diagnostics 사용 가능 | 0/5/10/30초 및 반복 결과 완료 | Yes | Agent 08 | Agent 06/07 완료 시 Phase 5 Review |
| Phase 5 / Agent 07 Hardening | Phase 4 승인, wire bounds | baseline과 patch 경계 확정 | optional/applied patch와 malformed 결과 완료 | Yes | Agent 08 | Agent 06/07 완료 시 Phase 5 Review |
| Phase 5 Review / Agent 08 | Phase 5 Workers 완료 | Phase 5 `WAITING_FOR_PHASE_LEADER` | PHASE_* 판정과 Supervisor handoff | N/A | Phase Leader | `PHASE_GO*`면 Agent 09 READY |
| Phase 6 / Agent 09 Supervisor | Phase 1~5 승인 | status `WAITING_FOR_SUPERVISOR` | independent audit와 GO 계열/NO-GO 판정 | No phase review | Final independent auditor | Agent 10 READY |
| Phase 7 / Agent 10 Final Manager | Supervisor 판정 | supervisor review 존재 | final report와 `FINAL_REPORTED` 상태 | No | Final reporter | workflow 종료 |

## Parallel Rules

- Phase 1의 Agent 01, 05, 06은 병렬 가능하다.
- Phase 5의 Agent 06 update와 Agent 07은 파일 소유권 충돌이 없으면 병렬 가능하다.
- Agent 02 -> 03 -> 04는 각 Phase Leader 승인 사이에 순차 실행한다.
- Phase Leader review 동안 해당 Phase 후속 Worker를 실행하지 않는다.
- Supervisor는 모든 Phase review 후 단독 실행한다.
- Final Manager는 Supervisor 이후 단독 실행한다.

## Initial Queue

1. `READY`: agent_01_structure_analysis
2. `READY`: agent_05_protocol_wire_check
3. `READY`: agent_06_runtime_connection_diag initial
4. `BLOCKED`: agent_08_phase_leader until Phase 1 Workers complete
5. `BLOCKED`: Phase 2 이후

## Active Dispatch

- run_id: `20260616_194256`
- prepared_at: `2026-06-16 19:58:00 KST`
- dispatch_group: `agent_05_protocol_wire_check agent_06_runtime_connection_diag agent_03_ryu_client_port agent_07_parser_hardening`
- execution_mode: parallel_first_wave_remediation
- current_status: `RETRY_REQUIRED`
- prerequisite_evidence: Agent 09 Supervisor output exists at `logs/20260616_194611_agent_09_supervisor.out`; `reports/supervisor_review.md` is non-empty and contains `SUPERVISOR_DECISION: NO_GO`; Supervisor listed remediation agents `agent_05_protocol_wire_check agent_06_runtime_connection_diag agent_03_ryu_client_port agent_07_parser_hardening agent_02_build_cmake agent_04_ftprdp_integration`
- completion_check: First-wave remediation must provide updated non-empty reports/artifacts and final `AGENT_STATUS` markers. Agent 05 must resolve or precisely document actual flight OBC compatibility/fix acceptance. Agent 06 must provide live GS/OBC, simulator, or safe harness runtime evidence or exact environment blocker. Agent 03 must refresh checksum/source-diff provenance or restore literal baseline plus approved diagnostic wrapper. Agent 07 must provide applied-hardening evidence or explicit operational-risk disposition with tests/build/checksum implications.
- retry_reason: Supervisor `NO_GO`: unresolved OBC wire incompatibility, no live Ryu/OBC runtime validation, final executable still links old FTP implementation objects, stale Ryu checksum provenance, and unapplied parser hardening.
- retry_instruction: Run first-wave remediation Agents 05/06/03/07 in parallel if environment permits. Do not run Agent 02/04 final build-profile remediation until Agent 03/07 disposition and Agent 05/06 evidence are available, because old FTP exclusion/final replacement depends on those results.
- next_gate: Orchestrator mechanically verifies first-wave outputs, then dispatches Agent 02 and Agent 04 for final build-profile / old FTP implementation exclusion work, then requests Agent 08 targeted re-review before any new Supervisor audit.
- forbidden_dispatch: Agent 10 Final Manager is blocked while Supervisor decision is `NO_GO`. Agent 02/04 final switch work is blocked until first-wave remediation outputs are available. No OBC server code, CSP/libcsp core behavior, protocol-incompatible change without OBC-side documentation, or irreversible deletion may be dispatched without required confirmation.
- user_confirmation_required: no

## Supervisor NO-GO Remediation Queue

| Remediation Target | Status | Required Output | Dispatch Rule |
|---|---|---|---|
| `agent_05_protocol_wire_check` | `READY` | Updated `protocol_wire_report.md`, raw/capture or exact missing flight-OBC evidence, compatibility/fix-acceptance verdict, `AGENT_STATUS` marker | First-wave parallel |
| `agent_06_runtime_connection_diag` | `READY` | Live/simulator/headless Ryu upload/download evidence, file size/CRC, baseline/peak/final counts, 0/5/10/30 table dumps, RDP close-wait/leak verdict, `AGENT_STATUS` marker | First-wave parallel |
| `agent_03_ryu_client_port` | `READY` | Refreshed checksum manifest/source-diff/deviation report and clean build evidence, or restored literal baseline plus approved diagnostic-wrapper boundary, `AGENT_STATUS` marker | First-wave parallel |
| `agent_07_parser_hardening` | `READY` | Applied-hardening patch/test/build/checksum evidence or explicit risk-acceptance package, `AGENT_STATUS` marker | First-wave parallel |
| `agent_02_build_cmake` | `BLOCKED` | Final build profile excluding old FTP implementation objects with link-map/nm evidence | After first-wave outputs, paired with Agent 04 |
| `agent_04_ftprdp_integration` | `BLOCKED` | Final selector/switch/rollback documentation and no-hidden-mixture evidence for old FTP exclusion | After first-wave outputs, paired with Agent 02 |
