/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */

#define GS_FTP_INTERNAL_USE 1

#include <gs/ftp/client.h>
#include <gs/ftp/internal/types.h>

#include <unistd.h>
#include <sys/stat.h>

#include <gs/csp/csp.h>
#include <csp/csp_endian.h>
#include <csp/switch.h>
#include <csp/delay.h>

#include <gs/util/crc32.h>
#include <gs/util/log.h>
#include <gs/util/string.h>
#include <gs/util/check.h>
#define DEFAULT_TIMEOUT     30000
#define DEFAULT_CHUNK_SIZE  185

/* Chunk status markers */
static const char * const packet_missing = "-";
static const char * const packet_ok = "+";
bool ftpavailable;
static uint8_t destination;

typedef struct {
    FILE       * fp;
    FILE       * fp_map;
    csp_conn_t * conn;
    uint32_t   timeout;
    char       file_name[GS_FTP_PATH_LENGTH];
    uint32_t   file_size;
    uint32_t   chunks;
    int        chunk_size;
    uint32_t   checksum;
    ftp_status_element_t last_status[GS_FTP_STATUS_CHUNKS];
    uint32_t   last_entries;
    gs_ftp_info_callback_t info_callback;
    void       * info_data;
} gs_ftp_state_t;

typedef struct {
    gs_ftp_backend_type_t backend;
    const char * path;
    uint32_t addr;
    uint32_t size;
} gs_ftp_url_t;

/**
   FTP zip action codes
*/

gs_ftp_state_t statef = {.fp = NULL,};

static gs_error_t ftp_error_to_gs_error(gs_ftp_return_t ret)
{
    switch (ret) {
        case GS_FTP_RET_OK:
            return GS_OK;
        case GS_FTP_RET_NOENT:
            return GS_ERROR_NOT_FOUND;
        case GS_FTP_RET_INVAL:
            return GS_ERROR_ARG;
        case GS_FTP_RET_NOSPC:
            return GS_ERROR_FULL;
        case GS_FTP_RET_IO:
            return GS_ERROR_IO;
        case GS_FTP_RET_FBIG:
            return GS_ERROR_OVERFLOW;
        case GS_FTP_RET_EXISTS:
            return GS_ERROR_EXIST;
        case GS_FTP_RET_NOTSUP:
            return GS_ERROR_NOT_SUPPORTED;
        case GS_FTP_RET_BUSY:
            return GS_ERROR_BUSY;
        case GS_FTP_RET_NOMEM:
            return GS_ERROR_ALLOC;
    }
    return GS_ERROR_UNKNOWN;
}


static int ftp_file_crc32()
{
    size_t bytes;
    char buf[128];
    uint32_t crc = gs_crc32_init();

    fseek(statef.fp, 0, SEEK_SET);
    do {
        bytes = fread(buf, 1, sizeof(buf), statef.fp);
        crc = gs_crc32_update(crc, buf, bytes);
    } while (bytes > 0);

    statef.checksum = crc = gs_crc32_finalize(crc);

    return 0;
}

/* parse URL's of the form:
   -file://<path_to_file>
   -mem://<addr>[++<length>]
   -<path_to_file>
*/
static gs_error_t ftp_parse_url(const char * url, gs_ftp_url_t * parsed_url)
{
    memset(parsed_url, 0, sizeof(*parsed_url));

    if (strstr(url, "mem://")) {
        char temp_url[strlen(url)+1];
        strcpy(temp_url, url);
        char * saveptr;
        char * token = strtok_r(temp_url + strlen("mem://"), "++", &saveptr);
        if (gs_string_empty(token) || gs_string_to_uint32(token, &parsed_url->addr)) {
            return GS_ERROR_ARG;
        }
        token = strtok_r(NULL, "++\r\n", &saveptr);
        if (token) {
            if (gs_string_to_uint32(token, &parsed_url->size)) {
                return GS_ERROR_ARG;
            }
        }
        parsed_url->backend = GS_FTP_BACKEND_RAM;
        parsed_url->path = "";
        return GS_OK;
    }
    parsed_url->backend = GS_FTP_BACKEND_FILE;
    if (strstr(url, "file://")) {
        parsed_url->path = url + strlen("file://");
    } else {
        parsed_url->path = url;
    }
    return GS_OK;
}

