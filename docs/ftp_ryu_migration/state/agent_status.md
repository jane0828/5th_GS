# Agent Status

## Status Vocabulary
`NOT_STARTED`, `READY`, `RUNNING`, `DONE`, `RETRY_REQUIRED`, `BLOCKED`, `FAILED`, `WAITING_FOR_PHASE_LEADER`, `APPROVED_BY_PHASE_LEADER`, `WAITING_FOR_SUPERVISOR`, `APPROVED_BY_SUPERVISOR`, `FINAL_REPORTED`

## agent_00_orchestrator
- status: `DONE`
- role_type: `orchestrator`
- depends_on: 없음
- phase: 전체 workflow
- allowed_files: `docs/ftp_ryu_migration/state/*.md`, orchestration/retry/handoff 문서
- forbidden_files: 모든 source/header/CMake/build script와 실제 동작 코드
- expected_outputs: current status, next/retry prompt, 갱신된 status/queue/decision log, handoff notes
- acceptance_criteria: dependency와 phase review gate를 지키고 기술 승인 없이 다음 작업을 정확히 dispatch
- last_result: Supervisor `NO_GO` output and `reports/supervisor_review.md` were mechanically verified; remediation targets were routed and state/queue/decision log were updated
- next_action: Dispatch first-wave remediation Agents 05/06/03/07, then after their outputs dispatch Agents 02/04 for final build-profile old FTP exclusion work
- handoff_notes: 기술 승인이나 최종 판정은 하지 않았다. Exact retry targets are `agent_05_protocol_wire_check agent_06_runtime_connection_diag agent_03_ryu_client_port agent_07_parser_hardening agent_02_build_cmake agent_04_ftprdp_integration`; Agent 10 remains blocked while Supervisor decision is `NO_GO`

## agent_01_structure_analysis
- status: `APPROVED_BY_PHASE_LEADER`
- role_type: `worker`
- depends_on: 없음
- phase: Phase 1 Analysis
- allowed_files: `reports/migration_map.md`, `reports/api_mapping.md`, `reports/dependency_report.md`
- forbidden_files: 모든 source/header/CMake/build script
- expected_outputs: migration map, API mapping, dependency report, handoff note
- acceptance_criteria: old/Ryu API, dependency, state, symbol, cleanup 차이가 파일 근거와 함께 분류됨
- last_result: migration/API/dependency 보고서 3개와 `AGENT_STATUS: DONE` 확인; Phase 1 `PHASE_GO_WITH_NOTES`
- next_action: Phase 2 이후 추가 구조 분석 요청이 있을 때까지 대기
- handoff_notes: callback enum/chunk macro compile blocker, 누락 file-management/cancellation API, old/new symbol 분리 요구를 Agent 02 이후에도 유지

## agent_02_build_cmake
- status: `BLOCKED`
- role_type: `worker`
- depends_on: Phase 1 `APPROVED_BY_PHASE_LEADER`
- phase: Phase 2 Build/CMake
- allowed_files: 새 Ryu library 디렉터리, 관련 CMake 최소 변경, build report
- forbidden_files: `src/miman_ftprdp.cpp`, 기존 `client.c`, FTP_Ryu 원본
- expected_outputs: 별도 static target, clean build log, archive object/symbol 목록, handoff note
- acceptance_criteria: old 호출부 미변경 상태에서 Ryu target build 성공, symbol 충돌/혼입 없음
- last_result: Supervisor `NO_GO`에서 final `BEE-1000`이 old FTP `client.c.2.o`/`force.c.2.o`와 old FTP symbols를 계속 link한다고 감사됨
- next_action: Agent 03 checksum/provenance 및 Agent 07 parser hardening disposition 이후, Agent 04와 함께 final build profile에서 old FTP implementation object exclusion과 link-map/nm evidence를 제출
- handoff_notes: Remediation target이지만 first-wave 병렬 실행 대상은 아니다. rollback/test coexistence와 final replacement build profile을 분리하고 old FTP build exclusion은 명시적 rollback 문서와 함께 수행해야 한다

