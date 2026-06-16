# Protocol/Wire Compatibility Report

Run: `20260616_194256` remediation pass for supervisor NO_GO  
Scope: analysis/report artifacts only. No product source, header, CMake, or protocol behavior was modified.

## Verdict

**OBC V2.1 verdict: 비호환 (provided source), actual flight build unverified.**

The old GS client, FTP_Ryu client, and OBC V2.1 headers define the same legacy
packet numbers, packed field widths, and status offsets. The supplied OBC V2.1
implementation is nevertheless unsafe/incompatible:

1. It converts `statusrep->entries` to network order and then uses the converted
   value as a host loop bound. On the configured little-endian target, host
   `entries == 1` becomes integer `256`, causing out-of-bounds access.
2. Reply handlers write `((ftp_packet_t *) reply)->type`, although `reply` is a
   `csp_packet_t *` and the payload starts at `reply->data`. This writes packet
   metadata/padding, not the FTP payload type byte.
3. The old and Ryu upload status receivers pass `inlen == -1` with a fixed
   261-byte `ftp_packet_t` stack destination. libcsp then copies the complete
   received packet without a destination bound.

Ryu client vs a correctly implemented legacy server is **조건부 호환** for the
legacy transfer packets. Conditions are: packet type is in `packet->data`,
status fields are converted only after host-side iteration, reply length is
bounded/validated, and the server uses the legacy return-code meanings expected
by the selected client. FTP_Ryu's `ftp_ret_t` values are not wire-compatible
with legacy `gs_ftp_return_t` for failures, although success remains byte `0`.

Remediation update: the diagnostic was rebuilt as a standalone three-mode
tool (`WIRE_IMPL_OLD`, `WIRE_IMPL_RYU`, `WIRE_IMPL_OBC`) and was run against
the current in-tree Ryu port under `lib/libftp_client_ryu`, not only the older
external `../FTP_Ryu` path. It confirms the same legacy status byte positions
and lengths for old GS, Ryu, and supplied OBC V2.1 headers.

## Evidence Set

| Role | Evidence |
|---|---|
| old GS | `lib/gscsp/lib/libftp_client/include/gs/ftp/{types.h,internal/types.h}`, `src/client.c` |
| Ryu | `lib/libftp_client_ryu/port/client/src/ftpnew_client.c`, `lib/libftp_client_ryu/port/config/ftpnew_types*.h`, legacy GS headers included by the port |
| OBC V2.1 | `fsw/FTP V_2.1/io_csp_ftp.c`, bundled GS/CSP headers, `csp_autoconfig.h` |
| libcsp copy behavior | `lib/gscsp/lib/libgscsp/lib/libcsp/src/csp_io.c:444` |

No separate flight OBC repository, build manifest, object map, or capture was
found. `fsw/FTP V_2.1` has no Makefile/CMake/wscript. Its configured CSP target
is little-endian with `CSP_CONN_MAX=32`.

## Packet Type Matrix

All three use a one-byte packed legacy `gs_ftp_type_t` enum and a one-byte
`ftp_packet_t.type`. Ryu extension packet numbers are kept out of the legacy
range and are carried in `ftp_ext_packet_t.type`, also a one-byte field at
offset 0. The Ryu extension enum type itself is an unpacked 4-byte C enum, but
that enum is not embedded as the packet discriminator.

| Type | Value | Type | Value |
|---|---:|---|---:|
| UPLOAD_REQUEST / REPLY | 0 / 1 | DOWNLOAD_REQUEST / REPLY | 2 / 3 |
| STATUS_REQUEST / REPLY | 5 / 6 | LIST_REQUEST / REPLY / ENTRY | 7 / 8 / 9 |
| MOVE_REQUEST / REPLY | 10 / 11 | REMOVE_REQUEST / REPLY | 12 / 13 |
| CRC_REQUEST / REPLY | 14 / 15 | DATA / DONE / ABORT | 16 / 17 / 18 |
| MKFS_REQUEST / REPLY | 19 / 20 | ZIP_REQUEST / REPLY | 21 / 22 |
| COPY_REQUEST / REPLY | 23 / 24 | MKDIR_REQUEST / REPLY | 25 / 26 |
| RMDIR_REQUEST / REPLY | 27 / 28 | Ryu extensions | 50 through 66 (`FTP_TYPE_NONE=127`) |

Ryu extensions do not collide with legacy values. They require an FTP_Ryu
server and are not supported by OBC V2.1.

## ABI And Layout

The old and OBC internal headers differ only by the OBC disabling the
`GS_FTP_INTERNAL_USE` include guard. The current Ryu port includes the legacy
GS internal header for the transfer ABI and defines `GS_FTP_MAX_CHUNK_SIZE` as
`GS_FTP_CHUNK_SIZE`; both are 256 and produce the same ABI.

