# Risk Register

가능성/영향도: L(낮음), M(중간), H(높음).

| 위험 설명 | 가능성 | 영향 | 탐지 방법 | 완화 방법 | 담당 |
|---|---:|---:|---|---|---|
| old `client.c`와 Ryu 코드 혼합 | M | H | diff, 파일 경계 review | 별도 port/library, 혼합 금지 | 03/08 |
| 동일/유사 symbol 충돌 | M | H | linker error, `nm` | `ftpnew_*` namespace, link map | 02 |
| 일부 호출만 Ryu, 일부 old 유지 | H | H | `rg gs_ftp_`, call graph | test selector와 최종 symbol gate | 04/08 |
| OBC protocol 불일치 | M | H | capture, type/size/offset 비교 | 제거 전 compatibility gate | 05 |
| status parser 길이/범위 검증 부족 | H | H | malformed packet test | optional validated parser patch | 07 |
| RDP delayed close 지속 | H | H | 0/5/10/30초 table | close 결과/timeout/CSP 설정 분석 | 06 |
| `CSP_CONN_MAX` 부족 | M | H | config 및 peak connection 수 | leak 제거 후 sizing 검토 | 06 |
| connection pool 고갈 | H | H | 반복 test, connect 실패 | 단일 cleanup, close 관측, rate control | 06/03 |
| thread argument lifetime 오류 | M | H | sanitizer/review | worker-owned immutable copy | 04 |
| GUI buffer pointer dangling | M | H | UI/worker lifetime review | 문자열 deep copy | 04 |
| global flag와 local state 충돌 | M | H | symbol/state map | Ryu local state, adapter 최소화 | 01/03 |
| `csp_packet_t` ownership 오해 | M | H | API contract/error path review | alloc/free owner 표 작성 | 01/06 |
| `csp_transaction_persistent(...,-1)` buffer overflow | M | H | call-site와 reply buffer size 비교 | bounded input length | 05/07 |
| chunk size mismatch | M | H | request/capture/config 비교 | 양단 최대값 및 test matrix | 05 |
| endian 변환 중복/누락 | H | H | raw hex와 decoded 값 비교 | field별 conversion matrix | 05 |
| Ryu 원본 수정으로 literal 성격 상실 | M | M | checksum/diff | adapter 우선, 변경 승인 | 03/08 |
| build 성공하나 old library 링크 | M | H | verbose link, `nm`, map | object-level allowlist | 02/08 |
| OBC V2.1이 `entries` 변환 후 loop하여 순서 버그 | H | H | server source line review/capture | OBC patch 필요 여부 별도 판정 | 05 |

