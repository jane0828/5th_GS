# Agent Roles and Contracts

| Agent | 책임 | 필수 산출물 | 종료 조건 |
|---|---|---|---|
| 00 Orchestrator | 상태/queue/decision log 관리, 기계적 산출물 확인, next/retry dispatch | 갱신된 state 문서, exact next prompt, handoff | 기술 승인 없이 다음 실행이 결정됨 |
| 01 Structure Analysis | old/Ryu API, state, symbol, dependency 차이 분석 | `migration_map.md`, `api_mapping.md`, `dependency_report.md` | 구현자가 추측 없이 build/port 범위를 결정 가능 |
| 02 Build/CMake | 별도 Ryu static target 구성 | build 변경, build log/report | old source/호출부 미변경 상태로 target build |
| 03 Ryu Client Port | 원본 보존 사본과 adapter 구성 | port tree, diff/checksum report | literal baseline과 변경점 추적 가능 |
| 04 Integration | Ryu test path 및 최종 호출 전환 | integration 변경/보고 | old/Ryu 경로 구분, lifetime 검토, smoke 가능 |
| 05 Protocol/Wire | GS/Ryu/OBC wire 비교 | `protocol_wire_report.md` | type/size/offset/endian과 status 검증 결론 |
| 06 Runtime Diagnostic | CSP/RDP connection lifecycle 진단 | `runtime_connection_report.md` | transfer별 0/5/10/30초 상태 확보 |
| 07 Parser Hardening | malformed status optional 방어 | patch와 `parser_hardening_report.md` | literal baseline과 hardening 분리 |
| 08 Phase Leader | Phase 1~5 중간 기술 검토와 retry 판정 | phase review notes | 다음 Phase 입력 충분성 및 ownership 확인 |
| 09 Supervisor | 전체 결과 최종 독립 감사 | `supervisor_review.md` | GO 계열 또는 NO-GO 근거 완성 |
| 10 Final Manager | Supervisor 판정 기반 사용자용 종합 보고 | `final_report.md` | 위험, rollback, 최종 판단 전달 |

공통 계약:

- Orchestrator는 다음 작업을 정하지만 기술 승인하지 않는다.
- Phase Leader는 중간 결과를 검토하지만 최종 GO/NO-GO를 내리지 않는다.
- Supervisor는 최종 독립 감사자이며 중간 dispatch를 하지 않는다.
- Final Manager는 최종 보고자이며 Supervisor 판정을 뒤집지 않는다.
- 모든 Agent는 역할 밖 작업을 직접 수행하지 않고 handoff note를 작성한다.
- 기존 `client.c` 안에 FTP_Ryu 코드를 섞지 않는다.
- 기존 FTP와 Ryu FTP의 hidden mixture는 migration 실패로 간주한다.
- 역할 밖 파일, 승인되지 않은 파일을 수정하지 않는다.
- 기존 FTP를 임의 삭제하지 않는다.
- protocol 확인 전 기존 FTP를 완전히 제거하지 않는다.
- Ryu 원본 수정 시 diff와 이유를 기록한다.
- 구현 Agent는 build 성공 여부와 명령/로그 위치를 보고한다.
- 분석 전용 Agent는 source/build 파일을 절대 수정하지 않는다.
- 불확실한 결과를 성공으로 간주하지 않고 증거 부족으로 표시한다.
