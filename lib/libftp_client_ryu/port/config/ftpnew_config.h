/**
 * @file ftpnew_config.h
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * @brief Server-client mission configurations.
 * @version 1.0
 * @date 2026-06-04
 * 
 * Astrodynamics & Control Lab, Yonsei University.
 */
#ifndef _FTPNEW_CONFIG_H_
#define _FTPNEW_CONFIG_H_

/**
 * Maximum number of log tags in the ftpnew_get_log_tags() reply.
 * Each tag is 8-byte payload. A value of 20 is recommended to keep the reply
 * below 200 bytes.
 */
#define FTP_EXT_LOG_REPLY_MAX_TAGS 20

/**
 * Maximum message field size in the log entries (NUL-inclusive).
 */
#define FTP_LOG_MESSAGE_LENGTH     128

/**
 * Magic value for ftpnew_kill(), invoking _exit().
 */
#define FTP_EXT_KILL_MAGIC_EXIT   0xDEADC0DE

/**
 * Magic value for ftpnew_kill(), invoking a POSIX reboot.
 */
#define FTP_EXT_KILL_MAGIC_REBOOT 0xB007C0DE

#endif
