# FTP Ryu Build/CMake Report

- Run ID: `20260616_132857`
- Retry scope: Phase 4 Build/CMake ownership retry from `20260616_134158_agent_08_phase_leader.out`
- Target: `ftp_client_ryu`
- CMake alias: `FTP_Ryu::client`
- Archive: `<build>/lib/libftp_client_ryu/libftp_client_ryu.a`
- Result: **standalone build, clean rebuild, and product link validation succeeded**
- Product integration: **top-level CMake link wiring is now owned and validated by Agent 02**

## Phase Leader Deficiency Addressed

Phase Leader item 4 required Agent 02 to own and validate the top-level
`CMakeLists.txt` product link wiring for `FTP_Ryu::client`, including duplicate
`libmimancsp.a` link impact and symbol/link-map evidence.

Agent 02 reviewed the current top-level CMake delta and accepts these two build
changes as the build-owner wiring for Phase 4:

```cmake
ADD_SUBDIRECTORY(${MIMAN_LIB}/libftp_client_ryu)
TARGET_LINK_LIBRARIES(BEE-1000 FTP_Ryu::client)
```

No changes were made to `src/miman_ftprdp.cpp`, existing old FTP sources, or the
preserved Ryu source files during this retry.

## CMake Target

`lib/libftp_client_ryu/CMakeLists.txt` defines the isolated static target from
an explicit source allowlist:

```text
lib/libftp_client_ryu/port/client/src/ftpnew_client.c
lib/libftp_client_ryu/port/client/src/ftpnew_client_crc32.c
lib/libftp_client_ryu/adapter/src/ryu_ftp_client_adapter.c
```

No old FTP implementation source is part of the target. In particular,
`client.c`, `force.c`, and `cmd_ftp.c` are not target sources.

Public include contract:

```text
lib/libftp_client_ryu/port/client/inc
lib/libftp_client_ryu/port/config
lib/libftp_client_ryu/adapter/include
lib/gscsp/lib/libftp_client/include
lib/gscsp/lib/libutil/include
```

Private build includes:

```text
lib/gscsp/build
lib/gscsp/lib/libgscsp/include
lib/gscsp/lib/libgscsp/lib/libcsp/include
lib/gscsp/lib/libgscsp/lib/libcsp/src
lib/gscsp/build/lib/libgscsp/lib/libcsp/include
```

The target's explicit link dependency remains the existing
`lib/gscsp/build/libmimancsp.a`, exposed through imported target
`ftp_client_ryu_gscsp`. The child CMake directory clears inherited global
product link libraries, so the Ryu target does not inherit OpenGL/UI libraries.

## Reproduction Commands

Standalone verbose build:

```sh
rm -rf /tmp/5th_gs_agent02_retry_build
cmake -S . -B /tmp/5th_gs_agent02_retry_build \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_EXE_LINKER_FLAGS='-Wl,-Map,/tmp/5th_gs_agent02_retry_build/BEE-1000.map'
cmake --build /tmp/5th_gs_agent02_retry_build \
  --target ftp_client_ryu --verbose -j2
```

Clean rebuild:

```sh
rm -rf /tmp/5th_gs_agent02_retry_clean
cmake -S . -B /tmp/5th_gs_agent02_retry_clean -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/5th_gs_agent02_retry_clean \
  --target ftp_client_ryu --clean-first --verbose -j2
```

Product link validation:

```sh
cmake --build /tmp/5th_gs_agent02_retry_build \
  --target BEE-1000 --verbose -j2
```

All three commands returned `0`. Full logs are:

```text
/tmp/5th_gs_agent02_retry_configure.log
/tmp/5th_gs_agent02_retry_ryu_verbose.log
/tmp/5th_gs_agent02_retry_clean_configure.log
/tmp/5th_gs_agent02_retry_clean_build.log
/tmp/5th_gs_agent02_retry_bee_verbose.log
/tmp/5th_gs_agent02_retry_build/BEE-1000.map
```

## Verbose Build Inputs

