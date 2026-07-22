#ifndef RYU_FTP_CONNECTION_DIAG_H_
#define RYU_FTP_CONNECTION_DIAG_H_

#include <stdint.h>

#include <csp/csp.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FTP_RYU_CONN_DIAG
void ryu_ftp_diag_set_transfer(uint32_t transfer_id,
                               const char *engine,
                               const char *direction);
void ryu_ftp_diag_clear_transfer(void);
void ryu_ftp_diag_dump(const char *phase);
csp_conn_t *ryu_ftp_diag_connect(const char *site,
                                 uint8_t prio,
                                 uint8_t dest,
                                 uint8_t dport,
                                 uint32_t timeout,
                                 uint32_t opts);
int ryu_ftp_diag_close(const char *site, csp_conn_t *conn);
#else
static inline void ryu_ftp_diag_set_transfer(uint32_t transfer_id,
                                             const char *engine,
                                             const char *direction)
{
    (void) transfer_id;
    (void) engine;
    (void) direction;
}

static inline void ryu_ftp_diag_clear_transfer(void)
{
}

static inline void ryu_ftp_diag_dump(const char *phase)
{
    (void) phase;
}
#endif

#ifdef __cplusplus
}
#endif

#endif
