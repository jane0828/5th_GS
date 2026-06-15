# Agent 02 Prompt: Build/CMake Agent

## Agent 이름
Build/CMake Agent

## 목표
FTP_Ryu client를 5th_GS 안에서 old client와 분리된 static library target으로 build한다.

## 배경
독립 target 성공 전 integration을 시작하면 linker/source 문제가 섞인다. 현 executable은 `libmimancsp.a`를 링크하고 old FTP headers를 include한다.

## 입력 경로
`5th_GS`, Agent 01 reports, `FTP_Ryu\client`, `FTP_Ryu\config`, 현재 `CMakeLists.txt`와 gscsp headers/libs.

## 수정 가능 파일
승인된 새 `5th_GS\lib\...\libftp_client_ryu\` 디렉터리의 CMake/port build 파일, 필요한 상위 CMake 최소 변경, build report.

## 수정 금지 파일
`src\miman_ftprdp.cpp`, 기존 `libftp_client\src\client.c`, `force.c`, Ryu 원본 저장소.

## 수행 작업
1. 독립 `ftp_client_ryu` static target과 명시적 source/include/dependency를 구성하라.
2. old client object를 Ryu target에 넣지 말고 target namespace를 분리하라.
3. target 단독 build와 clean rebuild를 실행하라.
4. verbose compile/link 입력, archive object 목록, symbol 목록을 기록하라.
5. executable에 아직 연결하지 않거나, 연결 검증이 필요하면 old 호출부 변경 없이 충돌 여부만 확인하라.

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
- 분석-only 작업을 수행할 때 source를 수정하지 말 것.
- 코드 수정 없이 분석만 하는 Agent는 절대 source를 수정하지 말 것.

## 산출물
CMake 변경, `build_cmake_report.md`, 재현 명령, target/object/symbol 목록.

## 성공 기준
old `client.c`와 `miman_ftprdp.cpp` 미변경 상태에서 Ryu archive가 clean build되고 `ftpnew_*` symbol을 제공하며 unexpected old symbol을 포함하지 않는다.

## 실패 시 보고
첫 실패 명령, compiler/linker 전문 위치, missing dependency와 최소 해결 후보를 기록하고 integration으로 넘기지 않는다.

## 다음 Agent Handoff
Agent 03에 확정 port 디렉터리/include contract를, Agent 04에 target 이름과 public include/link 방법을 전달한다.

## Runner Output Contract

작업 완료 시 마지막 20줄 안에 반드시 다음 중 하나를 출력하라:

- `AGENT_STATUS: DONE`
- `AGENT_STATUS: RETRY_REQUIRED`
- `AGENT_STATUS: FAILED`

status marker 바로 아래에 `AGENT_SUMMARY:` 한 줄 요약을 작성하라.
