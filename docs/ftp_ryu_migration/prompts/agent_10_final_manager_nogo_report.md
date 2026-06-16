# Agent 10 Prompt: Final Manager NO_GO Report

## Agent 이름
Final Manager Agent

## 목표
Supervisor가 최종 `NO_GO`를 낸 현재 상태를 사용자에게 제출 가능한 최종 보고서로 정리한다. 절대 migration 성공 또는 deployment GO라고 주장하지 마라.

## 배경
FTP_Ryu migration multi-agent workflow는 Phase 1~5를 진행했고, Phase 4/5는 targeted review를 통해 GO_WITH_NOTES까지 도달했다. 그러나 최종 Supervisor audit은 NO_GO를 냈다.

Supervisor NO_GO 주요 근거:
- OBC wire incompatibility remains unresolved
- no live Ryu/OBC runtime validation exists
- final BEE-1000 still links old FTP objects
- Ryu checksum provenance is stale
- parser hardening is unapplied

Agent 06 runtime diagnostics는 diagnostic readiness는 완료했으나 live connection lifecycle proof를 만들 수 없었다. 현재 환경에는 reachable OBC, simulator, confirmed CSP endpoint, safe headless Ryu transfer runner가 없다.

## 입력 파일
- docs/ftp_ryu_migration/reports/supervisor_review.md
- docs/ftp_ryu_migration/reports/final_report.md, if exists
- docs/ftp_ryu_migration/reports/runtime_connection_report.md
- docs/ftp_ryu_migration/reports/protocol_wire_report.md
- docs/ftp_ryu_migration/reports/phase_5_runtime_limited_re_review.md
- docs/ftp_ryu_migration/reports/thread_arg_ownership_report.md
- docs/ftp_ryu_migration/reports/ftprdp_integration_report.md
- docs/ftp_ryu_migration/reports/build_cmake_report.md
- docs/ftp_ryu_migration/state/decision_log.md
- latest logs under docs/ftp_ryu_migration/logs

## 수정 가능 파일
- docs/ftp_ryu_migration/reports/final_manager_nogo_report.md
- docs/ftp_ryu_migration/state/decision_log.md

## 수정 금지 파일
- 모든 source/header/CMake/build files
- FTP_Ryu preserved source
- legacy client.c
- protocol core

## 수행 작업
1. Supervisor NO_GO 결정을 그대로 보존한다.
2. 현재까지 완료된 작업과 미완료 blocker를 분리해서 정리한다.
3. “코드 이식 준비/빌드/정적 검증은 진행되었지만 deployment GO는 아니다”라고 명확히 쓴다.
4. 수정된 주요 파일 목록을 정리한다.
5. old FTP와 Ryu FTP의 현재 관계를 설명한다.
6. Ryu source 보존 여부와 adapter/wrapper 구조를 설명한다.
7. build 결과를 요약한다.
8. protocol/wire compatibility 상태를 요약한다.
9. runtime diagnostic 상태를 요약한다.
10. parser hardening 상태가 optional/unapplied인지 명시한다.
11. Supervisor NO_GO blocker를 표로 정리한다.
12. real GS/OBC에서 필요한 manual validation 절차를 정리한다.
13. rollback 방법을 정리한다.
14. 다음 작업 제안을 우선순위별로 정리한다.
15. final verdict는 반드시 NO_GO로 유지한다.

## 보고서 필수 섹션
1. Executive Summary
2. Final Verdict
3. What Was Completed
4. What Was Not Completed
5. Modified Files
6. Old FTP vs Ryu FTP Current State
7. Build and Static Validation
8. Protocol/Wire Status
9. Runtime/Connection Diagnostic Status
10. Parser Hardening Status
11. Supervisor NO_GO Reasons
12. Required Manual GS/OBC Validation
13. Rollback Plan
14. Recommended Next Steps
15. Final Manager Statement

## 출력 계약
마지막 20줄 안에 반드시 다음을 출력하라:

FINAL_MANAGER_STATUS: FINAL_REPORTED
FINAL_SUMMARY: Supervisor NO_GO preserved; final deployment remains blocked pending OBC wire/runtime validation and remaining hardening/removal work.
