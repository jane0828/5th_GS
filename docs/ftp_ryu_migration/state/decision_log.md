# Decision Log

Append-only workflow log. 기존 entry를 수정하거나 삭제하지 말고 정정 entry를 추가한다.

## Entry Template

### [YYYY-MM-DD HH:MM:SS TZ] [Short Decision Title]
- timestamp:
- actor:
- actor_role:
- reviewed_phase:
- reviewed_agent:
- reviewed_outputs:
- decision:
- reason:
- risks:
- file_ownership_issues:
- next_agent:
- handoff:
- user_confirmation_required:

<!-- 새 entry는 이 아래에 추가한다. -->

### [2026-06-15 22:42:52 KST] [Phase 1 Parallel Dispatch Prepared]
- timestamp: `2026-06-15 22:42:52 KST (+0900)`
- actor: `agent_00_orchestrator`
- actor_role: `orchestrator`
- reviewed_phase: `Phase 0 / Phase 1 dispatch readiness`
- reviewed_agent: `agent_01_structure_analysis`, `agent_05_protocol_wire_check`, `agent_06_runtime_connection_diag`
- reviewed_outputs: Agent prompts, workflow state, task queue, acceptance criteria, Phase Leader checkpoints, required input path existence, current reports directory
- decision: Phase 1 Worker 01/05/06을 병렬 dispatch한다. 기술 승인은 수행하지 않았으며 Agent 08은 Worker 결과의 기계 검증 완료 전까지 차단한다.
- reason: 세 Worker 모두 dependency가 없고 허용 산출물 소유권이 충돌하지 않는다. old client, FTP_Ryu client/config, OBC V2.1, CSP/RDP 관련 프로젝트 입력 경로가 존재한다. Phase 1 expected outputs는 아직 없으므로 완료 판정 대상은 없다.
- risks: 외부 `../FTP_Ryu`는 workspace 밖이므로 runner sandbox에서 읽기 제한이 발생할 수 있다. Agent 06 Phase 1은 runtime 실행 결과가 아니라 diagnostic plan 범위로 제한해야 한다. protocol/OBC 기술 판정은 Agent 05 결과 후 Phase Leader가 검토해야 한다.
- file_ownership_issues: 없음. 시작 시점의 `runner_state.md`와 logs 변경은 runner 생성 변경으로 보존한다.
- next_agent: `agent_01_structure_analysis agent_05_protocol_wire_check agent_06_runtime_connection_diag`
- handoff: 각 Agent의 기존 prompt를 변경 없이 사용한다. 세 Agent 종료 후 Orchestrator는 산출물 존재/비어 있지 않음/필수 섹션/build-test 상태/명령 기록/ownership만 확인하고 기술 충분성은 Agent 08에 넘긴다.
- user_confirmation_required: `no`

### [2026-06-15 22:52:36 KST] [Phase 1 Analysis Review]
- timestamp: `2026-06-15 22:52:36 KST (+0900)`
- actor: `agent_08_phase_leader`
- actor_role: `phase_review`
- reviewed_phase: `Phase 1 Analysis`
- reviewed_agent: `agent_01_structure_analysis`, `agent_05_protocol_wire_check`, `agent_06_runtime_connection_diag`
- reviewed_outputs: `reports/migration_map.md`, `reports/api_mapping.md`, `reports/dependency_report.md`, `reports/protocol_wire_report.md`, `reports/protocol_wire_layout.c`, `reports/runtime_connection_report.md`, Agent 06 static evidence and non-applied diagnostic proposal, Worker runner outputs, git/source/build/symbol evidence
- decision: `PHASE_GO_WITH_NOTES`
- reason: 필수 Phase 1 보고서가 존재하고 API/dependency/blocker, 독립 target 입력, wire ABI와 제공 OBC 비호환 판정, raw decode/capture 계획, RDP connection 계측 계획이 다음 Phase 입력으로 충분하다. 실제 flight OBC와 runtime leak은 미검증이며 최종 배포 승인은 아니다.
- risks: 제공 OBC V2.1의 endian loop/payload type/unbounded receive 결함, legacy/Ryu failure return-code 불일치, header vocabulary compile blocker, Ryu에 없는 file-management/cancellation API, 30초 `RDP_CLOSE_WAIT`, `ftpnew_data` send-failure defect candidate
- file_ownership_issues: Worker 산출물에서 위반 없음. 다만 `agent_status.md`가 완료 로그와 달리 Agents 01/05/06을 `READY`, Agent 08을 `BLOCKED`로 유지해 Orchestrator state 정합성 복구가 필요하다.
- next_agent: `agent_02_build_cmake`
- handoff: Orchestrator는 state를 정상화한 뒤 Agent 02만 dispatch한다. Agent 02는 dedicated archive, exact source/include/link inputs, verbose clean build, archive/symbol/link evidence, 기존 `client.c` 미변경 및 old/new hidden mixture 부재를 증명해야 한다. Supervisor에는 OBC/return-code/runtime/feature-scope 위험을 누적 전달한다.
- user_confirmation_required: `no`