static gs_ftp_backend_type_t gs_ftp_get_backend(const gs_ftp_url_t * url, const gs_ftp_settings_t * settings)
{
    if (url) {
        switch (url->backend) {
            case GS_FTP_BACKEND_RAM:
            case GS_FTP_BACKEND_FAT:
            case GS_FTP_BACKEND_UFFS:
                return url->backend;
            case GS_FTP_BACKEND_FILE:
                break;
        }
    }
    if (settings) {
        switch (settings->mode) {
            case GS_FTP_MODE_GATOSS:
                return GS_FTP_BACKEND_UFFS;
            case GS_FTP_MODE_STANDARD:
                break;
        }
    }
    return GS_FTP_BACKEND_FILE;
}

uint32_t gs_ftp_get_timeout_force(const gs_ftp_settings_t * settings)
{
    return (settings && settings->timeout) ? settings->timeout : DEFAULT_TIMEOUT;
}
    
uint32_t gs_ftp_get_chunk_size_force(const gs_ftp_settings_t * settings)
{
    return (settings && settings->chunk_size) ? settings->chunk_size : DEFAULT_CHUNK_SIZE;
}

uint8_t gs_ftp_get_csp_port_force(const gs_ftp_settings_t * settings)
{
    if (settings) {
        if (settings->port) {
            return settings->port;
        }
        switch (settings->mode) {
            case GS_FTP_MODE_GATOSS:
                return GS_FTP_CSP_PORT_GATOSS;
            case GS_FTP_MODE_STANDARD:
                break;
        }
    }

    return GS_CSP_PORT_FTP;
}

gs_error_t gs_conn_create_force(const gs_ftp_settings_t * settings)
{
    csp_log_protocol("FTP: Create Connection.");
    if(statef.conn != NULL)
    {
        csp_log_error("FTP: There is connection already.");
        return GS_ERROR_ALLOC;
    }
    statef.conn = csp_connect(CSP_PRIO_NORM, settings->host, gs_ftp_get_csp_port_force(settings), gs_ftp_get_timeout_force(settings),
                             CSP_O_RDP | CSP_O_CRC32);
    if (statef.conn == NULL) {
        return GS_ERROR_IO;
    }

    return GS_OK;
}

gs_error_t gs_ftp_upload_force_request(const gs_ftp_settings_t * settings, const char * local_url, const char * remote_url,
                         gs_ftp_info_callback_t info_callback, void * info_data)
{
    csp_log_protocol("FTP: Start FTP Uplink");
    gs_ftp_url_t local_url_info;
    if (ftp_parse_url(local_url, &local_url_info)) {
        log_error("%s: Invalid local URL: [%s]", __FUNCTION__, local_url);
        return GS_ERROR_ARG;
    }
    if (local_url_info.backend != GS_FTP_BACKEND_FILE) {
        log_error("%s: Not supported local URL: [%s]", __FUNCTION__, local_url);
        return GS_ERROR_NOT_SUPPORTED;
    }

    gs_ftp_url_t remote_url_info;
    if (ftp_parse_url(remote_url, &remote_url_info)) {
        log_error("%s: Invalid remote URL: [%s]", __FUNCTION__, remote_url);
        return GS_ERROR_ARG;
    }

    /* Open file and read size */
    
    statef.fp = fopen(local_url_info.path, "rb");
    if (statef.fp == NULL) {
        return GS_ERROR_NOT_FOUND;
    }
    struct stat statbuf;
    stat(local_url_info.path, &statbuf);
    /* Calculate CRC32 */
    ftp_file_crc32(&statef);

    statef.timeout    = gs_ftp_get_timeout_force(settings);
    statef.chunk_size = gs_ftp_get_chunk_size_force(settings);
    statef.file_size = (uint32_t) statbuf.st_size;
    statef.chunks = (statef.file_size + statef.chunk_size - 1) / statef.chunk_size;
    GS_STRNCPY(statef.file_name, local_url_info.path);
    statef.info_callback = info_callback;
    statef.info_data     = info_data;
    
    csp_log_protocol("FTP: Confirmed File URLs.");

    /* Assemble upload request */
    ftp_packet_t req;
    req.type = FTP_UPLOAD_REQUEST;
    req.up.chunk_size = csp_hton16(statef.chunk_size);
    req.up.size = csp_hton32(statef.file_size);
    req.up.crc32 = csp_hton32(statef.checksum);
    req.up.mem_addr = csp_hton32(remote_url_info.addr);
    req.up.backend = gs_ftp_get_backend(&remote_url_info, settings);
    GS_STRNCPY(req.up.path, remote_url_info.path);
    int req_length = sizeof(req.type) + sizeof(req.up);

    
    if (statef.conn == NULL) {
        return GS_ERROR_IO;
    }
    else
    {
        destination = settings->host;
    }
    csp_log_protocol("FTP: Connection Created. Send File Request");
    ftp_packet_t rep;
    int rep_length = sizeof(rep.type) + sizeof(rep.uprep);
    int res_length = 0;
    if (res_length = csp_transaction_persistent(statef.conn, gs_ftp_get_timeout_force(settings) + rx_delay_ms(rep_length, 3), &req, req_length, &rep, rep_length) != rep_length) {
        log_error("Length mismatch. Expected %d, got %d", rep_length, res_length);
        return GS_ERROR_TIMEOUT;
    }

    if (rep.type != FTP_UPLOAD_REPLY || rep.uprep.ret != GS_FTP_RET_OK) {
        sleep(gs_ftp_get_timeout_force(settings) / 1000);
        sleep(1);
        return ftp_error_to_gs_error(rep.uprep.ret);
    }

    if (info_callback) {
        gs_ftp_info_t info = {
            .user_data = info_data,
            .type      = GS_FTP_INFO_UL_FILE,
            .u.file = {
                .size = statef.file_size,
                .crc  = statef.checksum,
            },
        };
        info_callback(&info);
    }

    return GS_OK;
}

