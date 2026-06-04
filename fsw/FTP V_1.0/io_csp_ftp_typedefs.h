#ifndef _IO_CSP_FTP_TYPEDEFS_H_
#define _IO_CSP_FTP_TYPEDEFS_H_


#include "cfe.h"
#include <stdio.h>
#include <gs/ftp/types.h>
#include <gs/ftp/internal/types.h>


#define IO_CSP_FTP_MAX_FILENAME         128


typedef struct {
    uint32 size;                                            //upload request reply
    uint32 crc;                                             //upload request reply
    char filename[IO_CSP_FTP_MAX_FILENAME];                 //upload request reply
} io_csp_ftp_filespec;

typedef struct {
    uint32                  chunks;                         //status request reply
    uint32                  complete;                       //status request reply                   
    uint16                  entries;                        //status request reply
    ftp_status_element_t    entry[GS_FTP_STATUS_CHUNKS];    //status request reply
} io_csp_ftp_status;

typedef enum __attribute__((packed)) {
    IO_CSP_FTP_UPLINK       = 1,
    IO_CSP_FTP_DOWNLINK     = 2,
} io_csp_ftp_trans_direction;

typedef struct {
    io_csp_ftp_status       status;                         //status request reply
    io_csp_ftp_filespec     filespec;                       //upload request reply
    FILE*                   fp;                             //status request reply
    FILE*                   mapfp;                          //status request reply
    csp_conn_t*             conn;                           //csp_accept
    uint32                  timeout;                        //default
    uint16                  chunk_size;                     //upload request reply
    gs_ftp_backend_type_t   backend;                        //upload request reply
    io_csp_ftp_trans_direction  direction;                  //only for downlink
    bool                    busy;                           //ftp_done
} io_csp_ftp_transfer_spec;


typedef enum {

    /* Generic status. */
    MPL_OK                      = 0,
    MPL_ERROR                   = 1,

    MPL_ERR_NULL                = 11,
    MPL_ERR_NOT_FOUND           = 12,

    /* Standard library status code. */
    MPL_ERR_FOPEN               = 30,
    MPL_ERR_FSEEK               = 31,
    MPL_ERR_FWRITE              = 32,
    MPL_ERR_FREAD               = 33,
    
    /* POSIX library status code. */
    MPL_ERR_OPEN                = 40,
    MPL_ERR_WRITE               = 41,
    MPL_ERR_PRINTF              = 42,
    MPL_ERR_PTHREAD_CREATE      = 44,
    MPL_ERR_STATBUF             = 45,

    /* CSP API status code. */
    MPL_ERR_CSP_INIT            = 50,
    MPL_ERR_CSP_SOCKETCAN_INIT  = 51,
    MPL_ERR_CSP_BUF_INIT        = 52,
    MPL_ERR_CSP_RTABLE          = 53,
    MPL_ERR_CSP_SOCKET          = 54,
    MPL_ERR_CSP_BIND            = 55,
    MPL_ERR_CSP_ACCEPT          = 56,
    MPL_ERR_CSP_READ            = 57,
    MPL_ERR_CSP_TRANS           = 58,

    MPL_ERR_CMD_LEN             = 60,
    MPL_ERR_CMD_TYPE            = 61,

    /* FTP server status code. */
    MPL_ERR_FTP_BAD_TYPE        = 80,
    MPL_ERR_FTP_BAD_RET         = 81,
    MPL_ERR_FTP_FILENAME_TOO_LONG   = 82,
    MPL_ERR_FTP_EXISTS          = 83,

    MPL_ERR_GEV_RET             = 100,
    MPL_ERR_FINDER_ALLOCATE     = 101,

} mpl_status;



static int ftp_task_canceled_socket   = MPL_ERR_CSP_SOCKET;
static int ftp_task_canceled_bind     = MPL_ERR_CSP_BIND;
static int ftp_task_canceled          = MPL_ERROR;


#endif
