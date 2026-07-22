#pragma once
#ifndef _MIMAN_FTPRDP_INTEGRATION_H_
#define _MIMAN_FTPRDP_INTEGRATION_H_

#include <stdint.h>

typedef struct {
    uint32_t transfer_id;
    int return_code;
    int cleanup_done;
    char engine[8];
    char direction[16];
    char local_path[256];
    char remote_path[256];
} miman_ftp_transfer_status_t;

typedef struct miman_ftp_worker_arg miman_ftp_worker_arg_t;

miman_ftp_worker_arg_t * miman_ftp_create_worker_arg(const char * local_path,
                                                     const char * remote_path);
void miman_ftp_destroy_worker_arg(miman_ftp_worker_arg_t * arg);

void * ftp_ryu_uplink_onorbit(void * param);
void * ftp_ryu_downlink_onorbit(void * param);
int miman_ftp_get_last_transfer_status(miman_ftp_transfer_status_t * status);

#endif
