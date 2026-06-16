#include "ryu_ftp_connection_diag.h"

#ifdef FTP_RYU_CONN_DIAG

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <csp/arch/csp_time.h>
#include <csp_conn.h>

static __thread uint32_t diag_transfer_id;
static __thread char diag_engine[8];
static __thread char diag_direction[16];

static const char *conn_state_name(csp_conn_state_t state)
{
    switch (state) {
    case CONN_CLOSED: return "CONN_CLOSED";
    case CONN_OPEN: return "CONN_OPEN";
    }
    return "CONN_UNKNOWN";
}

static const char *rdp_state_name(csp_rdp_state_t state)
{
    switch (state) {
    case RDP_CLOSED: return "RDP_CLOSED";
    case RDP_SYN_SENT: return "RDP_SYN_SENT";
    case RDP_SYN_RCVD: return "RDP_SYN_RCVD";
    case RDP_OPEN: return "RDP_OPEN";
    case RDP_CLOSE_WAIT: return "RDP_CLOSE_WAIT";
    case RDP_ABORT: return "RDP_ABORT";
    }
    return "RDP_UNKNOWN";
}

static int conn_index(const csp_conn_t *needle)
{
    size_t count = 0;
    const csp_conn_t *table = csp_conn_get_array(&count);

    for (size_t i = 0; i < count; ++i) {
        if (&table[i] == needle)
            return (int) i;
    }

    return -1;
}

static uint32_t age_ms(uint32_t now, uint32_t then)
{
    return (then == 0) ? 0 : (now - then);
}

static void log_conn(const char *event,
                     const char *site,
                     const csp_conn_t *conn,
                     int ret)
{
    const uint32_t now = csp_get_ms();

    if (conn == NULL) {
        fprintf(stderr,
                "[FTP_RYU_CONN_DIAG] t=%" PRIu32 " transfer=%" PRIu32
                " engine=%s direction=%s event=%s site=%s conn=NULL ret=%d\n",
                now, diag_transfer_id, diag_engine, diag_direction, event,
                site ? site : "-", ret);
        return;
    }

    fprintf(stderr,
            "[FTP_RYU_CONN_DIAG] t=%" PRIu32 " transfer=%" PRIu32
            " engine=%s direction=%s event=%s site=%s conn=%p idx=%d ret=%d"
            " state=%s rdp=%s src=%u dst=%u dport=%u sport=%u flags=0x%02x"
            " age_ms=%" PRIu32 " close_wait_ms=%" PRIu32 "\n",
            now, diag_transfer_id, diag_engine, diag_direction, event,
            site ? site : "-", (const void *) conn, conn_index(conn), ret,
            conn_state_name(conn->state), rdp_state_name(conn->rdp.state),
            conn->idin.src, conn->idin.dst, conn->idin.dport,
            conn->idin.sport, conn->idin.flags, age_ms(now, conn->timestamp),
            (conn->rdp.state == RDP_CLOSE_WAIT) ? age_ms(now, conn->timestamp) : 0);
}

void ryu_ftp_diag_set_transfer(uint32_t transfer_id,
                               const char *engine,
                               const char *direction)
{
    diag_transfer_id = transfer_id;
    snprintf(diag_engine, sizeof(diag_engine), "%s", engine ? engine : "-");
    snprintf(diag_direction, sizeof(diag_direction), "%s",
             direction ? direction : "-");
}

void ryu_ftp_diag_clear_transfer(void)
{
    diag_transfer_id = 0;
    diag_engine[0] = '\0';
    diag_direction[0] = '\0';
}

void ryu_ftp_diag_dump(const char *phase)
{
    size_t count = 0;
    const csp_conn_t *table = csp_conn_get_array(&count);
    const uint32_t now = csp_get_ms();
    size_t open = 0;

    for (size_t i = 0; i < count; ++i) {
        if (table[i].state == CONN_OPEN)
            ++open;
    }

    fprintf(stderr,
            "[FTP_RYU_CONN_DIAG] t=%" PRIu32 " transfer=%" PRIu32
            " engine=%s direction=%s dump=%s csp_conn_max=%zu open=%zu\n",
            now, diag_transfer_id, diag_engine, diag_direction,
            phase ? phase : "-", count, open);

    for (size_t i = 0; i < count; ++i) {
        const csp_conn_t *conn = &table[i];
        fprintf(stderr,
                "[FTP_RYU_CONN_DIAG] t=%" PRIu32 " transfer=%" PRIu32
                " dump=%s idx=%zu conn=%p state=%s rdp=%s src=%u dst=%u"
                " dport=%u sport=%u flags=0x%02x age_ms=%" PRIu32
                " close_wait_ms=%" PRIu32 "\n",
                now, diag_transfer_id, phase ? phase : "-", i,
                (const void *) conn, conn_state_name(conn->state),
                rdp_state_name(conn->rdp.state), conn->idin.src,
                conn->idin.dst, conn->idin.dport, conn->idin.sport,
                conn->idin.flags, age_ms(now, conn->timestamp),
                (conn->rdp.state == RDP_CLOSE_WAIT) ?
                    age_ms(now, conn->timestamp) : 0);
    }
}

csp_conn_t *ryu_ftp_diag_connect(const char *site,
                                 uint8_t prio,
                                 uint8_t dest,
                                 uint8_t dport,
                                 uint32_t timeout,
                                 uint32_t opts)
{
    fprintf(stderr,
            "[FTP_RYU_CONN_DIAG] t=%" PRIu32 " transfer=%" PRIu32
            " engine=%s direction=%s event=csp_connect_call site=%s"
            " prio=%u dest=%u dport=%u timeout=%" PRIu32 " opts=0x%08" PRIx32 "\n",
            csp_get_ms(), diag_transfer_id, diag_engine, diag_direction,
            site ? site : "-", prio, dest, dport, timeout, opts);

    csp_conn_t *conn = csp_connect(prio, dest, dport, timeout, opts);
    log_conn("csp_connect_return", site, conn, conn ? 0 : -1);
    ryu_ftp_diag_dump("post-connect");
    return conn;
}

int ryu_ftp_diag_close(const char *site, csp_conn_t *conn)
{
    log_conn("csp_close_call", site, conn, 0);
    int ret = csp_close(conn);
    log_conn("csp_close_return", site, conn, ret);
    ryu_ftp_diag_dump("post-close");
    return ret;
}

#endif
