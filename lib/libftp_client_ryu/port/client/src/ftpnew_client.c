/**
 * @file ftpnew_client.c
 * @author Han-Gyeol Ryu (ryu@yonsei.ac.kr)
 * @brief Ground-side client for the ftpnew backend.
 * @version 1.0
 * @date 2026-06-04
 * 
 * Astrodynamics & Control Lab, Yonsei University.
 *
 * Endian convention: all multi-byte payload integers cross the wire through
 * csp_hton32 / csp_ntoh32, including the extension requests.
 * The historical blunder for FTP_DATA chunk index is still an exception,
 * which is always LE.
 */
#include "ftpnew_client.h"
#include "ftpnew_types_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <csp/csp.h>
#include <csp/csp_endian.h>

#include <gs/util/error.h>
#include <gs/ftp/client.h>

#define GS_FTP_INTERNAL_USE 1
#include <gs/ftp/internal/types.h>
#undef  GS_FTP_INTERNAL_USE

#ifdef FTP_RYU_CONN_DIAG
#include "ryu_ftp_connection_diag.h"
#define FTPNEW_CSP_CONNECT(site, prio, dest, dport, timeout, opts) \
    ryu_ftp_diag_connect((site), (prio), (dest), (dport), (timeout), (opts))
#define FTPNEW_CSP_CLOSE(site, conn) ryu_ftp_diag_close((site), (conn))
#else
#define FTPNEW_CSP_CONNECT(site, prio, dest, dport, timeout, opts) \
    csp_connect((prio), (dest), (dport), (timeout), (opts))
#define FTPNEW_CSP_CLOSE(site, conn) csp_close((conn))
#endif

#define LOG_ERR(fmt, ...) fprintf(stderr, "[ftpnew_client] " fmt "\n", ##__VA_ARGS__)

/* Chunk status markers, matching the libftp_client convention. */
static const char chunk_missing = '-';
static const char chunk_ok      = '+';

/* ---- Settings getters (replicated locally; libftp_client provides
 *      these too but we want to avoid linking against it). */
static uint8_t setting_host(const gs_ftp_settings_t* s)
{
    return (s && s->host) ? s->host : csp_get_address();
}

static uint32_t setting_timeout(const gs_ftp_settings_t* s)
{
    return (s && s->timeout) ? s->timeout : FTPNEW_CLIENT_DEFAULT_TIMEOUT;
}

static uint32_t setting_chunk_size(const gs_ftp_settings_t* s)
{
    if (s && s->chunk_size && s->chunk_size <= GS_FTP_MAX_CHUNK_SIZE)
        return s->chunk_size;
    return FTPNEW_CLIENT_DEFAULT_CHUNK_SIZE;
}

static uint8_t setting_csp_port(const gs_ftp_settings_t* s)
{
    return (s && s->port) ? s->port : FTPNEW_CLIENT_DEFAULT_PORT;
}

typedef struct {
    gs_ftp_backend_type_t backend;
    const char* path;
    uint32_t    addr;
    uint32_t    size;
} ftpnew_url_t;

typedef struct {
    FILE*       fp;
    FILE*       fp_map;
    csp_conn_t* conn;
    uint32_t    timeout;
    char        file_name[GS_FTP_PATH_LENGTH];
    uint32_t    file_size;
    uint32_t    chunks;
    int         chunk_size;
    uint32_t    checksum;
    ftp_status_element_t last_status[GS_FTP_STATUS_CHUNKS];
    uint32_t    last_entries;
    gs_ftp_info_callback_t info_callback;
    void*       info_data;
    uint32_t    interpacket_delay_ms;
    bool        use_rdp;
} ftpnew_state_t;

/* 
 * Per-chunk inter-packet gap for UDP-mode uploads.
 */
#define FTPNEW_UDP_INTERPKT_US  500

/**
 * @brief Map an ftpnew wire ret value (ftp_ret_t) to gs_error_t.
 *
 * @details The wire field is whatever the server stored via the @c ftp_ret_t
 *          enum (defined in @c config/ftpnew_types.h). No intermediate
 *          gs_ftp_return_t hop -- the value on the wire IS the ftp_ret_t.
 */
static gs_error_t wire_ret_to_gs(int wire_ret)
{
    switch ((ftp_ret_t)wire_ret) {
    case FTP_OK:          return GS_OK;
    case FTP_ERR_INVAL:   return GS_ERROR_ARG;
    case FTP_ERR_BUSY:    return GS_ERROR_BUSY;
    case FTP_ERR_NOENT:   return GS_ERROR_NOT_FOUND;
    case FTP_ERR_EXISTS:  return GS_ERROR_EXIST;
    case FTP_ERR_NOSPC:   return GS_ERROR_FULL;
    case FTP_ERR_IO:      return GS_ERROR_IO;
    case FTP_ERR_NOMEM:   return GS_ERROR_ALLOC;
    case FTP_ERR_NOTSUP:  return GS_ERROR_NOT_SUPPORTED;
    case FTP_ERR_TIMEOUT: return GS_ERROR_TIMEOUT;
    }
    return GS_ERROR_UNKNOWN;
}

static uint32_t connect_opts(bool use_rdp)
{
    return use_rdp ? CSP_O_RDP | CSP_O_CRC32 : CSP_O_CRC32;
}

static void str_copy_path(char* dst, size_t dst_size, const char* src)
{
    if (dst_size == 0)
        return;
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = 0;
}