The Ryu target compile lines use GNU C11 and only the listed Ryu/gscsp include
roots. `ftpnew_client.c` is compiled with the target-local compatibility include:

```text
-include lib/libftp_client_ryu/adapter/include/ryu_ftp_compat.h
```

The archive command was:

```sh
/usr/bin/ar qc libftp_client_ryu.a \
  CMakeFiles/ftp_client_ryu.dir/port/client/src/ftpnew_client.c.o \
  CMakeFiles/ftp_client_ryu.dir/port/client/src/ftpnew_client_crc32.c.o \
  CMakeFiles/ftp_client_ryu.dir/adapter/src/ryu_ftp_client_adapter.c.o
/usr/bin/ranlib libftp_client_ryu.a
```

The product link line contains the old gscsp archive first, then the Ryu archive,
then the Ryu target's imported gscsp dependency:

```text
/mnt/c/Users/energ/Desktop/ACL/GS/5th_GS/lib/gscsp/build/libmimancsp.a
lib/libftp_client_ryu/libftp_client_ryu.a
/mnt/c/Users/energ/Desktop/ACL/GS/5th_GS/lib/libftp_client_ryu/../gscsp/build/libmimancsp.a
```

## Archive Evidence

`ar t /tmp/5th_gs_agent02_retry_build/lib/libftp_client_ryu/libftp_client_ryu.a`:

```text
ftpnew_client.c.o
ftpnew_client_crc32.c.o
ryu_ftp_client_adapter.c.o
```

Defined public Ryu symbols:

```text
ftpnew_clear_log
ftpnew_client_file_crc
ftpnew_download
ftpnew_get_csp_buffers
ftpnew_get_log_entry
ftpnew_get_log_tags
ftpnew_get_timeout
ftpnew_kill
ftpnew_ping
ftpnew_ram_read
ftpnew_ram_write
ftpnew_set_timeout
ftpnew_shell_cmd
ftpnew_upload
ryu_ftp_download
ryu_ftp_upload
```

No defined archive symbol matched `gs_ftp_`, `ftp_done`, or `ftpavailable`.

Non-libc unresolved dependencies from the archive are:

```text
csp_buffer_free
csp_close
csp_connect
csp_get_address
csp_htole32
csp_hton16
csp_hton32
csp_letoh32
csp_ntoh16
csp_ntoh32
csp_read
csp_transaction
csp_transaction_persistent
ftpnew_client_file_crc
```

`ftpnew_client_file_crc` is resolved by `ftpnew_client_crc32.c.o` in the same
archive. The CSP symbols resolve from `libmimancsp.a`.

## Product Link and Duplicate CSP Impact

`BEE-1000` links successfully with both the existing direct
`lib/gscsp/build/libmimancsp.a` and the Ryu target's imported reference to the
same archive. There were no `multiple definition` or `undefined reference`
diagnostics in the product link log.

The link map shows old FTP objects selected from the first old-product gscsp
archive because current product objects still reference old rollback functions:

```text
lib/gscsp/build/libmimancsp.a(client.c.2.o)
  CMakeFiles/BEE-1000.dir/src/miman_ftprdp.cpp.o (gs_ftp_upload)
lib/gscsp/build/libmimancsp.a(force.c.2.o)
  lib/gscsp/build/libmimancsp.a(client.c.2.o) (ftpavailable)
```

The same link map shows Ryu objects selected only from the Ryu archive:

```text
lib/libftp_client_ryu/libftp_client_ryu.a(ryu_ftp_client_adapter.c.o)
  CMakeFiles/BEE-1000.dir/src/miman_ftprdp.cpp.o (ryu_ftp_upload)
lib/libftp_client_ryu/libftp_client_ryu.a(ftpnew_client.c.o)
  lib/libftp_client_ryu/libftp_client_ryu.a(ryu_ftp_client_adapter.c.o) (ftpnew_upload)
lib/libftp_client_ryu/libftp_client_ryu.a(ftpnew_client_crc32.c.o)
  lib/libftp_client_ryu/libftp_client_ryu.a(ftpnew_client.c.o) (ftpnew_client_file_crc)
```