gs_error_t gs_ftp_upload_force(const gs_ftp_settings_t * settings, const char * local_url, const char * remote_url,
                         gs_ftp_info_callback_t info_callback, void * info_data, int iteration)
{
    gs_error_t status = 0;

    for (int i = 0; i < iteration; i++)
    {
        if (statef.conn == NULL)
        {
            status = gs_conn_create_force(settings);
        }
        else
        {
            csp_rdp_flush_all(&statef.conn);
            csp_log_protocol("RDP: Flush Duplicated Connections.");
            status = GS_ERROR_CONNECTION_RESET;
            i--;
        }
        if(status == GS_OK)
            break;
    }

    
    if(status && statef.conn != NULL)
    {
        ftp_done_force(0);

        ftp_close_force();
        return status;
    }
    else if (status)
        return status;

    status = -1;

    for (int i = 0; i < iteration; i++)
    {
        if(status != 0)
            status = gs_ftp_upload_force_request(settings, local_url, remote_url, info_callback, info_data);
        else
            break;
    }
    
    if(status)
    {
        ftp_done_force(0);
        ftp_close_force();
        return status;
    }

    status = -1;

    csp_log_protocol("FTP: Try FTP Upload.");
    /* Handle data stage */
    for (int i = 0; i < iteration; i++)
    {
        status = ftp_status_request(&statef);

        if (status)
            sleep(gs_ftp_get_timeout_force(settings) / 1000);
        else
            break;

        sleep(1.5);
    }

    if(status)
    {
        ftp_done_force(0);
        ftp_close_force();
        return status;
    }
    printf("Start Streaming.\n");
    set_now_streaming(1);
    status = ftp_data(0);

    if (!status)
    {
        ftp_done_force(0);
        ftp_close_force();
        sleep(1);
        return GS_OK;
    }
    else if (status == -100)
    {
        ftp_done_force(0);
        ftp_close_force();
        return status;
    }
    else{
        ftp_done_force(0);
        ftp_close_force();
        sleep(1);
        return status;
    }
}


