# Phase 3 Ryu Client Port Review

## Reviewed phase

`Phase 3 Ryu Client Port`

## Reviewed agents

`agent_03_ryu_client_port`

## Reviewed outputs

- `lib/libftp_client_ryu/port/` literal source/header/config tree
- `lib/libftp_client_ryu/adapter/` compatibility and callback adapter
- `lib/libftp_client_ryu/checksums.sha256`
- `lib/libftp_client_ryu/ryu_source_diff.md`
- Updated owned target file `lib/libftp_client_ryu/CMakeLists.txt`
- Agent 03 runner output/full log
- Independent checksum, byte comparison, git diff, clean build, archive, symbol,
  and global-object checks

## Missing outputs

None for the Phase 3 checkpoint.

## Checkpoint evidence

- Original preservation: `sha256sum -c checksums.sha256` passed for all seven
  source/port pairs and all port-owned build/adapter files. Independent `cmp`
  found no difference in any source/header/config pair.
- Adapter scope: preserved files have zero deviations. Compatibility is
  confined to `ryu_ftp_compat.h`; transfer callback direction conversion is
  confined to `ryu_ftp_client_adapter.c`.
- Diff record: `ryu_source_diff.md` lists the complete 1:1 source mapping,
  zero preserved-source deviations, adapter rationale, build/symbol results,
  and Agent 04/05/07 handoffs.
- Old-client isolation: git diff is empty for
  `lib/gscsp/lib/libftp_client/src/client.c`,
  `lib/gscsp/lib/libftp_client/src/force.c`, and
  `src/miman_ftprdp.cpp`. Searches found no `ftpnew_*` or `ryu_ftp_*` code in
  those files.
- Port build: independent clean verbose build of `ftp_client_ryu` succeeded.
  The archive contains only `ftpnew_client.c.o`,
  `ftpnew_client_crc32.c.o`, and `ryu_ftp_client_adapter.c.o`.
- Symbols/state: the archive defines the 14 literal `ftpnew_*` functions and
  `ryu_ftp_upload`/`ryu_ftp_download`; it defines no old `gs_ftp_*`,
  `ftp_done`, or `ftpavailable` symbols and no global/weak object symbols.

## File ownership violations

None found. Agent 03 changes are confined to the approved new Ryu port,
adapter, checksum/diff report, and its owned target file. Existing FTP,
integration source, top-level CMake, and external FTP_Ryu files were not
changed by this phase.

## Technical risks

- The literal source must compile with generic callback names mapped to upload
  event values. Direction-correct download events are guaranteed only through
  `ryu_ftp_download`; Agent 04 must not call raw `ftpnew_download` for the
  product transfer path.
- The public target still exposes literal `ftpnew_*` extension APIs. Phase 4
  must distinguish intentional extension calls from an accidental transfer
  adapter bypass when checking hidden old/new or semantic mixture.
- The target still imports the prebuilt `lib/gscsp/build/libmimancsp.a`; a
  clean `ftp_client_ryu` build does not rebuild that dependency.
- Literal baseline defects and protocol/parser risks remain intentionally
  unchanged for later protocol, integration, and hardening review.

## Decision

`PHASE_GO_WITH_NOTES`

The literal baseline is traceable, all deviations are outside the preserved
tree and explained, the adapter is narrowly scoped, the target rebuilds, and
no old client mixture is present. This is not approval of product integration
behavior or protocol compatibility.

## Required retry items

None.

## Handoff to Orchestrator

Mark Phase 3 and Agent 03 approved by the Phase Leader and make only Agent 04
ready. Agent 04 must include `ryu_ftp_client_adapter.h`, link
`FTP_Ryu::client`, use `ryu_ftp_upload`/`ryu_ftp_download` for transfer paths,
and provide call-graph evidence that no raw transfer API or old client path
bypasses the adapter. Do not dispatch Phase 5 Workers.

## Handoff to Supervisor if needed

Retain the zero-diff baseline checksums, adapter-bypass risk, public raw
extension API surface, prebuilt CSP archive dependency, and the fact that
protocol/parser/runtime risks are not resolved by the Phase 3 port.
