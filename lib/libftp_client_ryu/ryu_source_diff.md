# Ryu Client Literal Port Report

Run ID: `20260615_224121`

## Port Tree

The following files are preserved byte-for-byte:

| FTP_Ryu source | In-tree port |
|---|---|
| `client/inc/ftpnew_client.h` | `port/client/inc/ftpnew_client.h` |
| `client/inc/ftpnew_client_config.h` | `port/client/inc/ftpnew_client_config.h` |
| `client/src/ftpnew_client.c` | `port/client/src/ftpnew_client.c` |
| `client/src/ftpnew_client_crc32.c` | `port/client/src/ftpnew_client_crc32.c` |
| `config/ftpnew_config.h` | `port/config/ftpnew_config.h` |
| `config/ftpnew_types.h` | `port/config/ftpnew_types.h` |
| `config/ftpnew_types_internal.h` | `port/config/ftpnew_types_internal.h` |

`cmp` and `diff -u` found no differences for all seven pairs. The source and
port SHA-256 values are recorded in `checksums.sha256`.

## Adapter Boundary

`adapter/include/ryu_ftp_compat.h` supplies only the vocabulary missing from
the current 5th_GS headers:

- `GS_FTP_MAX_CHUNK_SIZE` maps to the existing `GS_FTP_CHUNK_SIZE`.
- The Ryu generic callback names map to upload event values for raw source
  compilation.

`adapter/src/ryu_ftp_client_adapter.c` exports:

- `ryu_ftp_upload`
- `ryu_ftp_download`

The upload wrapper forwards directly. The download wrapper bridges callbacks
and converts `GS_FTP_INFO_UL_FILE`, `GS_FTP_INFO_UL_COMPLETED`, and
`GS_FTP_INFO_UL_PROGRESS` to their `GS_FTP_INFO_DL_*` counterparts. Callback
payload and caller `user_data` are preserved.

Consumers that need transfer callbacks must use `ryu_ftp_upload()` and
`ryu_ftp_download()` from `ryu_ftp_client_adapter.h`, rather than calling the
raw transfer functions. Extension APIs remain the literal `ftpnew_*` API.

## Deviations

Unavoidable changes to preserved FTP_Ryu files: **none**.

| Item | Reason | Impact | Alternative | Approval |
|---|---|---|---|---|
| None | Literal copies compile through an external compatibility header and adapter. | Original SHA-256 remains unchanged. | Direct source edits were unnecessary. | Not required |

No parser hardening, protocol rewrite, or changes to existing FTP sources were
included.

## Build And Symbols

Clean configure and target build succeeded in:

`/tmp/5th_gs_agent03_build_20260615_231108`

Archive:

`lib/libftp_client_ryu/libftp_client_ryu.a`

A full `BEE-1000` rebuild also succeeded. It emitted the project's existing
warnings; no Ryu port compilation warning was emitted. Product integration
remains deferred because `BEE-1000` is not linked to `FTP_Ryu::client` in this
phase.

Archive members:

```text
ftpnew_client.c.o
ftpnew_client_crc32.c.o
ryu_ftp_client_adapter.c.o
```

Defined public symbols are the 14 literal `ftpnew_*` functions plus
`ryu_ftp_upload` and `ryu_ftp_download`. No `gs_ftp_*`, `ftp_done`, or
`ftpavailable` symbol is defined.

A link-map probe through `ryu_ftp_upload` succeeded against
`lib/gscsp/build/libmimancsp.a`. It selected no old `client.c`, `force.c`, or
`cmd_ftp.c` object, so the Ryu target is not a hidden implementation mixture.

`readelf -Ws` found no global or weak object symbols in the archive. The
literal source has only file-local constants (`chunk_missing`, `chunk_ok`, and
the CRC table); transfer state and adapter callback bridge state are automatic
stack objects. CSP runtime state remains an external library dependency.

## Handoff

Agent 04:

- Include `ryu_ftp_client_adapter.h`.
- Link `FTP_Ryu::client`.
- Use `ryu_ftp_upload`/`ryu_ftp_download` for transfers.
- Raw `ftpnew_*` extension APIs remain available.

Agent 05 and Agent 07:

- Treat the seven files under `port/` and their hashes in
  `checksums.sha256` as the literal baseline.
- Changes under `port/` require a new source/port diff and an explicit
  deviation record.
- Compatibility work belongs under `adapter/`; do not mix Ryu code into the
  existing `libftp_client/src/client.c`.
