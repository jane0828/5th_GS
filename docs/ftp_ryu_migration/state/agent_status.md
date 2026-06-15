# Agent Status

## Status Vocabulary
`NOT_STARTED`, `READY`, `RUNNING`, `DONE`, `RETRY_REQUIRED`, `BLOCKED`, `FAILED`, `WAITING_FOR_PHASE_LEADER`, `APPROVED_BY_PHASE_LEADER`, `WAITING_FOR_SUPERVISOR`, `APPROVED_BY_SUPERVISOR`, `FINAL_REPORTED`

## agent_00_orchestrator
- status: `READY`
- role_type: `orchestrator`
- depends_on: 없음
- phase: 전체 workflow
- allowed_files: `docs/ftp_ryu_migration/state/*.md`, orchestration/retry/handoff 문서
- forbidden_files: 모든 source/header/CMake/build script와 실제 동작 코드
- expected_outputs: current status, next/retry prompt, 갱신된 status/queue/decision log, handoff notes
- acceptance_criteria: dependency와 phase review gate를 지키고 기술 승인 없이 다음 작업을 정확히 dispatch
- last_result: 초기 계층형 workflow 상태 생성
- next_action: Phase 1 Worker 01/05/06 병렬 dispatch
- handoff_notes: Worker 결과를 기계적으로 확인한 뒤 Phase Leader에게 기술 검토 요청

## agent_01_structure_analysis
- status: `READY`
- role_type: `worker`
- depends_on: 없음
- phase: Phase 1 Analysis
- allowed_files: `reports/migration_map.md`, `reports/api_mapping.md`, `reports/dependency_report.md`
- forbidden_files: 모든 source/header/CMake/build script
- expected_outputs: migration map, API mapping, dependency report, handoff note
- acceptance_criteria: old/Ryu API, dependency, state, symbol, cleanup 차이가 파일 근거와 함께 분류됨
- last_result: 실행 전
- next_action: Agent 05/06과 병렬 실행
- handoff_notes: Phase Leader가 Build/CMake 입력 충분성을 검토할 수 있게 blocker와 미확인을 구분

## agent_02_build_cmake
- status: `BLOCKED`
- role_type: `worker`
- depends_on: Phase 1 `APPROVED_BY_PHASE_LEADER`
- phase: Phase 2 Build/CMake
- allowed_files: 새 Ryu library 디렉터리, 관련 CMake 최소 변경, build report
- forbidden_files: `src/miman_ftprdp.cpp`, 기존 `client.c`, FTP_Ryu 원본
- expected_outputs: 별도 static target, clean build log, archive object/symbol 목록, handoff note
- acceptance_criteria: old 호출부 미변경 상태에서 Ryu target build 성공, symbol 충돌/혼입 없음
- last_result: Phase 1 review 대기
- next_action: Phase 1 PHASE_GO 후 READY
- handoff_notes: Phase Leader에게 CMake diff, build 명령, symbol 증거 전달

## agent_03_ryu_client_port
- status: `BLOCKED`
- role_type: `worker`
- depends_on: Phase 2 `APPROVED_BY_PHASE_LEADER`
- phase: Phase 3 Ryu Client Port
- allowed_files: 새 Ryu port 디렉터리, adapter, checksum/diff report
- forbidden_files: 기존 `client.c`, `miman_ftprdp.cpp`, FTP_Ryu 원본 저장소
- expected_outputs: port tree, adapter, checksum manifest, source diff report, build 결과, handoff note
- acceptance_criteria: 원본 대응이 추적되고 deviation이 설명되며 기존 client에 혼입되지 않음
- last_result: Phase 2 review 대기
- next_action: Phase 2 PHASE_GO 후 READY
- handoff_notes: 원본/port checksum과 adapter 경계를 Phase Leader에게 전달

## agent_04_ftprdp_integration
- status: `BLOCKED`
- role_type: `worker`
- depends_on: Phase 3 `APPROVED_BY_PHASE_LEADER`
- phase: Phase 4 Integration
- allowed_files: `src/miman_ftprdp.cpp`, 필요한 별도 wrapper, integration report
- forbidden_files: FTP_Ryu 보존 source, 기존 `client.c`, protocol core
- expected_outputs: Ryu test/final call path, lifetime review, build/smoke 결과, rollback path, handoff note
- acceptance_criteria: intended Ryu API 호출, pointer lifetime 안전, hidden mixture 없음, 전환/rollback 경로 명확
- last_result: Phase 3 review 대기
- next_action: Phase 3 PHASE_GO 후 READY
- handoff_notes: Phase Leader에게 call graph, old/new symbol search, rollback 증거 전달

