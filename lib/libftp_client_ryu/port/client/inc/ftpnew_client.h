/**
 * @file ftpnew_client.h
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * @brief Ground-side client for the ftpnew backend.
 * @version 1.0
 * @date 2026-06-04
 * 
 * Astrodynamics & Control Lab, Yonsei University.
 *
 * Design notes
 * ============
 * 1. Upload / download
 *    The transfer flow is ported from libftp/lib/libftp_client/src/client.c
 *    (gs_ftp_upload, gs_ftp_download, and the static helpers they call).
 *    The protocol-level state machine is identical. Below are two deviations
 *    from the original source:
 * 
 *      1) upload/download requests now can choose between RDP and UDP for the
 *          connection, controlled by the @p use_rdp argument. For RDP mode,
 *          the same CSP_O_RDP|CSP_O_CRC32 flag is used; for UDP mode, it's
 *          CSP_O_CRC32 alone.
 *          The UDP mode data stage (ftpnew_data) puts a small delay between
 *          packets to avoid rx queue overrun at the server.
 *      2) The file download request respects the @p mem_addr and @p mem_size 
 *          fields, interpreted as the file offset and size, respectively,
 *          so that partial download is supported. Detailed behavior:
 *            - @p mem_addr larger than the file size is rejected.
 *            - @p mem_size larger than (file size - @p mem_addr) is truncated
 *              to it.
 *            - Zero @p mem_size ignores the mem_size field, i.e., downloads
 *              the whole file starting from @p mem_addr.
 *            - The orginal client semantics correspond to mem_addr=0, mem_size=0.
 *          Upload still ignores these members (always uploads the whole target).
 * 
 * 2. Extension layer (FTP_EXT_*)
 *    Provides extended utility requests. All calls are stateless one-shot
 *    transactions (the server closes the connection after replying).
 * 
 * 3. All ftp_settings_t are null-allowed. If null, the client fetches default values.
 */
#ifndef _FTPNEW_CLIENT_H_
#define _FTPNEW_CLIENT_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <gs/util/error.h>
#include <gs/ftp/client.h>

#include "ftpnew_types.h"
#include "ftpnew_client_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 *                         Upload / Download
 * ====================================================================== */

/**
 * @brief Upload @p local_url to @p remote_url. URL grammar matches 
 *        gs_ftp_upload(): file://<path>, mem://<addr>[++<length>], or a bare
 *        path. @p info_callback and @p info_data are null-allowed.
 *
 * @param use_rdp If true, connect with CSP_O_RDP|CSP_O_CRC32; else CSP_O_CRC32
 *                alone, with @p interpacket_delay_ms delay between packets in
 *                the upload data stage.
 * @param interpacket_delay_ms Per-chunk inter-packet gap for UDP-mode uploads.
 *                             Ignored if @p use_rdp is true.
 */
gs_error_t ftpnew_upload(const gs_ftp_settings_t* settings,
                         const char* local_url,
                         const char* remote_url,
                         gs_ftp_info_callback_t info_callback,
                         void* info_data,
                         bool use_rdp,
                         uint32_t interpacket_delay_ms);

/**
 * @brief Download @p remote_url to @p local_url. URL grammar matches
 *        gs_ftp_download(): file://<path>, mem://<addr>[++<length>], or a bare
 *        path. @p info_callback and @p info_data are null-allowed.
 * 
 * @param use_rdp If true, connect with CSP_O_RDP|CSP_O_CRC32; else CSP_O_CRC32
 *                alone, with @p interpacket_delay_ms delay between packets in
 *                the upload data stage.
 */
gs_error_t ftpnew_download(const gs_ftp_settings_t* settings,
                           const char* local_url,
                           const char* remote_url,
                           gs_ftp_info_callback_t info_callback,
                           void* info_data,
                           bool use_rdp);

/**
 * @brief Upload @p local_url to @p remote_url using RDP.
 *        See ftpnew_upload() for details.
 */
static inline gs_error_t ftpnew_upload_rdp(const gs_ftp_settings_t* settings,
                                           const char* local_url,
                                           const char* remote_url,
                                           gs_ftp_info_callback_t info_callback,
                                           void* info_data) {
    return ftpnew_upload(settings, local_url, remote_url,
                         info_callback, info_data, true, 0);
}

/**
 * @brief Upload @p local_url to @p remote_url using UDP.
 *        See ftpnew_upload() for details.
 */
static inline gs_error_t ftpnew_upload_udp(const gs_ftp_settings_t* settings,
                                           const char* local_url,
                                           const char* remote_url,
                                           gs_ftp_info_callback_t info_callback,
                                           void* info_data,
                                           uint32_t interpacket_delay_ms) {
    return ftpnew_upload(settings, local_url, remote_url,
                    info_callback, info_data, false, interpacket_delay_ms);
}