## agent_03_ryu_client_port
- status: `READY`
- role_type: `worker`
- depends_on: Phase 2 `APPROVED_BY_PHASE_LEADER`
- phase: Phase 3 Ryu Client Port
- allowed_files: 새 Ryu port 디렉터리, adapter, checksum/diff report
- forbidden_files: 기존 `client.c`, `miman_ftprdp.cpp`, FTP_Ryu 원본 저장소
- expected_outputs: port tree, adapter, checksum manifest, source diff report, build 결과, handoff note
- acceptance_criteria: 원본 대응이 추적되고 deviation이 설명되며 기존 client에 혼입되지 않음
- last_result: Supervisor `NO_GO`에서 `sha256sum -c lib/libftp_client_ryu/checksums.sha256`가 `port/client/src/ftpnew_client.c`와 `CMakeLists.txt`에 대해 실패한다고 감사됨
- next_action: Ryu source/CMake checksum deviation을 re-own/re-baseline하거나 literal baseline plus approved diagnostic wrapper approach로 복구하고 checksum/source-diff/build evidence를 갱신
- handoff_notes: Diagnostic wrapper가 필요한 경우 literal baseline과 deviation을 분리해 기록한다. 승인 없는 parser hardening을 literal baseline에 섞지 말고 Agent 07 disposition과 충돌하지 않게 한다

## agent_04_ftprdp_integration
- status: `BLOCKED`
- role_type: `worker`
- depends_on: Phase 3 `APPROVED_BY_PHASE_LEADER`
- phase: Phase 4 Integration
- allowed_files: `src/miman_ftprdp.cpp`, 필요한 별도 wrapper, integration report
- forbidden_files: FTP_Ryu 보존 source, 기존 `client.c`, protocol core
- expected_outputs: Ryu test/final call path, lifetime review, build/smoke 결과, rollback path, handoff note
- acceptance_criteria: intended Ryu API 호출, pointer lifetime 안전, hidden mixture 없음, 전환/rollback 경로 명확
- last_result: Supervisor `NO_GO`에서 final executable old FTP implementation linkage and explicit old/Ryu coexistence가 final replacement criterion을 만족하지 못한다고 감사됨
- next_action: Agent 02 final build-profile remediation과 함께 old FTP implementation exclusion, final selector/rollback documentation, and no hidden-mixture evidence를 제출
- handoff_notes: Remediation target이지만 Agent 02 final link work와 결합되어야 한다. Agent 05/06/03/07 first-wave remediation 결과 전에는 final switch를 주장하지 않는다

## agent_05_protocol_wire_check
- status: `READY`
- role_type: `worker`
- depends_on: 없음
- phase: Phase 1 Analysis
- allowed_files: protocol reports와 독립 diagnostic snippets
- forbidden_files: core source, CMake, protocol behavior
- expected_outputs: protocol wire report, wire matrix, raw/decode 절차, OBC verdict, handoff note
- acceptance_criteria: type/size/offset/packing/endian/status 길이와 `16777216` 원인이 증거로 판정됨
- last_result: Supervisor `NO_GO`에서 supplied OBC V2.1 wire incompatibility and absent actual flight OBC build/capture가 final blocker로 감사됨
- next_action: actual flight OBC build/source/config/capture를 확보해 compatibility를 재검증하거나 required OBC-side fix acceptance/documentation을 제출
- handoff_notes: OBC server code 수정은 사용자 confirmation 대상이다. 본 Agent는 core/protocol behavior를 수정하지 말고 wire verdict, raw capture/decode, exact OBC-side requirements만 갱신한다

## agent_06_runtime_connection_diag
- status: `READY`
- role_type: `worker`
- depends_on: Phase 1 계획은 없음; Phase 5 실행은 Phase 4 `APPROVED_BY_PHASE_LEADER`
- phase: Phase 1 Analysis 및 Phase 5 Diagnostics
- allowed_files: runtime report, migration 범위 내 diagnostic patch/logging wrapper
- forbidden_files: CSP/libcsp core behavior, protocol semantics, unrelated subsystem
- expected_outputs: 초기 plan, 이후 connection report와 0/5/10/30초 logs, handoff note
- acceptance_criteria: connect/close 대응, close 반환값, baseline/peak/final 상태와 delayed-close/leak 판정
- last_result: Supervisor `NO_GO`에서 no live Ryu upload/download, OBC/simulator smoke, connection-table recovery, `RDP_CLOSE_WAIT` residency, or pool-exhaustion evidence가 final blocker로 감사됨
- next_action: real GS/OBC, simulator, or safe headless harness에서 Ryu upload/download diagnostics를 실행하고 required runtime evidence를 제출
- handoff_notes: baseline/peak/final counts, complete+0/+5/+10/+30 table dumps, close-return correlation, file size/CRC, normal/error/abort/burst behavior, RDP close-wait/leak verdict가 필요하다. protocol/core/parser behavior는 변경하지 않는다