gs_error_t gs_ftp_download_force(const gs_ftp_settings_t * settings, const char * local_url,  const char * remote_url,
                           gs_ftp_info_callback_t info_callback, void * info_data, int iteration)
{
    gs_error_t status = 0;
    for (int i = 0; i < iteration; i++)
    {
        if (statef.conn == NULL)
        {
            status = gs_conn_create_force(settings);
        }
        else
        {
            csp_rdp_flush_all(&statef.conn);
            csp_log_protocol("RDP: Flush Duplicated Connections.");
            status = GS_ERROR_CONNECTION_RESET;
            i--;
        }
        if(status == GS_OK)
            break;
    }

    if(status && statef.conn != NULL)
    {
        ftp_done_force(0);

        ftp_close_force();
        return status;
    }
    else if (status)
        return status;
    csp_log_protocol("FTP: Start FTP Downlink");
    gs_ftp_url_t local_url_info;
    if (ftp_parse_url(local_url, &local_url_info)) {
        log_error("%s: Invalid local URL: [%s]", __FUNCTION__, local_url);
        return GS_ERROR_ARG;
    }
    if (local_url_info.backend != GS_FTP_BACKEND_FILE) {
        log_error("%s: Not supported local URL: [%s]", __FUNCTION__, local_url);
        return GS_ERROR_NOT_SUPPORTED;
    }

    gs_ftp_url_t remote_url_info;
    if (ftp_parse_url(remote_url, &remote_url_info) ||
        ((remote_url_info.backend == GS_FTP_BACKEND_RAM) && (remote_url_info.size == 0))) {
        log_error("%s: Invalid remote URL: [%s]", __FUNCTION__, remote_url);
        return GS_ERROR_ARG;
    }

    statef.fp = NULL;
    statef.chunk_size = gs_ftp_get_chunk_size_force(settings);
    statef.timeout = gs_ftp_get_timeout_force(settings);
    statef.info_callback = info_callback;
    statef.info_data = info_data;
    GS_STRNCPY(statef.file_name, local_url_info.path);
    csp_log_protocol("FTP: Confirmed File URLs.");

    ftp_packet_t req;
    req.type = FTP_DOWNLOAD_REQUEST;
    req.down.chunk_size = csp_hton16(statef.chunk_size);
    req.down.mem_addr = csp_hton32(remote_url_info.addr);
    req.down.mem_size = csp_hton32(remote_url_info.size);
    req.down.backend = gs_ftp_get_backend(&remote_url_info, settings);
    GS_STRNCPY(req.down.path, remote_url_info.path);
    int req_length = sizeof(req.type) + sizeof(req.down);

    csp_log_protocol("FTP: Connection Created. Send File Request");

    ftp_packet_t rep;
    int rep_length = sizeof(req.type) + sizeof(req.downrep);
    int res_length = csp_transaction_persistent(statef.conn, gs_ftp_get_timeout_force(settings) + rx_delay_ms(rep_length, 3), &req, req_length, &rep, rep_length);
    if (res_length != rep_length) {
        log_error("Length mismatch. Expected %d, got %d", rep_length, res_length);
        sleep(gs_ftp_get_timeout_force(settings) / 1000);
        ftp_done_force(0);
        ftp_close_force();
        return GS_ERROR_TIMEOUT;
    }

    if (rep.type != FTP_DOWNLOAD_REPLY || rep.downrep.ret != GS_FTP_RET_OK) {
        sleep(gs_ftp_get_timeout_force(settings) / 1000);
        ftp_done_force(0);
        ftp_close_force();
        return ftp_error_to_gs_error(rep.downrep.ret);
    }
    statef.file_size = csp_ntoh32(rep.downrep.size);
    statef.checksum = csp_ntoh32(rep.downrep.crc32);
    statef.chunks = (statef.file_size + statef.chunk_size - 1) / statef.chunk_size;
    if (info_callback) {
        gs_ftp_info_t info = {
            .user_data = info_data,
            .type      = GS_FTP_INFO_DL_FILE,
            .u.file = {
                .size = statef.file_size,
                .crc  = statef.checksum,
            },
        };
        info_callback(&info);
    }

    /* Map file name */
    char map[100];

    /* Try to open file */
    bool new_file = false;
    statef.fp = fopen(local_url_info.path, "r+");
    if (statef.fp == NULL) {
        /* Create new file */
        new_file = true;
        statef.fp = fopen(local_url_info.path, "w+");
        if (statef.fp == NULL) {
            log_error("Client: Failed to create data file");
            sleep(gs_ftp_get_timeout_force(settings) / 1000);
            ftp_done_force(0);
            ftp_close_force();
            return GS_ERROR_IO;
        }
    }

    /* Try to create a new bitmap */
    sprintf(map, "%s.map", local_url_info.path);

    /* Check if file already exists */
    statef.fp_map = fopen(map, "r+");
    if (statef.fp_map == NULL) {
        unsigned int i;

        /* Abort if data file exists but map doesn't */
        if (!new_file) {
            sleep(gs_ftp_get_timeout_force(settings) / 1000);
            ftp_done_force(0);
            ftp_close_force();
            return GS_ERROR_EXIST;
        }

        /* Create new file */
        statef.fp_map = fopen(map, "w+");
        if (statef.fp_map == NULL) {
            log_error("Failed to create bitmap");
            sleep(gs_ftp_get_timeout_force(settings) / 1000);
            ftp_done_force(0);
            ftp_close_force();
            return GS_ERROR_IO;
        }

        /* Clear contents */
        for (i = 0; i < statef.chunks; i++) {
            if (fwrite(packet_missing, 1, 1, statef.fp_map) < 1) {
                log_error("Failed to clear bitmap");
                sleep(gs_ftp_get_timeout_force(settings) / 1000);
                ftp_done_force(0);
                ftp_close_force();
                return GS_ERROR_IO;
            }
        }

        fflush(statef.fp_map);
        fsync(fileno(statef.fp_map));
    }

    /* Handle data stage */
    sleep(1);
    csp_log_protocol("FTP: Start FTP Download.");
    status = ftp_status_reply();
    csp_log_protocol("FTP: Finish FTP Download. FTP Status : %d", status);
    sleep(1);
    if (status) {
        sleep(gs_ftp_get_timeout_force(settings) / 1000);
        ftp_done_force(0);
        ftp_close_force();
        sleep(1);
        return GS_ERROR_UNKNOWN;
    }
    

    status = ftp_crc();
    sleep(1);
    if (status) {
        sleep(gs_ftp_get_timeout_force(settings) / 1000);
        ftp_done_force(0);
        ftp_close_force();
        sleep(1);
        return status;
    }
    csp_log_protocol("FTP: CRC Checking : %d", status);
    sleep(gs_ftp_get_timeout_force(settings) / 1000);
    ftp_done_force(1);
    ftp_close_force();
    sleep(1);
    return GS_OK;
}