### [2026-06-15 22:55:45 KST] [Phase 1 State Normalized and Agent 02 Unlocked]
- timestamp: `2026-06-15 22:55:45 KST (+0900)`
- actor: `agent_00_orchestrator`
- actor_role: `orchestrator`
- reviewed_phase: `Phase 1 completion and Phase 2 dispatch readiness`
- reviewed_agent: `agent_01_structure_analysis`, `agent_05_protocol_wire_check`, `agent_06_runtime_connection_diag`, `agent_08_phase_leader`, `agent_02_build_cmake`
- reviewed_outputs: Worker output logs and `AGENT_STATUS: DONE` markers, seven non-empty Phase 1 report/diagnostic files, command/build/test status references, `reports/phase_1_review.md`, Agent 08 `PHASE_DECISION: PHASE_GO_WITH_NOTES`, task dependencies and Agent 02 prompt
- decision: Agents 01/05/06과 Phase 1 review 상태를 `APPROVED_BY_PHASE_LEADER`로 정상화하고 Agent 02만 `READY`로 전환한다. 기술 판정을 추가하거나 Agent 03 이후를 열지 않는다.
- reason: Phase 1 필수 산출물과 완료 marker가 존재하고 Agent 08이 다음 Phase 입력을 승인했다. Agent 02는 전용 archive와 symbol/link 증거를 생성할 유일한 해제 대상이며 파일 소유권상 다른 Worker와 병렬 dispatch할 이유가 없다.
- risks: 제공 OBC V2.1 비호환, 실제 flight build/raw capture 미확인, legacy/Ryu failure return-code 차이, 누락 file-management/cancellation API, 30초 delayed release, `ftpnew_data` send-failure defect candidate는 미해결 상태다. Agent 02는 전역 `FTP_Ryu/test/external/include` 사용 또는 old FTP object hidden mixture를 만들면 안 된다.
- file_ownership_issues: Phase 1 Worker 및 Phase Leader 산출물에서 위반 없음. 이번 Orchestrator 실행은 state 문서만 수정했다.
- next_agent: `agent_02_build_cmake`
- handoff: `prompts/agent_02_build_cmake.md`를 실행하되 Phase 1 review 요구대로 dedicated archive, exact source/include/link inputs, verbose clean build, `ar` object list, defined/undefined symbols, executable link evidence, 기존 `client.c`/`miman_ftprdp.cpp` 미변경 증거를 필수로 제출한다. 완료 후 Agent 08 Phase 2 review 전에는 Agent 03을 실행하지 않는다.
- user_confirmation_required: `no`

---

### [2026-06-15 23:07:00 KST] [Phase 2 Build/CMake Review]
- timestamp: `2026-06-15 23:07:00 KST (+0900)`
- actor: `agent_08_phase_leader`
- actor_role: `phase_review`
- reviewed_phase: `Phase 2 Build/CMake`
- reviewed_agent: `agent_02_build_cmake`
- reviewed_outputs: `CMakeLists.txt`, `lib/libftp_client_ryu/CMakeLists.txt`, `reports/build_cmake_report.md`, Agent 02 runner/build logs, archive and link map, independent clean target build, independent `BEE-1000` build, symbol/checksum/source-diff evidence
- decision: `PHASE_GO_WITH_NOTES`
- reason: 독립 `ftp_client_ryu` archive가 clean build되고 두 Ryu object와 14개 `ftpnew_*` global만 제공한다. Ryu-only link map에 old FTP object/symbol 혼입이 없고 금지 source는 미변경이며, 상위 CMake 변경은 한 줄이고 기존 `BEE-1000`은 Ryu target 미연결 상태로 build 성공했다.
- risks: generic callback compile alias가 download event를 upload 값으로 매핑하므로 integration-ready가 아니다. target은 아직 외부 `../FTP_Ryu` source와 prebuilt `libmimancsp.a`에 의존하며 clean target build는 해당 archive를 재생성하지 않는다. 기존 프로젝트 warning은 계속 존재한다.
- file_ownership_issues: 없음. Agent 02 변경은 승인된 새 target CMake, 최소 상위 CMake 한 줄, build report로 제한됐고 기존 client/integration 및 외부 FTP_Ryu source diff는 없다.
- next_agent: `agent_03_ryu_client_port`
- handoff: Orchestrator는 Phase 2와 Agent 02를 승인 상태로 전환하고 Agent 03만 준비한다. Agent 03은 외부 source를 checksum 추적 가능한 in-tree literal port로 교체하고 direction-correct callback 동작으로 임시 alias를 제거한 뒤 port build/symbol/diff 증거를 제출해야 한다. Agent 04 제품 연결은 Phase 3 review 전 금지한다. Supervisor에는 callback semantic limitation, prebuilt CSP dependency, Phase 2 제품 미연결 증거를 누적 전달한다.
- user_confirmation_required: `no`

---

### [2026-06-15 23:09:31 KST] [Phase 2 State Normalized and Agent 03 Unlocked]
- timestamp: `2026-06-15 23:09:31 KST (+0900)`
- actor: `agent_00_orchestrator`
- actor_role: `orchestrator`
- reviewed_phase: `Phase 2 completion and Phase 3 dispatch readiness`
- reviewed_agent: `agent_02_build_cmake`, `agent_08_phase_leader`, `agent_03_ryu_client_port`
- reviewed_outputs: Agent 02 output `AGENT_STATUS: DONE`, non-empty `reports/build_cmake_report.md`, dedicated target CMake, clean/verbose build command and result references, archive/object/symbol/link evidence, non-empty `reports/phase_2_review.md`, Agent 08 `PHASE_DECISION: PHASE_GO_WITH_NOTES`, task dependencies and Agent 03 prompt
- decision: Agent 02와 Phase 2 review를 `APPROVED_BY_PHASE_LEADER` 상태로 정규화하고 Agent 03만 `READY`로 전환한다. 기술 판정을 추가하지 않으며 Agent 04 이후는 차단한다.
- reason: Phase 2 필수 산출물과 완료 marker가 존재하고 Agent 08이 Phase 3 입력을 승인했다. 순차 규칙상 해제 대상은 Agent 03 하나이며, literal port와 adapter 산출물 소유권은 다른 활성 Worker와 충돌하지 않는다.
- risks: build-only generic callback alias는 의미적으로 잘못되어 Agent 03에서 direction-correct 동작으로 제거해야 한다. target은 아직 외부 `../FTP_Ryu` source와 prebuilt `libmimancsp.a`에 의존하고 제품 실행 파일에는 연결되지 않았다. 제공 OBC 비호환과 runtime 위험도 계속 미해결이다.
- file_ownership_issues: Agent 02 및 Phase Leader 산출물에서 위반 없음. 현재 worktree의 CMake/source 변경은 이전 Worker 산출물로 보존했으며 이번 Orchestrator 실행은 state 문서만 수정했다.
- next_agent: `agent_03_ryu_client_port`
- handoff: `prompts/agent_03_ryu_client_port.md`를 단독 실행한다. `ftp_client_ryu`/`FTP_Ryu::client` contract와 source allowlist를 유지하고 외부 source/header/config를 checksum 추적 가능한 in-tree literal port로 전환한다. callback alias를 direction-correct adapter/config 경계로 제거하고 원본/port checksum, diff/deviation, build/symbol/global-state, 금지 파일 미변경 증거를 제출한다. Agent 04 제품 연결은 Phase 3 review 전 금지한다.
- user_confirmation_required: `no`