## agent_07_parser_hardening
- status: `READY`
- role_type: `worker`
- depends_on: Phase 4 `APPROVED_BY_PHASE_LEADER`, Agent 05 wire bounds
- phase: Phase 5 Hardening
- allowed_files: optional/applied validation patch, tests, parser hardening report
- forbidden_files: old `client.c`, integration source, OBC server, CSP/libcsp core
- expected_outputs: validation patch 상태, malformed tests, validation matrix, build/test 결과, handoff note
- acceptance_criteria: literal baseline과 hardening diff가 분리되고 malformed input이 bounded error로 종료
- last_result: Supervisor `NO_GO`에서 optional parser hardening이 unapplied이고 product binary가 documented unbounded status-reply parser risk에 노출되어 있다고 감사됨
- next_action: hardening을 approved/applied state로 전환할지, 또는 explicit operational risk로 accept할지 결정 가능한 patch/test/build/checksum evidence를 제출
- handoff_notes: 적용 시 Agent 03 checksum/provenance 갱신과 함께 literal baseline deviation을 분리해 기록한다. old `client.c`, integration source, OBC server, CSP/libcsp core는 수정하지 않는다

## agent_08_phase_leader
- status: `BLOCKED`
- role_type: `phase_review`
- depends_on: 각 Phase Worker 완료 및 `WAITING_FOR_PHASE_LEADER`
- phase: Phase 1~5 Review
- allowed_files: phase review notes, `state/decision_log.md`의 review entry
- forbidden_files: 모든 source/header/CMake/build script, Worker 산출물 직접 수정
- expected_outputs: phase review note, ownership audit, PHASE_* 판정, retry/handoff
- acceptance_criteria: 해당 Phase 증거와 다음 Phase 입력 충분성을 기술적으로 검토하고 최종 GO/NO-GO는 내리지 않음
- last_result: Supervisor `NO_GO` completed; remediation Worker outputs are required before a new technical review
- next_action: First-wave remediation outputs from Agents 05/06/03/07 and later final build-profile outputs from Agents 02/04 are completed and mechanically verified by Orchestrator, then perform targeted re-review
- handoff_notes: Review should focus on Supervisor blockers: OBC compatibility, live runtime evidence, checksum provenance, parser hardening disposition, and final old FTP exclusion/no hidden mixture

## agent_09_supervisor
- status: `DONE`
- role_type: `final_audit`
- depends_on: Phase 1~5 모두 `APPROVED_BY_PHASE_LEADER`, workflow `WAITING_FOR_SUPERVISOR`
- phase: Phase 6 Final Audit
- allowed_files: `reports/supervisor_review.md`
- forbidden_files: 모든 source/header/CMake/build script, 중간 dispatch와 Worker 산출물 수정
- expected_outputs: orchestration/phase/evidence audit, findings, `GO`/`GO WITH RISKS`/`NO-GO`
- acceptance_criteria: old/new 혼합, symbols, build, wire, runtime, risk/rollback을 독립 검토
- last_result: Run `20260616_194256` submitted `reports/supervisor_review.md` and `SUPERVISOR_DECISION: NO_GO`
- next_action: Wait for remediation and Phase Leader re-review before any subsequent Supervisor re-audit
- handoff_notes: Final deployment is blocked by unresolved OBC wire incompatibility, no live runtime validation, old FTP objects still linked in final executable, stale Ryu checksum provenance, and unapplied parser hardening

## agent_10_final_manager
- status: `BLOCKED`
- role_type: `final_report`
- depends_on: agent_09 Supervisor 판정
- phase: Phase 7 Final Report
- allowed_files: `reports/final_report.md`
- forbidden_files: 모든 source/header/CMake/build script, 중간 검토/dispatch, Supervisor 판정 변경
- expected_outputs: orchestration/phase/audit summary를 포함한 사용자용 final report
- acceptance_criteria: Supervisor 판정과 모순 없이 범위, 결과, 위험, rollback, 최종 판단을 보고
- last_result: Supervisor returned `NO_GO`; final report is blocked until remediation and a new Supervisor disposition, unless explicitly asked to report a blocked outcome
- next_action: Remain blocked while retry targets run
- handoff_notes: Do not convert Supervisor `NO_GO` into a final deployment approval. If later requested to report current state only, report blocked/no-go status without altering the decision