static gs_error_t ftp_status_request()
{
    /* Request */
    ftp_packet_t req;
    req.type = FTP_STATUS_REQUEST;
    int req_length = sizeof(req.type);
    memset(statef.last_status, 0, sizeof(ftp_status_element_t) * 10);

    /* Transaction */
    ftp_packet_t rep;
    csp_log_protocol("FTP: Request Status Reply.");
    if (csp_transaction_persistent(statef.conn, statef.timeout, &req, req_length, &rep, -1) == 0) {
        log_error("Failed to receive status reply");
        return GS_ERROR_IO;
    }
    if (rep.type != FTP_STATUS_REPLY || rep.statusrep.ret != GS_FTP_RET_OK) {
        log_error("Reply was not STATUS_REPLY. rep.type : %d, rep.statusrep.ret : %d", rep.type, rep.statusrep.ret);
        return GS_ERROR_DATA;
    }
    csp_log_protocol("FTP: Received Status Reply.");

    rep.statusrep.complete = csp_ntoh32(rep.statusrep.complete);
    rep.statusrep.total = csp_ntoh32(rep.statusrep.total);
    rep.statusrep.entries = csp_ntoh16(rep.statusrep.entries);

    printf("Start info_callback.\n");
    if (statef.info_callback) {
        gs_ftp_info_t info = {
            .user_data = statef.info_data,
            .type      = GS_FTP_INFO_UL_COMPLETED,
            .u.completed = {
                .completed_chunks = rep.statusrep.complete,
                .total_chunks     = rep.statusrep.total,
            },
        };
        statef.info_callback(&info);
    }
    csp_log_warn("Received statusrep : entries %u count %u", rep.statusrep.entries, rep.statusrep.entry[0].count);
    if(rep.statusrep.entries == csp_hton16(1))
    {
        csp_log_protocol("FTP: csp_hton error in FSW. Now Filtering...");
        

        rep.statusrep.entries = csp_ntoh16(rep.statusrep.entries);
        rep.statusrep.entry[0].count = csp_ntoh32(rep.statusrep.entry[0].count);
    }

    
    if (rep.statusrep.complete != rep.statusrep.total) {
        int i;
        for (i = 0; i < rep.statusrep.entries; i++) {
            printf("Input method %d\n", i);
            rep.statusrep.entry[i].next = csp_ntoh32(rep.statusrep.entry[i].next);
            rep.statusrep.entry[i].count = csp_ntoh32(rep.statusrep.entry[i].count);
            statef.last_status[i].next = rep.statusrep.entry[i].next;
            statef.last_status[i].count = rep.statusrep.entry[i].count;
        }

        statef.last_entries = rep.statusrep.entries;
    } else {
        statef.last_entries = 0;
    }
    csp_log_warn("Now statusrep      : entries %u count %u", rep.statusrep.entries, rep.statusrep.entry[0].count);
    return GS_OK;
}