/**
 * @brief Download @p remote_url to @p local_url using RDP.
 *        See ftpnew_download() for details.
 */
static inline gs_error_t ftpnew_download_rdp(const gs_ftp_settings_t* settings,
                                             const char* local_url,
                                             const char* remote_url,
                                             gs_ftp_info_callback_t info_callback,
                                             void* info_data) {
    return ftpnew_download(settings, local_url, remote_url,
                           info_callback, info_data, true);
}

 /**
  * @brief Download @p remote_url to @p local_url using UDP.
  *        See ftpnew_download() for details.
  */
static inline gs_error_t ftpnew_download_udp(const gs_ftp_settings_t* settings,
                                             const char* local_url,
                                             const char* remote_url,
                                             gs_ftp_info_callback_t info_callback,
                                             void* info_data) {
    return ftpnew_download(settings, local_url, remote_url,
                           info_callback, info_data, false);
}

/* ========================================================================
 *                         Extension layer   
 * ======================================================================*/

/**
 * @brief Ping the server with 32-byte payload. Payload is an incrementing
 *        byte pattern (1, 2, ..., 32).
 * 
 * @param settings The FTP settings. Uses defaults if null.
 * @return GS_OK: The server echoes back.
 *         GS_ERROR_IO: transaction failed.
 *         GS_ERROR_DATA: reply type is not FTP_EXT_PING_REPLY or the payload
 *                        does not match the request.
 */
gs_error_t ftpnew_ping(const gs_ftp_settings_t* settings);

 /**
  * @brief Query the server-side CSP buffer pool.
  * 
  * @param settings The FTP settings. Uses defaults if null.
  * @param[out] remaining Free CSP buffers on the server at the moment.
  * @param[out] size      Configured CSP buffer size on the server.
  * @return GS_OK: Success.
  *         GS_ERROR_IO: transaction failed.
  *         GS_ERROR_DATA: reply type is not FTP_EXT_CSP_BUFS_REPLY.
  */
gs_error_t ftpnew_get_csp_buffers(const gs_ftp_settings_t* settings,
                                  uint32_t* remaining, uint32_t* size);

/**
 * @brief Read the server's current operational timeout (ms).
 *
 * @param settings The FTP settings. Uses defaults if null.
 * @param[out] timeout_ms Current timeout value in milliseconds.
 * @param[out] interpacket_delay_ms Current inter-packet delay in milliseconds.
 * @return GS_OK: Success.
 *         GS_ERROR_IO: transaction failed.
 *         GS_ERROR_DATA: reply type is not FTP_EXT_TIMEOUT_REPLY.
 */
gs_error_t ftpnew_get_timeout(const gs_ftp_settings_t* settings,
                              uint32_t* timeout_ms,
                              uint32_t* interpacket_delay_ms);

/**
 * @brief Set the server's operational timeout (ms).
 * 
 * @details
 *      - The timeout applies to the future transactions after this call;
 *        any currently in-flight sessions are unaffected.
 *      - The server echoes the actually set values in the reply for
 *        validation.
 *
 * @param settings The FTP settings. Uses defaults if null.
 * @param timeout_ms New timeout value in milliseconds.
 * @param interpacket_delay_ms New inter-packet delay in milliseconds.
 * @return GS_OK: Success.
 *         GS_ERROR_IO: transaction failed.
 *         GS_ERROR_DATA: reply type is not FTP_EXT_TIMEOUT_REPLY or the reply
 *                        values do not match the request.
 */
gs_error_t ftpnew_set_timeout(const gs_ftp_settings_t* settings,
                              uint32_t timeout_ms,
                              uint32_t interpacket_delay_ms);

/**
 * @brief Get log entry tags from the server. The server returns up to @p count
 *        recent tags. The most recent one is the first of returned @p tags.
 *        The caller can use ftpnew_get_log_entry() to fetch the full entry
 *        with the message for any tag of interest.
 *
 * @param settings   The FTP settings. Uses defaults if null.
 * @param count      Maximum number of tags to return.
 * @param[out] tags  Array to receive the tags.
 * @param[out] n_out Actual number of tags returned by the server.
 * @return GS_OK: Success.
 *         GS_ERROR_IO: transaction failed.
 *         GS_ERROR_DATA: reply type is not FTP_EXT_LOG_TAGS_REPLY or the
 *                        number of tags does not match the reply length.
 */
gs_error_t ftpnew_get_log_tags(const gs_ftp_settings_t* settings,
                               uint16_t count,
                               ftp_log_tag_t* tags,
                               int* n_out);