No additional old FTP object was selected from the Ryu target's second gscsp
archive scan. The duplicate archive path is therefore link-clean in this build:
it is redundant but not currently conflicting. Build-owner recommendation is to
keep the explicit dependency on `FTP_Ryu::client` for Phase 4 validation, then
deduplicate the shared CSP archive only when the broader product link model is
cleaned up.

`nm -u /tmp/5th_gs_agent02_retry_build/BEE-1000` shows no unresolved
`ftpnew_*`, `ryu_ftp_*`, `gs_ftp_*`, `ftp_done`, `ftpavailable`, or `csp_*`
symbols.

## Product Symbol Snapshot

The final executable intentionally contains both namespaces during Phase 4:

```text
old/rollback: gs_ftp_upload, gs_ftp_download, gs_ftp_* helpers,
              ftp_done, ftpavailable, gs_ftp_*_force
Ryu:          ryu_ftp_upload, ryu_ftp_download, ftpnew_* APIs
```

This is not a hidden mixture in the Ryu archive: the old symbols come from the
existing product link and old rollback call sites, while Ryu transfer symbols
come from `libftp_client_ryu.a`.

## Warnings

The Ryu target build produced no new warnings in the collected logs. The product
build still emits pre-existing warnings from UI/model/autopilot headers and
sources; no product warning was tied to `lib/libftp_client_ryu`.

## Ownership and Checksums

Checksums after verification:

```text
fd33d196df20b302fac00263f5be11030ba4880d61e09bb736b01b5848a2bc9d  src/miman_ftprdp.cpp
a8d9031c8706627d2e90242147381e07f8eb72cd11b54129736ded29e699f2fd  lib/gscsp/lib/libftp_client/src/client.c
924dd9cfc4876c663729679c14409bea1ea450173a4ae47346f4d16024d2273d  lib/gscsp/lib/libftp_client/src/force.c
43515728506b1f9b17c3ad86e90dfb7fa16af8a7c507c135a8a58e78ad8dda1a  lib/libftp_client_ryu/port/client/src/ftpnew_client.c
7fb32ce382f35c2c5f95393813a62e54a93ff77ab0a80d556495f25b8dca6d82  lib/libftp_client_ryu/port/client/src/ftpnew_client_crc32.c
ebe86e6f444626b391013d270491e038ade69c4f4b7c932c8a5660574412a80a  lib/libftp_client_ryu/adapter/src/ryu_ftp_client_adapter.c
92539ba66338bdbc694b267352b8c90c7d958d9095e8ddfc964e7f51a4612579  CMakeLists.txt
f67d31e6957c4b6a5adf7c243a51fff963926d01d153200c16cd74098330d59c  lib/libftp_client_ryu/CMakeLists.txt
```

Agent 02 did not modify `src/miman_ftprdp.cpp`, old FTP source files, or the
Ryu port source files during this retry.

## Handoff

Agent 03:

- Confirmed port directory: `lib/libftp_client_ryu`.
- Public include contract: `ftpnew_client.h` for raw extension APIs and
  `ryu_ftp_client_adapter.h` for transfer callback adaptation.
- Preserve the static archive source split shown above.

Agent 04:

- Target name: `ftp_client_ryu`; namespaced alias: `FTP_Ryu::client`.
- Product link form is validated: `TARGET_LINK_LIBRARIES(BEE-1000 FTP_Ryu::client)`.
- Use `#include <ryu_ftp_client_adapter.h>` for transfer calls.
- Ryu archive defines `ryu_ftp_upload`, `ryu_ftp_download`, and expected
  `ftpnew_*` symbols, with no old FTP implementation symbols.

## Verdict

The separate Ryu static archive clean-builds and provides the expected
`ftpnew_*` and `ryu_ftp_*` namespace without old FTP implementation symbols or
objects. The Phase 4 product link with `FTP_Ryu::client` succeeds, and the
duplicate `libmimancsp.a` scan is redundant but not link-conflicting in the
validated build.
