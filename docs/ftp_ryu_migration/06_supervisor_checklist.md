# Supervisor Checklist

Supervisor는 중간 Phase 검토자가 아니라 전체 결과의 최종 독립 감사자다.

- [ ] Orchestrator의 `agent_status.md`가 실제 산출물 및 현재 상태와 일치하는가?
- [ ] `task_queue.md`의 dependency와 최소 사용자 confirmation 조건이 지켜졌는가?
- [ ] `decision_log.md`에 retry, 예외, 위험 수용, dispatch 근거가 누락 없이 기록됐는가?
- [ ] Phase 1~5의 Phase Leader review notes와 PHASE_* 판정이 존재하는가?
- [ ] Phase Leader notes의 미해결 위험과 retry가 최종 결과에 반영됐는가?
- [ ] 기존 FTP와 Ryu FTP가 한 source 또는 state flow에 섞이지 않았는가?
- [ ] final 단계에서 기존 `client.c`가 compile/link되는가?
- [ ] old `gs_ftp_*`, `gs_ftp_state_t`, `ftpavailable`, `ftp_done` symbol이 남았는가?
- [ ] Ryu source는 원본과 얼마나 다르며 모든 차이가 설명되었는가?
- [ ] `miman_ftprdp.cpp`가 old API를 아직 호출하는가?
- [ ] test path의 old/Ryu 선택이 명확한가?
- [ ] OBC와 packet type, size, offset, packing, endian이 호환되는가?
- [ ] `entries/count/next` endian 처리 순서가 정확한가?
- [ ] packet length, entries upper bound, next/count range/overflow 검증이 있는가?
- [ ] `csp_close` 반환값과 transfer ID가 로깅되는가?
- [ ] connection table을 0/5/10/30초에 진단할 수 있는가?
- [ ] `RDP_CLOSE_WAIT` 잔존과 `CSP_CONN_MAX` 대비 peak를 확인했는가?
- [ ] clean build와 verbose link 결과가 있는가?
- [ ] upload/download 및 반복 실행 결과가 있는가?
- [ ] old library 미링크를 `nm`/link map으로 증명했는가?
- [ ] 단계별 rollback 방법이 실제로 가능한가?

## 판정

- **GO:** 모든 필수 gate 통과, protocol 호환, build/smoke/repeat 성공, connection baseline 회복, blocker 없음.
- **GO WITH RISKS:** 기능과 rollback은 검증됐으나 제한적 hardening 또는 OBC 확인이 남아 있고, 위험 소유자/완화/기한/운영 제한이 명시됨.
- **NO-GO:** build/link 실패, wire 불일치, crash/data corruption, connection 미회복, old/new 혼합, 근거 없는 원본 변경, rollback 부재 중 하나 이상.
