#ifndef _IO_CSP_FTP_TYPEDEFS_H_
#define _IO_CSP_FTP_TYPEDEFS_H_


#include "cfe.h"
#include <stdio.h>
#include <csp/csp_types.h>
#include <gs/ftp/types.h>
#include <gs/ftp/internal/types.h>



typedef struct {
    FILE*   fp;
    FILE*   mapfp;
    char    filename[GS_FTP_PATH_LENGTH];
    char*   mapbuf;
    uint32  size;
    uint32  chunks;
} io_csp_ftp_filespec;

typedef struct {
    uint32                  chunks;
    uint32                  complete;
    uint16                  entries;
    ftp_status_element_t    entry[GS_FTP_STATUS_CHUNKS];
} io_csp_ftp_status;

typedef enum __attribute__((packed)) {
    IO_CSP_FTP_IDLE         = 0,
    IO_CSP_FTP_UPLOAD       = 1,
    IO_CSP_FTP_DOWNLOAD     = 2,
} io_csp_ftp_trans_direction;

typedef struct {

    io_csp_ftp_filespec     filespec;

    ftp_upload_request_t    upspec;
    ftp_download_request_t  downspec;

    csp_conn_t*             conn;
    uint32                  timeout;
    gs_ftp_backend_type_t   backend;
    io_csp_ftp_trans_direction  transfer_status;

} io_csp_ftp_transfer_spec;



typedef enum __attribute__((packed)) {
    
    IO_CSP_FTP_RET_OK               = 0,
    IO_CSP_FTP_RET_ERROR            = 1,
    IO_CSP_FTP_RET_ERR_NULL         = 2,
    IO_CSP_FTP_RET_ERR_INVAL        = 3,
    IO_CSP_FTP_RET_ERR_RANGE        = 4,

    IO_CSP_FTP_RET_ERR_STATUS       = 10,

    IO_CSP_FTP_RET_ERR_SEND         = 20,
    IO_CSP_FTP_RET_ERR_READ         = 21,
    IO_CSP_FTP_RET_ERR_TRANS        = 21,

    IO_CSP_FTP_RET_ERR_NOBUF        = 30,
    IO_CSP_FTP_RET_ERR_CHUNK_WRITE  = 31,
    IO_CSP_FTP_RET_ERR_CHUNK_READ   = 32,

} io_csp_ftp_ret;



#endif
