#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Standalone wire-layout diagnostic only.
 *
 * Compile one mode at a time:
 *   -DWIRE_IMPL_OLD for lib/gscsp/lib/libftp_client headers
 *   -DWIRE_IMPL_OBC for fsw/FTP V_2.1 headers
 *   -DWIRE_IMPL_RYU for the in-tree FTP_Ryu port plus legacy GS headers
 *
 * This file must not be linked into product code.
 */

#define GS_FTP_INTERNAL_USE 1
#include <gs/ftp/types.h>
#include <gs/ftp/internal/types.h>

#if defined(WIRE_IMPL_RYU)
#include "ftpnew_types_internal.h"
#endif

#define SHOW_SIZE(type) printf("%-32s %3zu\n", #type, sizeof(type))
#define SHOW_OFFSET(type, field) \
    printf("%-32s %-12s %3zu\n", #type, #field, offsetof(type, field))

static void show_legacy_layout(void)
{
    puts("legacy packet types:");
    printf("upload_req=%d upload_rep=%d download_req=%d download_rep=%d\n",
           FTP_UPLOAD_REQUEST, FTP_UPLOAD_REPLY,
           FTP_DOWNLOAD_REQUEST, FTP_DOWNLOAD_REPLY);
    printf("status_req=%d status_rep=%d data=%d done=%d abort=%d\n",
           FTP_STATUS_REQUEST, FTP_STATUS_REPLY, FTP_DATA, FTP_DONE, FTP_ABORT);
    printf("list_req=%d list_rep=%d list_entry=%d move_req=%d move_rep=%d\n",
           FTP_LIST_REQUEST, FTP_LIST_REPLY, FTP_LIST_ENTRY,
           FTP_MOVE_REQUEST, FTP_MOVE_REPLY);
    printf("remove_req=%d remove_rep=%d crc_req=%d crc_rep=%d\n",
           FTP_REMOVE_REQUEST, FTP_REMOVE_REPLY, FTP_CRC_REQUEST, FTP_CRC_REPLY);
    printf("mkfs_req=%d mkfs_rep=%d zip_req=%d zip_rep=%d copy_req=%d copy_rep=%d\n",
           FTP_MKFS_REQUEST, FTP_MKFS_REPLY, FTP_ZIP_REQUEST, FTP_ZIP_REPLY,
           FTP_COPY_REQUEST, FTP_COPY_REPLY);
    printf("mkdir_req=%d mkdir_rep=%d rmdir_req=%d rmdir_rep=%d\n",
           FTP_MKDIR_REQUEST, FTP_MKDIR_REPLY, FTP_RMDIR_REQUEST, FTP_RMDIR_REPLY);

    puts("\nlegacy sizes:");
    SHOW_SIZE(gs_ftp_type_t);
    SHOW_SIZE(gs_ftp_backend_type_t);
    SHOW_SIZE(gs_ftp_return_t);
    SHOW_SIZE(ftp_upload_request_t);
    SHOW_SIZE(ftp_upload_reply_t);
    SHOW_SIZE(ftp_download_request_t);
    SHOW_SIZE(ftp_download_reply_t);
    SHOW_SIZE(ftp_data_t);
    SHOW_SIZE(ftp_status_element_t);
    SHOW_SIZE(ftp_status_reply_t);
    SHOW_SIZE(ftp_crc_reply_t);
    SHOW_SIZE(ftp_packet_t);

    puts("\nstatus payload offsets:");
    SHOW_OFFSET(ftp_status_reply_t, ret);
    SHOW_OFFSET(ftp_status_reply_t, complete);
    SHOW_OFFSET(ftp_status_reply_t, total);
    SHOW_OFFSET(ftp_status_reply_t, entries);
    SHOW_OFFSET(ftp_status_reply_t, entry);
    SHOW_OFFSET(ftp_status_element_t, next);
    SHOW_OFFSET(ftp_status_element_t, count);

    puts("\nfull packet offsets:");
    SHOW_OFFSET(ftp_packet_t, type);
    SHOW_OFFSET(ftp_packet_t, statusrep);
    printf("%-32s %3zu\n", "packet.status.complete",
           offsetof(ftp_packet_t, statusrep) +
           offsetof(ftp_status_reply_t, complete));
    printf("%-32s %3zu\n", "packet.status.total",
           offsetof(ftp_packet_t, statusrep) +
           offsetof(ftp_status_reply_t, total));
    printf("%-32s %3zu\n", "packet.status.entries",
           offsetof(ftp_packet_t, statusrep) +
           offsetof(ftp_status_reply_t, entries));
    printf("%-32s %3zu\n", "packet.status.entry",
           offsetof(ftp_packet_t, statusrep) +
           offsetof(ftp_status_reply_t, entry));

    puts("\nstatus lengths including type:");
    for (unsigned entries = 0; entries <= GS_FTP_STATUS_CHUNKS; ++entries) {
        printf("entries=%u length=%zu\n", entries,
               offsetof(ftp_packet_t, statusrep.entry) +
               entries * sizeof(ftp_status_element_t));
    }
}

#if defined(WIRE_IMPL_RYU)
static void show_ryu_extension_layout(void)
{
    puts("\nryu extension packet types:");
    printf("log_req=%d log_rep=%d timeout_set=%d timeout_get=%d timeout_rep=%d\n",
           FTP_EXT_LOG_REQUEST, FTP_EXT_LOG_REPLY, FTP_EXT_TIMEOUT_SET,
           FTP_EXT_TIMEOUT_GET, FTP_EXT_TIMEOUT_REPLY);
    printf("bufs=%d bufs_rep=%d shell=%d shell_rep=%d ram_write=%d ram_write_rep=%d\n",
           FTP_EXT_CSP_BUFS, FTP_EXT_CSP_BUFS_REPLY, FTP_EXT_SHELL_CMD,
           FTP_EXT_SHELL_REPLY, FTP_EXT_RAM_WRITE, FTP_EXT_RAM_WRITE_REPLY);
    printf("ram_read=%d ram_read_rep=%d ping=%d ping_rep=%d kill=%d kill_rep=%d none=%d\n",
           FTP_EXT_RAM_READ, FTP_EXT_RAM_READ_REPLY, FTP_EXT_PING,
           FTP_EXT_PING_REPLY, FTP_EXT_KILL, FTP_EXT_KILL_REPLY, FTP_TYPE_NONE);

    puts("\nryu extension sizes:");
    SHOW_SIZE(ftp_ret_t);
    SHOW_SIZE(ftp_ext_type_t);
    SHOW_SIZE(ftp_log_tag_t);
    SHOW_SIZE(ftp_log_entry_t);
    SHOW_SIZE(ftp_ext_log_request_t);
    SHOW_SIZE(ftp_ext_log_reply_t);
    SHOW_SIZE(ftp_ext_timeout_set_request_t);
    SHOW_SIZE(ftp_ext_timeout_reply_t);
    SHOW_SIZE(ftp_ext_csp_buffers_reply_t);
    SHOW_SIZE(ftp_ext_shell_cmd_request_t);
    SHOW_SIZE(ftp_ext_shell_cmd_reply_t);
    SHOW_SIZE(ftp_ext_ram_write_request_t);
    SHOW_SIZE(ftp_ext_ram_write_reply_t);
    SHOW_SIZE(ftp_ext_ram_read_request_t);
    SHOW_SIZE(ftp_ext_ram_read_reply_t);
    SHOW_SIZE(ftp_ext_ping_t);
    SHOW_SIZE(ftp_ext_kill_request_t);
    SHOW_SIZE(ftp_ext_kill_reply_t);
    SHOW_SIZE(ftp_ext_packet_t);

    puts("\nryu extension offsets:");
    SHOW_OFFSET(ftp_ext_packet_t, type);
    SHOW_OFFSET(ftp_ext_packet_t, logrep);
    SHOW_OFFSET(ftp_ext_log_reply_t, ret);
    SHOW_OFFSET(ftp_ext_log_reply_t, count);
}
#endif

int main(void)
{
#if defined(WIRE_IMPL_OLD)
    puts("implementation=old-gs");
#elif defined(WIRE_IMPL_OBC)
    puts("implementation=obc-v2.1-supplied");
#elif defined(WIRE_IMPL_RYU)
    puts("implementation=ftp-ryu-port");
#else
#error "Define one of WIRE_IMPL_OLD, WIRE_IMPL_OBC, or WIRE_IMPL_RYU"
#endif

    show_legacy_layout();

#if defined(WIRE_IMPL_RYU)
    show_ryu_extension_layout();
#endif

    return 0;
}