static gs_error_t ftpnew_parse_url(const char* url,
                                   ftpnew_url_t* parsed)
{
    memset(parsed, 0, sizeof(*parsed));

    if (strstr(url, "mem://")) {
        char tmp[strlen(url) + 1];
        strcpy(tmp, url);

        char* saveptr;
        char* token = strtok_r(tmp + strlen("mem://"), "++", &saveptr);
        if (!token || *token == 0)
            return GS_ERROR_ARG;

        char* end = NULL;
        unsigned long addr = strtoul(token, &end, 0);
        if (end == token)
            return GS_ERROR_ARG;
        parsed->addr = (uint32_t)addr;

        token = strtok_r(NULL, "++\r\n", &saveptr);
        if (token) {
            unsigned long size = strtoul(token, &end, 0);
            if (end == token)
                return GS_ERROR_ARG;
            parsed->size = (uint32_t)size;
        }

        parsed->backend = GS_FTP_BACKEND_RAM;
        parsed->path = "";
        return GS_OK;
    }

    parsed->backend = GS_FTP_BACKEND_FILE;
    if (strstr(url, "file://"))
        parsed->path = url + strlen("file://");
    else
        parsed->path = url;
    return GS_OK;
}

static gs_ftp_backend_type_t ftpnew_get_backend(const ftpnew_url_t* url,
                                                const gs_ftp_settings_t* settings)
{
    if (url)
        switch (url->backend) {
        case GS_FTP_BACKEND_RAM:
        case GS_FTP_BACKEND_FAT:
        case GS_FTP_BACKEND_UFFS:
            return url->backend;
        case GS_FTP_BACKEND_FILE:
            break;
        }

    if (settings)
        switch (settings->mode) {
        case GS_FTP_MODE_GATOSS:
            return GS_FTP_BACKEND_UFFS;
        case GS_FTP_MODE_STANDARD:
            break;
        }

    return GS_FTP_BACKEND_FILE;
}

uint32_t ftpnew_client_file_crc(FILE* fp);

/* ========================================================================
 *           Upload-side helper (server holds the map)
 * ====================================================================== */

static gs_error_t ftpnew_status_request(ftpnew_state_t* state)
{
    ftp_packet_t req;
    req.type = FTP_STATUS_REQUEST;
    int req_length = sizeof(req.type);

    ftp_packet_t rep;
    if (csp_transaction_persistent(state->conn, state->timeout,
                                   &req, req_length, &rep, -1) == 0) {
        LOG_ERR("Failed to receive status reply");
        return GS_ERROR_IO;
    }

    if (rep.type != FTP_STATUS_REPLY || rep.statusrep.ret != FTP_OK) {
        LOG_ERR("Reply was not STATUS_REPLY");
        return GS_ERROR_DATA;
    }

    rep.statusrep.complete = csp_ntoh32(rep.statusrep.complete);
    rep.statusrep.total    = csp_ntoh32(rep.statusrep.total);
    rep.statusrep.entries  = csp_ntoh16(rep.statusrep.entries);

    if (state->info_callback) {
        gs_ftp_info_t info = {
            .user_data = state->info_data,
            .type      = GS_FTP_INFO_COMPLETED,
            .u.completed = {
                .completed_chunks = rep.statusrep.complete,
                .total_chunks     = rep.statusrep.total,
            },
        };
        state->info_callback(&info);
    }

    if (rep.statusrep.complete != rep.statusrep.total) {
        for (int i = 0; i < rep.statusrep.entries; i++) {
            rep.statusrep.entry[i].next  = csp_ntoh32(rep.statusrep.entry[i].next);
            rep.statusrep.entry[i].count = csp_ntoh32(rep.statusrep.entry[i].count);
            state->last_status[i].next   = rep.statusrep.entry[i].next;
            state->last_status[i].count  = rep.statusrep.entry[i].count;
        }
        state->last_entries = rep.statusrep.entries;
    }
    else
        state->last_entries = 0;

    return GS_OK;
}

static gs_error_t ftpnew_data(ftpnew_state_t* state)
{
    ftp_packet_t packet;
    packet.type = FTP_DATA;

    for (uint32_t i = 0; i < state->last_entries; i++)
    {
        ftp_status_element_t* n = &state->last_status[i];

        for (uint32_t j = 0; j < n->count; j++)
        {
            packet.data.chunk = n->next + j;

            if (state->info_callback)
            {
                gs_ftp_info_t info = {
                    .user_data = state->info_data,
                    .type      = GS_FTP_INFO_PROGRESS,
                    .u.progress = {
                        .current_chunk = packet.data.chunk,
                        .total_chunks  = state->chunks,
                        .chunk_size    = state->chunk_size,
                    },
                };
                state->info_callback(&info);
            }

            if ((unsigned)ftell(state->fp) != packet.data.chunk * state->chunk_size)
                fseek(state->fp, packet.data.chunk * state->chunk_size, SEEK_SET);

            int ret = fread(packet.data.bytes, 1, state->chunk_size, state->fp);
            if (ret < 0 && !feof(state->fp))
                break;

            /* If the read is short of chunk_size, zero-fill the remainder so
             * a full chunk_size is always transmitted (server-side last-chunk
             * math relies on the agreed chunk_size). */
            if (ret < state->chunk_size)
                memset(&packet.data.bytes[ret], 0, state->chunk_size - ret);

            /* Chunk number MUST be little-endian on the wire.
             * This is preserved from the original client (historical quirk). */
            packet.data.chunk = csp_htole32(packet.data.chunk);

            int length = sizeof(packet.type) + sizeof(uint32_t) + state->chunk_size;
            if (csp_transaction_persistent(state->conn, state->timeout,
                                           &packet, length, NULL, 0) != 1)
            {
                LOG_ERR("Data transaction failed");
                FTPNEW_CSP_CLOSE("ftpnew_data_error", state->conn);
                break;
            }

            /* UDP mode has no ACK pacing -- let the server drain its CSP
             * interface buffers before slamming it with the next chunk. */
            if (!state->use_rdp)
                usleep(state->interpacket_delay_ms * 1000);
        }
    }

    return GS_OK;
}

