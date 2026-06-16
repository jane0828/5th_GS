# Agent 08 Prompt: Phase 4 Targeted Re-review After Agent 04b

## Agent 이름
Phase Leader Targeted Re-review Agent

## 목표
Phase 4 Integration에서 blocking 되었던 GUI/autopilot pthread argument ownership 문제가 Agent 04b에 의해 해결되었는지 targeted re-review한다.

## 배경
이전 Phase 4 review는 PHASE_BLOCKED였다. 이유는 src/miman_imgui.cpp 및 src/miman_autopilot.cpp의 GUI/autopilot FTP pthread creation sites가 Ryu runtime path에서도 shared State.ftplistup[NowFTP] 또는 shared ftpinfo pointer를 직접 pthread_create()에 넘길 수 있었기 때문이다.

Agent 04b는 src/miman_imgui.cpp와 src/miman_autopilot.cpp에 explicit State.ftp_version == 3 Ryu branches를 추가하고, Ryu path에서 pthread_create() 전에 miman_ftp_create_worker_arg()를 호출하도록 수정했다고 보고했다.

## 입력 파일
- src/miman_imgui.cpp
- src/miman_autopilot.cpp
- src/miman_ftprdp.cpp
- src/miman_ftprdp_integration.h
- docs/ftp_ryu_migration/reports/thread_arg_ownership_report.md
- docs/ftp_ryu_migration/reports/phase_4_review.md
- docs/ftp_ryu_migration/reports/ftprdp_integration_report.md
- docs/ftp_ryu_migration/state/decision_log.md

## 수정 가능 파일
- docs/ftp_ryu_migration/reports/phase_4_targeted_re_review.md
- docs/ftp_ryu_migration/state/decision_log.md

## 수정 금지 파일
- 모든 source/header/CMake/build files
- FTP_Ryu preserved source
- legacy client.c
- protocol core

## 수행 작업
1. Agent 04b diff를 검토하라.
2. src/miman_imgui.cpp와 src/miman_autopilot.cpp에서 Ryu path가 State.ftp_version == 3으로 명확히 분리되어 있는지 확인하라.
3. Ryu path가 pthread_create() 전에 miman_ftp_create_worker_arg()를 호출하는지 확인하라.
4. Ryu pthread_create()에 &State.ftplistup[NowFTP] 또는 shared ftpinfo*가 직접 전달되지 않는지 확인하라.
5. pthread_create() 실패 시 miman_ftp_destroy_worker_arg()가 호출되는지 확인하라.
6. old FTP V1/V2 rollback path가 유지되는지 확인하라.
7. build 성공 evidence를 확인하라.
8. Phase 5 runtime/diagnostic 단계로 진입 가능한지 판단하라.
9. docs/ftp_ryu_migration/reports/phase_4_targeted_re_review.md를 작성하라.
10. docs/ftp_ryu_migration/state/decision_log.md에 review 결과를 append하라.

## 판정 기준
다음 중 하나를 마지막 20줄 안에 반드시 출력하라.

- PHASE_DECISION: PHASE_GO
- PHASE_DECISION: PHASE_GO_WITH_NOTES
- PHASE_DECISION: PHASE_RETRY_REQUIRED
- PHASE_DECISION: PHASE_BLOCKED

바로 아래에 PHASE_SUMMARY: 한 줄 요약을 작성하라.

## 주의사항
- source를 수정하지 마라.
- build를 새로 수행하지 않아도 되지만, Agent 04b build evidence를 검토하라.
- 실제 OBC runtime smoke가 수행되지 않았으면 그 위험은 Phase 5/Runtime Agent handoff로 남겨라.
- 이번 review는 Phase 4 blocker였던 thread argument ownership에 집중하라.
