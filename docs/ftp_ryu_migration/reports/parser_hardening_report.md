# Parser Hardening Report

Run ID: `20260616_154913`  
Agent: `agent_07_parser_hardening`  
Patch state: **optional only, not applied to literal baseline**

## Scope

Reviewed and prepared hardening for the Ryu upload-side status reply parser in
`lib/libftp_client_ryu/port/client/src/ftpnew_client.c`.

No parser hardening changes were applied to the current Ryu port source, old
`lib/gscsp/lib/libftp_client/src/client.c`, integration source, OBC/server
source, or CSP/libcsp core. The current Ryu source still contains the
unbounded status call at `ftpnew_client.c:220`, confirming the optional patch is
not applied.

Residual artifact note: `lib/libftp_client_ryu/checksums.sha256` records
`port/client/src/ftpnew_client.c` as
`43515728506b1f9b17c3ad86e90dfb7fa16af8a7c507c135a8a58e78ad8dda1a`,
while the current file hashes to
`0f4c16cc67b0ae10a610f6194ab97e8d9180095de828808d63a7ad3681655f35`.
This mismatch predates the optional hardening artifact because the current
source exactly matches the temporary before-copy used to generate the patch and
does not contain `ftpnew_validate_status_reply`.

Artifacts produced:

- Optional patch: `docs/ftp_ryu_migration/reports/parser_hardening_optional.patch`
- Standalone malformed tests: `docs/ftp_ryu_migration/reports/parser_hardening_status_tests.c`
- This report: `docs/ftp_ryu_migration/reports/parser_hardening_report.md`

## Finding

The current Ryu status request path uses:

```c
csp_transaction_persistent(..., &rep, -1)
```

This preserves the literal baseline, but it has two hardening gaps:

1. libcsp skips reply length equality checks when `inlen == -1` and copies the
   received payload length into the fixed stack `ftp_packet_t rep`.
2. The parser converts `entries` and then iterates that value without checking
   `entries <= GS_FTP_STATUS_CHUNKS` or that the received payload contains the
   required entry array bytes.

## Optional Patch Behavior

The optional patch replaces the upload status transaction with explicit
`csp_send()` / `csp_read()` so the actual CSP payload length is available before
copying. It rejects packets larger than `sizeof(ftp_packet_t)` before `memcpy`.

Validation policy:

| Check | Policy | Failure |
|---|---|---|
| Actual length | `length >= 12 + 8 * entries` and `length <= sizeof(ftp_packet_t)` | `GS_ERROR_DATA` |
| Packet type | Must be `FTP_STATUS_REPLY` | `GS_ERROR_DATA` |
| Return code | `FTP_OK` required for parsing; non-OK maps through `wire_ret_to_gs()` | mapped error |
| Entries bound | `entries <= GS_FTP_STATUS_CHUNKS` | `GS_ERROR_DATA` |
| Progress fields | `complete <= total` and `total == state->chunks` | `GS_ERROR_DATA` |
| Complete reply | `complete == total` requires `entries == 0` | `GS_ERROR_DATA` |
| Incomplete reply | `complete < total` requires `entries > 0` | `GS_ERROR_DATA` |
| Entry count | `count > 0` | `GS_ERROR_DATA` |
| Overflow | `next <= UINT32_MAX - count` before `next + count` | `GS_ERROR_DATA` |
| Range | `next < total` and `next + count <= total` | `GS_ERROR_DATA` |
| Duplicate/overlap | Entries must be monotonic and non-overlapping (`next >= previous_end`) | `GS_ERROR_DATA` |

Cleanup behavior is unchanged: `ftpnew_upload()` already calls
`ftpnew_done(&state, 0)` for non-OK status/data/CRC stages, closing local file
handles, sending `FTP_DONE` when possible, closing the CSP connection, and
leaving no success rename path for failed uploads.

## Malformed Test Matrix

The standalone harness mirrors the optional validation helper and covers:

| Case | Expected |
|---|---|
| fixed 92-byte valid reply | accept |
| variable-length valid reply | accept |
| truncated entry payload | bounded `GS_ERROR_DATA` |
| `entries > GS_FTP_STATUS_CHUNKS` | bounded `GS_ERROR_DATA` |
| endian-crafted `entries == 256` | bounded `GS_ERROR_DATA` |
| `next + count` overflow | bounded `GS_ERROR_DATA` |
| `next >= total` | bounded `GS_ERROR_DATA` |
| `next + count > total` | bounded `GS_ERROR_DATA` |
| zero `count` | bounded `GS_ERROR_DATA` |
| overlapping ranges | bounded `GS_ERROR_DATA` |
| duplicate ranges | bounded `GS_ERROR_DATA` |
| `complete > total` | bounded `GS_ERROR_DATA` |
| incomplete reply with zero entries | bounded `GS_ERROR_DATA` |
| complete reply with nonzero entries | bounded `GS_ERROR_DATA` |
| reply `total` mismatches requested upload chunks | bounded `GS_ERROR_DATA` |

## Build And Test

Standalone malformed parser harness:

```sh
cc -std=c11 -Wall -Wextra -Werror \
  docs/ftp_ryu_migration/reports/parser_hardening_status_tests.c \
  -o /tmp/parser_hardening_status_tests
/tmp/parser_hardening_status_tests
```

Result: all 15 cases passed.

Product build after adding only report/test/patch artifacts:

```sh
cmake -S . -B /tmp/5th_gs_agent07_build \
  -DALLOW_OLD_FTP_BUILD_EXCLUSION=1 \
  -DALLOW_FINAL_REPLACEMENT=1
cmake --build /tmp/5th_gs_agent07_build --target BEE-1000 -j2
```

Result: `Built target BEE-1000`. Existing project warnings remain outside this
agent's scope. No source hardening patch was applied for this build.

Optional patch syntax:

```sh
git apply --check docs/ftp_ryu_migration/reports/parser_hardening_optional.patch
```

Result: success.

## Risk If Not Applied

If the optional patch remains unapplied, malformed or hostile status replies can
still:

- Overflow the fixed stack `ftp_packet_t rep` through unbounded `inlen == -1`.
- Cause out-of-bounds reads/writes by advertising more than 10 entries.
- Drive retransmission loops with zero-count, overlapping, duplicate, or
  out-of-range chunks.
- Hide contradictory progress such as `complete > total` or incomplete status
  with no retry entries until later CRC/timeouts.

Operational mitigation while deferred:

- Do not expose Ryu FTP upload to untrusted CSP nodes.
- Capture actual CSP payload length and first 92 status bytes during any Ryu
  test window.
- Keep old FTP rollback path active.
- Abort Ryu runtime tests on any status reply where `entries > 10`,
  `length < 12 + 8 * entries`, `complete > total`, or `next/count` ranges are
  not monotonic and inside `total`.

## Recommendation

Apply the optional patch before any non-lab Ryu upload testing against a real
OBC. Keep it recorded as a deliberate hardening deviation from the literal port,
not as part of the Phase 3 baseline.

## Agent 08 Handoff

- Patch status: **not applied**; baseline literal port remains preserved.
- Coverage: standalone malformed-status harness covers truncated, oversized
  entries, endian-crafted entries, overflow, range, zero-count, duplicate,
  overlap, and contradictory complete/total cases.
- Residual risk: optional patch has not been integrated into the actual Ryu
  target, so runtime protection is not present until Phase Leader approves and
  applies the deviation. The checksum manifest mismatch above also needs Agent
  03/08 disposition before relying on checksum evidence for literal-source
  identity.
- Literal-port deviation: applying the patch modifies
  `lib/libftp_client_ryu/port/client/src/ftpnew_client.c`; require checksum and
  source-diff report update before treating it as accepted migration code.
