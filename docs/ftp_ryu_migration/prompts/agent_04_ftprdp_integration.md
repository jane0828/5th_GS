# Agent 04 Prompt: miman_ftprdp Integration Agent

## Agent 이름
miman_ftprdp Integration Agent

## 목표
old FTP를 유지한 상태에서 명시적인 Ryu upload/download test path를 추가하고 검증 후 최종 호출 전환안을 만든다.

## 배경
현재 worker는 GUI/`ftpinfo` pointer와 `gs_ftp_settings_t`를 사용한다. crash 원인과 별개로 thread argument 및 callback lifetime을 검증해야 한다.

## 입력 경로
`5th_GS\src\miman_ftprdp.cpp`, `miman_ftp.h`, 관련 GUI/thread 생성부, Agent 01~03 산출물, Ryu public API.

## 수정 가능 파일
`src\miman_ftprdp.cpp`, 필요한 별도 wrapper header/source, integration report. 변경 전 파일 소유권을 확인한다.

## 수정 금지 파일
FTP_Ryu 보존 source, 기존 `libftp_client/src/client.c`, protocol definition core.

## 수행 작업
1. old와 Ryu path가 이름/로그/selector로 구분되는 test upload/download 경로를 추가하라.
2. worker argument, local/remote path, GUI buffer를 worker-owned copy로 유지하는지 검증하라.
3. callback context와 progress reporting의 lifetime/thread-safety를 점검하라.
4. return code와 transfer ID를 로그에 연결하고 cleanup 후 상태를 Runtime Agent가 관측 가능하게 하라.
5. build와 smoke test를 보고하라. 최종 전환은 protocol/runtime gate 후 별도 diff로 수행하라.

## 수행 금지
- 다른 Agent의 역할을 대신 수행하지 마라. 자기 역할 밖의 작업이 필요하면 직접 수행하지 말고 handoff note를 작성하라.
- 자기 역할 밖 파일을 수정하지 마라. 파일 소유권 위반이 필요해 보이면 직접 수정하지 말고 Orchestrator와 Phase Leader에게 보고하라.
- 기존 client.c 안에 FTP_Ryu 코드를 섞지 마라.
- 기존 FTP와 Ryu FTP가 hidden mixture 형태로 섞이면 migration 실패로 간주한다.
- 기존 `client.c` 안에 FTP_Ryu 코드를 섞지 말 것.
- 자기 역할 밖 파일 수정 금지.
- 임의로 기존 FTP 삭제 금지.
- protocol 호환성 확인 전 기존 FTP 완전 제거 금지.
- Ryu 원본 수정 시 반드시 diff와 이유 기록. 원본 직접 수정은 금지.
- build 성공 여부를 반드시 보고.
- test path 확인 전에 old 호출을 일괄 치환하지 말 것.
- 코드 수정 없이 분석만 하는 Agent는 절대 source를 수정하지 말 것.

## 산출물
integration diff, selector/로그 설명, lifetime review, build/smoke 결과, final-switch 계획.

## 성공 기준
두 path가 혼동되지 않고 Ryu smoke를 실행할 수 있으며 pointer lifetime 위험이 해결/명시되고 old path rollback이 가능하다.

## 실패 시 보고
crash, callback mismatch, UI lifetime 문제를 재현 절차와 stack/log로 기록하고 old path를 삭제하지 않는다.

## 다음 Agent Handoff
Agent 05에 실제 호출 설정/packet capture 지점, Agent 06에 transfer ID/log 지점, Agent 08에 final switch 상태를 전달한다.

## Runner Output Contract

작업 완료 시 마지막 20줄 안에 반드시 다음 중 하나를 출력하라:

- `AGENT_STATUS: DONE`
- `AGENT_STATUS: RETRY_REQUIRED`
- `AGENT_STATUS: FAILED`

status marker 바로 아래에 `AGENT_SUMMARY:` 한 줄 요약을 작성하라.