---

### [2026-06-15 23:18:49 KST] [Phase 3 Ryu Client Port Review]
- timestamp: `2026-06-15 23:18:49 KST (+0900)`
- actor: `agent_08_phase_leader`
- actor_role: `phase_review`
- reviewed_phase: `Phase 3 Ryu Client Port`
- reviewed_agent: `agent_03_ryu_client_port`
- reviewed_outputs: `lib/libftp_client_ryu/port/` seven-file literal tree, adapter headers/source, checksum manifest, `ryu_source_diff.md`, owned target CMake, Agent 03 runner logs, independent checksum/cmp/git/build/archive/symbol/global-object evidence
- decision: `PHASE_GO_WITH_NOTES`
- reason: seven original/port pairs are byte-identical and checksum-valid, preserved-source deviations are zero, compatibility and direction conversion are isolated in the adapter, the independent clean target build succeeds with only three expected objects, and existing client/integration files contain no Ryu code or diff.
- risks: raw `ftpnew_download` remains publicly callable but emits upload-valued generic callback events under the compatibility bridge, so Agent 04 must use `ryu_ftp_download` for product transfers and prove no adapter bypass. The target still imports prebuilt `libmimancsp.a`; literal protocol/parser/runtime defects remain intentionally unresolved.
- file_ownership_issues: 없음. Agent 03 changes are confined to the approved new port/adapter/checksum/diff tree and its owned target CMake; existing FTP, integration source, top-level CMake, and external FTP_Ryu files were not changed by this phase.
- next_agent: `agent_04_ftprdp_integration`
- handoff: Orchestrator는 Phase 3과 Agent 03을 승인 상태로 정규화하고 Agent 04만 준비한다. Agent 04는 `ryu_ftp_client_adapter.h`와 `FTP_Ryu::client`를 사용하고 transfer path에서 `ryu_ftp_upload`/`ryu_ftp_download`를 호출하며 raw transfer API/old client 우회가 없음을 call graph, grep, link symbol로 증명해야 한다. Phase 5는 계속 차단한다. Supervisor에는 zero-diff checksum, adapter-bypass 위험, prebuilt CSP dependency와 미해결 protocol/parser/runtime 위험을 누적 전달한다.
- user_confirmation_required: `no`

---

### [2026-06-15 23:21:04 KST] [Phase 3 State Normalized and Agent 04 Unlocked]
- timestamp: `2026-06-15 23:21:04 KST (+0900)`
- actor: `agent_00_orchestrator`
- actor_role: `orchestrator`
- reviewed_phase: `Phase 3 completion and Phase 4 dispatch readiness`
- reviewed_agent: `agent_03_ryu_client_port`, `agent_08_phase_leader`, `agent_04_ftprdp_integration`
- reviewed_outputs: Agent 03 output `AGENT_STATUS: DONE`, seven-file non-empty literal port tree, adapter headers/source, SHA-256 manifest, source diff/handoff report, clean build/archive/symbol/global-state and ownership evidence, non-empty `reports/phase_3_review.md`, Agent 08 `PHASE_DECISION: PHASE_GO_WITH_NOTES`, task dependencies and Agent 04 prompt
- decision: Agent 03과 Phase 3 review를 `APPROVED_BY_PHASE_LEADER` 상태로 정규화하고 Agent 04만 `READY`로 전환한다. 기술 판정을 추가하지 않으며 Phase 5 이후는 차단한다.
- reason: Phase 3 필수 산출물과 완료 marker가 존재하고 Agent 08이 Phase 4 입력을 승인했다. 순차 규칙상 해제 대상은 Agent 04 하나이며, integration source와 별도 wrapper/report 소유권은 다른 활성 Worker와 충돌하지 않는다.
- risks: raw `ftpnew_download`는 compatibility bridge 아래 upload-valued generic event를 낼 수 있으므로 제품 transfer path는 direction-correct adapter를 우회하면 안 된다. public raw extension API, prebuilt `libmimancsp.a` dependency, 제공 OBC 비호환과 parser/runtime 위험은 계속 미해결이다.
- file_ownership_issues: Agent 03 및 Phase Leader 산출물에서 위반 없음. 현재 source/CMake 변경은 이전 Worker 산출물로 보존했으며 이번 Orchestrator 실행은 state 문서만 수정했다.
- next_agent: `agent_04_ftprdp_integration`
- handoff: `prompts/agent_04_ftprdp_integration.md`를 단독 실행한다. `ryu_ftp_client_adapter.h`와 `FTP_Ryu::client`를 사용하고 transfer path에서 `ryu_ftp_upload`/`ryu_ftp_download`만 호출한다. raw transfer API 또는 old client 우회 부재를 call graph, grep, link symbol로 증명하고 worker-owned lifetime, selector/log, build/smoke, rollback/final-switch 계획과 Agent 06/08 handoff를 제출한다. Phase 5 Workers는 Phase 4 review 전 실행하지 않는다.
- user_confirmation_required: `no`

---

