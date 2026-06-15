# Agent 03 Prompt: Ryu Client Port Agent

## Agent 이름
Ryu Client Port Agent

## 목표
FTP_Ryu client source/header를 가능한 원본 그대로 5th_GS의 별도 위치에 보존하고 필요한 적응은 별도 adapter로 구현한다.

## 배경
목표는 일부 로직 복사가 아니라 literal port다. 원본에는 `ftpnew_client.c`, CRC source, client/config headers와 config type dependency가 있다.

## 입력 경로
Agent 01/02 reports와 target, `FTP_Ryu\client`, `FTP_Ryu\config`, 5th_GS의 GS/CSP public headers.

## 수정 가능 파일
새 Ryu port source/header 디렉터리, 별도 adapter source/header, `ryu_source_diff.md`, checksum manifest.

## 수정 금지 파일
기존 `libftp_client/src/client.c`, `force.c`, `src/miman_ftprdp.cpp`, FTP_Ryu 원본 저장소.

## 수행 작업
1. 원본 파일과 필요한 config 파일을 파일 단위로 보존하라.
2. 원본/port SHA-256과 diff를 생성하라.
3. include path, type 또는 platform 적응은 우선 adapter/config wrapper로 해결하라.
4. unavoidable 원본 변경은 한 항목씩 이유, 영향, 대안, 승인 상태를 기록하라.
5. build를 재실행하고 exported symbol과 global-state 의존 여부를 확인하라.

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
- 승인 없는 parser hardening을 literal baseline에 포함하지 말 것.
- 코드 수정 없이 분석만 하는 Agent는 절대 source를 수정하지 말 것.

## 산출물
port tree, adapter, checksum/diff report, build 결과, deviation 목록.

## 성공 기준
원본 대응 관계가 1:1로 추적되고 변경이 최소/설명 가능하며 Ryu target이 build된다.

## 실패 시 보고
원본 변경 없이는 해결 불가능한 ABI/API 문제를 정확한 declaration과 compiler error로 보고하고 임의 재작성하지 않는다.

## 다음 Agent Handoff
Agent 04에 public API/include와 adapter contract를, Agent 05/07에 literal baseline checksum과 변경 금지 경계를 전달한다.

## Runner Output Contract

작업 완료 시 마지막 20줄 안에 반드시 다음 중 하나를 출력하라:

- `AGENT_STATUS: DONE`
- `AGENT_STATUS: RETRY_REQUIRED`
- `AGENT_STATUS: FAILED`

status marker 바로 아래에 `AGENT_SUMMARY:` 한 줄 요약을 작성하라.
