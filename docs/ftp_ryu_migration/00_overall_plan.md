# FTP_Ryu Literal Port Overall Plan

## 1. 목표

`C:\Users\energ\Desktop\ACL\GS\FTP_Ryu\client`의 FTP client를 `5th_GS`에 가능한 한 원형 그대로 이식하고, CSP connection 고갈, RDP delayed close, status reply 해석 위험을 추적 가능한 단계로 검증한다. 최종 목표는 `miman_ftprdp.cpp`가 Ryu API를 사용하고 old FTP client가 실행 파일에서 제외된 상태에서 upload/download가 안정적으로 반복되는 것이다.

이번 문서 작성 단계에서는 source, header, CMake, build script를 수정하지 않는다.

## 2. Multi-Agent로 분리하는 이유

전체 이식을 한 번에 수행하면 build 문제, API 전환, wire incompatibility, resource leak가 한 변경에 섞여 원인 분리와 rollback이 어렵다. 분석, 독립 빌드, 원본 보존 이식, test path 연동, protocol 검증, runtime 진단, optional hardening, 독립 검토를 분리하면 각 단계의 증거와 실패 지점을 보존할 수 있다.

## 3. Literal Port 정의와 원칙

- FTP_Ryu client source/header를 `5th_GS`의 별도 위치에 보존한다.
- 기존 `libftp_client/src/client.c` 안에 Ryu 코드를 섞지 않는다.
- 기존 global FTP state에 의존하지 않고 transfer별 local state를 유지한다.
- Ryu client를 별도 static library target으로 먼저 빌드한다.
- `miman_ftprdp.cpp`는 old path를 즉시 제거하지 않고 명시적인 Ryu test upload/download path를 거친 뒤 전환한다.
- protocol compatibility 확인 전 기존 FTP를 제거하지 않는다.
- Ryu 원본 변경이 필요하면 원본 대비 diff, 변경 이유, 영향, 승인자를 기록한다.
- `gs_ftp_state_t`, `ftpavailable`, `ftp_done`, old `gs_ftp_*` symbol의 source 및 binary 잔존 여부를 확인한다.
- runtime connection table, `csp_connect`, `csp_close`, RDP close state를 진단한다.
- hardening은 literal baseline과 분리된 optional patch로 유지한다.

## 4. 단계적 전환과 Rollback

1. old FTP는 유지하고 Ryu client만 별도 target으로 빌드한다.
2. 원본 보존 사본과 필요한 adapter를 별도 디렉터리에 둔다.
3. old path와 구분되는 Ryu test path를 추가한다.
4. wire/runtime 검증 후 기본 호출을 Ryu로 전환한다.
5. 최종 검토가 끝난 뒤에만 old client를 build/link에서 제외한다.
6. `nm BEE-1000 | grep -E 'ftp|gs_ftp|ftpnew'`와 source `rg`로 old symbol 잔존을 확인한다.

Rollback은 단계별 commit, old path 유지 기간, build option 또는 명시적 test selector, 원본 체크섬/diff, 이전 binary와 build 명령 보존으로 가능하게 한다. 최종 old FTP 제거도 별도 commit으로 수행하여 되돌릴 수 있어야 한다.

## 5. Agent 역할

- **A Structure Analysis:** 수정 없이 API, dependency, struct, macro, symbol 차이를 분석하고 `migration_map.md`, `api_mapping.md`, `dependency_report.md`를 작성한다.
- **B Build/CMake:** Ryu client를 별도 static library target으로 빌드한다. 기존 `client.c`와 `miman_ftprdp.cpp`는 수정하지 않는다.
- **C Ryu Client Port:** source/header를 원본에 가깝게 보존하고 adapter는 별도 파일로 둔다. 변경 diff와 이유를 기록한다.
- **D Integration:** old path를 유지한 채 Ryu test upload/download path를 추가하고 thread 인자, GUI buffer, callback, progress lifetime을 검토한다. 검증 후 최종 전환을 담당한다.
- **E Protocol/Wire:** 세 구현의 packet type, size, offset, endian, packing, status `entries/count/next`를 비교한다. core source는 수정하지 않는다.
- **F Runtime/Connection:** connect/close, close 반환값, connection table, transfer id, `RDP_CLOSE_WAIT`를 완료 직후와 5/10/30초 후 관측한다.
- **G Parser Hardening:** literal port 후 packet length와 status 범위 검증을 optional patch로 준비한다.
- **H Supervisor:** 수정 없이 결과와 old/new 혼합 여부를 검토하여 `GO`, `GO WITH RISKS`, `NO-GO`를 판정한다.
- **I Final Manager:** 모든 증거를 종합해 사용자용 최종 보고서, 위험, OBC 확인 사항, rollback을 작성한다.

## 6. Phase Gates

| Phase | 실행 | 입력 | 출력 | 성공 기준 | 다음 단계 조건 |
|---|---|---|---|---|---|
| 1 분석, 병렬 | A, E, F | old client, Ryu client, OBC V2.1, CSP 설정 | mapping, wire report, diagnostic plan | API/dependency/wire/runtime 관측점 식별 | blocking incompatibility가 분류됨 |
| 2 Build, 순차 | B | Phase 1 dependency 결과 | 별도 static target, build log | old 호출부 수정 없이 Ryu target 단독 build | target과 dependency가 재현 가능 |
| 3 Port, 순차 | C | build target, 원본 | 보존 사본, adapter, diff report | 원본 변경 최소, 체크섬/diff 기록 | public API가 link 가능 |
| 4 Integration, 순차 | D | Ryu library/API | 구분된 Ryu test path | old path 유지, upload/download test 진입 가능 | wire 승인과 smoke test 준비 |
| 5 검증/hardening, 병렬 | E/F/G | test binary, captures | wire/runtime 결과, optional patch | close 상태와 parser 위험이 증거화됨 | 치명적 leak/incompatibility 없음 |
| 6 Review | H | 모든 산출물 | supervisor review | checklist 완료 및 판정 | NO-GO가 아니거나 위험 수용 승인 |
| 7 Report | I | supervisor 판정 | final report | 사용자 의사결정에 필요한 증거/rollback 포함 | 사용자 승인 후 final cleanup |

## 7. 완료 판정

- Ryu client가 별도 target으로 빌드된다.
- old 코드와 Ryu 코드가 한 source에 섞이지 않는다.
- final 단계에서 기존 `client.c`가 build/link 제외된다.
- `gs_ftp_state_t`, `ftpavailable`, `ftp_done`, old `gs_ftp_*` 사용 여부가 보고된다.
- `nm BEE-1000 | grep ftp` 결과가 검토된다.
- `miman_ftprdp.cpp`가 최종적으로 Ryu API를 호출한다.
- 전환 중 old/Ryu test path가 명확히 구분된다.
- status reply wire format과 OBC packet type/`sizeof`/`offsetof`/endian/packing이 확인된다.
- `csp_close` 이후 connection table을 즉시, 5, 10, 30초에 확인한다.
- malformed status 최소 방어가 적용되거나 명시적 잔여 위험으로 승인된다.
- 전체 build, upload/download smoke test가 성공한다.
- 반복 upload/download 횟수, 간격, 실패 기준, connection baseline을 포함한 test 계획이 있다.
- 단계별 rollback이 재현 가능하다.