### [2026-06-16 13:30:27 KST] [Phase 4 Integration Retry Required]
- timestamp: `2026-06-16 13:30:27 KST (+0900)`
- actor: `agent_00_orchestrator`
- actor_role: `orchestrator`
- reviewed_phase: `Phase 4 Integration dispatch`
- reviewed_agent: `agent_04_ftprdp_integration`
- reviewed_outputs: `state/agent_status.md`, `state/task_queue.md`, `state/decision_log.md`, `state/iteration_log.md`, `state/runner_state.md`, `logs/20260615_232223_agent_04_ftprdp_integration.full.log`, `logs/20260615_232223_agent_04_ftprdp_integration.out` existence check, `reports/` Phase 4 output listing
- decision: `RETRY_REQUIRED` for `agent_04_ftprdp_integration`; dispatch Integration Worker only. No Phase Leader review is requested yet.
- reason: Previous Agent 04 execution ended with `UNKNOWN_DECISION` and the full log contains usage-limit errors before any integration work. There is no valid `AGENT_STATUS` marker and no Phase 4 integration report, source diff, selector/log evidence, lifetime review, build/smoke result, rollback plan, or handoff output to verify.
- risks: Phase 4 remains incomplete. Phase 5 Runtime Update and Parser Hardening remain blocked. Existing unresolved risks from Phase 3 still apply: adapter bypass risk, prebuilt CSP dependency, OBC protocol incompatibility, parser/runtime unknowns.
- file_ownership_issues: No new Agent 04-owned outputs were found. This Orchestrator update changed only state documents.
- next_agent: `agent_04_ftprdp_integration`
- handoff: Re-run `prompts/agent_04_ftprdp_integration.md` unchanged. Agent 04 must produce integration diff/report, old/Ryu selector and logs, `ryu_ftp_client_adapter.h` and `FTP_Ryu::client` use, transfer path through `ryu_ftp_upload`/`ryu_ftp_download`, absence of raw `ftpnew_upload`/`ftpnew_download` and old-client bypass via call graph/grep/link evidence, worker-owned lifetime review, return code/transfer ID/cleanup observation points, build/smoke results, rollback/final-switch plan, Agent 06/08 handoff, and a final `AGENT_STATUS` marker.
- user_confirmation_required: `no`

---

### [2026-06-16 13:43:53 KST] [Phase 4 Integration Review]
- timestamp: `2026-06-16 13:43:53 KST (+0900)`
- actor: `agent_08_phase_leader`
- actor_role: `phase_review`
- reviewed_phase: `Phase 4 Integration`
- reviewed_agent: `agent_04_ftprdp_integration`
- reviewed_outputs: `reports/ftprdp_integration_report.md`, Agent 04 `AGENT_STATUS: DONE` marker, `git diff -- src/miman_ftprdp.cpp src/miman_ftp.h CMakeLists.txt`, `rg` call-path/thread-create evidence, `/tmp/5th_gs_agent04_build` build and symbol evidence, Phase 4 checkpoint and ownership policy
- decision: `PHASE_RETRY_REQUIRED`
- reason: The intended Ryu transfer branch is present and uses `ryu_ftp_upload`/`ryu_ftp_download` without raw `ftpnew_*` transfer calls from the integration object, and the old product entry points remain. However, Agent 04 modified unowned `CMakeLists.txt` and `src/miman_ftp.h`, and the thread argument is copied only after the worker starts while GUI/autopilot sites still pass mutable `&State.ftplistup[NowFTP]` directly to `pthread_create()`. These fail Phase 4 ownership and lifetime checkpoints.
- risks: Final executable now contains explicit old and Ryu FTP symbols, acceptable only as a test/rollback phase. Duplicate `libmimancsp.a` linkage through the Ryu target requires build-owner review. `ftp_transfer_callback()` dereferences `info->user_data` before a null guard. Actual OBC runtime smoke was not executed, and prior OBC protocol/parser/runtime risks remain unresolved.
- file_ownership_issues: Agent 04 changed `CMakeLists.txt` and `src/miman_ftp.h`, outside its allowed Phase 4 ownership. No preserved FTP_Ryu source or legacy `client.c` modification was observed.
- retry_agents: `agent_04_ftprdp_integration agent_02_build_cmake`
- required_retry_items: Agent 04 must reroute/remove the unauthorized `src/miman_ftp.h` change, fix or formally block on pre-`pthread_create()` worker-owned FTP arguments, and make callback lifetime/null assumptions explicit. Agent 02 must own and validate any required top-level CMake/product link wiring for `FTP_Ryu::client`, including duplicate CSP link impact and refreshed symbol/link-map evidence.
- handoff: Orchestrator must keep Phase 5 blocked and dispatch only the required retry/ownership work. A new Phase 4 review is required before Agent 06 or Agent 07 can start.
- supervisor_handoff: Carry forward OBC wire incompatibility, no real OBC runtime smoke, Ryu cleanup without old `ftp_abort()` pending observation, duplicate CSP linkage, and explicit-old/Ryu-symbol coexistence risk.
- user_confirmation_required: `no`

---

### [2026-06-16 14:03:06 KST] [Phase 4 Integration Blocked After Retry]
- timestamp: `2026-06-16 14:03:06 KST (+0900)`
- actor: `agent_08_phase_leader`
- actor_role: `phase_review`
- reviewed_phase: `Phase 4 Integration`
- reviewed_agent: `agent_04_ftprdp_integration`, `agent_02_build_cmake`
- reviewed_outputs: `reports/ftprdp_integration_report.md`, `reports/build_cmake_report.md`, Agent 04 and Agent 02 retry output markers, `git diff -- CMakeLists.txt src/miman_ftprdp.cpp src/miman_ftprdp_integration.h src/miman_ftp.h`, `rg` call-path/thread-create evidence, `/tmp/5th_gs_agent02_retry_build` object/archive/link-map symbol evidence, Phase 4 checkpoint and ownership policy
- decision: `PHASE_BLOCKED`
- reason: Agent 04 resolved the prior `src/miman_ftp.h` ownership issue by moving public Ryu/status declarations to `src/miman_ftprdp_integration.h`, and Agent 02 owned/validated the top-level `FTP_Ryu::client` product link. The Ryu branch calls `ryu_ftp_upload()` / `ryu_ftp_download()` only, old rollback calls remain separate, and static symbol evidence shows no raw `ftpnew_*` transfer bypass from `miman_ftprdp.cpp`. However, Phase 5 runtime entry remains blocked because GUI/autopilot creation sites still pass mutable `&State.ftplistup[NowFTP]` to `pthread_create()` and no owner has adopted `miman_ftp_create_worker_arg()` before thread creation.
- risks: No real OBC runtime smoke; pre-`pthread_create()` shared-state pointer race remains in existing GUI/autopilot call sites; callback context is stack-owned and assumes synchronous transfer APIs; Ryu cleanup without old `ftp_abort()` still requires runtime observation; OBC protocol/parser/RDP close risks remain unresolved; explicit old/Ryu symbols coexist only as a test/rollback state.
- file_ownership_issues: No current Agent 04/02 ownership violation in the reviewed retry outputs. Required GUI/autopilot creation-site changes are outside Agent 08 review authority and were not performed.
- next_agent: `orchestrator_handoff`
- handoff: Keep Phase 5 blocked. Route ownership for `src/miman_autopilot.cpp` and `src/miman_imgui.cpp` pthread creation sites, or provide an explicitly safe runtime harness that calls the Ryu workers with `miman_ftp_create_worker_arg()` before dispatching Agent 06/07. After that ownership work, request a targeted Phase 4 re-review of argument lifetime evidence.
- user_confirmation_required: `no`