/* ========================================================================
 *           Download-side helper (client holds the map)
 * ====================================================================== */

static gs_error_t ftpnew_status_reply(ftpnew_state_t* state)
{
    ftp_packet_t ftp_packet_req = { .type = FTP_STATUS_REPLY };
    ftp_status_reply_t* status = (ftp_status_reply_t*) &ftp_packet_req.statusrep;

    uint32_t next = 0;
    uint32_t count = 0;

    status->entries  = 0;
    status->complete = 0;

    for (uint32_t i = 0; i < state->chunks; i++) {
        if ((uint32_t)ftell(state->fp_map) != i) {
            if (fseek(state->fp_map, i, SEEK_SET) != 0) {
                LOG_ERR("fseek failed");
                return GS_ERROR_IO;
            }
        }

        char cstat;
        if (fread(&cstat, 1, 1, state->fp_map) != 1) {
            LOG_ERR("fread byte %u failed", i);
            return GS_ERROR_IO;
        }

        int s = (cstat == chunk_ok);
        if (s)
            status->complete++;

        if (status->entries < GS_FTP_STATUS_CHUNKS) {
            if (!s) {
                if (!count)
                    next = i;
                count++;
            }

            if (count > 0 && (s || i == state->chunks - 1)) {
                status->entry[status->entries].next  = csp_hton32(next);
                status->entry[status->entries].count = csp_hton32(count);
                status->entries++;
                count = 0;
            }
        }
    }

    if (state->info_callback) {
        gs_ftp_info_t info = {
            .user_data = state->info_data,
            .type      = GS_FTP_INFO_COMPLETED,
            .u.completed = {
                .completed_chunks = status->complete,
                .total_chunks     = state->chunks,
            },
        };
        state->info_callback(&info);
    }

    status->entries  = csp_hton16(status->entries);
    status->complete = csp_hton32(status->complete);
    status->total    = csp_hton32(state->chunks);
    status->ret      = FTP_OK;

    int req_len = sizeof(ftp_packet_req.type) + sizeof(ftp_packet_req.statusrep);
    if (csp_transaction_persistent(state->conn, state->timeout,
                                   &ftp_packet_req, req_len, NULL, 0) != 1) {
        LOG_ERR("Failed to send status reply");
        return GS_ERROR_IO;
    }

    if (state->chunks == 0)
        return GS_OK;

    /* Receive data chunks until we get the last one. */
    while (1) {
        csp_packet_t* packet = csp_read(state->conn, state->timeout);
        if (!packet) {
            LOG_ERR("Timeout while waiting for data");
            return GS_ERROR_TIMEOUT;
        }

        ftp_packet_t* fp = (ftp_packet_t*)&packet->data;
        fp->data.chunk = csp_letoh32(fp->data.chunk);

        /* Server overloads the type byte with an ftp_ret_t error code on a
         * mid-stream read failure. Decode as int8_t to recover the sign. */
        if ((int8_t)fp->type == FTP_ERR_IO) {
            LOG_ERR("Server failed to read chunk");
            csp_buffer_free(packet);
            return GS_ERROR_DATA;
        }

        if (fp->data.chunk >= state->chunks) {
            LOG_ERR("Bad chunk number %u > %u", fp->data.chunk, state->chunks);
            csp_buffer_free(packet);
            continue;
        }

        uint32_t size;
        if (fp->data.chunk == state->chunks - 1) {
            size = state->file_size % state->chunk_size;
            if (size == 0)
                size = state->chunk_size;
        }
        else
            size = state->chunk_size;

        if ((unsigned)ftell(state->fp) != fp->data.chunk * state->chunk_size) {
            if (fseek(state->fp, fp->data.chunk * state->chunk_size, SEEK_SET) != 0) {
                LOG_ERR("Seek error");
                csp_buffer_free(packet);
                return GS_ERROR_ACCESS;
            }
        }

        if (fwrite(fp->data.bytes, 1, size, state->fp) != size) {
            LOG_ERR("Write error");
            csp_buffer_free(packet);
            return GS_ERROR_IO;
        }
        fflush(state->fp);

        if ((unsigned)ftell(state->fp_map) != fp->data.chunk) {
            if (fseek(state->fp_map, fp->data.chunk, SEEK_SET) != 0) {
                LOG_ERR("Map Seek error");
                csp_buffer_free(packet);
                return GS_ERROR_IO;
            }
        }

        if (fwrite(&chunk_ok, 1, 1, state->fp_map) != 1) {
            LOG_ERR("Map write error");
            csp_buffer_free(packet);
            return GS_ERROR_IO;
        }
        fflush(state->fp_map);

        if (state->info_callback) {
            gs_ftp_info_t info = {
                .user_data = state->info_data,
                .type      = GS_FTP_INFO_PROGRESS,
                .u.progress = {
                    .current_chunk = fp->data.chunk,
                    .total_chunks  = state->chunks,
                    .chunk_size    = state->chunk_size,
                },
            };
            state->info_callback(&info);
        }

        bool last_chunk = (fp->data.chunk == state->chunks - 1);
        csp_buffer_free(packet);

        if (last_chunk)
            break;
    }

    fflush(state->fp);
    fsync(fileno(state->fp));
    fflush(state->fp_map);
    fsync(fileno(state->fp_map));

    return GS_OK;
}

