# FTP_Ryu Migration Final Report

## 1. 작업 요약
[목표, 기간, Supervisor 판정]

## 2. Orchestration Summary
[Phase별 dispatch, retry/blocked 이력, 사용자 confirmation, Orchestrator -> Phase Leader -> Supervisor -> Final Manager 판단 흐름]

## 3. Phase Leader Review Summary
[Phase 1~5 판정, retry, ownership issue, 누적 기술 위험]

## 4. Supervisor Final Audit Summary
[독립 findings, symbol/build/wire/runtime/risk audit, GO/GO WITH RISKS/NO-GO]

## 5. 이식 범위
[포함/제외 기능과 literal baseline]

## 6. 수정 파일 목록
| 파일 | 담당 | 변경 목적 | 원본 대비 차이 |
|---|---|---|---|

## 7. 기존 FTP와 Ryu FTP의 관계
[test 전환 경로, final build/link 상태, old symbol 검사]

## 8. Ryu 원본 보존 여부
[원본 commit/checksum, diff, 각 변경 이유]

## 9. 빌드 결과
[환경, 명령, target, 결과, link map/nm]

## 10. 테스트 결과
[upload/download smoke, CRC/size, 반복 횟수, 실패]

## 11. Connection Pool 진단 결과
[baseline/peak/0/5/10/30초, close 반환값, RDP state]

## 12. Status Reply/Parser 검증 결과
[length, entries, next/count, total/complete, malformed test]

## 13. OBC Server 호환성 판단
[packet type, sizeof, offsetof, packing, endian, 확인 필요 사항]

## 14. 남은 위험
| 위험 | 영향 | 완화 | 소유자 | 기한 |
|---|---|---|---|---|

## 15. Rollback 방법
[commit/tag, build option, old binary, 실행/검증 절차]

## 16. 다음 작업 제안
[우선순위와 gate]

## 17. 최종 판단
`GO` / `GO WITH RISKS` / `NO-GO`

[판정 근거와 운영 제한]