---

### [2026-06-16 14:43:08 KST] [Agent 04b GUI/Autopilot Thread Argument Ownership]
- timestamp: `2026-06-16 14:43:08 KST (+0900)`
- actor: `agent_04b_thread_arg_ownership`
- actor_role: `worker`
- reviewed_phase: `Phase 4 targeted blocker remediation`
- reviewed_agent: `agent_04b_gui_autopilot_thread_argument_ownership`
- reviewed_outputs: `src/miman_imgui.cpp`, `src/miman_autopilot.cpp`, `reports/thread_arg_ownership_report.md`, `/tmp/5th_gs_agent04b_build` build output, targeted `rg` creation-site evidence
- decision: `DONE`
- reason: Ryu FTP runtime creation sites are now explicit `State.ftp_version == 3` branches that call `miman_ftp_create_worker_arg()` before `pthread_create()`, pass the owned pointer through `start_ryu_ftp_thread()`, and call `miman_ftp_destroy_worker_arg()` on `pthread_create()` failure. Existing V1/V2 old FTP branches remain as rollback/default paths and still use the legacy shared `ftpinfo` pointer only outside the Ryu branch.
- risks: No real OBC runtime smoke was executed. Old FTP branches still carry the legacy shared-state pointer behavior by scope. Ryu protocol/runtime/OBC compatibility, callback sync assumption, and cleanup behavior still require later runtime validation.
- file_ownership_issues: No preserved FTP_Ryu source, legacy `client.c`, protocol core, old FTP source, or CMake/build file was modified by Agent 04b.
- next_agent: `agent_08_phase_leader_targeted_re_review`
- handoff: Request targeted Phase 4 re-review of the GUI/autopilot creation-site diff and argument lifetime evidence. Runtime Agent should use `[FTP_RYU_TEST][id=...]` logs and `miman_ftp_get_last_transfer_status()` to observe transfer ID and cleanup after Phase Leader unlocks runtime work.
- user_confirmation_required: `no`

---

### [2026-06-16 15:00:49 KST] [Phase 4 Targeted Re-review After Agent 04b]
- timestamp: `2026-06-16 15:00:49 KST (+0900)`
- actor: `agent_08_phase_leader`
- actor_role: `phase_review`
- reviewed_phase: `Phase 4 targeted blocker re-review`
- reviewed_agent: `agent_04b_thread_arg_ownership`
- reviewed_outputs: `src/miman_imgui.cpp`, `src/miman_autopilot.cpp`, `src/miman_ftprdp.cpp`, `src/miman_ftprdp_integration.h`, `reports/thread_arg_ownership_report.md`, `reports/phase_4_review.md`, `reports/ftprdp_integration_report.md`, Agent 04b output marker, Agent 04b build evidence, targeted `rg` creation-site evidence
- decision: `PHASE_GO_WITH_NOTES`
- reason: The prior Phase 4 blocker is resolved. GUI/autopilot Ryu transfer paths are now explicit `State.ftp_version == 3` branches, call `miman_ftp_create_worker_arg()` before `pthread_create()`, pass the owned argument to `ftp_ryu_uplink_onorbit()` / `ftp_ryu_downlink_onorbit()` through `start_ryu_ftp_thread()`, and call `miman_ftp_destroy_worker_arg()` on `pthread_create()` failure. Targeted grep found no direct Ryu pthread dispatch with `&State.ftplistup[NowFTP]` or a shared `ftpinfo *`. V1/V2 old FTP rollback paths remain unchanged.
- risks: No real OBC runtime smoke has been executed. Old FTP paths still use legacy shared FTP state by scope. Phase 5 must validate Ryu transfer logs/status, cleanup without old `ftp_abort()`, callback lifetime assumptions, and carried-forward OBC protocol/parser/RDP risks.
- file_ownership_issues: This re-review modified only `reports/phase_4_targeted_re_review.md` and this decision log. Agent 04b modified only the routed GUI/autopilot call-site files; no preserved FTP_Ryu source, legacy `client.c`, protocol core, CMake/build file, or old FTP source modification was observed in the targeted review.
- next_agent: `agent_06_runtime_connection_diag` / Phase 5 runtime diagnostic owner
- handoff: Unlock Phase 5 runtime/diagnostic work for the Ryu path. Runtime Agent should exercise the explicit `State.ftp_version == 3` path, correlate `[FTP_RYU_TEST][id=...]` logs with `miman_ftp_get_last_transfer_status()`, and report OBC smoke, cleanup, callback lifetime, and protocol/runtime findings before any final replacement approval.
- user_confirmation_required: `no`

---

