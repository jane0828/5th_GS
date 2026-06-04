/*******************************************************************************************
 * @file ftp_tools.h
 * 
 * @author Han-Gyeol Ryu (morimn21@gmail.com)
 * 
 * @brief Miscellaneous tools for debugging purposes.
 * 
 * @version 1.0
 * 
 * @date 2022-03-22
 * 
 * @copyright Copyright (c) 2021 Astrodynamics & Control Lab. Yonsei Univ.
 * 
 ******************************************************************************************/
#include "io_crc32.h"
#include "ftp_tools.h"
#include "io_csp_ftp_typedefs.h"

#include <csp/csp.h>
#include <gs/ftp/client.h>
#include <csp/csp_endian.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>



/**
 *  FOR DEBUGGING ONLY!
 */
enum CSP_NODES {
    CSP_NODE_OBC        = 3,
    CSP_NODE_AX100      = 5,
    CSP_NODE_GS_KISS    = 8,
    CSP_NODE_BBB        = 10,
    CSP_NODE_GS100      = 20,
};
enum CSP_PORTS {
    CSP_PORT_DOWNLINK       = 12,
    CSP_PORT_UPLINK         = 13,
    CSP_PORT_FTP            = 15,
    CSP_PORT_FTP_BBB        = 17,
    CSP_PORT_BBB            = 18,
    CSP_PORT_BEACON         = 31,
    CSP_PORT_RPT            = 25,
    CSP_PORT_AX100_RESET_GNDWDT= 9,
};
/**
 *  FOR DEBUGGING ONLY!
 */




#define IO_CSP_FTP_READ_DEFAULT_TIMEOUT 60000


/* Chunk status markers */
static const char * const packet_missing = "-";
static const char * const packet_ok = "+";



/**
 * @brief FTP transfer global spec.
 */
static io_csp_ftp_transfer_spec tspec;


/* Entry point. */
int32 io_csp_ftp_init(void);
static void* io_csp_ftp_server_task(void*);


/* GS FTP packet handlers. */
static io_csp_ftp_ret ftp_handler_download_request(ftp_download_request_t* request);
static io_csp_ftp_ret ftp_handler_upload_request(ftp_upload_request_t* request);
static io_csp_ftp_ret ftp_handler_data(ftp_data_t* data);
static io_csp_ftp_ret ftp_handler_status_request(void);
static io_csp_ftp_ret ftp_handler_status_reply(ftp_status_reply_t* status);
static io_csp_ftp_ret ftp_handler_crc_request(void);
static io_csp_ftp_ret ftp_handler_abort(void);
static io_csp_ftp_ret ftp_handler_done(void);


/* ! NOT SUPPORTED ! */
// static io_csp_ftp_ret ftp_handler_list_request(void);
// static io_csp_ftp_ret ftp_handler_move_request(void);
// static io_csp_ftp_ret ftp_handler_copy_request(void);
// static io_csp_ftp_ret ftp_handler_remove_request(void);
// static io_csp_ftp_ret ftp_handler_mkdir_request(void);
// static io_csp_ftp_ret ftp_handler_rmdir_request(void);
// static io_csp_ftp_ret ftp_handler_zip_request(void);
// static io_csp_ftp_ret ftp_handler_mkfs_request(void);
/* * * * * * * * * * */


/* Custom handlers. */
static io_csp_ftp_ret ftp_handler_restart_server_task(void);
static io_csp_ftp_ret ftp_handler_set_timeout(uint32 timeout);
static io_csp_ftp_ret ftp_handler_filespec_cleanup(void);
// static io_csp_ftp_ret ftp_handler_set_


/* Static helpers. */
static gs_ftp_return_t ftp_helper_init_upload(void);
static gs_ftp_return_t ftp_helper_read_upload_status(ftp_status_reply_t* status);
static gs_ftp_return_t ftp_helper_read_chunk(void* data, const io_csp_ftp_filespec* filespec, unsigned chunk, unsigned chunk_size);
static gs_ftp_return_t ftp_helper_write_chunk(const void* data, const io_csp_ftp_filespec* filespec, unsigned chunk, unsigned chunk_size);
static gs_ftp_return_t ftp_helper_filespec_cleanup(void);
static gs_ftp_return_t ftp_helper_conn_timeout(void);


