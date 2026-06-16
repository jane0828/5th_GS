# Agent 08 Prompt: Phase 5 Runtime-Limited Targeted Re-review

## Agent 이름
Phase 5 Runtime-Limited Targeted Re-review Agent

## 목표
Phase 5에서 요구되었던 live Ryu runtime diagnostic evidence가 현재 Codex 환경에서 수행 불가능한 경우, Agent 06의 runtime-limited diagnostic handoff가 충분한지 targeted re-review한다.

## 배경
이전 Phase 5 review는 PHASE_RETRY_REQUIRED였다. 이유는 live Ryu transfer evidence가 없었기 때문이다:
- connection table dumps
- close-return correlation
- RDP close-wait timeline
- pool exhaustion sequence

이후 Agent 06 retry는 live OBC/runtime transfer를 조작하거나 허위로 주장하지 않고, runtime-limited diagnostic readiness, manual live test procedure, expected log schema, open runtime risks를 정리하는 방향으로 수행되었다.

## 입력 파일
- docs/ftp_ryu_migration/reports/runtime_connection_report.md
- docs/ftp_ryu_migration/reports/phase_5_review.md
- docs/ftp_ryu_migration/reports/parser_hardening_report.md
- docs/ftp_ryu_migration/reports/thread_arg_ownership_report.md
- docs/ftp_ryu_migration/state/decision_log.md
- latest Agent 06 runtime-limited output under docs/ftp_ryu_migration/logs

## 수정 가능 파일
- docs/ftp_ryu_migration/reports/phase_5_runtime_limited_re_review.md
- docs/ftp_ryu_migration/state/decision_log.md

## 수정 금지 파일
- 모든 source/header/CMake/build files
- FTP_Ryu preserved source
- legacy client.c
- protocol core

## 수행 작업
1. Agent 06 runtime-limited retry output을 검토하라.
2. live runtime evidence가 실제로 수행되지 않았는지, 그리고 그 사실이 정직하게 명시되었는지 확인하라.
3. runtime diagnostic readiness가 충분한지 확인하라.
4. manual live test procedure가 충분히 구체적인지 확인하라.
5. expected log markers, connection table schedule, close-return correlation, RDP close-wait observation, CSP_CONN_MAX/pool exhaustion criteria가 포함되었는지 확인하라.
6. Agent 07 parser hardening은 optional/unapplied hardening으로 충분한지 기존 review를 유지하라.
7. Phase 6 Supervisor로 넘길 수 있는지 판단하라.
8. live OBC smoke 미수행은 반드시 open risk로 유지하라.
9. docs/ftp_ryu_migration/reports/phase_5_runtime_limited_re_review.md를 작성하라.
10. docs/ftp_ryu_migration/state/decision_log.md에 결과를 append하라.

## 판정 기준
이번 targeted review는 hardware/runtime unavailability를 이유로 전체 migration을 무조건 block하지 않는다. 대신 다음 기준으로 판단한다.

- Diagnostic readiness, manual procedure, open risk documentation이 충분하면 PHASE_GO_WITH_NOTES.
- Runtime-limited report가 불충분하거나 live evidence를 허위로 주장하면 PHASE_RETRY_REQUIRED 또는 PHASE_BLOCKED.

마지막 20줄 안에 반드시 다음 중 하나를 출력하라:

- PHASE_DECISION: PHASE_GO
- PHASE_DECISION: PHASE_GO_WITH_NOTES
- PHASE_DECISION: PHASE_RETRY_REQUIRED
- PHASE_DECISION: PHASE_BLOCKED

바로 아래에 PHASE_SUMMARY: 한 줄 요약을 작성하라.

## 주의사항
- source를 수정하지 마라.
- 실제 OBC runtime smoke가 수행되지 않았으면 open risk로 남겨라.
- Supervisor에게 live runtime validation is still pending이라고 넘겨라.