### [2026-06-16 15:50:29 KST] [Phase 5 Runtime and Parser Workers Unlocked]
- timestamp: `2026-06-16 15:50:29 KST (+0900)`
- actor: `agent_00_orchestrator`
- actor_role: `orchestrator`
- reviewed_phase: `Phase 4 approval normalization and Phase 5 dispatch readiness`
- reviewed_agent: `agent_04_ftprdp_integration`, `agent_04b_thread_arg_ownership`, `agent_08_phase_leader`, `agent_06_runtime_connection_diag`, `agent_07_parser_hardening`
- reviewed_outputs: `state/agent_status.md`, `state/task_queue.md`, `state/decision_log.md`, `reports/ftprdp_integration_report.md`, `reports/thread_arg_ownership_report.md`, `reports/phase_4_targeted_re_review.md`, Agent 04b runner output marker, Phase 5 prompts for Agent 06 and Agent 07
- decision: Phase 4 targeted re-review result is normalized as `APPROVED_BY_PHASE_LEADER`; Agent 06 runtime update and Agent 07 parser hardening are marked `READY` for parallel dispatch. No technical approval of Phase 5 is performed.
- reason: Agent 08 recorded `PHASE_GO_WITH_NOTES` for the targeted Phase 4 re-review, the non-empty review/remediation reports exist, and Agent 04b produced `AGENT_STATUS: DONE`. Task queue parallel rules allow Phase 5 Agent 06 and Agent 07 to run together because their report/diagnostic and optional parser hardening ownership boundaries do not inherently conflict.
- risks: No real OBC runtime smoke has been executed; old FTP rollback paths remain; OBC wire incompatibility, parser bounds, RDP delayed close/leak, Ryu cleanup without old `ftp_abort()`, callback synchronous-lifetime assumption, and explicit old/Ryu symbol coexistence remain open until Phase 5/Supervisor review.
- file_ownership_issues: None in this Orchestrator run. Only state documents were edited. Agent 06 must not change protocol/core behavior; Agent 07 must keep hardening separate from the literal baseline and must not modify old `client.c`, integration source, OBC server, or CSP/libcsp core.
- next_agent: `agent_06_runtime_connection_diag agent_07_parser_hardening`
- handoff: Run `prompts/agent_06_runtime_connection_diag.md` and `prompts/agent_07_parser_hardening.md` in parallel. Agent 06 should correlate `[FTP_RYU_TEST][id=...]` logs, `miman_ftp_get_last_transfer_status()`, close return values, connection table snapshots at baseline/0/5/10/30 seconds, and repeated upload/download outcomes. Agent 07 should provide optional/applied patch status, malformed tests, validation matrix, build/test result, and baseline-vs-hardening separation. After both complete, Orchestrator must mechanically verify outputs and then request Agent 08 Phase 5 review.
- user_confirmation_required: `no`

---

### [2026-06-16 16:06:39 KST] [Phase 5 Hardening Diagnostics Review]
- timestamp: `2026-06-16 16:06:39 KST (+0900)`
- actor: `agent_08_phase_leader`
- actor_role: `phase_review`
- reviewed_phase: `Phase 5 Hardening/Diagnostics`
- reviewed_agent: `agent_06_runtime_connection_diag`, `agent_07_parser_hardening`
- reviewed_outputs: `reports/runtime_connection_report.md`, `reports/20260616_154913_agent06_diagnostic.diff`, `logs/20260616_154913_agent06_build_status.txt`, `logs/20260616_154913_agent06_runtime_table_dumps.md`, Agent 06 output marker, `reports/parser_hardening_report.md`, `reports/parser_hardening_optional.patch`, `reports/parser_hardening_status_tests.c`, Agent 07 output marker, Phase 5 checkpoint, current source/diff/grep evidence, `reports/phase_5_review.md`
- decision: `PHASE_RETRY_REQUIRED`
- reason: Agent 06 produced a default-off diagnostic wrapper and both diagnostics-off/on builds linked, but no live Ryu transfer was executed and no required baseline/0/5/10/30 table dumps, close-return correlation, `RDP_CLOSE_WAIT` timeline, baseline/peak/final counts, or pool-exhaustion sequence were measured. Agent 07 satisfied optional parser hardening separation, validation matrix, malformed harness, build/test, and patch-state requirements, so parser hardening itself does not require retry.
- risks: Runtime delayed-close versus leak is still unresolved; parser hardening is optional and not reflected in the product binary; `ftpnew_client.c` checksum manifest mismatch weakens literal-baseline evidence until disposed; no real OBC/simulator smoke has been executed; OBC wire compatibility, cleanup without old `ftp_abort()`, and callback synchronous-lifetime assumptions remain open.
- file_ownership_issues: No Phase Leader source/CMake edits. Agent 07 made no forbidden source/integration/old-client/OBC/CSP edits. Agent 06 diagnostic-only changes are within the approved logging wrapper scope while `FTP_RYU_CONN_DIAG` remains default-off. No legacy `client.c` Ryu mixture or hidden old/new mixture was found in reviewed evidence.
- next_agent: `agent_06_runtime_connection_diag`
- retry_agents: `agent_06_runtime_connection_diag`
- required_retry_items: Run diagnostics-enabled Ryu upload/download on a real endpoint, simulator, or safe harness; capture transfer-correlated `csp_connect_return`, `csp_close_return`, connection pointer/index, connection table dumps at baseline/post-connect/post-close/complete+0s/+5s/+10s/+30s, RDP state and `RDP_CLOSE_WAIT` residency; provide baseline/peak/final counts and pool-exhaustion sequence; execute or environment-block normal/timeout/remote-error/abort/burst upload/download cases without changing protocol/core/parser state.
- handoff: Orchestrator must keep Phase 5 in retry state and dispatch only Agent 06. Preserve Agent 07 optional hardening artifacts as reviewed; do not dispatch Supervisor until a new Phase 5 review has live runtime evidence or a documented repeated environment blocker.
- supervisor_handoff: Carry forward optional-unapplied parser hardening, unmeasured runtime close/leak behavior, `ftpnew_client.c` checksum mismatch, no real OBC smoke, OBC protocol compatibility risk, and Ryu cleanup/callback lifetime assumptions.
- user_confirmation_required: `no`

---