/* ========================================================================
 *           Shared helpers used by both upload and download
 * ====================================================================== */

static gs_error_t ftpnew_crc(ftpnew_state_t* state)
{
    ftp_packet_t packet;
    packet.type = FTP_CRC_REQUEST;

    int repsiz = sizeof(packet.type) + sizeof(packet.crcrep);
    if (csp_transaction_persistent(state->conn, state->timeout,
                                   &packet, sizeof(packet.type),
                                   &packet, repsiz) != repsiz)
        return GS_ERROR_IO;

    if (packet.type != FTP_CRC_REPLY)
        return GS_ERROR_DATA;

    if (packet.crcrep.ret != FTP_OK)
        return wire_ret_to_gs(packet.crcrep.ret);

    /* Recompute the local CRC and compare against the server's. For upload,
     * state->fp is the source we just sent; for download, state->fp is the
     * .tmp file we just wrote into. ftpnew_client_file_crc rewinds before
     * computing, so the previous fp position doesn't matter. */
    state->checksum = ftpnew_client_file_crc(state->fp);

    packet.crcrep.crc = csp_ntoh32(packet.crcrep.crc);

    if (state->info_callback) {
        gs_ftp_info_t info = {
            .user_data = state->info_data,
            .type      = GS_FTP_INFO_CRC,
            .u.crc = {
                .remote = packet.crcrep.crc,
                .local  = state->checksum,
            },
        };
        state->info_callback(&info);
    }

    if (packet.crcrep.crc != state->checksum)
        return GS_ERROR_DATA;

    return GS_OK;
}

/* @p remove_map: 0 = leave map for resume; 1 = success path (delete map and
 * rename .tmp to the final name). Matches the libftp_client convention. */
static void ftpnew_done(ftpnew_state_t* state, bool remove_map)
{
    if (state->fp_map)
        fclose(state->fp_map);
    if (state->fp)
        fclose(state->fp);

    if (remove_map) {
        char map[GS_FTP_PATH_LENGTH + 4];
        snprintf(map, sizeof(map), "%s.map", state->file_name);
        if (remove(map) != 0)
            LOG_ERR("Failed to remove %s", map);

        char tmp[GS_FTP_PATH_LENGTH + 4];
        snprintf(tmp, sizeof(tmp), "%s.tmp", state->file_name);
        if (rename(tmp, state->file_name) != 0)
            LOG_ERR("Failed to rename %s -> %s", tmp, state->file_name);
    }

    if (state->conn) {
        ftp_packet_t packet;
        packet.type = FTP_DONE;
        csp_transaction_persistent(state->conn, state->timeout,
                                   &packet, sizeof(packet.type), NULL, 0);
        FTPNEW_CSP_CLOSE("ftpnew_done", state->conn);
        state->conn = NULL;
    }
}

/* ========================================================================
 *                          Upload / Download
 * ====================================================================== */

gs_error_t ftpnew_upload(const gs_ftp_settings_t* settings,
                         const char* local_url, const char* remote_url,
                         gs_ftp_info_callback_t info_callback, void* info_data,
                         bool use_rdp, uint32_t interpacket_delay_ms)
{
    if (!local_url || !remote_url)
        return GS_ERROR_ARG;

    ftpnew_url_t local_url_info;

    if (ftpnew_parse_url(local_url, &local_url_info)) {
        LOG_ERR("Invalid local URL: %s", local_url);
        return GS_ERROR_ARG;
    }

    if (local_url_info.backend != GS_FTP_BACKEND_FILE) {
        LOG_ERR("Unsupported local URL backend: %s", local_url);
        return GS_ERROR_NOT_SUPPORTED;
    }

    ftpnew_url_t remote_url_info;
    if (ftpnew_parse_url(remote_url, &remote_url_info)) {
        LOG_ERR("Invalid remote URL: %s", remote_url);
        return GS_ERROR_ARG;
    }

    ftpnew_state_t state;
    memset(&state, 0, sizeof(state));

    state.fp = fopen(local_url_info.path, "rb");
    if (!state.fp)
        return GS_ERROR_NOT_FOUND;

    struct stat statbuf;
    if (stat(local_url_info.path, &statbuf) != 0) {
        fclose(state.fp);
        return GS_ERROR_IO;
    }

    state.checksum = ftpnew_client_file_crc(state.fp);

    state.timeout       = setting_timeout(settings);
    state.chunk_size    = setting_chunk_size(settings);
    state.file_size     = (uint32_t)statbuf.st_size;
    state.chunks        = (state.file_size + state.chunk_size - 1) / state.chunk_size;
    state.info_callback = info_callback;
    state.info_data     = info_data;
    state.use_rdp       = use_rdp;
    state.interpacket_delay_ms = interpacket_delay_ms;
    str_copy_path(state.file_name, sizeof(state.file_name), local_url_info.path);

    if (info_callback) {
        gs_ftp_info_t info = {
            .user_data = info_data,
            .type      = GS_FTP_INFO_FILE,
            .u.file = {
                .size = state.file_size,
                .crc  = state.checksum,
            },
        };
        info_callback(&info);
    }

    ftp_packet_t req;
    req.type = FTP_UPLOAD_REQUEST;
    req.up.chunk_size = csp_hton16(state.chunk_size);
    req.up.size       = csp_hton32(state.file_size);
    req.up.crc32      = csp_hton32(state.checksum);
    req.up.mem_addr   = csp_hton32(remote_url_info.addr);
    req.up.backend    = ftpnew_get_backend(&remote_url_info, settings);
    str_copy_path(req.up.path, sizeof(req.up.path), remote_url_info.path);
    int req_length = sizeof(req.type) + sizeof(req.up);

    state.conn = FTPNEW_CSP_CONNECT("ftpnew_upload",
                                    CSP_PRIO_NORM,
                                    setting_host(settings),
                                    setting_csp_port(settings),
                                    state.timeout,
                                    connect_opts(use_rdp));
    if (!state.conn) {
        ftpnew_done(&state, 0);
        return GS_ERROR_IO;
    }

    ftp_packet_t rep;
    int rep_length = sizeof(rep.type) + sizeof(rep.uprep);
    if (csp_transaction_persistent(state.conn, state.timeout,
                                   &req, req_length, &rep, rep_length) != rep_length) {
        ftpnew_done(&state, 0);
        return GS_ERROR_TIMEOUT;
    }

    if (rep.type != FTP_UPLOAD_REPLY || rep.uprep.ret != FTP_OK) {
        ftpnew_done(&state, 0);
        return wire_ret_to_gs(rep.uprep.ret);
    }

    /**
     * If the file is empty, we're done.
     */
    if (state.file_size == 0) {
        ftpnew_done(&state, 0);
        return GS_OK;
    }

    /* The data stage may take multiple status rounds: request status,
     * fill in the chunks the server still needs, repeat until complete. */
    while (1) {
        gs_error_t s = ftpnew_status_request(&state);
        if (s) {
            ftpnew_done(&state, 0);
            return s;
        }
        if (state.last_entries == 0)
            break;
        s = ftpnew_data(&state);
        if (s) {
            ftpnew_done(&state, 0);
            return s;
        }
    }

    gs_error_t s = ftpnew_crc(&state);
    if (s) {
        ftpnew_done(&state, 0);
        return s;
    }

    ftpnew_done(&state, 0);
    return GS_OK;
}

