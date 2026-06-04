#ifndef GS_FTP_SERVER_H
#define GS_FTP_SERVER_H
/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */
/**
   @file

   FTP server interface.
*/

#include <gs/ftp/types.h>
#include <gs/util/log.h>
#include <gs/csp/csp.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Declare ftp logging group for use by backends */
LOG_GROUP_EXTERN(log_ftp);

/** Backend operations */
typedef struct {
    /** Initialize new backend state */
    gs_ftp_return_t (*init)(void **state);
    /** Release backend state */
    gs_ftp_return_t (*release)(void * state);
    /** Start new upload */
    gs_ftp_return_t (*upload)(void * state, char const * path, uint32_t memaddr, uint32_t size, uint32_t chunk_size);
    /** Start new download */
    gs_ftp_return_t (*download)(void * state, char const * path, uint32_t memaddr, uint32_t memsize, uint32_t chunk_size, uint32_t * size, uint32_t * crc);
    /** Write chunk */
    gs_ftp_return_t (*chunk_write)(void * state, uint32_t chunk, uint8_t * data, uint32_t size);
    /** Read chunk */
    gs_ftp_return_t (*chunk_read)(void * state, uint32_t chunk, uint8_t * data, uint32_t size);
    /** Get chunk status */
    gs_ftp_return_t (*status_get)(void * state, uint32_t chunk, int * status);
    /** Set chunk status */
    gs_ftp_return_t (*status_set)(void * state, uint32_t chunk);
    /** List files */
    gs_ftp_return_t (*list)(void * state, const char * path, uint16_t * entries);
    /** Next list entry */
    gs_ftp_return_t (*entry)(void * state, gs_ftp_list_entry_t * ent);
    /** Remove file */
    gs_ftp_return_t (*remove)(void * state, const char * path);
    /** Move file */
    gs_ftp_return_t (*move)(void * state, const char * from, const char * to);
    /** Copy file */
    gs_ftp_return_t (*copy)(void * state, const char * from, const char * to);
    /** Make file system */
    gs_ftp_return_t (*mkfs)(void * state, const char * path, uint8_t force);
    /** Make directory in file system */
    gs_ftp_return_t (*mkdir)(void * state, const char * path, uint32_t mode);
    /** Delete directory in file system */
    gs_ftp_return_t (*rmdir)(void * state, const char * path);
    /** Get CRC of file */
    gs_ftp_return_t (*crc)(void * state, uint32_t * crc);
    /** Zip file */
    gs_ftp_return_t (*zip)(void * state, const char * src, const char * dest, uint8_t action, uint32_t * comp_sz, uint32_t * decomp_sz);
    /** End transfer */
    gs_ftp_return_t (*done)(void * state);
    /** Abort transfer */
    gs_ftp_return_t (*abort)(void * state);
    /** Timeout of transfer */
    gs_ftp_return_t (*timeout)(void * state);
} gs_ftp_backend_t;

/**
   Server backend implementation for RAM.
*/
const gs_ftp_backend_t * gs_ftp_server_backend_ram(void);

/**
   Server backend implementation for file.
*/
const gs_ftp_backend_t * gs_ftp_server_backend_file(void);

/**
   Register backend implementation.
*/
gs_error_t gs_ftp_server_register_backend(gs_ftp_backend_type_t type, const gs_ftp_backend_t * backend);

/**
   Set stack size for the CSP service handler.

   @version 5.0.5
   @param[in] stack_size stack size in bytes, default 16000.
   @return_gs_error_t
*/
gs_error_t gs_ftp_csp_service_handler_set_stack_size(size_t stack_size);

/**
   CSP service handler.

   The handler will create a \a thread for handling the request/connection. The \a thread terminates when complete.

   @version 5.0.5
   @param[in] conn CSP connection.
   @return_gs_error_t
   @see gs_ftp_csp_service_handler_set_stack_size()
*/
gs_error_t gs_ftp_csp_service_handler(csp_conn_t * conn);

/**
   Handle/process incoming FTP connection.

   @deprecated use gs_ftp_csp_service_handler()
   @param[in] conn CSP connection.
   @return_gs_error_t
*/
static inline gs_error_t gs_ftp_server_handle_connection(csp_conn_t * conn)
{
    return gs_ftp_csp_service_handler(conn);
}

#ifdef __cplusplus
}
#endif
#endif
