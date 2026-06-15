# Orchestration Policy

## 역할 경계

- **Orchestrator:** 다음 작업을 정한다.
- **Phase Leader:** 중간 결과를 검토한다.
- **Supervisor:** 전체 결과를 독립 감사한다.
- **Final Manager:** 사용자에게 최종 보고한다.

Orchestrator는 기술 승인자가 아니다. Phase Leader는 최종 GO/NO-GO 감사자가 아니다. Supervisor는 중간 dispatch를 하지 않는다. Final Manager는 판정을 만들거나 뒤집지 않는다.

## Orchestrator가 필요한 이유

상태, dependency, 병렬/순차 실행, retry와 handoff를 일관되게 관리하여 사용자가 Agent마다 다음 지시를 다시 작성하지 않도록 한다. Orchestrator는 산출물 존재와 형식을 기계적으로 확인하고 기술 판단은 Phase Leader로 넘긴다.

## Phase Leader가 필요한 이유

Worker 산출물이 파일만 존재하는지와 다음 Phase에 기술적으로 충분한지는 다른 문제다. Phase Leader는 각 Phase 종료 시 evidence, file ownership, hidden mixture, 다음 Phase 입력 완전성을 검토하고 `PHASE_GO`, `PHASE_GO_WITH_NOTES`, `PHASE_RETRY_REQUIRED`, `PHASE_BLOCKED`를 판정한다.

## Supervisor와 Phase Leader의 차이

Phase Leader는 Phase 1~5의 중간 gate이며 retry 범위를 Orchestrator에 반환한다. Supervisor는 모든 Phase가 끝난 뒤 old/new 혼합, symbol/link, protocol, runtime, 위험과 rollback을 독립 감사하여 `GO`, `GO WITH RISKS`, `NO-GO`를 판정한다.

## Final Manager와 Supervisor의 차이

Supervisor는 최종 기술 판정을 내린다. Final Manager는 그 판정과 전체 산출물을 사용자 보고서로 정리할 뿐, 판정을 변경하거나 누락된 감사를 대신하지 않는다.

## 자동 진행 정책

- 일반적인 FTP_Ryu migration scope 안에서는 사용자 확인 없이 source-level 변경까지 진행할 수 있다.
- 기존 FTP build exclusion과 Ryu path final replacement도 migration scope 안이면 자동 진행할 수 있다.
- 단계별 git recovery 또는 동등한 rollback path와 변경 증거를 유지해야 한다.
- 각 Phase는 Phase Leader review를 통과해야 한다.
- 최종 결과는 Supervisor audit와 Final Manager report를 통과해야 한다.

## 병렬 실행

병렬 가능:
- Phase 1: Agent 01, 05, 06
- Phase 5: Agent 06 runtime update, Agent 07 hardening

병렬 금지:
- Build/CMake -> Port -> Integration
- 동일 파일 소유 Agent
- Phase Leader review와 다음 Phase Worker
- Supervisor와 미완료 Worker/Phase review
- Final Manager와 Supervisor 이전 작업

## 결과 검토와 RETRY_REQUIRED

Orchestrator는 expected outputs 존재, 비어 있지 않음, 필수 섹션, build/test 상태, 허용 파일 범위를 기계적으로 확인한다. 누락, 빈 결과, 명령/로그 부재, ownership 위반, handoff 부재는 `RETRY_REQUIRED`다.

기술적 정확성, 다음 Phase 입력 충분성, hidden mixture 가능성은 Phase Leader가 검토한다. 기존 FTP와 Ryu FTP가 hidden mixture 형태로 섞이면 migration 실패로 간주하고 rollback 또는 재작업 전에는 다음 Phase를 승인하지 않는다.

## Gate 기준

- **Phase Leader:** 해당 Phase Worker 완료 및 Orchestrator 1차 확인 후 실행.
- **Supervisor:** Phase 1~5가 모두 `APPROVED_BY_PHASE_LEADER`, 필수 evidence와 rollback 존재 시 실행.
- **Final Manager:** Supervisor 판정과 review report가 존재할 때 실행.

## 사용자 Confirmation 최소 조건

- 5th_GS 외부 unrelated subsystem 수정
- OBC server code 수정
- CSP/libcsp core behavior 변경
- protocol-incompatible 변경을 하면서 OBC-side update를 문서화하지 않는 경우
- git으로 복구 불가능한 영구 삭제
- Supervisor가 `NO-GO`인데 blocking issue 해결 없이 강행하려는 경우

일반 migration scope 내 CMake, port, integration, old FTP build exclusion, Ryu final replacement는 위 조건에 해당하지 않으면 별도 확인 없이 진행 가능하다.

## Source 수정과 Review 역할 분리

Worker만 허용 범위 내 동작 코드를 수정할 수 있다. Orchestrator, Phase Leader, Supervisor, Final Manager는 review-only이며 source/header/CMake/build script를 수정하지 않는다. 각 Agent는 역할 밖 작업을 직접 수행하지 않고 handoff note를 작성한다.