static gs_error_t ftp_status_reply()
{
    ftp_packet_t ftp_packet_req = { .type = FTP_STATUS_REPLY};
    ftp_status_reply_t * status = (ftp_status_reply_t *) &ftp_packet_req.statusrep;

    /* Build status reply */
    unsigned int i = 0, next = 0, count = 0;

    status->entries = 0;
    status->complete = 0;
    for (i = 0; i < statef.chunks; i++) {
        int s;
        char cstat;

        /* Read chunk status */
        if ((unsigned int) ftell(statef.fp_map) != i) {
            if (fseek(statef.fp_map, i, SEEK_SET) != 0) {
                log_error("fseek failed");
                return GS_ERROR_IO;
            }
        }
        if (fread(&cstat, 1, 1, statef.fp_map) != 1) {
            log_error("fread byte %u failed", i);
            return GS_ERROR_IO;
        }

        s = (cstat == *packet_ok);

        /* Increase complete counter if chunk was received */
        if (s) status->complete++;

        /* Add chunk status to packet */
        if (status->entries < GS_FTP_STATUS_CHUNKS) {
            if (!s) {
                if (!count)
                    next = i;
                count++;
            }

            if (count > 0 && (s || i == statef.chunks - 1)) {
                status->entry[status->entries].next = csp_hton32(next);
                status->entry[status->entries].count = csp_hton32(count);
                status->entries++;
            }
        }
    }

    status->entries = csp_hton16(status->entries);
    // status->entries = csp_hton16(0xFFFF);
    status->complete = csp_hton32(status->complete);
    status->total = csp_hton32(statef.chunks);
    status->ret = GS_FTP_RET_OK;

    /* Send reply */
    if (csp_transaction_persistent(statef.conn, statef.timeout,
                                   &ftp_packet_req, sizeof(ftp_packet_req.type) + sizeof(ftp_packet_req.statusrep), NULL, 0) != 1) {
        log_error("Failed to send status reply");
        return GS_ERROR_IO;
    }

    if (statef.info_callback) {
        gs_ftp_info_t info = {
            .user_data = statef.info_data,
            .type      = GS_FTP_INFO_DL_COMPLETED,
            .u.completed = {
                .completed_chunks = status->complete,
                .total_chunks     = statef.chunks,
            },
        };
        statef.info_callback(&info);
    }

    /* File size is 0 */
    if (statef.chunks == 0) {
        return GS_OK;
    }

    /* Read data */
    csp_packet_t * packet;
    while (1) {
        if (!ftpavailable)
            return GS_ERROR_BREAK;
        packet = csp_read(statef.conn, statef.timeout);

        if (!packet) {
            log_error("Timeout while waiting for data");
            return GS_ERROR_TIMEOUT;
        }

        ftp_packet_t * ftp_packet = (ftp_packet_t *) &packet->data;

        ftp_packet->data.chunk = csp_ntoh32(ftp_packet->data.chunk);
        unsigned int size;

        if (ftp_packet->type == GS_FTP_RET_IO) {
            log_error("Server failed to read chunk");
            csp_buffer_free(packet);
            return GS_ERROR_DATA;
        }

        if (ftp_packet->data.chunk >= statef.chunks) {
            log_error("Bad chunk number %" PRIu32 " > %" PRIu32, ftp_packet->data.chunk, statef.chunks);
            csp_buffer_free(packet);
            continue;
        }

        if (ftp_packet->data.chunk == statef.chunks - 1) {
            size = statef.file_size % statef.chunk_size;
            if (size == 0)
                size = statef.chunk_size;
        } else {
            size = statef.chunk_size;
        }

        if ((unsigned int) ftell(statef.fp) != ftp_packet->data.chunk * statef.chunk_size) {
            if (fseek(statef.fp, ftp_packet->data.chunk * statef.chunk_size, SEEK_SET) != 0) {
                log_error("Seek error");
                csp_buffer_free(packet);
                return GS_ERROR_ACCESS;
            }
        }

        if (fwrite(ftp_packet->data.bytes, 1, size, statef.fp) != size) {
            log_error("Write error");
            csp_buffer_free(packet);
            return GS_ERROR_IO;
        }
        fflush(statef.fp);

        if ((unsigned int) ftell(statef.fp_map) != ftp_packet->data.chunk) {
            if (fseek(statef.fp_map, ftp_packet->data.chunk, SEEK_SET) != 0) {
                log_error("Map Seek error");
                csp_buffer_free(packet);
                return GS_ERROR_IO;
            }
        }

        if (fwrite(packet_ok, 1, 1, statef.fp_map) != 1) {
            log_error("Map write error");
            csp_buffer_free(packet);
            return GS_ERROR_IO;
        }
        fflush(statef.fp_map);

        /* Show progress bar */
        if (statef.info_callback) {
            gs_ftp_info_t info = {
                .user_data = statef.info_data,
                .type      = GS_FTP_INFO_DL_PROGRESS,
                .u.progress = {
                    .current_chunk = ftp_packet->data.chunk,
                    .total_chunks  = statef.chunks,
                    .chunk_size    = statef.chunk_size,
                },
            };
            statef.info_callback(&info);
        }

        /* Free buffer element */
        csp_buffer_free(packet);

        /* Break if all packets were received */
        if (ftp_packet->data.chunk == statef.chunks - 1)
            break;
    }

    /* Sync file to disk */
    fflush(statef.fp);
    fsync(fileno(statef.fp));


    fflush(statef.fp_map);
    fsync(fileno(statef.fp_map));

    return 0;
}

