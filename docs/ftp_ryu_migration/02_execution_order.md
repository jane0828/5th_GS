# Execution Order

Agent 00 Orchestrator는 각 Phase 시작 전과 완료 후 상태와 실제 산출물을 기계적으로 확인하고 다음 Agent를 dispatch한다. 기술적 충분성은 각 Phase 뒤 Agent 08 Phase Leader가 검토한다.

## Phase 1: 병렬 분석

Agent 01, 05, 06을 병렬 실행한다. 서로의 source를 변경하지 않으며 reports만 작성한다. 출력은 API migration map, wire compatibility matrix, runtime instrumentation/test plan이다. 세 보고서에서 dependency와 protocol blocker가 분류되어야 Phase 2로 간다.

Phase 1 종료 후 Orchestrator가 세 산출물의 존재와 형식을 확인하고 Phase Leader review를 dispatch한다. 일반 migration scope 내 Agent 02 진입에는 별도 사용자 confirmation이 필요하지 않다.

## Phase 1 Review

Agent 08이 분석 완전성, 별도 target 입력, protocol 위험, runtime 계획과 ownership을 검토한다. `PHASE_GO` 또는 `PHASE_GO_WITH_NOTES` 후 Agent 02가 열린다.

## Phase 2: Build/CMake

Agent 02가 Ryu 전용 static target을 구성한다. `client.c`와 `miman_ftprdp.cpp`는 그대로 둔다. clean build 명령, 실제 linked objects, 실패 로그를 남긴다. 별도 target build 성공 전 Phase 3 금지.

## Phase 2 Review

Agent 08이 별도 target, symbol 충돌, 기존 FTP 보존, 최소 CMake diff와 build 결과를 승인해야 Phase 3으로 간다.

## Phase 3: Literal Port

Agent 03이 원본 source/header를 별도 port 디렉터리에 보존하고 필요한 adapter만 별도 작성한다. 원본 체크섬과 diff를 남긴다. 원본과 port의 차이가 모두 설명되어야 Phase 4로 간다.

## Phase 3 Review

Agent 08이 원본 보존, adapter 범위, diff 기록과 old `client.c` 비혼입을 검토한다.

## Phase 4: Integration

Agent 04가 old path를 유지하면서 명시적인 Ryu test upload/download path를 추가한다. thread argument와 GUI path buffer는 worker 종료까지 유효한 owned copy여야 한다. callback/progress context lifetime도 검증한다. 기본 경로 전환은 Phase 5 결과와 Supervisor gate 전에 완료로 간주하지 않는다.

## Phase 4 Review

Agent 08이 Ryu call path, lifetime, GUI pointer, hidden mixture, 전환과 rollback을 검토한다.

## Phase 5: 병렬 검증

Agent 05는 capture/size/offset/endian 결론을 갱신하고, Agent 06은 transfer ID별 connection table을 0/5/10/30초에 수집한다. Agent 07은 parser hardening을 별도 optional patch로 준비한다. smoke 실패, connection 수 미복구, status 범위 위반은 Supervisor에 blocker로 전달한다.

## Phase 5 Review

Agent 08이 connection diagnostics, close logging, delayed close, parser validation과 optional/applied patch 구분을 검토한다. 승인 후 workflow는 `WAITING_FOR_SUPERVISOR`가 된다.

## Phase 6: Supervisor Final Audit

Orchestrator가 Agent 09를 dispatch한다. Supervisor는 Phase review를 반복하는 중간 검토자가 아니라 전체 결과의 독립 감사자이며 `GO`, `GO WITH RISKS`, `NO-GO`를 판정한다.

## Phase 7: Final Manager

Orchestrator가 Agent 10을 dispatch한다. Final Manager는 Supervisor 판정을 변경하지 않고 orchestration, Phase review, audit 결과를 사용자용 보고서로 정리한다.

## 병렬/순차 규칙

- 병렬 가능: 01/05/06 초기 분석, 05/06/07 검증.
- 순차 필수: Phase review를 사이에 둔 02 -> 03 -> 04, 이후 09 -> 10.
- 동일 파일을 소유하는 Agent는 동시에 실행하지 않는다.
- 각 phase는 입력, 출력, 성공 기준, gate 증거가 reports에 있어야 닫힌다.
- 각 Phase 사이의 상태 전환과 next prompt는 Orchestrator가 기록하고 기술 승인은 Phase Leader가 기록한다.