gs_error_t ftpnew_download(const gs_ftp_settings_t* settings,
                           const char* local_url, const char* remote_url,
                           gs_ftp_info_callback_t info_callback, void* info_data,
                           bool use_rdp)
{
    if (!local_url || !remote_url)
        return GS_ERROR_ARG;

    ftpnew_url_t local_url_info;
    if (ftpnew_parse_url(local_url, &local_url_info)) {
        LOG_ERR("Invalid local URL: %s", local_url);
        return GS_ERROR_ARG;
    }

    if (local_url_info.backend != GS_FTP_BACKEND_FILE) {
        LOG_ERR("Unsupported local URL backend: %s", local_url);
        return GS_ERROR_NOT_SUPPORTED;
    }

    ftpnew_url_t remote_url_info;
    if (ftpnew_parse_url(remote_url, &remote_url_info) ||
        (remote_url_info.backend == GS_FTP_BACKEND_RAM && remote_url_info.size == 0)) {
        LOG_ERR("Invalid remote URL: %s", remote_url);
        return GS_ERROR_ARG;
    }

    ftpnew_state_t state;
    memset(&state, 0, sizeof(state));
    state.chunk_size    = setting_chunk_size(settings);
    state.timeout       = setting_timeout(settings);
    state.info_callback = info_callback;
    state.info_data     = info_data;
    str_copy_path(state.file_name, sizeof(state.file_name), local_url_info.path);

    ftp_packet_t req;
    req.type = FTP_DOWNLOAD_REQUEST;
    req.down.chunk_size = csp_hton16(state.chunk_size);
    req.down.mem_addr   = csp_hton32(remote_url_info.addr);
    req.down.mem_size   = csp_hton32(remote_url_info.size);
    req.down.backend    = ftpnew_get_backend(&remote_url_info, settings);
    str_copy_path(req.down.path, sizeof(req.down.path), remote_url_info.path);
    int req_length = sizeof(req.type) + sizeof(req.down);

    state.conn = FTPNEW_CSP_CONNECT("ftpnew_download",
                                    CSP_PRIO_NORM,
                                    setting_host(settings),
                                    setting_csp_port(settings),
                                    state.timeout,
                                    connect_opts(use_rdp));
    if (!state.conn)
        return GS_ERROR_IO;

    ftp_packet_t rep;
    int rep_length = sizeof(req.type) + sizeof(req.downrep);
    int got = csp_transaction_persistent(state.conn, state.timeout,
                                         &req, req_length, &rep, rep_length);
    if (got != rep_length) {
        LOG_ERR("Length mismatch. Expected %d, got %d", rep_length, got);
        ftpnew_done(&state, 0);
        return GS_ERROR_TIMEOUT;
    }

    if (rep.type != FTP_DOWNLOAD_REPLY || rep.downrep.ret != FTP_OK) {
        LOG_ERR("download reply rejected: type=%u ret=%u size=%u",
                rep.type, rep.downrep.ret, csp_ntoh32(rep.downrep.size));
        ftpnew_done(&state, 0);
        return wire_ret_to_gs(rep.downrep.ret);
    }

    state.file_size = csp_ntoh32(rep.downrep.size);
    state.checksum  = csp_ntoh32(rep.downrep.crc32);
    state.chunks    = (state.file_size + state.chunk_size - 1) / state.chunk_size;

    if (info_callback) {
        gs_ftp_info_t info = {
            .user_data = info_data,
            .type      = GS_FTP_INFO_FILE,
            .u.file = {
                .size = state.file_size,
                .crc  = state.checksum,
            },
        };
        info_callback(&info);
    }

    /* Refuse to overwrite an existing final file. */
    state.fp = fopen(local_url_info.path, "r+");
    if (state.fp) {
        ftpnew_done(&state, 0);
        return GS_ERROR_EXIST;
    }

    /* Continue or create the .tmp file. */
    char tmp[GS_FTP_PATH_LENGTH + 4];
    snprintf(tmp, sizeof(tmp), "%s.tmp", local_url_info.path);

    bool new_file = false;
    state.fp = fopen(tmp, "r+");
    if (!state.fp) {
        new_file = true;
        state.fp = fopen(tmp, "w+");
        if (!state.fp)
        {
            LOG_ERR("Failed to create %s", tmp);
            ftpnew_done(&state, 0);
            return GS_ERROR_IO;
        }
    }

    /* Open or create the chunk-bitmap file. */
    char map[GS_FTP_PATH_LENGTH + 4];
    snprintf(map, sizeof(map), "%s.map", local_url_info.path);

    state.fp_map = fopen(map, "r+");
    if (!state.fp_map) {
        /* .tmp existed but the map didn't: we'd be unable to know which
         * chunks are valid. Bail and keep .tmp for forensic inspection. */
        if (!new_file) {
            ftpnew_done(&state, 0);
            return GS_ERROR_EXIST;
        }

        state.fp_map = fopen(map, "w+");
        if (!state.fp_map) {
            LOG_ERR("Failed to create %s", map);
            ftpnew_done(&state, 0);
            return GS_ERROR_IO;
        }

        for (uint32_t i = 0; i < state.chunks; i++) {
            if (fwrite(&chunk_missing, 1, 1, state.fp_map) < 1)
            {
                LOG_ERR("Failed to clear bitmap");
                ftpnew_done(&state, 0);
                return GS_ERROR_IO;
            }
        }
        fflush(state.fp_map);
        fsync(fileno(state.fp_map));
    }

    gs_error_t s = ftpnew_status_reply(&state);
    if (s) {
        ftpnew_done(&state, 0);
        return s;
    }

    s = ftpnew_crc(&state);
    if (s) {
        ftpnew_done(&state, 0);
        return s;
    }

    /* Success: delete map, rename .tmp to the real name. */
    ftpnew_done(&state, 1);
    return GS_OK;
}