| Object | old | Ryu fixture | OBC V2.1 | Packing |
|---|---:|---:|---:|---|
| `gs_ftp_type_t` | 1 | 1 | 1 | packed enum |
| `gs_ftp_backend_type_t` | 1 | 1 | 1 | packed enum |
| `gs_ftp_return_t` / `ftp_ret_t` | 1 | 1 | 1 | packed enum |
| `ftp_upload_request_t` | 65 | 65 | 65 | packed |
| `ftp_upload_reply_t` | 1 | 1 | 1 | packed |
| `ftp_download_request_t` | 61 | 61 | 61 | packed |
| `ftp_download_reply_t` | 9 | 9 | 9 | packed |
| `ftp_data_t` | 260 | 260 | 260 | packed |
| `ftp_status_element_t` | 8 | 8 | 8 | packed |
| `ftp_status_reply_t` | 91 | 91 | 91 | packed |
| `ftp_crc_reply_t` | 5 | 5 | 5 | packed |
| `ftp_packet_t` | 261 | 261 | 261 | packed |

Ryu extension-only diagnostics from the in-tree port: `ftp_ext_type_t` is 4,
`ftp_ext_log_reply_t` is 163, `ftp_ext_ram_write_request_t` is 264, and
`ftp_ext_packet_t` is 265. These packets require an FTP_Ryu server and are
outside the OBC V2.1 legacy compatibility surface.

Status offsets are identical:

| Field | status payload offset | full FTP packet offset | width | wire order |
|---|---:|---:|---:|---|
| `type` | n/a | 0 | 1 | byte |
| `ret` | 0 | 1 | 1 | byte |
| `complete` | 1 | 2 | 4 | big/network |
| `total` | 5 | 6 | 4 | big/network |
| `entries` | 9 | 10 | 2 | big/network |
| `entry[0].next` | 11 | 12 | 4 | big/network |
| `entry[0].count` | 15 | 16 | 4 | big/network |
| `entry[N]` | `11 + 8*N` | `12 + 8*N` | 8 | big/network |

`FTP_DATA.chunk` is the intentional legacy exception: little-endian on wire.
Old upload and OBC V2.1 use `htole32/letoh32`; Ryu preserves this behavior.

## Status Length

The packed fixed status struct is 91 bytes; including packet type it is 92.
The semantically required variable length is:

`required_length(entries) = 12 + 8 * entries`, for `0 <= entries <= 10`.

| Entries | Required | Entries | Required |
|---:|---:|---:|---:|
| 0 | 12 | 5 | 52 |
| 1 | 20 | 6 | 60 |
| 2 | 28 | 7 | 68 |
| 3 | 36 | 8 | 76 |
| 4 | 44 | 9 | 84 |
| 10 | 92 | | |

All reviewed senders currently transmit the fixed 92 bytes. A parser must
accept a fixed 92-byte reply, but should validate `received >= 12 + 8*entries`
and reject `entries > 10`. Neither old nor Ryu status request does this.

## Endian Flow

| Flow | Sender | Receiver | Finding |
|---|---|---|---|
| upload status reply | OBC V2.1 builds host `complete/total/entries/next/count`; converts all to network order | old/Ryu converts header then each entry | OBC loop-order bug corrupts entries after `entries` conversion |
| upload status reply | Ryu server converts each entry while host `entries` remains local, then converts header | old/Ryu client | correct order |
| download status reply | old/Ryu client converts each entry during construction, then header | OBC/Ryu server converts header before loops, then entries | correct order in Ryu; OBC receiver lacks bounds/length checks |
| data chunk index | old/Ryu sender uses little-endian | OBC/Ryu receiver uses little-endian | compatible legacy exception |

OBC defect location: `io_csp_ftp.c:269-276`. The loop must use a saved host
entry count or run before `entries = hton16(entries)`. This is an OBC-owned
change and was not made by this agent.

## `16777216` Finding

The hypothesis is verified by source order and byte arithmetic.

For `count == 1`, network bytes are:

```text
00 00 00 01
```

Reading those four bytes directly as a little-endian host `uint32_t` yields
`0x01000000 == 16777216`. Applying `csp_ntoh32` yields `1`.

Representative one-entry status packet:

```text
06 00 00 00 00 00 00 00 00 01 00 01 00 00 00 00 00 00 00 01
|  |complete=0 |total=1    |ent=1|next=0     |count=1
```

Length is 20 bytes in variable form or 92 bytes with nine zero-filled unused
entries. Capture is still required to prove which deployed binary emitted the
observed log.

## Chunk And Transaction Bounds

| Implementation | default chunk | accepted maximum | data wire maximum |
|---|---:|---:|---:|
| old GS | 185 | not enforced by getter | 256 |
| Ryu | 200 | 256 via `GS_FTP_MAX_CHUNK_SIZE` | 256 |
| OBC V2.1 | request supplied | no explicit `<=256` check | struct capacity 256 |

OBC V2.1 must reject `chunk_size == 0` and `chunk_size > 256` before file/data
processing. The supplied source does neither.