/**
 * @brief Get a log entry from the server by index. The index corresponds to the
 *        tag index returned by ftpnew_get_log_tags().
 *
 * @param settings The FTP settings. Uses defaults if null.
 * @param index    Log entry index to fetch.
 * @param[out] out Full log entry with message.
 * @return GS_OK: Success.
 *         GS_ERROR_IO: transaction failed.
 *         GS_ERROR_DATA: reply type is not FTP_EXT_LOG_ENTRY_REPLY or the entry
 *                        index is out of range.
 */
gs_error_t ftpnew_get_log_entry(const gs_ftp_settings_t* settings,
                                uint16_t index,
                                ftp_log_entry_t* out);

/**
 * @brief Clear the server-side log.
 *
 * @param settings The FTP settings. Uses defaults if null.
 * @return GS_OK: Success.
 *         GS_ERROR_IO: Transaction failed.
 *         GS_ERROR_DATA: Reply type is not FTP_EXT_LOG_CLEAR_REPLY.
 */
gs_error_t ftpnew_clear_log(const gs_ftp_settings_t* settings);

/**
 * @brief Execute a shell command on the server via system(3).
 *
 * @note The wire reply carries only @c ret and @c sysret; the server does
 *       not forward the command's stdout. If the output needs to be captured,
 *       redirect it server-side (e.g. @c "cmd > /tmp/out") and fetch the
 *       file via ftpnew_download.
 *
 * @param settings    FTP settings (uses defaults if NULL).
 * @param cmd         Command to execute (NUL-terminated, up to
 *                    @c FTP_EXT_SHELL_CMD_MAX_LENGTH-1 bytes).
 * @param[out] out_sysret  If non-NULL, receives the command's exit status.
 * @return GS_OK: Success.
 *         GS_ERROR_ARG: Null @p cmd.
 *         GS_ERROR_IO: Transaction failed.
 *         GS_ERROR_DATA: Reply type is not FTP_EXT_SHELL_REPLY.
 */
gs_error_t ftpnew_shell_cmd(const gs_ftp_settings_t* settings,
                            const char* cmd,
                            int* out_sysret);

/**
 * @brief Read @p size bytes from satellite RAM at @p offset into @p out_buf.
 *        Size must be larger than zero and at most FTP_EXT_RAM_MAX_BYTES.
 * 
 * @param settings    FTP settings (uses defaults if NULL).
 * @param address     RAM address to read from.
 * @param size        Bytes to read.
 * @param[out] out_buf Buffer to receive the data.
 * @return GS_OK: Success.
 *         GS_ERROR_ARG: Null @p out_buf, or invalid @p size.
 *         GS_ERROR_IO: Transaction failed or reply size mismatch.
 *         GS_ERROR_DATA: Reply type is not FTP_EXT_RAM_READ_REPLY.
 */
gs_error_t ftpnew_ram_read(const gs_ftp_settings_t* settings,
                           uint32_t address,
                           uint32_t size,
                           void* out_buf);

/**
 * @brief Write @p size bytes from @p data to satellite RAM at @p offset.
 *        Size must be larger than zero and at most FTP_EXT_RAM_MAX_BYTES.
 * 
 * @param settings    FTP settings (uses defaults if NULL).
 * @param offset      RAM address to write to.
 * @param size        Bytes to write.
 * @param data        Buffer containing the data to write.
 * @return GS_OK: Success.
 *         GS_ERROR_ARG: Null @p data, or invalid @p size.
 *         GS_ERROR_IO: Transaction failed.
 *         GS_ERROR_DATA: Reply type is not FTP_EXT_RAM_WRITE_REPLY.
 */
gs_error_t ftpnew_ram_write(const gs_ftp_settings_t* settings,
                            uint32_t address,
                            uint32_t size,
                            const void* data);

/**
 * @brief Send a kill command to the server with the given magic value.
 *        The server is expected to kill the process or the system; no reply
 *        is expected. If any is received, it's an error.
 * 
 * @param settings  FTP settings (uses defaults if NULL).
 * @param magic     FTP_EXT_KILL_MAGIC_EXIT to _exit() with @p status.
 *                  FTP_EXT_KILL_MAGIC_REBOOT to reboot (status ignored).
 * @param status    Exit status for _exit(). Ignored for reboot.
 * @return GS_OK: Success (no reply received).
 *         GS_ERROR_IO: Transaction failed or any reply received.
 */
gs_error_t ftpnew_kill(const gs_ftp_settings_t* settings,
                       uint32_t magic,
                       int status);

#ifdef __cplusplus
}
#endif
#endif