/* ========================================================================
 *                          Extension layer
 * ====================================================================== */

gs_error_t ftpnew_ping(const gs_ftp_settings_t* settings)
{
    ftp_ext_packet_t req;
    memset(&req, 0, sizeof(req));
    req.type = FTP_EXT_PING;
    for (uint8_t i = 0; i < sizeof(req.ping.data); i++)
        req.ping.data[i] = i + 1;

    ftp_ext_packet_t rep;
    int got = csp_transaction(CSP_PRIO_NORM,
                              setting_host(settings),
                              setting_csp_port(settings),
                              setting_timeout(settings),
                              &req, sizeof(req.type) + sizeof(req.ping),
                              &rep, sizeof(rep.type) + sizeof(rep.ping));
    if (got <= 0)
        return GS_ERROR_IO;
    if (rep.type != FTP_EXT_PING_REPLY ||
        memcmp(req.ping.data, rep.ping.data, sizeof(req.ping.data)) != 0)
            return GS_ERROR_DATA;
    return GS_OK;
}

gs_error_t ftpnew_get_csp_buffers(const gs_ftp_settings_t* settings,
                                  uint32_t* remaining, uint32_t* size)
{
    ftp_ext_packet_t req;
    memset(&req, 0, sizeof(req));
    req.type = FTP_EXT_CSP_BUFS;

    ftp_ext_packet_t rep;
    int rep_len = sizeof(rep.type) + sizeof(rep.buffersrep);
    int got = csp_transaction(CSP_PRIO_NORM,
                              setting_host(settings),
                              setting_csp_port(settings),
                              setting_timeout(settings),
                              &req, sizeof(req.type),
                              &rep, rep_len);
    if (got != rep_len)
        return GS_ERROR_IO;
    if (rep.type != FTP_EXT_CSP_BUFS_REPLY)
        return GS_ERROR_DATA;

    if (remaining)
        *remaining = csp_ntoh32(rep.buffersrep.count);
    if (size)
        *size = csp_ntoh32(rep.buffersrep.size);
    return GS_OK;
}

gs_error_t ftpnew_get_timeout(const gs_ftp_settings_t* settings,
                              uint32_t* timeout_ms,
                              uint32_t* interpacket_delay_ms)
{
    ftp_ext_packet_t req;
    memset(&req, 0, sizeof(req));
    req.type = FTP_EXT_TIMEOUT_GET;

    ftp_ext_packet_t rep;
    int rep_len = sizeof(rep.type) + sizeof(rep.timeoutrep);
    int got = csp_transaction(CSP_PRIO_NORM,
                              setting_host(settings),
                              setting_csp_port(settings),
                              setting_timeout(settings),
                              &req, sizeof(req.type),
                              &rep, rep_len);
    if (got != rep_len)
        return GS_ERROR_IO;
    if (rep.type != FTP_EXT_TIMEOUT_REPLY)
        return GS_ERROR_DATA;

    if (timeout_ms)
        *timeout_ms = csp_ntoh32(rep.timeoutrep.timeout);
    if (interpacket_delay_ms)
        *interpacket_delay_ms = csp_ntoh32(rep.timeoutrep.interpacket_delay);
    return GS_OK;
}

