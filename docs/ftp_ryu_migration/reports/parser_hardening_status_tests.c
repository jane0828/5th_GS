/*
 * Standalone malformed-status validation harness for the optional Ryu FTP
 * parser hardening patch. It intentionally mirrors the optional patch helper
 * and does not include or modify the literal port source.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FTP_OK 0
#define FTP_STATUS_REPLY 6
#define GS_FTP_STATUS_CHUNKS 10

#define GS_OK 0
#define GS_ERROR_DATA -2004

typedef struct __attribute__((packed)) {
    uint32_t next;
    uint32_t count;
} ftp_status_element_t;

typedef struct __attribute__((packed)) {
    uint8_t ret;
    uint32_t complete;
    uint32_t total;
    uint16_t entries;
    ftp_status_element_t entry[GS_FTP_STATUS_CHUNKS];
} ftp_status_reply_t;

typedef struct __attribute__((packed)) {
    uint8_t type;
    ftp_status_reply_t statusrep;
} ftp_packet_t;

typedef struct {
    uint32_t chunks;
    ftp_status_element_t last_status[GS_FTP_STATUS_CHUNKS];
    uint32_t last_entries;
} ftpnew_state_t;

static uint16_t be16(uint16_t v)
{
    return (uint16_t)((v >> 8) | (v << 8));
}

static uint32_t be32(uint32_t v)
{
    return ((v & 0x000000ffU) << 24) |
           ((v & 0x0000ff00U) << 8) |
           ((v & 0x00ff0000U) >> 8) |
           ((v & 0xff000000U) >> 24);
}

#define STATUS_FIXED_HEADER_LEN 12U
#define STATUS_REQUIRED_LEN(entries) \
    (STATUS_FIXED_HEADER_LEN + ((uint32_t)(entries) * (uint32_t)sizeof(ftp_status_element_t)))

static int validate_status_reply(ftpnew_state_t *state, ftp_packet_t *rep, int reply_length)
{
    if (reply_length < (int)STATUS_FIXED_HEADER_LEN)
        return GS_ERROR_DATA;

    if (rep->type != FTP_STATUS_REPLY || rep->statusrep.ret != FTP_OK)
        return GS_ERROR_DATA;

    rep->statusrep.complete = be32(rep->statusrep.complete);
    rep->statusrep.total = be32(rep->statusrep.total);
    rep->statusrep.entries = be16(rep->statusrep.entries);

    if (rep->statusrep.entries > GS_FTP_STATUS_CHUNKS)
        return GS_ERROR_DATA;

    if (reply_length < (int)STATUS_REQUIRED_LEN(rep->statusrep.entries))
        return GS_ERROR_DATA;

    if (rep->statusrep.complete > rep->statusrep.total)
        return GS_ERROR_DATA;

    if (rep->statusrep.total != state->chunks)
        return GS_ERROR_DATA;

    if (rep->statusrep.complete == rep->statusrep.total) {
        if (rep->statusrep.entries != 0)
            return GS_ERROR_DATA;
        state->last_entries = 0;
        return GS_OK;
    }

    if (rep->statusrep.entries == 0)
        return GS_ERROR_DATA;

    uint32_t previous_end = 0;
    for (uint32_t i = 0; i < rep->statusrep.entries; i++) {
        uint32_t next = be32(rep->statusrep.entry[i].next);
        uint32_t count = be32(rep->statusrep.entry[i].count);

        if (count == 0)
            return GS_ERROR_DATA;

        if (next > UINT32_MAX - count)
            return GS_ERROR_DATA;

        uint32_t end = next + count;
        if (next >= rep->statusrep.total || end > rep->statusrep.total)
            return GS_ERROR_DATA;

        if (next < previous_end)
            return GS_ERROR_DATA;

        previous_end = end;
        state->last_status[i].next = next;
        state->last_status[i].count = count;
    }

    state->last_entries = rep->statusrep.entries;
    return GS_OK;
}

static void make_status(ftp_packet_t *p, uint32_t complete, uint32_t total, uint16_t entries)
{
    memset(p, 0, sizeof(*p));
    p->type = FTP_STATUS_REPLY;
    p->statusrep.ret = FTP_OK;
    p->statusrep.complete = be32(complete);
    p->statusrep.total = be32(total);
    p->statusrep.entries = be16(entries);
}

static void set_entry(ftp_packet_t *p, uint32_t index, uint32_t next, uint32_t count)
{
    p->statusrep.entry[index].next = be32(next);
    p->statusrep.entry[index].count = be32(count);
}

static int run_case(const char *name, ftp_packet_t packet, int length, uint32_t chunks, int expected)
{
    ftpnew_state_t state;
    memset(&state, 0, sizeof(state));
    state.chunks = chunks;

    int got = validate_status_reply(&state, &packet, length);
    if (got != expected) {
        printf("FAIL %-24s expected=%d got=%d\n", name, expected, got);
        return 1;
    }

    printf("PASS %-24s rc=%d entries=%u\n", name, got, state.last_entries);
    return 0;
}

int main(void)
{
    int failures = 0;
    ftp_packet_t p;

    make_status(&p, 0, 3, 1);
    set_entry(&p, 0, 0, 3);
    failures += run_case("valid fixed", p, sizeof(p), 3, GS_OK);

    make_status(&p, 0, 3, 1);
    set_entry(&p, 0, 0, 3);
    failures += run_case("valid variable", p, STATUS_REQUIRED_LEN(1), 3, GS_OK);

    make_status(&p, 0, 3, 1);
    set_entry(&p, 0, 0, 3);
    failures += run_case("truncated entry", p, STATUS_REQUIRED_LEN(1) - 1, 3, GS_ERROR_DATA);

    make_status(&p, 0, 3, GS_FTP_STATUS_CHUNKS + 1);
    failures += run_case("oversized entries", p, sizeof(p), 3, GS_ERROR_DATA);

    make_status(&p, 0, 3, 256);
    failures += run_case("endian crafted entries", p, sizeof(p), 3, GS_ERROR_DATA);

    make_status(&p, 0, UINT32_MAX, 1);
    set_entry(&p, 0, UINT32_MAX - 1, 4);
    failures += run_case("next count overflow", p, STATUS_REQUIRED_LEN(1), UINT32_MAX, GS_ERROR_DATA);

    make_status(&p, 0, 3, 1);
    set_entry(&p, 0, 3, 1);
    failures += run_case("range next total", p, STATUS_REQUIRED_LEN(1), 3, GS_ERROR_DATA);

    make_status(&p, 0, 3, 1);
    set_entry(&p, 0, 2, 2);
    failures += run_case("range end total", p, STATUS_REQUIRED_LEN(1), 3, GS_ERROR_DATA);

    make_status(&p, 0, 3, 1);
    set_entry(&p, 0, 1, 0);
    failures += run_case("zero count", p, STATUS_REQUIRED_LEN(1), 3, GS_ERROR_DATA);

    make_status(&p, 0, 5, 2);
    set_entry(&p, 0, 1, 3);
    set_entry(&p, 1, 3, 1);
    failures += run_case("overlap", p, STATUS_REQUIRED_LEN(2), 5, GS_ERROR_DATA);

    make_status(&p, 0, 5, 2);
    set_entry(&p, 0, 1, 1);
    set_entry(&p, 1, 1, 1);
    failures += run_case("duplicate", p, STATUS_REQUIRED_LEN(2), 5, GS_ERROR_DATA);

    make_status(&p, 4, 3, 0);
    failures += run_case("complete over total", p, STATUS_REQUIRED_LEN(0), 3, GS_ERROR_DATA);

    make_status(&p, 2, 3, 0);
    failures += run_case("incomplete no entries", p, STATUS_REQUIRED_LEN(0), 3, GS_ERROR_DATA);

    make_status(&p, 3, 3, 1);
    set_entry(&p, 0, 0, 1);
    failures += run_case("complete with entries", p, STATUS_REQUIRED_LEN(1), 3, GS_ERROR_DATA);

    make_status(&p, 0, 4, 1);
    set_entry(&p, 0, 0, 1);
    failures += run_case("wrong total", p, STATUS_REQUIRED_LEN(1), 3, GS_ERROR_DATA);

    return failures ? 1 : 0;
}
