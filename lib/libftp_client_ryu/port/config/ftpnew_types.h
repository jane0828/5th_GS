/**
 * @file ftpnew_types.h
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * @brief Shared type definitions for the ftpnew APIs and backend.
 *        Included by both server and client.
 * @version 1.0
 * @date 2026-06-04
 * 
 * Astrodynamics & Control Lab, Yonsei University.
 */
#ifndef _FTPNEW_TYPES_H_
#define _FTPNEW_TYPES_H_

#include "ftpnew_config.h"

#include <stdint.h>

/**
 * @brief ftpnew wire return code.
 *
 * @details Shared by server and client. The server stores these values
 *          directly in the @c ret field of every reply packet; the client
 *          translates them straight to @c gs_error_t via @c wire_ret_to_gs.
 *          No intermediate @c gs_ftp_return_t hop.
 *
 *          The per-thread debug message captured by @c ftp_debug_impl
 *          disambiguates within a class (e.g. multiple I/O errors all map
 *          to @c FTP_ERR_IO).
 * 
 *          Error codes are positive since the legacy client expects a
 *          uint8_t ret field.
 */
typedef enum __attribute__((packed)) {
    FTP_OK          = 0, /* success */
    FTP_ERR_INVAL   = 1, /* null ptr / bad arg / bad value from client */
    FTP_ERR_BUSY    = 2, /* wrong state for this packet type */
    FTP_ERR_NOENT   = 3, /* file not found / cannot open */
    FTP_ERR_EXISTS  = 4, /* target file exists but bitmap doesn't (resume conflict) */
    FTP_ERR_NOSPC   = 5, /* offset/index past end of available data */
    FTP_ERR_IO      = 6, /* any I/O: read / write / seek / fstat / CSP send / remove */
    FTP_ERR_NOMEM   = 7, /* malloc / CSP buffer alloc failed */
    FTP_ERR_NOTSUP  = 8, /* unknown packet type / unsupported operation */
    FTP_ERR_TIMEOUT = 9, /* server csp_read() timeout */
} ftp_ret_t;

/**
 * Log entry tag.
 */
typedef struct __attribute__((packed)) {
    uint32_t  csp_time;
    uint16_t  index;
    ftp_ret_t ret;
    uint8_t   type;
} ftp_log_tag_t;

/**
 * Full log entry.
 */
typedef struct __attribute__((packed)) {
    ftp_log_tag_t tag;
    char message[FTP_LOG_MESSAGE_LENGTH];
} ftp_log_entry_t;

#endif
