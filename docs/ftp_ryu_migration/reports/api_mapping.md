# FTP Ryu API Mapping

Status vocabulary: **확정 (confirmed)** = directly established from source/build output, **추정 (inferred)** = implementation decision follows from confirmed facts, **미확인 (unverified)** = requires another agent or runtime/protocol evidence.

## Integration call mapping

| 5th_GS call | Call site | Ryu candidate | Status | Migration note |
|---|---|---|---|---|
| `gs_ftp_upload(settings, local, remote, cb, data)` | `src/miman_ftprdp.cpp:144` | `ftpnew_upload_rdp(...)` or `ftpnew_upload(..., true, 0)` | confirmed | Same settings/URL/callback arguments; Ryu adds transport selection. Current RDP path should select `use_rdp=true`. |
| `gs_ftp_download(settings, local, remote, cb, data)` | `src/miman_ftprdp.cpp:103` | `ftpnew_download_rdp(...)` or `ftpnew_download(..., true)` | confirmed | Same settings/URL/callback arguments; Ryu adds transport selection. |
| `gs_ftp_list(...)` | `src/miman_ftprdp.cpp:172` | none in Ryu public API | confirmed blocker | Do not silently keep old list in the final Ryu path. Add/port an explicitly namespaced Ryu operation, or formally remove/disable this UI capability after protocol-owner approval. |
| `gs_ftp_move(...)` | `src/miman_ftprdp.cpp:191` | none | confirmed blocker | Same rule as list. |
| `gs_ftp_remove(...)` | `src/miman_ftprdp.cpp:209` | none | confirmed blocker | Same rule as list. |
| `gs_ftp_copy(...)` | `src/miman_ftprdp.cpp:227` | none | confirmed blocker | Same rule as list. |
| `gs_ftp_mkdir(...)` | `src/miman_ftprdp.cpp:246` | none | confirmed blocker | Same rule as list. |
| `gs_ftp_rmdir(...)` | `src/miman_ftprdp.cpp:264` | none | confirmed blocker | Same rule as list. |
| `ftp_avail()` / `ftp_abort()` | `src/miman_ftprdp.cpp:80,111,123,154` | none | confirmed blocker | Ryu has transfer-local state and no public cancellation API. Removing these calls without a cancellation/lifetime decision changes behavior. |

Ryu-only public operations are `ftpnew_ping`, CSP buffer query, timeout get/set, log operations, shell command, RAM read/write, and kill (`FTP_Ryu/client/inc/ftpnew_client.h:154-331`). They have no old integration call to migrate in Phase 1.

## Callback mapping

The function type itself is unchanged:

```c
typedef void (*gs_ftp_info_callback_t)(const gs_ftp_info_t *info);
```

The callback payload struct layout is also compatible, but enum names and numeric meanings differ.

| Event | 5th_GS enum | Ryu source enum | Required adapter/port action |
|---|---|---|---|
| upload file metadata | `GS_FTP_INFO_UL_FILE` | `GS_FTP_INFO_FILE` | Ryu port must emit `GS_FTP_INFO_UL_FILE` when compiled against current 5th_GS headers, or introduce an isolated compatibility type/adapter. |
| download file metadata | `GS_FTP_INFO_DL_FILE` | `GS_FTP_INFO_FILE` | Emit `GS_FTP_INFO_DL_FILE`. |
| CRC | `GS_FTP_INFO_CRC` | `GS_FTP_INFO_CRC` | Name matches, but numeric value differs if Ryu test header replaces the project header. Never include both competing `client.h` variants. |
| upload completed/status | `GS_FTP_INFO_UL_COMPLETED` | `GS_FTP_INFO_COMPLETED` | Emit upload-specific project enum. |
| download completed/status | `GS_FTP_INFO_DL_COMPLETED` | `GS_FTP_INFO_COMPLETED` | Emit download-specific project enum. |
| upload progress | `GS_FTP_INFO_UL_PROGRESS` | `GS_FTP_INFO_PROGRESS` | Emit upload-specific project enum. |
| download progress | `GS_FTP_INFO_DL_PROGRESS` | `GS_FTP_INFO_PROGRESS` | Emit download-specific project enum. |

