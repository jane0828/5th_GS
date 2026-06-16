# Agent 04b Prompt: GUI/Autopilot Thread Argument Ownership Agent

## Agent 이름
GUI/Autopilot Thread Argument Ownership Agent

## 목표
Phase 4 Integration review에서 지적된 blocker를 해결한다. Ryu FTP runtime path가 `pthread_create()` 전에 worker-owned argument를 생성하도록 `src/miman_imgui.cpp` 및 `src/miman_autopilot.cpp`의 FTP thread creation sites를 수정한다.

## 배경
Phase 4 static integration은 acceptable로 판정되었지만, Phase 5 runtime entry는 blocked 상태이다. 이유는 GUI/autopilot pthread creation sites가 여전히 mutable shared state인 `State.ftplistup[NowFTP]` 또는 그 주소를 직접 worker thread에 넘길 가능성이 있기 때문이다.

Agent 04는 `miman_ftp_create_worker_arg()` 및 `miman_ftp_destroy_worker_arg()`를 추가했고, Ryu worker-owned argument path를 제공했다. 그러나 실제 `pthread_create()` 호출부는 `src/miman_imgui.cpp` 및 `src/miman_autopilot.cpp`에 있어 Agent 04의 기존 소유 범위 밖이었다.

이번 Agent 04b는 해당 creation-site ownership만 명시적으로 부여받는다.

## 입력 경로
- Project root: `C:\Users\energ\Desktop\ACL\GS\5th_GS`
- WSL root: `/mnt/c/Users/energ/Desktop/ACL/GS/5th_GS`
- Main files:
  - `src/miman_imgui.cpp`
  - `src/miman_autopilot.cpp`
  - `src/miman_ftprdp.cpp`
  - `src/miman_ftprdp_integration.h`
- Reports/logs:
  - `docs/ftp_ryu_migration/reports/phase_4_review.md`
  - `docs/ftp_ryu_migration/reports/ftprdp_integration_report.md`
  - `docs/ftp_ryu_migration/logs/20260616_140131_agent_08_phase_leader.out`
  - `docs/ftp_ryu_migration/logs/20260616_134746_agent_04_ftprdp_integration.out`

## 수정 가능 파일
- `src/miman_imgui.cpp`
- `src/miman_autopilot.cpp`
- 필요한 경우 `src/miman_ftprdp.cpp`
- 필요한 경우 `src/miman_ftprdp_integration.h`
- `docs/ftp_ryu_migration/reports/thread_arg_ownership_report.md`
- `docs/ftp_ryu_migration/state/decision_log.md`

## 수정 금지 파일
- FTP_Ryu preserved source
- `lib/gscsp/lib/libftp_client/src/client.c`
- protocol definition core
- 기존 old FTP source 내부
- unrelated GUI/autopilot logic
- unrelated CMake/build files unless absolutely required, in which case stop and report instead of modifying

## 수행 작업

1. `src/miman_imgui.cpp`와 `src/miman_autopilot.cpp`에서 FTP 관련 `pthread_create()` 호출부를 찾는다.
2. Ryu FTP worker 또는 Ryu smoke path를 호출하는 creation site가 shared `State.ftplistup[NowFTP]`, `&State.ftplistup[NowFTP]`, 또는 shared `ftpinfo*`를 직접 넘기지 않도록 수정한다.
3. Ryu path에서는 반드시 `pthread_create()` 전에 `miman_ftp_create_worker_arg()`를 호출하여 worker-owned argument를 생성한다.
4. `pthread_create()`에는 worker-owned argument pointer를 넘긴다.
5. `pthread_create()` 실패 시 반드시 `miman_ftp_destroy_worker_arg()`로 해제한다.
6. thread detach/join 정책을 기존 코드와 일치시킨다.
7. old FTP path는 rollback/default path로 유지한다. old FTP path를 Ryu path로 임의 치환하지 않는다.
8. old FTP path가 기존 shared-state pointer를 사용하는 것은 이번 scope에서 고치지 않는다. 단, Ryu path와 old path가 명확히 구분되어야 한다.
9. Ryu path가 `ftp_ryu_uplink_onorbit()` / `ftp_ryu_downlink_onorbit()` 또는 명시적 Ryu worker를 호출하는지 확인한다.
10. build를 수행하고 결과를 보고한다.
11. static validation을 수행한다:
    - Ryu pthread creation site가 `miman_ftp_create_worker_arg()`를 사용함
    - Ryu pthread creation site가 shared `State.ftplistup[NowFTP]` 주소를 직접 넘기지 않음
    - `pthread_create()` failure path에서 destroy 호출이 있음
    - old FTP path는 유지됨