int ftp_abort()
{
    ftpavailable = false;
}

int ftp_avail()
{
    ftpavailable = true;
}

static gs_error_t ftp_data(int count)
{
    unsigned int i, j;
    int ret;
    unsigned int destination = 0;

    // uint16_t destination = statef.conn->idout.dst;
    csp_log_protocol("FTP: Start Data Uplink. last_entries : %u", statef.last_entries);

    /* Request */
    ftp_packet_t streamdata;
    streamdata.type = FTP_DATA;
    ftpavailable = true;
    switch_to_tx(destination);
    usleep(switch_delay(destination));
    printf("chunk size : %d\n", statef.chunk_size);

    for (i = 0; i < statef.last_entries; i++) {
        ftp_status_element_t * n = &statef.last_status[i];
        if(!statef.conn)
            return GS_ERROR_CONNECTION_RESET;
        

        for (j = 0; j < n->count; j++) {
            if(!get_now_streaming())
                set_now_streaming(1);
            if((!ftpavailable) || j == 10)
            {
                switch_to_rx(destination);
                usleep(switch_delay(destination));
                return GS_ERROR_BREAK;
            }
            csp_log_protocol("FTP: Iteration : %u %u", i, j);
            /* Calculate chunk number */
            streamdata.data.chunk = n->next + j;

            /* Print progress bar */
            if (statef.info_callback) {
                gs_ftp_info_t info = {
                    .user_data = statef.info_data,
                    .type      = GS_FTP_INFO_UL_PROGRESS,
                    .u.progress = {
                        .current_chunk = streamdata.data.chunk,
                        .total_chunks  = statef.chunks,
                        .chunk_size    = statef.chunk_size,
                    },
                };
                statef.info_callback(&info);
            }

            if(statef.fp == NULL)
                return GS_ERROR_NOT_FOUND;

            /* Read chunk */
            if ((unsigned int) ftell(statef.fp) != streamdata.data.chunk * statef.chunk_size)
                fseek(statef.fp, streamdata.data.chunk * statef.chunk_size, SEEK_SET);
            ret = fread(streamdata.data.bytes, 1, statef.chunk_size, statef.fp);

            if (ret < 0) {
                if (!feof(statef.fp))
                    break;
            }

            // if read is short of 'chunck', clear remaining as full 'chunk' is always sent
            if (ret < statef.chunk_size) {
                memset(&streamdata.data.bytes[ret], 0, statef.chunk_size - ret);
            }

            /* Chunk number MUST be little-endian!
             * Note: Yes, this is due to an old mistake, and now we are stuck with it! :( */
            streamdata.data.chunk = csp_htole32(streamdata.data.chunk);

            /* Send data */
            int length = sizeof(streamdata.type) + sizeof(uint32_t) + statef.chunk_size;
            csp_packet_t * packet = csp_buffer_get(length);
            if (packet == NULL)
            {
                csp_log_error("Fatal : Cannot Create Packet.");;
                return GS_ERROR_DATA;
            }

            /* Copy the request */
            if (length > 0 && &packet != NULL)
                memcpy(packet->data, &streamdata, length);
            packet->length = length;
            printf("packet length : %u\n", packet->length);

            if (!csp_send_streaming(statef.conn, packet, statef.timeout)) {
                log_error("Data transaction failed");
                return GS_ERROR_BUSY;
                break;
            }
            usleep(100000);
        }
        
    }
    csp_log_info("FTP Temporary Patch : Switch to RX in Force.");
    
    switch_to_rx(destination);
    usleep(switch_delay(destination));

    return GS_OK;

}

