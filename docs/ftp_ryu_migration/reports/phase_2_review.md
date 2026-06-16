# Phase 2 Build/CMake Review

## Reviewed phase

`Phase 2 Build/CMake`

## Reviewed agents

`agent_02_build_cmake`

## Reviewed outputs

- `lib/libftp_client_ryu/CMakeLists.txt`
- Top-level `CMakeLists.txt` one-line subdirectory addition
- `reports/build_cmake_report.md`
- Agent 02 runner output and full log
- Agent 02 verbose/clean build logs and linker map under `/tmp`
- Independent Phase Leader configure/build, archive, symbol, link-map, checksum,
  source-diff, and existing executable build checks

## Missing outputs

None for the Phase 2 checkpoint.

## Checkpoint evidence

- Separate target: clean configure and `--clean-first --verbose` build of
  `ftp_client_ryu` succeeded independently. The archive contains only
  `ftpnew_client.c.o` and `ftpnew_client_crc32.c.o`.
- Symbol isolation: the archive defines 14 `ftpnew_*` globals and no
  `gs_ftp_*`, `ftp_done`, or `ftpavailable`. The Ryu-only link map selected
  both Ryu objects and no old `client.c.2.o`, `force.c.2.o`, or
  `cmd_ftp.c.2.o`.
- Existing FTP preservation: git diff is empty for
  `src/miman_ftprdp.cpp`, `lib/gscsp/lib/libftp_client/src/client.c`, and
  `force.c`; their SHA-256 values match the Agent 02 report.
- Minimal CMake change: the parent change is one `ADD_SUBDIRECTORY` line.
  Target sources, includes, definitions, and the imported CSP archive are
  confined to the new owned directory.
- Build result: independent Release configure, clean target build, and full
  `BEE-1000` build returned exit status 0. The existing executable link line
  contains `libmimancsp.a` but no `ftp_client_ryu` or `ftpnew` input.

## File ownership violations

None found. Agent 02 changed only its approved new CMake directory, the minimal
parent CMake entry, and its build report. No Worker source, integration source,
or external FTP_Ryu source was modified.

## Technical risks

- Three target-private generic callback aliases map download callbacks onto
  upload event values. They prove compilation only and are not semantically
  integration-ready.
- The Phase 2 target intentionally compiles external `../FTP_Ryu` sources.
  Agent 03 must replace these paths with the approved in-tree literal port.
- The target imports the prebuilt `lib/gscsp/build/libmimancsp.a`; the clean
  target build does not rebuild that dependency. The archive/link isolation
  probe is sufficient for this phase, but Phase 3 must retain reproducible
  dependency evidence for the port target.
- Existing project builds emit many pre-existing warnings. No warning observed
  in the two Ryu C source compilations was introduced by this target.

## Decision

`PHASE_GO_WITH_NOTES`

The independent archive, namespace isolation, old-client preservation, minimal
CMake change, and existing executable build satisfy the Phase 2 gate. This is
not approval to link the Ryu target into the product.

## Required retry items

None.

## Handoff to Orchestrator

Mark Phase 2 and Agent 02 approved by the Phase Leader and make only Agent 03
ready. Do not dispatch Agent 04. Pass Agent 03 the target name
`ftp_client_ryu`, alias `FTP_Ryu::client`, public header contract, source
allowlist, checksums, and the mandatory callback-alias removal requirement.

## Handoff to Supervisor if needed

Retain the callback semantic limitation, external-source-to-in-tree transition,
prebuilt CSP archive dependency, and proof that the product executable remains
unlinked from `ftp_client_ryu` at Phase 2.
