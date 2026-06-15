# Acceptance Criteria

## Phase Review Gates

- [ ] Phase 1~5마다 Agent 08 review note와 `PHASE_GO` 또는 `PHASE_GO_WITH_NOTES`가 있다.
- [ ] Phase Leader가 expected outputs, 기술적 충분성, file ownership, 다음 Phase 입력을 확인했다.
- [ ] `PHASE_RETRY_REQUIRED` 항목은 재실행 후 닫혔고 decision log에 이력이 남았다.
- [ ] hidden old/new mixture가 없으며 발견 시 rollback 또는 재작업됐다.

## Build/Separation

- [ ] `ftpnew_client.c`와 CRC source가 별도 static library target으로 build된다.
- [ ] Ryu 코드가 기존 `client.c` 또는 `force.c`에 병합되지 않았다.
- [ ] test 전환 기간에는 old/Ryu path 선택이 로그와 코드에서 명확하다.
- [ ] final 단계에는 기존 `client.c` object/library가 `BEE-1000`에 link되지 않는다.
- [ ] clean build가 성공하고 명령, compiler, linker 입력이 기록된다.

## Symbols/API

- [ ] `miman_ftprdp.cpp`의 최종 upload/download 호출은 `ftpnew_*` API다.
- [ ] `rg`로 `gs_ftp_state_t`, `ftpavailable`, `ftp_done`, old `gs_ftp_upload/download` 잔존을 분류했다.
- [ ] `nm BEE-1000 | grep -E 'ftp|gs_ftp|ftpnew'` 결과에서 허용/금지 symbol을 판정했다.
- [ ] build는 성공했으나 old library가 링크되는 false success를 link map으로 배제했다.

## Protocol/Parser

- [ ] 5th_GS old client, Ryu client, OBC V2.1의 packet type 값이 일치한다.
- [ ] wire struct의 `sizeof`, `offsetof`, packing, chunk size가 일치한다.
- [ ] `entries`, `count`, `next`, `total`, `complete` endian 변환 순서가 확인된다.
- [ ] raw hex dump와 host-decoded 값이 함께 보존된다.
- [ ] packet length와 `entries <= GS_FTP_STATUS_CHUNKS` 검증이 적용되었거나 잔여 위험으로 승인된다.
- [ ] `next + count <= total` 및 overflow 검증이 적용되었거나 명시적으로 보류된다.

## Runtime/Test

- [ ] `csp_connect`/`csp_close`와 close 반환값이 transfer ID와 함께 관측된다.
- [ ] 완료 직후, 5, 10, 30초 connection table과 `RDP_CLOSE_WAIT` 상태가 기록된다.
- [ ] upload와 download smoke test가 성공하고 파일 size/CRC가 일치한다.
- [ ] 정상, timeout, malformed reply, remote error cleanup path가 검토된다.
- [ ] 반복 test 계획에 횟수, 파일 크기, 간격, CSP_CONN_MAX, baseline/peak/final connection 수, 중단 기준이 있다.
- [ ] rollback 명령/commit/binary가 검증된다.

## Final Audit Gate

- [ ] Agent 09 Supervisor가 Orchestrator state/decision log와 모든 Phase Leader notes를 감사했다.
- [ ] `nm`, `rg/grep`, link map, build, wire, runtime, parser와 rollback 증거가 독립 검토됐다.
- [ ] Supervisor가 `GO`, `GO WITH RISKS`, `NO-GO` 중 하나를 근거와 함께 판정했다.

## Final Report Gate

- [ ] Agent 10 Final Manager가 orchestration, Phase review, Supervisor audit 흐름을 요약했다.
- [ ] Final report가 Supervisor 판정을 변경하지 않는다.
- [ ] 작업 범위, 수정 파일, 위험, rollback과 최종 판단이 사용자 관점에서 완결됐다.

모든 필수 항목이 통과하면 GO 후보, 비치명적 미완료가 소유자와 기한 및 완화책을 가지면 GO WITH RISKS 후보, protocol mismatch/resource leak/build 실패가 있으면 NO-GO다.
