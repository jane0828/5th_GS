# File Ownership

| Agent | 수정 가능 | 수정 금지 |
|---|---|---|
| 00 Orchestrator | state, orchestration/retry/handoff 문서 | 모든 source/header/CMake/build script |
| 01 Structure | `docs/ftp_ryu_migration/reports/*analysis*`, mapping reports | 모든 source/header/CMake/build script |
| 02 Build/CMake | 승인된 새 `libftp_client_ryu` 디렉터리의 CMake, 관련 상위 CMake 최소 변경, build report | `src/miman_ftprdp.cpp`, 기존 `libftp_client/src/client.c` |
| 03 Ryu Port | 새 Ryu port 디렉터리, 별도 adapter, diff report | 기존 `client.c`, `miman_ftprdp.cpp` |
| 04 Integration | `src/miman_ftprdp.cpp`, 필요한 별도 wrapper header/source, integration report | FTP_Ryu 보존 source 직접 수정 |
| 05 Protocol | reports, 독립 diagnostic snippet | core source와 protocol 동작 |
| 06 Runtime | reports, 승인된 diagnostic patch/logging wrapper | protocol semantics 변경 |
| 07 Hardening | 별도 optional validation patch와 report | 명시 승인 없는 Ryu 원본 직접 변경 |
| 08 Phase Leader | phase review notes, decision log review entry | 모든 source/header/CMake/build script, Worker 산출물 직접 수정 |
| 09 Supervisor | supervisor review 문서 | 모든 source/header/CMake/build script, 중간 dispatch/state 조작 |
| 10 Final Manager | final report 문서 | 모든 source/header/CMake/build script, 중간 검토, Supervisor 판정 변경 |

동일 파일 동시 수정은 금지한다. 범위 추가가 필요하면 작업을 멈추고 파일, 이유, 최소 변경, 영향, 승인 필요 여부를 보고한다.

Orchestrator, Phase Leader, Supervisor, Final Manager는 review-only Agent다. 이들은 실제 동작 코드나 CMake를 수정하지 않고 역할 밖 작업은 handoff note로 넘긴다.