Evidence:

- Current enum: `lib/gscsp/lib/libftp_client/include/gs/ftp/client.h:28-36`.
- Ryu test enum: `FTP_Ryu/test/external/include/gs/ftp/client.h:28-33`.
- Existing callback consumes direction-specific values: `src/miman_ftpfcd.cpp:59-113`.
- Ryu emits generic values: `FTP_Ryu/client/src/ftpnew_client.c:225-234,265-276,364-373,455-465,616-625,768-777`.
- A syntax-only compile against project headers fails on all three generic enum names.

`ftp_list_callback(uint16_t, const gs_ftp_list_entry_t *, void *)` has no Ryu counterpart because the Ryu public client has no list operation. Its current implementation also omits a return value despite returning `int` (`src/miman_ftprdp.cpp:59-76`); Agent 04 should treat that as an existing integration defect, not as a Ryu callback mapping.

## Settings and behavior

`gs_ftp_settings_t` is reused by Ryu (`ftpnew_client.h:73-95`). Current integration initializes all fields, so Ryu's null/default behavior is not used.

| Item | old 5th_GS | Ryu |
|---|---|---|
| default timeout | 30000 ms (`client.c:23`) | 30000 ms (`ftpnew_client_config.h:17`) |
| default chunk | 185 (`client.c:24`) | 200 (`ftpnew_client_config.h:23`) |
| default port | `GS_CSP_PORT_FTP` / mode logic (`client.c:177-192`) | 9 (`ftpnew_client_config.h:29`) |
| transport | RDP + CRC32 fixed (`client.c:259-260,362`) | caller-selected RDP or UDP (`ftpnew_client.c:119-122`) |
| host when settings/host is zero | direct `settings->host` in transfer calls | `csp_get_address()` (`ftpnew_client.c:43-46`) |
| download destination | final path directly | writes `<path>.tmp`, then renames on success (`ftpnew_client.c:780-849`) |

Integration should call the RDP convenience wrappers for the current `miman_ftprdp.cpp` path. UDP delay policy is an open product/runtime decision, not an automatic replacement.

## Symbol classification

| Symbol | Definition | Uses / binary state | Classification |
|---|---|---|---|
| `gs_ftp_state_t` | private typedef in `client.c:28-42`; another private copy in `force.c:29-43`; active duplicate text in `src/miman_coms.h:5688-5704` | old global `state` at `client.c:56` | old implementation/private; must not be reused by Ryu |
| `ftpavailable` | `force.c:26` | old loops in `client.c:863,1025,1038`; exported BSS in `libmimancsp.a` | old global cancellation gate |
| `ftp_done` | `client.c:1110-1146`, declared publicly at `client.h:345` | many old transfer error/success paths; exported by archive | old global cleanup API |
| `gs_ftp_*` | public declarations `client.h:138-345`; implementations in old client/force/cmd | integration calls at `miman_ftprdp.cpp:103-264`; archive exports upload/download/list/etc. | old namespace |
| `ftpnew_*` | declarations `FTP_Ryu/client/inc/ftpnew_client.h:73-331`; definitions `ftpnew_client.c:565-1213` | absent from current source tree/archive | Ryu namespace, not integrated |
| `ftpnew_state_t` | private typedef `ftpnew_client.c:72-88` | stack-local in upload/download at `591-614,722-728` | Ryu transfer-local state |

## Commands used

```sh
rg -n -S "gs_ftp_state_t|ftpavailable|ftp_done|gs_ftp_*|ftpnew_*" ...
rg -n "^gs_error_t ftpnew_|^static inline gs_error_t ftpnew_" FTP_Ryu/client/inc/ftpnew_client.h
rg -n "^gs_error_t gs_ftp_|^int ftp_(abort|avail)|^void ftp_done" lib/gscsp/lib/libftp_client/include/gs/ftp/client.h
diff -u lib/gscsp/lib/libftp_client/include/gs/ftp/client.h FTP_Ryu/test/external/include/gs/ftp/client.h
nm -g --defined-only lib/gscsp/build/libmimancsp.a | rg "gs_ftp_|ftpavailable|ftp_done|ftpnew_"
```