12. `docs/ftp_ryu_migration/reports/thread_arg_ownership_report.md`를 작성한다.
13. `docs/ftp_ryu_migration/state/decision_log.md`에 Agent 04b 수행 결과를 append한다.

## 수행하지 말아야 할 작업
- 기존 `client.c` 안에 FTP_Ryu 코드를 섞지 말 것.
- FTP_Ryu preserved source를 직접 수정하지 말 것.
- old FTP를 삭제하지 말 것.
- protocol 호환성 확인 전 기존 FTP 완전 제거 금지.
- old FTP path를 임의로 Ryu path로 일괄 치환하지 말 것.
- `src/miman_imgui.cpp` / `src/miman_autopilot.cpp`의 FTP thread creation 관련 부분 밖을 불필요하게 수정하지 말 것.
- unrelated formatting churn 금지.
- build 실패를 숨기지 말 것.
- runtime OBC smoke를 실제 수행했다고 주장하지 말 것. 실제 수행하지 않았다면 static/build validation이라고 명확히 써라.

## 산출물
- 수정 diff
- `docs/ftp_ryu_migration/reports/thread_arg_ownership_report.md`
- `docs/ftp_ryu_migration/state/decision_log.md` append
- build 결과
- static evidence:
  - grep/rg evidence for pthread creation sites
  - `miman_ftp_create_worker_arg()` usage
  - `miman_ftp_destroy_worker_arg()` failure cleanup
  - Ryu/old path separation

## 성공 기준
- Ryu FTP runtime thread creation path가 `pthread_create()` 전에 worker-owned argument를 만든다.
- Ryu `pthread_create()`에 shared `State.ftplistup[NowFTP]` 주소가 직접 전달되지 않는다.
- `pthread_create()` 실패 시 worker-owned argument가 해제된다.
- old FTP rollback path가 유지된다.
- build가 성공한다.
- Phase 4 targeted re-review에 필요한 evidence가 report에 정리된다.

## 실패 시 보고 방식
수정 불가, call-site ambiguity, build failure, missing helper declaration, symbol/link error가 발생하면 다음을 보고한다.

- 어떤 파일/라인에서 막혔는지
- 어떤 호출부가 ambiguous한지
- 어떤 ownership 위험이 남았는지
- 어떤 build error가 발생했는지
- rollback 가능 상태인지

## 다음 Agent에게 넘겨야 할 정보
- Phase Leader에게 targeted Phase 4 re-review를 요청할 수 있도록 creation-site diff와 argument lifetime evidence를 넘긴다.
- Runtime Agent에게 Ryu transfer ID/log point와 cleanup observation point를 넘긴다.
- Supervisor에게 old/Ryu path separation, worker-owned arg adoption, remaining runtime/OBC risks를 넘긴다.

## Runner Output Contract

작업 완료 시 마지막 20줄 안에 반드시 다음 중 하나를 출력하라:

- `AGENT_STATUS: DONE`
- `AGENT_STATUS: RETRY_REQUIRED`
- `AGENT_STATUS: FAILED`

status marker 바로 아래에 `AGENT_SUMMARY:` 한 줄 요약을 작성하라.