### [2026-06-16 19:29:15 KST] [Agent 06 Runtime-Limited Diagnostic Handoff]
- timestamp: `2026-06-16 19:29:15 KST (+0900)`
- actor: `agent_06_runtime_connection_diag`
- actor_role: `runtime_diagnostic_worker`
- reviewed_phase: `Phase 5 Runtime/Connection Diagnostic retry`
- reviewed_outputs: `reports/runtime_connection_report.md`, `logs/20260616_192915_agent06_runtime_limited_readiness.log`, `logs/20260616_192915_agent06_runtime_limited_table_dumps.md`, diagnostic hook presence in `src/miman_ftprdp.cpp` and `lib/libftp_client_ryu/`
- decision: `DONE` for runtime-limited readiness; live runtime/OBC evidence remains open
- reason: Diagnostic hooks for transfer ID, `csp_connect`, connection pointer/index, `csp_close`, RDP state, close-wait age, and table dumps are present and a fresh `FTP_RYU_CONN_DIAG=ON` build linked `BEE-1000`. The local executable probe failed before transfer startup with `Glfw Error 65544: X11: Failed to open display :0`, and this environment exposes no real OBC, CSP endpoint, simulator, or headless Ryu upload/download runner. Live table dumps and RDP close-wait evidence cannot be truthfully produced here.
- risks: Runtime delayed-close versus leak remains unresolved; baseline/peak/final connection counts are unmeasured; pool exhaustion sequence is unmeasured; cleanup behavior without old `ftp_abort()` remains unverified on real transfers; `CSP_CONN_MAX=10` should remain unchanged until live evidence is collected.
- file_ownership_issues: This retry changed only runtime report/state/log documentation. It did not modify protocol behavior, packet layout, transfer algorithm, legacy `client.c`, preserved Ryu source, or diagnostic source hooks.
- next_agent: `agent_08_phase_leader`
- handoff: Treat Agent 06 as diagnostically ready but runtime-limited. Phase Leader/Supervisor must carry live GS/OBC validation as an explicit open risk. Manual procedure is documented in `reports/runtime_connection_report.md` and requires Ryu upload/download, baseline/complete+0/+5/+10/+30 connection table snapshots, transfer ID correlation, connect/close return evidence, src/dst/ports, RDP state/flags, active count, `CSP_CONN_MAX`, and pool exhaustion sequence if reproduced.
- user_confirmation_required: `no`

---

### [2026-06-16 19:38:43 KST] [Phase 5 Runtime-Limited Targeted Re-review]
- timestamp: `2026-06-16 19:38:43 KST (+0900)`
- actor: `agent_08_phase_leader`
- actor_role: `phase_review`
- reviewed_phase: `Phase 5 Runtime/Connection Diagnostic retry targeted re-review`
- reviewed_agent: `agent_06_runtime_connection_diag`, `agent_07_parser_hardening`
- reviewed_outputs: `reports/runtime_connection_report.md`, `reports/phase_5_review.md`, `reports/parser_hardening_report.md`, `reports/thread_arg_ownership_report.md`, `logs/20260616_192754_agent_06_runtime_connection_diag_runtime_limited.out`, `logs/20260616_192915_agent06_runtime_limited_readiness.log`, `logs/20260616_192915_agent06_runtime_limited_table_dumps.md`, `reports/phase_5_runtime_limited_re_review.md`
- decision: `PHASE_GO_WITH_NOTES`
- reason: Agent 06 did not claim live OBC/runtime evidence and explicitly documented that no live Ryu upload/download, connection table dump, close-return correlation, RDP close-wait timeline, or pool-exhaustion sequence was executed in this Codex environment. The diagnostic-enabled build linked, the expected log schema covers transfer IDs, connect/close returns, connection pointer/index, RDP state, close-wait age, `CSP_CONN_MAX`, active open count, and pool exhaustion criteria, and the manual live test procedure is specific enough for real GS/OBC or simulator execution. Agent 07 parser hardening remains accepted as optional and unapplied.
- risks: Live runtime validation is still pending; no real OBC smoke was executed; delayed close versus leak remains unmeasured; baseline/peak/final open counts and pool exhaustion sequence remain unmeasured; cleanup without old `ftp_abort()` remains unverified; parser hardening is not reflected in the product binary; `ftpnew_client.c` checksum manifest mismatch remains for final disposition.
- file_ownership_issues: This targeted re-review modified only `reports/phase_5_runtime_limited_re_review.md` and this decision log. No source, header, CMake, build file, preserved FTP_Ryu source, legacy `client.c`, or protocol core file was modified.
- next_agent: `agent_09_supervisor`
- handoff: Phase 6 Supervisor may proceed with notes. Carry forward that live runtime validation is still pending and that Phase 5 is diagnostically ready but not live OBC validated. Require the documented manual procedure on real GS/OBC or simulator before final replacement/deployment approval.
- user_confirmation_required: `no`

---

### [2026-06-16 19:44:19 KST] [Phase 6 Supervisor Audit Unlocked]
- timestamp: `2026-06-16 19:44:19 KST (+0900)`
- actor: `agent_00_orchestrator`
- actor_role: `orchestrator`
- reviewed_phase: `Phase 5 completion normalization and Phase 6 dispatch readiness`
- reviewed_agent: `agent_06_runtime_connection_diag`, `agent_07_parser_hardening`, `agent_08_phase_leader`, `agent_09_supervisor`
- reviewed_outputs: `reports/runtime_connection_report.md`, `reports/phase_5_runtime_limited_re_review.md`, `logs/20260616_192754_agent_06_runtime_connection_diag_runtime_limited.out`, `logs/20260616_193744_agent_08_phase5_runtime_limited_re_review.out`, `reports/parser_hardening_report.md`, `state/agent_status.md`, `state/task_queue.md`
- decision: Phase 5 runtime-limited targeted re-review result is normalized as `APPROVED_BY_PHASE_LEADER`; Agent 09 Supervisor is marked `READY` for a single independent final audit. No technical approval or final GO/NO-GO judgement is performed by Orchestrator.
- reason: Required latest outputs are present and non-empty, Agent 06 runtime-limited retry output contains `AGENT_STATUS: DONE`, Agent 08 targeted re-review output and report contain `PHASE_DECISION: PHASE_GO_WITH_NOTES`, and the task queue unlock rule allows Supervisor after Phase 1~5 Phase Leader approval. Agent 07 remains accepted as optional/unapplied hardening from the prior Phase 5 review.
- risks: Live Ryu/OBC runtime validation remains pending; no real OBC smoke or simulator transfer was executed; connection table dumps, close-return recovery, `RDP_CLOSE_WAIT` residency, pool-exhaustion sequence, and cleanup without old `ftp_abort()` are unmeasured; parser hardening is optional and unapplied; `ftpnew_client.c` checksum manifest mismatch remains for final disposition; old/Ryu coexistence remains a test/rollback state until Supervisor/Final Manager disposition.
- file_ownership_issues: None in this Orchestrator run. Only `docs/ftp_ryu_migration/state/agent_status.md`, `docs/ftp_ryu_migration/state/task_queue.md`, and this decision log were edited.
- next_agent: `agent_09_supervisor`
- handoff: Run `prompts/agent_09_supervisor.md` exactly. Supervisor must independently audit orchestration state versus outputs, Phase Leader notes, old/new hidden mixture, build/symbol/link evidence, wire compatibility, parser hardening status, runtime diagnostic limits, rollback, ownership, and unresolved risks. Supervisor must not modify source/header/CMake/build scripts or state dispatch documents and must not treat Phase 5 as live OBC validated.
- user_confirmation_required: `no`