Old `client.c:498`, `force.c:596`, and current Ryu
`lib/libftp_client_ryu/port/client/src/ftpnew_client.c:221` use
`csp_transaction_persistent(..., &rep, -1)`. libcsp `csp_io.c:469-475` skips
the length check for `-1` and copies `packet->length` bytes into `rep`.
Use a bounded receive API or an exact 92-byte bound, retain the actual received
length, and validate it before field access.

Ryu extension helpers also use `inlen == -1` at `ftpnew_client.c:1002`,
`:1046`, and `:1219`; those are outside OBC V2.1 legacy status compatibility
but need the same bounded-receive policy if the extension client remains
enabled.

## Capture Procedure

Capture at the FTP payload boundary, before any endian conversion:

1. GS TX/RX: immediately before/after `csp_transaction_persistent` or
   `csp_read`, dumping exactly the returned CSP payload length.
2. OBC RX: after `csp_read` and before the packet-type switch.
3. OBC TX: after constructing `reply->data`, before `csp_send`.
4. Record build ID/git SHA, node/port, direction, CSP payload length, and raw
   hex. Do not dump a C struct using `sizeof` when the received length is known.

Host decode example:

```sh
printf '%s\n' \
  '06 00 00 00 00 00 00 00 00 01 00 01 00 00 00 00 00 00 00 01' |
python3 -c 'import sys,struct; b=bytes.fromhex(sys.stdin.read()); print(
{"type":b[0],"ret":b[1],"complete":int.from_bytes(b[2:6],"big"),
"total":int.from_bytes(b[6:10],"big"),"entries":int.from_bytes(b[10:12],"big"),
"next":int.from_bytes(b[12:16],"big"),"count":int.from_bytes(b[16:20],"big")})'
```

Expected decode: `type=6, ret=0, complete=0, total=1, entries=1, next=0,
count=1`.

The independent diagnostic is `protocol_wire_layout.c`. Example commands:

```sh
cc -std=c11 -DWIRE_IMPL_OLD \
  -Ilib/gscsp/lib/libftp_client/include \
  -Ilib/gscsp/lib/libutil/include \
  docs/ftp_ryu_migration/reports/protocol_wire_layout.c -o /tmp/wire-old

cc -std=c11 -DWIRE_IMPL_RYU \
  -Ilib/libftp_client_ryu/port/config \
  -Ilib/gscsp/lib/libftp_client/include \
  -Ilib/gscsp/lib/libutil/include \
  docs/ftp_ryu_migration/reports/protocol_wire_layout.c -o /tmp/wire-ryu

cc -std=c11 -DWIRE_IMPL_OBC -I'fsw/FTP V_2.1' \
  docs/ftp_ryu_migration/reports/protocol_wire_layout.c -o /tmp/wire-obc
```

## Blockers And Required Evidence

1. Obtain the actual flight OBC source revision and the exact
   `gs/ftp/internal/types.h`, `csp_types.h`, `csp_endian.h`, and
   `csp_autoconfig.h` used to build it.
2. Obtain compiler/version/options proving packed-enum support and target
   endian, plus the OBC FTP object/library build ID or map.
3. Capture one upload status reply with `entries=1`, including CSP payload
   length and the first 20 bytes.
4. Confirm whether deployed OBC writes `reply->data[0]` or contains the supplied
   `((ftp_packet_t *)reply)->type` defect.
5. Confirm deployed server return-code enum. Ryu and legacy failure values
   differ.

## Handoff

**Agent 07:** enforce `12 <= received_length <= sizeof(ftp_packet_t)`,
`entries <= 10`, `received_length >= 12 + 8*entries`,
`complete <= total`, `next < total`, `count <= total-next`, overflow-safe
`next+count`, and `chunk_size in [1,256]`. Remove unbounded `inlen=-1`.

**Agent 08:** do not approve OBC V2.1 compatibility from the supplied source.
Required OBC actions are: fix payload type placement, preserve host `entries`
until entry conversion completes, validate request/status/data lengths and
chunk bounds, and provide actual-build/capture evidence. Ryu extensions and
Ryu error-code semantics require an FTP_Ryu server or an explicit adapter.

## Build Status

Product build was not run because this agent is analysis-only and source/CMake
changes are forbidden. Existing evidence includes `build/BEE-1000` as an
x86-64 ELF and `../FTP_Ryu/server/build/arm32/libftpnew_server.so.1` as a
32-bit little-endian ARM ELF, but neither proves the supplied OBC V2.1 source
was the deployed build.

The independent diagnostic compiled and ran successfully against all three
current header sets during this remediation pass:

```sh
cc -std=c11 -DWIRE_IMPL_OLD ... && /tmp/wire-old
cc -std=c11 -DWIRE_IMPL_RYU ... && /tmp/wire-ryu
cc -std=c11 -DWIRE_IMPL_OBC ... && /tmp/wire-obc
```

Each run produced the packet values, sizes, offsets, and status lengths
recorded above. Product build was still not run by this analysis-only agent.
