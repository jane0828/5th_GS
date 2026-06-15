# Phase Leader Checkpoints

각 항목은 확인 방법, 필요한 증거, 실패 시 retry Agent를 함께 판정한다.

## Phase 1 Analysis Review

| 항목 | 확인 방법 | 필요한 증거 | 실패 시 Retry |
|---|---|---|---|
| Structure Analysis 완전성 | old/Ryu API와 state/symbol 표 대조 | migration/API/dependency 3개 report | Agent 01 |
| API mapping 충분성 | upload/download/callback/settings 대응 추적 | call mapping과 unresolved 목록 | Agent 01 |
| dependency 누락 | include/library/config/type 목록 검토 | dependency graph와 build inputs | Agent 01 |
| protocol 위험 | packet type/size/offset/endian 표 검토 | wire matrix, OBC verdict, raw decode 계획 | Agent 05 |
| 별도 target build 정보 | source/include/link 목록이 재현 가능한지 확인 | target input allowlist | Agent 01 또는 05 |
| runtime 계획 | connect/close ID와 0/5/10/30초 계획 검토 | diagnostic plan과 반복 기준 | Agent 06 |

다음 Phase 조건: 모든 필수 보고서 존재, blocker 분류, 별도 target 입력 확정, protocol incompatibility가 명시됨.

## Phase 2 Build/CMake Review

| 항목 | 확인 방법 | 필요한 증거 | 실패 시 Retry |
|---|---|---|---|
| 별도 target | CMake diff와 archive 확인 | target 이름, verbose build, archive listing | Agent 02 |
| symbol 충돌 | `nm`/link map 비교 | `ftpnew_*` 제공, unexpected old symbol 없음 | Agent 02 |
| 기존 FTP 보존 | git diff와 source hash 확인 | `client.c` 미변경 증거 | Agent 02 |
| 최소 CMake 변경 | 변경 line과 dependency 검토 | CMake diff 설명 | Agent 02 |
| build result | clean rebuild 재현 | 명령, exit status, log | Agent 02 |

다음 Phase 조건: 독립 target clean build, old/new symbol 충돌 없음, 기존 기능/build 경로 보존.

## Phase 3 Ryu Client Port Review

| 항목 | 확인 방법 | 필요한 증거 | 실패 시 Retry |
|---|---|---|---|
| 원본 보존 | 원본/port checksum 및 diff 비교 | checksum manifest | Agent 03 |
| adapter 범위 | adapter와 원본 수정 분리 검토 | adapter 목록과 이유 | Agent 03 |
| diff 기록 | 모든 deviation이 설명됐는지 확인 | source diff report | Agent 03 |
| old client 혼입 | `rg`, diff, object listing | `client.c` 내 Ryu 코드 없음 | Agent 03 |
| port build | target 재build | build log와 symbols | Agent 03 |

다음 Phase 조건: literal baseline 추적 가능, deviation 설명 완료, adapter 최소화, old client 혼입 없음.

## Phase 4 Integration Review

| 항목 | 확인 방법 | 필요한 증거 | 실패 시 Retry |
|---|---|---|---|
| `miman_ftprdp.cpp` call path | call graph와 `rg gs_ftp_/ftpnew_` | old/test/final path 표 | Agent 04 |
| thread argument lifetime | worker 생성부터 종료까지 ownership 추적 | lifetime review | Agent 04 |
| GUI pointer safety | path/buffer deep copy 확인 | source diff와 test evidence | Agent 04 |
| hidden mixture | source/link symbols 및 state 의존 검사 | grep/nm/link map | Agent 04 또는 02 |
| 전환/rollback | old exclusion과 Ryu replacement 경계 확인 | commit/diff/rollback 절차 | Agent 04 |

다음 Phase 조건: intended Ryu API 호출, dangling pointer 위험 없음, hidden mixture 없음, build/smoke와 rollback 가능.

## Phase 5 Hardening/Diagnostics Review

| 항목 | 확인 방법 | 필요한 증거 | 실패 시 Retry |
|---|---|---|---|
| connection table | baseline/peak/0/5/10/30초 비교 | timestamped table dumps | Agent 06 |
| `csp_close` logging | transfer ID별 close 결과 대조 | connect/close log | Agent 06 |
| RDP delayed close | state 체류 시간 측정 | `RDP_CLOSE_WAIT` timeline | Agent 06 |
| parser validation | length/entries/range/overflow test | validation matrix와 malformed results | Agent 07 |
| literal 원칙 | baseline과 hardening diff 비교 | separate optional/applied patch | Agent 07 |
| patch 상태 | 실제 binary 반영 여부 확인 | build/link evidence와 status | Agent 07 또는 02 |

Supervisor 진입 조건: connection 회복/잔여 위험 판정, parser 적용 상태 명확, optional/applied 구분, 모든 Phase review note와 rollback 증거 존재.

