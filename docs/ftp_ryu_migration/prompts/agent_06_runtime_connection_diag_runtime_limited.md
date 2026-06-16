# Agent 06 Prompt: Runtime/Connection Diagnostic Agent

## Agent 이름
Runtime/Connection Diagnostic Agent

## 목표
transfer별 CSP connection lifecycle과 RDP delayed close가 pool 고갈에 미치는 영향을 계측한다.

## 배경
downlink 중 `no more free connections`가 발생한다. close 호출 유무만으로 반환 시점과 connection slot 회복을 증명할 수 없다.

## 입력 경로
5th_GS CSP/RDP source/config, Ryu cleanup 경로, integration test binary/log, `CSP_CONN_MAX` 정의.

## 수정 가능 파일
runtime report, 승인된 diagnostic-only patch 또는 logging wrapper.

## 수정 금지 파일
protocol 동작, packet layout, transfer algorithm, unrelated source.

## 수행 작업
1. transfer ID와 함께 `csp_connect`, connection pointer/id, `csp_close` 호출/반환값을 기록하라.
2. 시작 전 baseline과 완료 직후, 5, 10, 30초 connection table을 dump하라.
3. state, src/dst/ports, RDP flags, `RDP_CLOSE_WAIT` 체류 시간을 기록하라.
4. 정상/timeout/remote error/abort 및 반복 upload/download를 비교하라.
5. `CSP_CONN_MAX`, baseline, peak, final count와 pool exhaustion 직전 sequence를 표로 만들라.
6. diagnostic patch는 별도 diff로 유지하고 production behavior를 바꾸지 말라.

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
- 로그를 위해 timeout/close semantics를 임의 변경하지 말 것.
- 코드 수정 없이 분석만 하는 Agent는 절대 source를 수정하지 말 것.

## 산출물
`runtime_connection_report.md`, timestamped logs/table dumps, 반복 test 계획/결과, diagnostic diff.

## 성공 기준
각 transfer의 connect와 close가 대응되고 slot 회복 시간 또는 leak 경로가 식별되며 build/test 상태가 명시된다.

## 실패 시 보고
connection table API 부재, 재현 불가, 권한/환경 문제를 명령과 로그로 기록하고 관측 대안을 제시한다.

## 다음 Agent Handoff
Agent 08에 leak/delayed-close verdict, 안전 반복 간격, `CSP_CONN_MAX` 판단과 unresolved path를 전달한다.

## Runner Output Contract

작업 완료 시 마지막 20줄 안에 반드시 다음 중 하나를 출력하라:

- `AGENT_STATUS: DONE`
- `AGENT_STATUS: RETRY_REQUIRED`
- `AGENT_STATUS: FAILED`

status marker 바로 아래에 `AGENT_SUMMARY:` 한 줄 요약을 작성하라.

## Retry Clarification: Runtime-Limited Environment

Phase Leader requested live Ryu transfer evidence: connection table dumps, close-return correlation, RDP close-wait timeline, and pool exhaustion sequence.

However, this Codex environment may not have access to a real OBC, live CSP network, or executable runtime setup capable of performing an actual Ryu FTP upload/download.

Do not fabricate live runtime evidence.

Your retry goal is to complete a truthful runtime-limited diagnostic handoff.

Perform the following:

1. Check whether live Ryu upload/download execution is actually possible in this environment.
2. If live runtime transfer is not possible, explicitly state that live runtime evidence is unavailable in this environment.
3. Do not claim that OBC runtime smoke, live transfer, connection table dump, or RDP close-wait observation was executed unless it actually was.
4. Complete diagnostic readiness instead:
   - verify diagnostic/logging hooks or reports already added by Agent 06
   - verify build/static readiness if applicable
   - identify exact runtime command path or GUI path the user must run manually
   - provide expected log markers
   - provide connection table dump schedule
   - provide pass/fail criteria
   - provide unresolved risk statement
5. Required manual runtime procedure must include:
   - baseline connection table before transfer
   - Ryu upload and Ryu download test path
   - immediately-after-transfer connection table
   - +5s, +10s, +30s connection table snapshots
   - repeated upload/download loop plan
   - capture of transfer id
   - csp_connect return/pointer or equivalent evidence
   - csp_close call/return evidence
   - src/dst/ports
   - RDP state/flags, especially close-wait behavior
   - active connection count
   - CSP_CONN_MAX
   - pool exhaustion sequence if reproduced
6. Required report updates:
   - update or create docs/ftp_ryu_migration/reports/runtime_connection_report.md
   - add a clear section titled "Runtime-Limited Status"
   - add a clear section titled "Manual Live Test Procedure"
   - add a clear section titled "Open Runtime Risks"
   - append a decision entry to docs/ftp_ryu_migration/state/decision_log.md
7. If diagnostic readiness is complete but live runtime evidence is unavailable, output AGENT_STATUS: DONE.
8. In AGENT_SUMMARY, explicitly say that live runtime/OBC transfer evidence remains open and must be completed manually on the real GS/OBC setup.
9. Do not modify protocol behavior, packet layout, transfer algorithm, or unrelated source.
10. Do not modify FTP_Ryu preserved source or legacy client.c.
11. Do not fake test success.

This retry should allow Phase Leader and Supervisor to carry live runtime validation as an explicit open risk rather than blocking the entire migration on unavailable hardware.