gs_error_t ftpnew_set_timeout(const gs_ftp_settings_t* settings,
                              uint32_t timeout_ms,
                              uint32_t interpacket_delay_ms)
{
    ftp_ext_packet_t req;
    memset(&req, 0, sizeof(req));
    req.type = FTP_EXT_TIMEOUT_SET;
    req.timeout.timeout = csp_hton32(timeout_ms);
    req.timeout.interpacket_delay = csp_hton32(interpacket_delay_ms);

    ftp_ext_packet_t rep;
    int req_len = sizeof(req.type) + sizeof(req.timeout);
    int rep_len = sizeof(rep.type) + sizeof(rep.timeoutrep);
    int got = csp_transaction(CSP_PRIO_NORM,
                              setting_host(settings),
                              setting_csp_port(settings),
                              setting_timeout(settings),
                              &req, req_len,
                              &rep, rep_len);
    if (got != rep_len)
        return GS_ERROR_IO;
    if (rep.type != FTP_EXT_TIMEOUT_REPLY)
        return GS_ERROR_DATA;

    if (timeout_ms != csp_ntoh32(rep.timeoutrep.timeout) ||
        interpacket_delay_ms != csp_ntoh32(rep.timeoutrep.interpacket_delay)) {
        LOG_ERR("Server failed to set timeout values: requested: %u, %u, got: %u, %u",
                timeout_ms, interpacket_delay_ms,
                csp_ntoh32(rep.timeoutrep.timeout), csp_ntoh32(rep.timeoutrep.interpacket_delay));
        return GS_ERROR_IO;
    }
    return GS_OK;
}

gs_error_t ftpnew_get_log_tags(const gs_ftp_settings_t* settings,
                               uint16_t count,
                               ftp_log_tag_t* tags,
                               int* n_out)
{
    if (count > FTP_EXT_LOG_REPLY_MAX_TAGS)
        count = FTP_EXT_LOG_REPLY_MAX_TAGS;

    ftp_ext_packet_t req;
    memset(&req, 0, sizeof(req));
    req.type       = FTP_EXT_LOG_REQUEST;
    req.log.action = 0;
    req.log.index  = csp_hton16(count);

    ftp_ext_packet_t rep;
    int req_len = sizeof(req.type) + sizeof(req.log);
    int got = csp_transaction(CSP_PRIO_NORM,
                              setting_host(settings),
                              setting_csp_port(settings),
                              setting_timeout(settings),
                              &req, req_len,
                              &rep, -1);
    if (got <= 0)
        return GS_ERROR_IO;
    if (rep.type != FTP_EXT_LOG_REPLY)
        return GS_ERROR_DATA;

    gs_error_t ret = wire_ret_to_gs(rep.logrep.ret);
    if (ret != GS_OK)
        return ret;

    int payload = got - (int)sizeof(rep.type) - (int)sizeof(rep.logrep.ret) - (int)sizeof(rep.logrep.count);
    int n = payload / (int)sizeof(ftp_log_tag_t);
    if (n > (int)count)
        n = (int)count;

    if (tags && n > 0) {
        memcpy(tags, rep.logrep.tags, n * sizeof(ftp_log_tag_t));
        for (int i = 0; i < n; i++) {
            tags[i].index    = csp_ntoh16(tags[i].index);
            tags[i].csp_time = csp_ntoh32(tags[i].csp_time);
        }
    }
    if (n_out)
        *n_out = n;
    return GS_OK;
}

gs_error_t ftpnew_get_log_entry(const gs_ftp_settings_t* settings,
                                uint16_t index,
                                ftp_log_entry_t* out)
{
    ftp_ext_packet_t req;
    memset(&req, 0, sizeof(req));
    req.type       = FTP_EXT_LOG_REQUEST;
    req.log.action = 1;
    req.log.index  = csp_hton16(index);

    ftp_ext_packet_t rep;
    int req_len = sizeof(req.type) + sizeof(req.log);
    int got = csp_transaction(CSP_PRIO_NORM,
                              setting_host(settings),
                              setting_csp_port(settings),
                              setting_timeout(settings),
                              &req, req_len,
                              &rep, -1);
    if (got <= 0)
        return GS_ERROR_IO;
    if (rep.type != FTP_EXT_LOG_REPLY)
        return GS_ERROR_DATA;

    gs_error_t ret = wire_ret_to_gs(rep.logrep.ret);
    if (ret != GS_OK)
        return ret;

    memcpy(out, &rep.logrep.log, sizeof(*out));
    out->tag.index    = csp_ntoh16(out->tag.index);
    out->tag.csp_time = csp_ntoh32(out->tag.csp_time);
    out->message[sizeof(out->message) - 1] = 0;
    return GS_OK;
}

gs_error_t ftpnew_clear_log(const gs_ftp_settings_t* settings)
{
    ftp_ext_packet_t req;
    memset(&req, 0, sizeof(req));
    req.type       = FTP_EXT_LOG_REQUEST;
    req.log.action = 2;
    req.log.index  = 0;

    ftp_ext_packet_t rep;
    int req_len = sizeof(req.type) + sizeof(req.log);
    int rep_len = sizeof(rep.type) + sizeof(req.logrep.ret) + sizeof(req.logrep.count);
    int got = csp_transaction(CSP_PRIO_NORM,
                              setting_host(settings),
                              setting_csp_port(settings),
                              setting_timeout(settings),
                              &req, req_len,
                              &rep, rep_len);
    if (got != rep_len)
        return GS_ERROR_IO;
    if (rep.type != FTP_EXT_LOG_REPLY)
        return GS_ERROR_DATA;
    return GS_OK;
}