## agent_05_protocol_wire_check
- status: `READY`
- role_type: `worker`
- depends_on: 없음
- phase: Phase 1 Analysis
- allowed_files: protocol reports와 독립 diagnostic snippets
- forbidden_files: core source, CMake, protocol behavior
- expected_outputs: protocol wire report, wire matrix, raw/decode 절차, OBC verdict, handoff note
- acceptance_criteria: type/size/offset/packing/endian/status 길이와 `16777216` 원인이 증거로 판정됨
- last_result: 실행 전
- next_action: Agent 01/06과 병렬 실행
- handoff_notes: OBC-side 수정 필요 가능성은 직접 수정하지 않고 명시

## agent_06_runtime_connection_diag
- status: `READY`
- role_type: `worker`
- depends_on: Phase 1 계획은 없음; Phase 5 실행은 Phase 4 `APPROVED_BY_PHASE_LEADER`
- phase: Phase 1 Analysis 및 Phase 5 Diagnostics
- allowed_files: runtime report, migration 범위 내 diagnostic patch/logging wrapper
- forbidden_files: CSP/libcsp core behavior, protocol semantics, unrelated subsystem
- expected_outputs: 초기 plan, 이후 connection report와 0/5/10/30초 logs, handoff note
- acceptance_criteria: connect/close 대응, close 반환값, baseline/peak/final 상태와 delayed-close/leak 판정
- last_result: 초기 계획 실행 전
- next_action: Phase 1 계획 작성 후 Phase 5에서 실행 결과 갱신
- handoff_notes: Phase별 결과를 구분하고 core 변경 필요 시 사용자 confirmation 대상으로 보고

## agent_07_parser_hardening
- status: `BLOCKED`
- role_type: `worker`
- depends_on: Phase 4 `APPROVED_BY_PHASE_LEADER`, Agent 05 wire bounds
- phase: Phase 5 Hardening
- allowed_files: optional/applied validation patch, tests, parser hardening report
- forbidden_files: old `client.c`, integration source, OBC server, CSP/libcsp core
- expected_outputs: validation patch 상태, malformed tests, validation matrix, build/test 결과, handoff note
- acceptance_criteria: literal baseline과 hardening diff가 분리되고 malformed input이 bounded error로 종료
- last_result: Phase 4 review 대기
- next_action: Phase 5 진입 시 Agent 06 update와 병렬 가능
- handoff_notes: optional과 applied patch를 구분하고 protocol incompatibility를 명시

## agent_08_phase_leader
- status: `BLOCKED`
- role_type: `phase_review`
- depends_on: 각 Phase Worker 완료 및 `WAITING_FOR_PHASE_LEADER`
- phase: Phase 1~5 Review
- allowed_files: phase review notes, `state/decision_log.md`의 review entry
- forbidden_files: 모든 source/header/CMake/build script, Worker 산출물 직접 수정
- expected_outputs: phase review note, ownership audit, PHASE_* 판정, retry/handoff
- acceptance_criteria: 해당 Phase 증거와 다음 Phase 입력 충분성을 기술적으로 검토하고 최종 GO/NO-GO는 내리지 않음
- last_result: 첫 Phase 완료 대기
- next_action: Phase 1 Worker 01/05/06 완료 후 READY
- handoff_notes: 판정을 Orchestrator에게 반환하고 전체 위험은 Supervisor handoff에 누적

## agent_09_supervisor
- status: `BLOCKED`
- role_type: `final_audit`
- depends_on: Phase 1~5 모두 `APPROVED_BY_PHASE_LEADER`, workflow `WAITING_FOR_SUPERVISOR`
- phase: Phase 6 Final Audit
- allowed_files: `reports/supervisor_review.md`
- forbidden_files: 모든 source/header/CMake/build script, 중간 dispatch와 Worker 산출물 수정
- expected_outputs: orchestration/phase/evidence audit, findings, `GO`/`GO WITH RISKS`/`NO-GO`
- acceptance_criteria: old/new 혼합, symbols, build, wire, runtime, risk/rollback을 독립 검토
- last_result: Phase review 완료 대기
- next_action: Orchestrator가 Phase 5 승인 후 READY
- handoff_notes: Final Manager에게 판정과 accepted/blocking risks 전달

## agent_10_final_manager
- status: `BLOCKED`
- role_type: `final_report`
- depends_on: agent_09 Supervisor 판정
- phase: Phase 7 Final Report
- allowed_files: `reports/final_report.md`
- forbidden_files: 모든 source/header/CMake/build script, 중간 검토/dispatch, Supervisor 판정 변경
- expected_outputs: orchestration/phase/audit summary를 포함한 사용자용 final report
- acceptance_criteria: Supervisor 판정과 모순 없이 범위, 결과, 위험, rollback, 최종 판단을 보고
- last_result: Supervisor 대기
- next_action: Agent 09 완료 후 READY
- handoff_notes: 사용자에게 판정, 운영 제한, rollback과 후속 작업을 명확히 전달