void ftp_done_force(unsigned int remove_map)
{
    /* Close file and map */
    set_now_streaming(2);
    if (statef.fp_map) {
        fclose(statef.fp_map);
        statef.fp_map = NULL;
    }
    if (statef.fp) {
        fclose(statef.fp);
        statef.fp = NULL;
    }

    /* Delete map file if it exits */
    if (remove_map) {
        char map[GS_FTP_PATH_LENGTH+4];
        snprintf(map, sizeof(map), "%s.map", statef.file_name);
        int status = remove(map);
        if (status != 0) {
            log_error("Failed to remove %s %d", map, status);
        }
    }
    csp_log_protocol("FTP: FTP Done Completed.");
}

void ftp_close_force()
{
    /* Request */
    ftp_packet_t packet;
    packet.type = FTP_DONE;

    if(statef.conn != NULL)
    {
        set_now_streaming(0);
        csp_transaction_persistent(statef.conn, statef.timeout, &packet, sizeof(packet.type), NULL, 0);
        (2);
        csp_log_info("FTP Temporary Patch : Switch to RX in Force.");
        switch_to_rx(destination);
        for(int iter = 0; iter < 8; iter++)
        {
            if(csp_conn_now_closed(statef.conn))
                break;

            sleep(1);
            if(iter == 7)
            {
                set_now_streaming(0);
                csp_close(statef.conn);
            }
        }
        statef.conn = NULL;
    }
    else
    {
        csp_log_protocol("FTP: Connection is already free.");
        sleep(8);

    }

    set_now_streaming(0);
    switch_to_rx(destination);
    if(statef.conn)
    {
        csp_close(statef.conn);
    }

    memset(&statef, 0, sizeof(gs_ftp_state_t));
    csp_log_protocol("FTP: FTP Close Completed.");
    switch_to_rx(destination);
    
}

static gs_error_t ftp_crc()
{
    /* Request */
    ftp_packet_t packet;
    packet.type = FTP_CRC_REQUEST;

    /* Reply */
    int repsiz = sizeof(packet.type) + sizeof(packet.crcrep);
    if (csp_transaction_persistent(statef.conn, statef.timeout, &packet, sizeof(packet.type), &packet, repsiz) != repsiz) {
        return GS_ERROR_IO;
    }
    if (packet.type != FTP_CRC_REPLY) {
        return GS_ERROR_DATA;
    }
    if (packet.crcrep.ret != GS_FTP_RET_OK) {
        return ftp_error_to_gs_error(packet.crcrep.ret);
    }

    /* Calculate CRC32 */
    ftp_file_crc32(&statef);

    /* Validate checksum */
    packet.crcrep.crc = csp_ntoh32(packet.crcrep.crc);
    if (statef.info_callback) {
        gs_ftp_info_t info = {
            .user_data = statef.info_data,
            .type      = GS_FTP_INFO_CRC,
            .u.crc = {
                .remote = packet.crcrep.crc,
                .local  = statef.checksum,
            },
        };
        statef.info_callback(&info);
    }
    if (packet.crcrep.crc != statef.checksum) {
        return GS_ERROR_DATA;
    }

    return GS_OK;

}

#define FTP_MODE_STANDARD "standard"
#define FTP_MODE_GATOSS   "gatoss"