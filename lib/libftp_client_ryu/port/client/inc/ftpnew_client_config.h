/**
 * @file ftpnew_client_config.c
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * @brief Ground-side client configurations.
 * @version 1.0
 * @date 2026-06-04
 * 
 * Astrodynamics & Control Lab, Yonsei University.
 */
#ifndef _FTPNEW_CLIENT_CONFIG_H_
#define _FTPNEW_CLIENT_CONFIG_H_

/**
 * Default CSP transaction timeout in milliseconds.
 * Used if no settings or zero timeout is provided to the client API calls.
 */
#define FTPNEW_CLIENT_DEFAULT_TIMEOUT     30000

/**
 * Default upload/download chunk size in bytes.
 * Used if no settings or zero chunk size is provided to the client API calls.
 */
#define FTPNEW_CLIENT_DEFAULT_CHUNK_SIZE  200

/**
 * Default CSP port for FTP transactions.
 * Used if no settings or zero port is provided to the client API calls.
 */
#define FTPNEW_CLIENT_DEFAULT_PORT        9


#endif
