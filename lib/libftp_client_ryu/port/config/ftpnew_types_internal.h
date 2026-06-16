/**
 * @file ftpnew_types_internal.h
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * @brief Internal type definitions for the ftpnew backend. Server and client
 *        share these definitions but they are not part of the public API.
 * @version 1.0
 * @date 2026-06-04
 * 
 * Astrodynamics & Control Lab, Yonsei University.
 */
#ifndef _FTPNEW_TYPES_INTERNAL_H_
#define _FTPNEW_TYPES_INTERNAL_H_

#include "ftpnew_types.h"

/* ========================================================================
 *                   Extended packet types
 * ====================================================================== */

typedef enum {
    FTP_EXT_LOG_REQUEST     = 50,
    FTP_EXT_LOG_REPLY       = 51,
    FTP_EXT_TIMEOUT_SET     = 52,
    FTP_EXT_TIMEOUT_GET     = 53,
    FTP_EXT_TIMEOUT_REPLY   = 54,
    FTP_EXT_CSP_BUFS        = 55,
    FTP_EXT_CSP_BUFS_REPLY  = 56,
    FTP_EXT_SHELL_CMD       = 57,
    FTP_EXT_SHELL_REPLY     = 58,
    FTP_EXT_RAM_WRITE       = 59,
    FTP_EXT_RAM_WRITE_REPLY = 60,
    FTP_EXT_RAM_READ        = 61,
    FTP_EXT_RAM_READ_REPLY  = 62,
    FTP_EXT_PING            = 63,
    FTP_EXT_PING_REPLY      = 64,
    FTP_EXT_KILL            = 65,
    FTP_EXT_KILL_REPLY      = 66,
    FTP_TYPE_NONE           = 127,
} ftp_ext_type_t;

typedef struct __attribute__((packed)) {
    /**
     * Request action.
     * 0: reply @a index entries with recent retcodes and types.
     * 1: reply @a index -th full log with a message.
     * 2: clear log. @a index is ignored.
     */
    uint8_t action;
    uint16_t index;
} ftp_ext_log_request_t;

typedef struct __attribute__((packed)) {
    uint8_t ret;
    uint16_t count; /* number of log entries */
    union {
        ftp_log_tag_t tags[FTP_EXT_LOG_REPLY_MAX_TAGS];
        ftp_log_entry_t log;
    };
} ftp_ext_log_reply_t;

typedef struct __attribute__((packed)) {
    uint32_t timeout;
    uint32_t interpacket_delay;
} ftp_ext_timeout_set_request_t;

typedef struct __attribute__((packed)) {
    uint32_t timeout;
    uint32_t interpacket_delay;
} ftp_ext_timeout_reply_t;


typedef struct __attribute__((packed)) {
    uint32_t count;
    uint32_t size;
} ftp_ext_csp_buffers_reply_t;

#define FTP_EXT_SHELL_CMD_MAX_LENGTH 200

typedef struct __attribute__((packed)) {
    char cmd[FTP_EXT_SHELL_CMD_MAX_LENGTH];
} ftp_ext_shell_cmd_request_t;

typedef struct __attribute__((packed)) {
    uint8_t ret;
    int32_t sysret;
} ftp_ext_shell_cmd_reply_t;

#define FTP_EXT_RAM_MAX_BYTES 256

typedef struct __attribute__((packed)) {
    uint32_t addr;
    uint32_t size;
    uint8_t bytes[FTP_EXT_RAM_MAX_BYTES];
} ftp_ext_ram_write_request_t;

typedef struct __attribute__((packed)) {
    uint8_t ret;
} ftp_ext_ram_write_reply_t;

typedef struct __attribute__((packed)) {
    uint32_t addr;
    uint32_t size;
} ftp_ext_ram_read_request_t;

typedef struct __attribute__((packed)) {
    uint8_t bytes[FTP_EXT_RAM_MAX_BYTES];
} ftp_ext_ram_read_reply_t;

typedef struct __attribute__((packed)) {
    uint8_t data[32];
} ftp_ext_ping_t;

typedef struct __attribute__((packed)) {
    uint32_t magic;  /* one of FTP_EXT_KILL_MAGIC_* */
    int32_t  status; /* Exit status for _exit(). Ignored for reboot. */
} ftp_ext_kill_request_t;

typedef struct __attribute__((packed)) {
    int32_t ret; /* possibly returns from reboot() - defined as int32_t */
} ftp_ext_kill_reply_t;

typedef struct __attribute__((packed)) {
    uint8_t type;
    union {
        ftp_ext_log_request_t log;
        ftp_ext_log_reply_t   logrep;

        ftp_ext_timeout_set_request_t timeout;
        ftp_ext_timeout_reply_t       timeoutrep;

        ftp_ext_csp_buffers_reply_t buffersrep;

        ftp_ext_shell_cmd_request_t shell;
        ftp_ext_shell_cmd_reply_t   shellrep;

        ftp_ext_ram_write_request_t write;
        ftp_ext_ram_write_reply_t   writerep;

        ftp_ext_ram_read_request_t read;
        ftp_ext_ram_read_reply_t   readrep;

        ftp_ext_ping_t ping;

        ftp_ext_kill_request_t kill;
        ftp_ext_kill_reply_t   killrep;
    };
} ftp_ext_packet_t;

#endif