static io_csp_ftp_ret ftp_handler_download_request(ftp_download_request_t* request) {

    csp_packet_t* reply;
    ftp_download_reply_t* downrep;
    size_t downrep_len = sizeof(gs_ftp_type_t) + sizeof(*downrep);

    if (tspec.transfer_status != IO_CSP_FTP_IDLE) {
        return IO_CSP_FTP_RET_ERR_STATUS;
    }

    /* Get buffer for reply packet. */
    if (!(reply = csp_buffer_get(downrep_len))) {
        return IO_CSP_FTP_RET_ERR_NOBUF;
    }
    reply->length = downrep_len;
    ((ftp_packet_t*) reply)->type = FTP_DOWNLOAD_REPLY;
    downrep = (ftp_download_reply_t*) reply->data;

    /* Start download sequence. */
    tspec.transfer_status = IO_CSP_FTP_DOWNLOAD;

    /* Store request info to the global spec. */
    request->chunk_size = csp_ntoh16(request->chunk_size);
    request->mem_addr = csp_ntoh32(request->mem_addr);
    request->mem_size = csp_ntoh32(request->mem_size);
    request->path[sizeof(request->path) - 1] = 0;
    memcpy(&tspec.downspec, request, sizeof(tspec.downspec));

    /* Open the target file. */
    if (!(tspec.filespec.fp = fopen(tspec.downspec.path, "r"))) {
        ftp_log_error("unable to open file %s: %s\n", tspec.downspec.path, strerror(errno));
        downrep->ret = GS_FTP_RET_IO;
    }
    else {
        /* Read file size. */
        struct stat statbuf;
        fstat(fileno(tspec.filespec.fp), &statbuf);
        tspec.filespec.size = statbuf.st_size;

        downrep->ret = GS_FTP_RET_OK;
        downrep->size = csp_hton32(tspec.filespec.size);
        downrep->crc32 = csp_hton32(io_crc_get_file_crc32(tspec.filespec.fp));
    }

    if (!csp_send(tspec.conn, reply, tspec.timeout)) {
        ftp_log_error("error sending downrep reply\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_ERR_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}


static io_csp_ftp_ret ftp_handler_upload_request(ftp_upload_request_t* request) {

    csp_packet_t* reply;
    ftp_upload_reply_t* uprep;
    size_t uprep_len = sizeof(gs_ftp_type_t) + sizeof(*uprep);

    if (!request) {
        return IO_CSP_FTP_RET_ERR_NULL;
    }

    if (tspec.transfer_status != IO_CSP_FTP_IDLE) {
        return IO_CSP_FTP_RET_ERR_STATUS;
    }

    /* Get buffer for reply packet. */
    if (!(reply = csp_buffer_get(uprep_len))) {
        return IO_CSP_FTP_RET_ERR_NOBUF;
    }
    reply->length = uprep_len;
    ((ftp_packet_t*) reply)->type = FTP_UPLOAD_REPLY;
    uprep = (ftp_upload_reply_t*) reply->data;

    /* Start upload sequence. */
    tspec.transfer_status = IO_CSP_FTP_UPLOAD;

    /* Store request info to the global spec. */
    request->size = csp_ntoh32(request->size);
    request->crc32 = csp_ntoh32(request->crc32);
    request->chunk_size = csp_ntoh16(request->chunk_size);
    request->mem_addr = csp_ntoh32(request->mem_addr);
    request->path[sizeof(request->path) - 1] = 0;
    memcpy(&tspec.upspec, request, sizeof(tspec.upspec));

    /* Send reply. */
    uprep->ret = ftp_helper_init_upload();
    if (!csp_send(tspec.conn, reply, tspec.timeout)) {
        ftp_log_error("error sending downrep reply\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_ERR_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}



static io_csp_ftp_ret ftp_handler_data(ftp_data_t* data) {

    unsigned datalen;

    if (tspec.transfer_status != IO_CSP_FTP_UPLOAD) {
        return IO_CSP_FTP_RET_ERR_STATUS;
    }
    
    data->chunk = csp_letoh32(data->chunk);
    if (data->chunk >= tspec.filespec.chunks) {
        return IO_CSP_FTP_RET_ERR_RANGE;
    }
    
    datalen = data->chunk == tspec.filespec.chunks - 1 
            ? tspec.filespec.size % tspec.upspec.chunk_size : tspec.upspec.chunk_size;
    if (ftp_helper_write_chunk(data->bytes, &tspec.filespec, data->chunk, tspec.upspec.chunk_size) != GS_FTP_RET_OK) {
        return IO_CSP_FTP_RET_ERR_CHUNK_WRITE;
    }

    return IO_CSP_FTP_RET_OK;

}




static io_csp_ftp_ret ftp_handler_status_request(void) {
    
    csp_packet_t* reply;
    ftp_status_reply_t* statusrep;
    size_t statusrep_len = sizeof(gs_ftp_type_t) + sizeof(*statusrep);

    if (tspec.transfer_status != IO_CSP_FTP_UPLOAD) {
        return IO_CSP_FTP_RET_ERR_STATUS;
    }

    /* Get buffer for reply packet. */
    if (!(reply = csp_buffer_get(statusrep_len))) {
        return IO_CSP_FTP_RET_ERR_NOBUF;
    }
    reply->length = statusrep_len;
    ((ftp_packet_t*) reply)->type = FTP_STATUS_REPLY;
    statusrep = (ftp_status_reply_t*) reply->data;

    /* Read file status. */
    memset(statusrep, 0, sizeof(*statusrep));
    statusrep->ret = ftp_helper_read_upload_status(statusrep);

    /* Handle the byte order. */
    statusrep->entries = csp_hton16(statusrep->entries);
    statusrep->total = csp_hton32(statusrep->total);
    statusrep->complete = csp_hton32(statusrep->complete);
    for (int i = 0; i < statusrep->entries; i++) {
        statusrep->entry[i].count = csp_hton32(statusrep->entry[i].count);
        statusrep->entry[i].next = csp_hton32(statusrep->entry[i].next);
    }

    /* Send reply. */
    if (!csp_send(tspec.conn, reply, tspec.timeout)) { // FIXME: timeout 100?
        ftp_log_error("status reply send error\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_ERR_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}



static io_csp_ftp_ret ftp_handler_status_reply(ftp_status_reply_t* status) {

    csp_packet_t* packet;
    ftp_packet_t* data;
    unsigned pktlen;
    unsigned datalen;
    unsigned remain;
    uint32 chunk;

    if (tspec.transfer_status != IO_CSP_FTP_DOWNLOAD) {
        return IO_CSP_FTP_RET_ERR_STATUS;
    }

    status->total = csp_ntoh32(status->total);
    status->complete = csp_ntoh32(status->complete);
    status->entries = csp_ntoh16(status->entries);

    /* Start data stage. */
    for (unsigned i = 0; i < status->entries; ++i) {
        status->entry[i].count = csp_ntoh32(status->entry[i].count);
        status->entry[i].next = csp_ntoh32(status->entry[i].next);
    
        /* Read a chunk and send it out. */
        for (unsigned j = 0; j < status->entry[i].count; ++j) {

            chunk = status->entry[i].next + j;
            remain = tspec.filespec.size - chunk * tspec.downspec.chunk_size;
            datalen = remain > tspec.downspec.chunk_size ? tspec.downspec.chunk_size : remain;
            pktlen = sizeof(data->type) + sizeof(data->data.chunk) + datalen;

            packet = csp_buffer_get(pktlen);
            packet->length = pktlen;

            data = (ftp_packet_t*) packet->data;
            data->data.chunk = csp_hton32(chunk);
            if ((data->type = ftp_helper_read_chunk(data->data.bytes, &tspec.filespec, 
                                                    chunk, tspec.downspec.chunk_size)) != GS_FTP_RET_OK) {
                if (!csp_send(tspec.conn, packet, 60000)) {
                    csp_buffer_free(packet);
                }
                return IO_CSP_FTP_RET_ERR_CHUNK_READ;
            }
            
            data->type = FTP_DATA;
            if (!csp_send(tspec.conn, packet, 60000)) {
                csp_buffer_free(packet);
                return IO_CSP_FTP_RET_ERR_SEND;
            }
        }
    }

    return IO_CSP_FTP_RET_OK;

}


static io_csp_ftp_ret ftp_handler_crc_request(void) {

    csp_packet_t* reply;
    ftp_crc_reply_t* crcrep;
    size_t crcrep_len = sizeof(gs_ftp_type_t) + sizeof(*crcrep);

    if (tspec.transfer_status != IO_CSP_FTP_DOWNLOAD && 
        tspec.transfer_status != IO_CSP_FTP_UPLOAD) {
        return IO_CSP_FTP_RET_ERR_STATUS;
    }

    /* Get buffer for reply packet. */
    if (!(reply = csp_buffer_get(crcrep_len))) {
        return IO_CSP_FTP_RET_ERR_NOBUF;
    }
    reply->length = crcrep_len;
    ((ftp_packet_t*) reply)->type = FTP_CRC_REPLY;
    crcrep = (ftp_crc_reply_t*) reply->data;

    crcrep->crc = io_crc_get_file_crc32(tspec.filespec.fp);

    crcrep->ret = GS_FTP_RET_OK;
    crcrep->crc = csp_hton32(crcrep->crc);

    /* Send reply. */
    if (!csp_send(tspec.conn, reply, tspec.timeout)) { // FIXME: timeout 100?
        ftp_log_error("crc reply send error\n");
        csp_buffer_free(reply);
        return IO_CSP_FTP_RET_ERR_SEND;
    }

    return IO_CSP_FTP_RET_OK;

}


static io_csp_ftp_ret ftp_handler_abort(void) {
    return ftp_handler_done();
}

static io_csp_ftp_ret ftp_handler_done(void) {

    if (tspec.transfer_status == IO_CSP_FTP_IDLE) {
        return IO_CSP_FTP_RET_ERR_STATUS;
    }

    return ftp_helper_filespec_cleanup() == GS_FTP_RET_OK ? IO_CSP_FTP_RET_OK : IO_CSP_FTP_RET_ERR_INVAL;

}


// static io_csp_ftp_ret ftp_handler_restart_server_task(void) {

// }
// static io_csp_ftp_ret ftp_handler_set_timeout(uint32 timeout) {
//     csp_packet_t* reply;
//     tspec.timeout = timeout;

// }
// static io_csp_ftp_ret ftp_handler_filespec_cleanup(void) {

// }




static gs_ftp_return_t ftp_helper_init_upload(void) {

    bool file_was_there = true;
    FILE* mapfp;
    char mapname[GS_FTP_PATH_LENGTH + 10];

    // FIXME: add a timeout handler for termination of previous links.

    /* Try opening the local target file. Create new if didn't exist. */
    sprintf(mapname, "%s.map", tspec.filespec.filename);
    if ((tspec.filespec.fp = fopen(tspec.filespec.filename, "r+")) == NULL) {
        if ((tspec.filespec.fp = fopen(tspec.filespec.filename, "w+")) == NULL) {
            ftp_log_error("cannot open file %s: %s\n", tspec->filespec.filename, strerror(errno));
            return GS_FTP_RET_IO;
        }
        file_was_there = false;
    }

    tspec.filespec.chunks = (tspec.upspec.size + tspec.upspec.chunk_size - 1) / tspec.upspec.chunk_size;
    if ((tspec.filespec.mapbuf = malloc(tspec.filespec.chunks)) == NULL) {
        fclose(tspec.filespec.fp);
        tspec.filespec.fp = NULL;
        return GS_FTP_RET_NOSPC;
    }

    /* Now try opening its bitmap. */
    if ((mapfp = fopen(mapname, "r+")) == NULL) {

        /* We can't have target file without a bitmap. */
        if (file_was_there) {
            fclose(tspec.filespec.fp);
            tspec.filespec.fp = NULL;
            free(tspec.filespec.mapbuf);
            tspec.filespec.mapbuf = NULL;
            return GS_FTP_RET_EXISTS;
        }

        /* Create a new bitmap and clear it. */
        if ((mapfp = fopen(mapname, "w+")) == NULL) {
            ftp_log_error("cannot create map %s: %s\n", tspec->filespec.filename, strerror(errno));
            return GS_FTP_RET_IO;
        }

        for (int i = 0; i < tspec.filespec.chunks; ++i) {
            if (fwrite(packet_missing, 1, 1, mapfp) != 1) {
                ftp_log_error("error clearing map: %s\n", strerror(errno));
                fclose(tspec.filespec.fp);
                tspec.filespec.fp = NULL;
                fclose(tspec.filespec.mapfp);
                tspec.filespec.mapfp = NULL;
                free(tspec.filespec.mapbuf);
                tspec.filespec.mapbuf = NULL;
                return GS_FTP_RET_IO;
            }
            tspec.filespec.mapbuf[i] = *packet_missing;
        }

        fflush(mapfp);
        fsync(fileno(mapfp));

        tspec.filespec.mapfp = mapfp;

    }
    else {
        /* The map exists. */
        if (fread(tspec.filespec.mapbuf, 1, tspec.filespec.chunks, mapfp) != tspec.filespec.chunks) {
            ftp_log_error("could not read from existing map %s.\n", mapname);
            fclose(tspec.filespec.fp);
            tspec.filespec.fp = NULL;
            fclose(tspec.filespec.mapfp);
            tspec.filespec.mapfp = NULL;
            free(tspec.filespec.mapbuf);
            tspec.filespec.mapbuf = NULL;
            return GS_FTP_RET_IO;
        }
    }

    return GS_FTP_RET_OK;

}

static gs_ftp_return_t ftp_helper_read_upload_status(ftp_status_reply_t* status) {

    unsigned count_in_entry = 0;
    unsigned next = 0;
    bool s;

    if (!tspec.filespec.mapbuf) {
        return GS_FTP_RET_INVAL;
    }

    for (unsigned i = 0; i < tspec.filespec.chunks; ++i) {
        if ((s = (tspec.filespec.mapbuf[i] == *packet_ok))) {
            status->complete++;
        }
        if (status->entries < GS_FTP_STATUS_CHUNKS) {
            if (!s) {
                if (!count_in_entry) {
                    next = i;
                }
                count_in_entry++;
            }
            if (count_in_entry > 0 && (s || i == tspec.filespec.chunks - 1)) {
                status->entry[status->entries].next = next;
                status->entry[status->entries].count = count_in_entry;
                status->entries++;
            }
        }
    }

    status->entries = status->entries;
    status->complete = status->complete;
    status->total = tspec.filespec.chunks;

    return GS_FTP_RET_OK;

}


static gs_ftp_return_t ftp_helper_read_chunk(void* data, const io_csp_ftp_filespec* filespec, unsigned chunk, unsigned chunk_size) {

    if (!data || !filespec) {
        return GS_FTP_RET_INVAL;
    }
    if (!filespec->fp) {
        return GS_FTP_RET_INVAL;
    }
    if (ftell(filespec->fp) != chunk * chunk_size) {
        if (fseek(filespec->fp, chunk * chunk_size, SEEK_SET) != 0) {
            return GS_FTP_RET_IO;
        }
    }
    if (fread(data, 1, chunk_size, filespec->fp) != chunk_size) {
        return GS_FTP_RET_IO;
    }
    return GS_FTP_RET_OK;

}


static gs_ftp_return_t ftp_helper_write_chunk(const void* data, const io_csp_ftp_filespec* filespec, unsigned chunk, unsigned chunk_size) {

    if (!data || !filespec) {
        return GS_FTP_RET_INVAL;
    }
    if (!filespec->fp || !filespec->mapfp || !filespec->mapbuf) {
        return GS_FTP_RET_INVAL;
    }

    if (ftell(filespec->fp) != chunk * chunk_size) {
        if (fseek(filespec->fp, chunk * chunk_size, SEEK_SET) != 0) {
            return GS_FTP_RET_IO;
        }
    }
    if (fwrite(data, 1, chunk_size, filespec->fp) != chunk_size) {
        return GS_FTP_RET_IO;
    }

    if (ftell(filespec->mapfp) != chunk) {
        if (fseek(filespec->mapfp, chunk, SEEK_SET) != 0) {
            return GS_FTP_RET_IO;
        }
    }
    if (fwrite(packet_ok, 1, 1, filespec->fp) != 1) {
        return GS_FTP_RET_IO;
    }

    filespec->mapbuf[chunk] = *packet_ok;

    fflush(filespec->fp);
    fsync(fileno(filespec->fp));

    fflush(filespec->mapfp);
    fsync(fileno(filespec->mapfp));

    return GS_FTP_RET_OK;

}



static gs_ftp_return_t ftp_helper_filespec_cleanup(void) {

    char mapname[GS_FTP_PATH_LENGTH + 10];

    if (tspec.filespec.fp) {
        fclose(tspec.filespec.fp);
        tspec.filespec.fp = NULL;
    }

    if (tspec.filespec.mapfp) {
        fclose(tspec.filespec.mapfp);
        tspec.filespec.mapfp = NULL;
    }

    if (tspec.filespec.mapbuf) {
        free(tspec.filespec.mapbuf);
        tspec.filespec.mapbuf = NULL;
    }

    snprintf(mapname, sizeof(mapname), "%s.map", tspec.filespec.filename);
    remove(mapname);

    return GS_FTP_RET_OK;

}


static gs_ftp_return_t ftp_helper_conn_timeout(void) {
    ftp_helper_filespec_cleanup();
    tspec.transfer_status = IO_CSP_FTP_IDLE;
    return GS_FTP_RET_OK;
}



int32 io_csp_ftp_init(void) {

    int ret;
    pthread_t tid;
    csp_socket_t*   server_sock;

    memset(&tspec, 0, sizeof(tspec));
    tspec.transfer_status = IO_CSP_FTP_IDLE;
    tspec.timeout = IO_CSP_FTP_READ_DEFAULT_TIMEOUT;

    if ((server_sock = csp_socket(CSP_O_RDP | CSP_O_CRC32)) == NULL) {
        ftp_log_error("ftp socket creat error\n");
        return -1;
    }

    if (csp_bind(server_sock, CSP_PORT_FTP) != 0) {
        ftp_log_error("bind error for ftp port %d\n", CSP_PORT_FTP);
        return -1;
    }
    
    if (csp_listen(server_sock, 10) < 0) {
        ftp_log_error("listen error for ftp port %d\n", CSP_PORT_FTP);
        return -1;
    }

    if ((ret = pthread_create(&tid, NULL, io_csp_ftp_server_task, server_sock)) != 0) {
        ftp_log_error("pthread_create error: %d\n", ret);
        return -1;
    }

    ftp_log_info("FTP task created with tid %lu\n", (unsigned) tid);
    return 0;

}


static void* io_csp_ftp_server_task(void* server_sock) {

    csp_packet_t*   packet;
    ftp_packet_t*   ftp_packet;
    csp_socket_t* socket = server_sock;
    io_csp_ftp_ret ret;

    while (1) {

        if (tspec.conn = csp_accept(socket, CSP_MAX_DELAY)) {
            ftp_log_info("incoming connection from node %d\n", csp_conn_src(tspec.conn));
        }

        if (!tspec.conn) {
            ftp_log_error("no connection accepted\n");
            continue;
        }

        while (1) {

            if ((packet = csp_read(tspec.conn, tspec.timeout)) == NULL) {
                ftp_helper_conn_timeout();
                break;
            }

            ftp_packet = (ftp_packet_t*) packet->data;
            switch (ftp_packet->type) {
                case FTP_DOWNLOAD_REQUEST:
                    ret = ftp_handler_download_request(&ftp_packet->down);
                    break;
                case FTP_STATUS_REPLY:
                    ret = ftp_handler_status_reply(&ftp_packet->statusrep);
                    break;
                case FTP_UPLOAD_REQUEST:
                    ret = ftp_handler_upload_request(&ftp_packet->up);
                    break;
                case FTP_STATUS_REQUEST:
                    ret = ftp_handler_status_request();
                    break;
                case FTP_DATA:
                    ret = ftp_handler_data(&ftp_packet->data);
                    break;
                case FTP_CRC_REQUEST:
                    ret = ftp_handler_crc_request();
                    break;
                case FTP_DONE:
                    ret = ftp_handler_done();
                    break;
                case FTP_ABORT:
                    ret = ftp_handler_abort();
                    break;
                default:
                    ftp_log_error("Invalid ftp packet type: %d\n", ftp_packet->type);
                    break;
            }

            csp_buffer_free(packet);

            // ADD RET LOG HERE

        }

        if (tspec.conn) {
            csp_close(tspec.conn);
            tspec.conn = NULL;
        }
        ftp_helper_conn_timeout();

    }

    pthread_exit(NULL);

}