---

### [2026-06-16 19:58:00 KST] [Supervisor NO-GO Remediation Routed]
- timestamp: `2026-06-16 19:58:00 KST (+0900)`
- actor: `agent_00_orchestrator`
- actor_role: `orchestrator`
- reviewed_phase: `Phase 6 Supervisor NO-GO remediation routing`
- reviewed_agent: `agent_09_supervisor`, `agent_05_protocol_wire_check`, `agent_06_runtime_connection_diag`, `agent_03_ryu_client_port`, `agent_07_parser_hardening`, `agent_02_build_cmake`, `agent_04_ftprdp_integration`, `agent_10_final_manager`
- reviewed_outputs: `logs/20260616_194611_agent_09_supervisor.out`, `reports/supervisor_review.md`, `state/agent_status.md`, `state/task_queue.md`, existing Worker prompts and ownership policy
- decision: Supervisor `NO_GO` is accepted as the current final audit disposition. Agent 10 remains blocked. Remediation is routed to exact retry targets `agent_05_protocol_wire_check agent_06_runtime_connection_diag agent_03_ryu_client_port agent_07_parser_hardening agent_02_build_cmake agent_04_ftprdp_integration`; first-wave parallel dispatch is Agents 05/06/03/07, with Agents 02/04 blocked until first-wave evidence is available.
- reason: Supervisor produced the required non-empty report and `SUPERVISOR_DECISION: NO_GO` marker. The reported blockers map to distinct Worker ownership: Agent 05 for OBC wire compatibility or fix acceptance, Agent 06 for live/simulator runtime validation, Agent 03 for Ryu checksum/source provenance, Agent 07 for parser hardening application or risk disposition, and Agents 02/04 for final build-profile old FTP implementation exclusion and final switch/rollback evidence. Technical approval is not performed by Orchestrator.
- risks: OBC wire compatibility remains unresolved; no live Ryu/OBC runtime validation exists; final executable still links old FTP implementation objects; checksum manifest provenance is stale; parser hardening is unapplied. OBC server code edits, CSP/libcsp core behavior changes, protocol-incompatible changes without OBC-side documentation, and irreversible deletion still require confirmation under policy.
- file_ownership_issues: No new file ownership violation by Orchestrator. This routing changed only `docs/ftp_ryu_migration/state/agent_status.md`, `docs/ftp_ryu_migration/state/task_queue.md`, and this decision log.
- next_agent: `agent_05_protocol_wire_check agent_06_runtime_connection_diag agent_03_ryu_client_port agent_07_parser_hardening`
- handoff: Run first-wave remediation in parallel if environment permits. Agent 05 must obtain/validate actual flight OBC compatibility or document exact required OBC-side fix acceptance without modifying protocol/core behavior. Agent 06 must run live GS/OBC, simulator, or safe headless harness Ryu upload/download diagnostics and capture required table dumps/counts/CRC/RDP evidence, or report exact environment blockers. Agent 03 must re-own checksum/deviation evidence or restore literal baseline plus an approved diagnostic-wrapper boundary. Agent 07 must submit applied-hardening evidence or an explicit operational-risk package with tests/build/checksum implications. After first-wave output verification, dispatch Agents 02/04 together for final build-profile old FTP implementation exclusion and final switch/rollback/no-hidden-mixture evidence, then request Agent 08 targeted re-review before any new Supervisor audit.
- user_confirmation_required: `no`

---

### [2026-06-16 20:19:49 KST] [Final Manager NO-GO Report Submitted]
- timestamp: `2026-06-16 20:19:49 KST (+0900)`
- actor: `agent_10_final_manager`
- actor_role: `final_manager`
- reviewed_phase: `Final reporting after Supervisor audit`
- reviewed_agent: `agent_09_supervisor`, `agent_06_runtime_connection_diag`, `agent_05_protocol_wire_check`, `agent_08_phase_leader`, `agent_04_ftprdp_integration`, `agent_04b_thread_arg_ownership`, `agent_02_build_cmake`, `agent_07_parser_hardening`
- reviewed_outputs: `reports/supervisor_review.md`, `reports/runtime_connection_report.md`, `reports/protocol_wire_report.md`, `reports/phase_5_runtime_limited_re_review.md`, `reports/thread_arg_ownership_report.md`, `reports/ftprdp_integration_report.md`, `reports/build_cmake_report.md`, `reports/parser_hardening_report.md`, latest `docs/ftp_ryu_migration/logs` listing, this decision log
- decision: `FINAL_REPORTED_NO_GO`
- reason: Supervisor `NO_GO` is preserved unchanged. The final manager report separates completed code-port/build/static diagnostic work from unresolved deployment blockers: OBC wire incompatibility, no live Ryu/OBC runtime validation, old FTP objects still linked in `BEE-1000`, stale Ryu checksum provenance, and parser hardening optional/unapplied.
- risks: Final deployment remains blocked. The current state is test/rollback coexistence and diagnostic readiness only; it is not a final migration success or deployment `GO`.
- file_ownership_issues: None. Agent 10 modified only `docs/ftp_ryu_migration/reports/final_manager_nogo_report.md` and this append-only decision log entry.
- next_agent: `agent_05_protocol_wire_check agent_06_runtime_connection_diag agent_03_ryu_client_port agent_07_parser_hardening agent_02_build_cmake agent_04_ftprdp_integration`
- handoff: Use `reports/final_manager_nogo_report.md` as the user-submittable final state report. Continue remediation under the Supervisor NO-GO blockers before any new final audit or deployment decision.
- user_confirmation_required: `no`