gs_error_t ftpnew_shell_cmd(const gs_ftp_settings_t* settings,
                            const char* cmd,
                            int* out_sysret)
{
    if (!cmd)
        return GS_ERROR_ARG;

    ftp_ext_packet_t req;
    memset(&req, 0, sizeof(req));
    req.type = FTP_EXT_SHELL_CMD;
    strncpy(req.shell.cmd, cmd, sizeof(req.shell.cmd) - 1);
    req.shell.cmd[sizeof(req.shell.cmd) - 1] = 0;

    ftp_ext_packet_t rep;
    int req_len = sizeof(req.type) + sizeof(req.shell);
    int rep_len = sizeof(rep.type) + sizeof(rep.shellrep);
    int got = csp_transaction(CSP_PRIO_NORM,
                              setting_host(settings),
                              setting_csp_port(settings),
                              setting_timeout(settings),
                              &req, req_len,
                              &rep, rep_len);
    if (got != rep_len)
        return GS_ERROR_IO;
    if (rep.type != FTP_EXT_SHELL_REPLY)
        return GS_ERROR_DATA;

    /* Server byte-swaps both ret and sysret on the way out; undo here. */
    int32_t ret_field    = (int32_t)csp_ntoh32((uint32_t)rep.shellrep.ret);
    int32_t sysret_field = (int32_t)csp_ntoh32((uint32_t)rep.shellrep.sysret);

    if (ret_field != FTP_OK)
        return wire_ret_to_gs(ret_field);

    if (out_sysret)
        *out_sysret = sysret_field;
    return GS_OK;
}

gs_error_t ftpnew_ram_read(const gs_ftp_settings_t* settings,
                           uint32_t address,
                           uint32_t size,
                           void* out_buf)
{
    if (!out_buf)
        return GS_ERROR_ARG;
    if (size == 0 || size > FTP_EXT_RAM_MAX_BYTES)
        return GS_ERROR_ARG;

    ftp_ext_packet_t req;
    memset(&req, 0, sizeof(req));
    req.type = FTP_EXT_RAM_READ;
    req.read.addr = csp_hton32(address);
    req.read.size = csp_hton32(size);

    ftp_ext_packet_t rep;
    int req_len = sizeof(req.type) + sizeof(req.read);
    /* Server sends type + exactly `size` data bytes (no length echoed). */
    int rep_len = sizeof(rep.type) + (int)size;
    int got = csp_transaction(CSP_PRIO_NORM,
                              setting_host(settings),
                              setting_csp_port(settings),
                              setting_timeout(settings),
                              &req, req_len,
                              &rep, rep_len);
    if (got != rep_len)
        return GS_ERROR_IO;
    if (rep.type != FTP_EXT_RAM_READ_REPLY)
        return GS_ERROR_DATA;

    memcpy(out_buf, rep.readrep.bytes, size);
    return GS_OK;
}

gs_error_t ftpnew_ram_write(const gs_ftp_settings_t* settings,
                            uint32_t address,
                            uint32_t size,
                            const void* data)
{
    if (!data)
        return GS_ERROR_ARG;
    if (size == 0 || size > sizeof(((ftp_ext_ram_write_request_t*)0)->bytes))
        return GS_ERROR_ARG;

    ftp_ext_packet_t req;
    memset(&req, 0, sizeof(req));
    req.type = FTP_EXT_RAM_WRITE;
    req.write.addr = csp_hton32(address);
    req.write.size = csp_hton32(size);
    memcpy(req.write.bytes, data, size);

    ftp_ext_packet_t rep;
    /* only send the bytes we're actually writing */
    int req_len = sizeof(req.type)
                + sizeof(req.write.addr)
                + sizeof(req.write.size)
                + (int)size;
    int rep_len = sizeof(rep.type) + sizeof(rep.writerep);
    int got = csp_transaction(CSP_PRIO_NORM,
                              setting_host(settings),
                              setting_csp_port(settings),
                              setting_timeout(settings),
                              &req, req_len,
                              &rep, rep_len);
    if (got != rep_len)
        return GS_ERROR_IO;
    if (rep.type != FTP_EXT_RAM_WRITE_REPLY)
        return GS_ERROR_DATA;

    int32_t ret_field = (int32_t)csp_ntoh32((uint32_t)rep.writerep.ret);
    if (ret_field != FTP_OK)
        return wire_ret_to_gs(ret_field);
    return GS_OK;
}

gs_error_t ftpnew_kill(const gs_ftp_settings_t* settings,
                       uint32_t magic,
                       int status)
{
    ftp_ext_packet_t req;
    memset(&req, 0, sizeof(req));
    req.type = FTP_EXT_KILL;
    req.kill.magic = csp_hton32(magic);
    req.kill.status = csp_hton32((uint32_t)status);

    ftp_ext_packet_t rep;
    int req_len = sizeof(req.type) + sizeof(req.kill);
    int got = csp_transaction(CSP_PRIO_NORM,
                              setting_host(settings),
                              setting_csp_port(settings),
                              setting_timeout(settings),
                              &req, req_len,
                              &rep, -1);
    if (got <= 0)
        return GS_OK; /* no reply expected if successful */
    if (rep.type != FTP_EXT_KILL_REPLY)
        return GS_ERROR_DATA;

    return GS_ERROR_UNKNOWN;
}
