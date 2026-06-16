# FTP Ryu Dependency Report

## Required source and headers

The literal client unit consists of:

| File | Role | Required |
|---|---|---|
| `FTP_Ryu/client/src/ftpnew_client.c` | public API and transfer/extension implementation | yes |
| `FTP_Ryu/client/src/ftpnew_client_crc32.c` | self-contained file CRC implementation | yes |
| `FTP_Ryu/client/inc/ftpnew_client.h` | public declarations | yes |
| `FTP_Ryu/client/inc/ftpnew_client_config.h` | timeout/chunk/port defaults | yes |
| `FTP_Ryu/config/ftpnew_types.h` | Ryu wire return/log public types | yes |
| `FTP_Ryu/config/ftpnew_types_internal.h` | extension packet types | yes |
| `FTP_Ryu/config/ftpnew_config.h` | extension size/magic macros | yes |

Do not use `FTP_Ryu/test/external/include` as a production include root. It contains replacement copies of `gs/ftp/client.h` and `gs/ftp/internal/types.h` that conflict with this project.

## Include graph

```text
ftpnew_client.h
  -> gs/util/error.h
  -> gs/ftp/client.h
       -> gs/ftp/types.h -> gs/util/error.h
  -> ftpnew_types.h -> ftpnew_config.h
  -> ftpnew_client_config.h

ftpnew_client.c
  -> ftpnew_client.h
  -> ftpnew_types_internal.h -> ftpnew_types.h
  -> csp/csp.h
  -> csp/csp_endian.h
  -> gs/ftp/internal/types.h (with GS_FTP_INTERNAL_USE=1)
  -> libc/POSIX: stdio, stdlib, string, unistd, sys/stat
```

Confirmed current project include roots relevant to the port are in `CMakeLists.txt:70-79`: generated gscsp headers, util, gscsp, libcsp include/src, and old FTP public headers. Agent 02 must additionally expose the chosen in-tree Ryu `client/inc` and `config` directories only to the new target/consumers.

## Library and symbol dependencies

Direct external functions used by Ryu client:

- CSP: `csp_get_address`, `csp_connect`, `csp_close`, `csp_transaction`, `csp_transaction_persistent`, `csp_read`, `csp_buffer_free`, endian conversion helpers.
- POSIX/libc: file I/O, `stat`, `fsync`, `usleep`, `remove`, `rename`.
- GS types only: `gs_error_t`, `gs_ftp_settings_t`, callback/info types, backend types, and old internal packet structs.
- CRC source is self-contained and does not require `gs_crc32_*`.

Ryu's own test Makefile links `-lcsp -lgsutil -lpthread -lrt` (`FTP_Ryu/test/Makefile:66-70`) and compiles both client sources (`:86-89`). In 5th_GS, `libmimancsp.a` already contains CSP and GS utility objects and currently contains old FTP objects. Agent 02 must prove the new static target's unresolved symbols are satisfied without pulling old FTP implementation objects as an accidental dependency.

Recommended target dependency shape (inferred):

```text
BEE-1000
  -> dedicated ftpnew static library
       sources: ftpnew_client.c + ftpnew_client_crc32.c
       private includes: Ryu config, gs/ftp/internal
       public includes: Ryu client/inc, Ryu config as needed by public header
       link/interface deps: CSP + GS util/type providers
```

The dedicated archive should export only `ftpnew_*` plus `ftpnew_client_file_crc`; it should not define `gs_ftp_*`, `ftp_done`, or `ftpavailable`.

## Header compatibility blockers

1. **Confirmed compile blocker:** project internal header defines `GS_FTP_CHUNK_SIZE` (`gs/ftp/internal/types.h:25`), while Ryu source expects `GS_FTP_MAX_CHUNK_SIZE` (`ftpnew_client.c:55`). Ryu's test header renamed the macro.
2. **Confirmed compile blocker:** project callback enum is direction-specific (`GS_FTP_INFO_UL_*`, `GS_FTP_INFO_DL_*`), while Ryu expects generic `GS_FTP_INFO_FILE/COMPLETED/PROGRESS`.
3. **Confirmed ABI risk:** replacing the project `gs/ftp/client.h` with Ryu's test copy changes enum numeric values and removes custom declarations (`ftp_abort`, force APIs). This would affect existing `miman_ftpfcd.cpp` and old archive code. It is not an acceptable global include substitution.
4. **Confirmed architectural dependency:** despite comments saying the client avoids linking old `libftp_client`, it still compiles against old public and internal FTP types (`ftpnew_client.c:27-32`, `ftpnew_client.h:48-52`). Agent 03 needs an isolated compatibility boundary, not copied implementation logic inside old `client.c`.

Syntax verification command:

```sh
cc -std=gnu11 -fsyntax-only \
  -IFTP_Ryu/client/inc -IFTP_Ryu/config \
  -Ilib/gscsp/build -Ilib/gscsp/lib/libftp_client/include \
  -Ilib/gscsp/lib/libutil/include -Ilib/gscsp/lib/libgscsp/include \
  -Ilib/gscsp/lib/libgscsp/lib/libcsp/include \
  -Ilib/gscsp/lib/libgscsp/lib/libcsp/src \
  -Ilib/gscsp/build/lib/libgscsp/lib/libcsp/include \
  FTP_Ryu/client/src/ftpnew_client.c FTP_Ryu/client/src/ftpnew_client_crc32.c
```

Result: **failed** with undeclared `GS_FTP_MAX_CHUNK_SIZE`, `GS_FTP_INFO_FILE`, `GS_FTP_INFO_COMPLETED`, and `GS_FTP_INFO_PROGRESS`.

## Current build and symbols

- Existing `build/` invocation: **failed before compilation** because `CMakeCache.txt` references `/home/hyvrid/Desktop/0609/BEE_GS`.
- Clean configure in `/tmp/5th_gs_agent01_build`: **succeeded**.
- Clean current-project build in `/tmp`: **succeeded**; the resulting executable exports old FTP/global cleanup symbols and no `ftpnew_*`.
- Ryu target: **not present**, and Ryu syntax check against project headers **failed** as above.
- `nm -g --defined-only lib/gscsp/build/libmimancsp.a` confirms old `gs_ftp_*`, `ftp_done`, `ftp_done_force`, and `ftpavailable`; no `ftpnew_*`.

Useful Agent 02 checks:

```sh
nm -g --defined-only <ftpnew-archive> | rg "ftpnew_|gs_ftp_|ftp_done|ftpavailable"
nm -u <ftpnew-archive> | sort -u
nm -g <BEE-1000> | rg "ftpnew_|gs_ftp_|ftp_done|ftpavailable"
ar t <ftpnew-archive>
cmake --build <clean-build-dir> --verbose
```

## Handoff

- **Agent 02, confirmed:** add a separate target and scoped include roots; do not add Ryu test external headers globally. Resolve the two header vocabulary mismatches and prove archive symbols.
- **Agent 03, confirmed:** preserve Ryu files as a separate client unit. Compatibility changes belong in that unit or a dedicated compatibility header, with diff/reason; never paste them into old `client.c`.
- **Agent 04, confirmed:** include `ftpnew_client.h` from the integrated target and select RDP wrappers. Old list/move/remove/copy/mkdir/rmdir and cancellation have no Ryu API.
- **Unverified:** final CSP link granularity and whether old FTP object files can be excluded from `libmimancsp.a` without affecting force-mode/UI commands. Agent 02 owns the link-map proof.
