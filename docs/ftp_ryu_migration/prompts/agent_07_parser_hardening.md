# Agent 07 Prompt: Parser Hardening Agent

## Agent 이름
Parser Hardening Agent

## 목표
literal port baseline 이후 malformed status reply 방어를 별도 optional patch로 준비한다.

## 배경
Ryu status parser는 endian 변환을 수행하지만 실제 수신 길이, entries upper bound, range/overflow 검증은 별도 확인이 필요하다. baseline 원본 보존과 hardening을 섞지 않는다.

## 입력 경로
Agent 03 baseline/diff, Agent 05 wire report, Ryu status request/data 경로, GS FTP type limits.

## 수정 가능 파일
별도 optional validation patch/branch, test, `parser_hardening_report.md`.

## 수정 금지 파일
명시 승인 없는 literal Ryu 보존 source 직접 변경, old `client.c`, integration source.

## 수행 작업
1. 실제 reply length가 fixed header와 `entries` 배열 요구 길이를 만족하는지 검증하라.
2. `entries <= GS_FTP_STATUS_CHUNKS`, `complete <= total`을 검증하라.
3. 각 `next`, `count`, `next + count` overflow와 total 범위를 검증하라.
4. duplicate/overlap/zero count 정책과 malformed return/cleanup을 정의하라.
5. truncated, oversized entries, endian-crafted, overflow packet test를 작성하라.
6. patch 미적용 시 위험과 운영 완화를 보고하라.

## 수행 금지
- 다른 Agent의 역할을 대신 수행하지 마라. 자기 역할 밖의 작업이 필요하면 직접 수행하지 말고 handoff note를 작성하라.
- 자기 역할 밖 파일을 수정하지 마라. 파일 소유권 위반이 필요해 보이면 직접 수정하지 말고 Orchestrator와 Phase Leader에게 보고하라.
- 기존 client.c 안에 FTP_Ryu 코드를 섞지 마라.
- 기존 FTP와 Ryu FTP가 hidden mixture 형태로 섞이면 migration 실패로 간주한다.
- 기존 `client.c` 안에 FTP_Ryu 코드를 섞지 말 것.
- 자기 역할 밖 파일 수정 금지.
- 임의로 기존 FTP 삭제 금지.
- protocol 호환성 확인 전 기존 FTP 완전 제거 금지.
- Ryu 원본 수정 시 반드시 diff와 이유 기록.
- build 성공 여부를 반드시 보고.
- hardening을 literal baseline인 것처럼 제출하지 말 것.
- 코드 수정 없이 분석만 하는 Agent는 절대 source를 수정하지 말 것.

## 산출물
optional patch, malformed tests, validation matrix, build/test 결과, 적용/보류 권고.

## 성공 기준
정상 packet 동작을 유지하면서 모든 정의된 malformed case가 bounded error와 cleanup으로 종료되고 baseline 대비 diff가 분리된다.

## 실패 시 보고
수신 API가 실제 길이를 제공하지 않는 등 구조적 제약을 정확한 signature와 함께 보고하고 우회 안전성을 과장하지 않는다.

## 다음 Agent Handoff
Agent 08에 patch 적용 상태, coverage, residual risk와 literal-port deviation을 전달한다.

## Runner Output Contract

작업 완료 시 마지막 20줄 안에 반드시 다음 중 하나를 출력하라:

- `AGENT_STATUS: DONE`
- `AGENT_STATUS: RETRY_REQUIRED`
- `AGENT_STATUS: FAILED`

status marker 바로 아래에 `AGENT_SUMMARY:` 한 줄 요약을 작성하라.
