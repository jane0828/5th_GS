#include "io_csp.h"
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

#define IO_FTP_FSW_ROOT             "/root/exe/cpu1"

/* Chunk status markers */
static const char * const packet_missing = "-";
static const char * const packet_ok = "+";






/**
 * @brief FTP transfer global spec.
 */
static io_csp_ftp_transfer_spec tspec;



/* Forward declarations. */
static void* io_csp_ftp_server_task(void*);



int32 io_csp_ftp_init(void) {
    int ret;
    pthread_t tid;
    if ((ret = pthread_create(&tid, NULL, io_csp_ftp_server_task, NULL)) != 0) {
        ftp_log_error("pthread_create error: %d\n", ret);
        return MPL_ERR_PTHREAD_CREATE;
    }
    ftp_log_info("FTP task created with tid %lu\n", (unsigned) tid);
    return IO_LIB_SUCCESS;
}


static mpl_status io_csp_ftp_get_filespec(io_csp_ftp_transfer_spec* tspec, const char* path, const char* root) {

    // char filename_ext[256];

    // if  (root) {
    //     if (strlen(path) + strlen(IO_FTP_FSW_ROOT) > sizeof(filename_ext) - 1) {
    //         return MPL_ERR_FTP_FILENAME_TOO_LONG;
    //     }
    //     strcpy(filename_ext, IO_FTP_FSW_ROOT);
    //     strcat(filename_ext, path);
    // }
    // else {
    //     strcpy(filename_ext, path);
    // }

    if (!(tspec->fp = fopen(path, "r"))) {
        ftp_log_error("unable to open file %s: %s\n", path, strerror(errno));
        return MPL_ERR_NOT_FOUND;
    }

    struct stat statbuf;
    fstat(fileno(tspec->fp), &statbuf);
    tspec->filespec.size = statbuf.st_size;
    tspec->filespec.crc = io_crc_get_file_crc32(tspec->fp);

    return MPL_OK;

}


static mpl_status io_csp_ftp_reply_download_request(io_csp_ftp_transfer_spec* tspec, ftp_packet_t* request) {

    mpl_status ret;
    ftp_packet_t reply;

    if (!request || !tspec) {
        return MPL_ERR_NULL;
    }

    if (tspec->busy) {
        reply.downrep.ret = GS_FTP_RET_BUSY;
    }
    else {

        tspec->busy = true;
        tspec->direction = IO_CSP_FTP_DOWNLINK;
        tspec->backend = request->down.backend;

        switch (tspec->backend) {

            case GS_FTP_BACKEND_FILE:
                /* Open the local file. */
                ret = io_csp_ftp_get_filespec(tspec, request->down.path, NULL);
                if (ret == MPL_OK) {
                    tspec->chunk_size = csp_ntoh16(request->down.chunk_size);
                    reply.downrep.ret = GS_FTP_RET_OK;
                    reply.downrep.size = csp_hton32(tspec->filespec.size);
                    reply.downrep.crc32 = csp_hton32(tspec->filespec.crc);
                    ftp_log_info("chunk size: %d\n", tspec->chunk_size);
                    ftp_log_info("filespec.size = %d\n", tspec->filespec.size);
                    ftp_log_info("filespec.crc = %d\n", tspec->filespec.crc);
                }
                else {
                    reply.downrep.ret = GS_FTP_RET_IO;
                }
                break;

            case GS_FTP_BACKEND_FAT:
            case GS_FTP_BACKEND_RAM:
            case GS_FTP_BACKEND_UFFS:
                ftp_log_error("unsupported backend type %d\n", request->down.backend);
                reply.downrep.ret = GS_FTP_RET_NOTSUP;
                break;

            default:
                ftp_log_error("unknown backend type %d\n", request->down.backend);
                reply.downrep.ret = GS_FTP_RET_INVAL;
                break;

        }
    }

    /* Send the reply. */
    reply.type = FTP_DOWNLOAD_REPLY;
    uint32 downrep_len = sizeof(reply.type) + sizeof(reply.downrep);

    if (csp_transaction_persistent(tspec->conn, tspec->timeout, &reply, downrep_len, NULL, 0) != 1) {
        ftp_log_error("error sending downrep reply\n");
        // io_csp_ftp_upload_cleanup(tspec);
        return MPL_ERR_CSP_TRANS;
    }

    return MPL_OK;

}



static mpl_status io_csp_ftp_process_status_reply(io_csp_ftp_transfer_spec* tspec, ftp_packet_t* status_reply) {

    if (!status_reply || !tspec) {
        return MPL_ERR_NULL;
    }

    /* Reject if the status reply is not sane. */
    if (status_reply->type != FTP_STATUS_REPLY) {
        ftp_log_error("packet type %d is not a status reply\n", status_reply->type);
        // io_csp_ftp_upload_cleanup(tspec);
        return MPL_ERR_FTP_BAD_TYPE;
    }
    if (status_reply->statusrep.ret != GS_FTP_RET_OK) {
        ftp_log_error("bad ret code from status reply: %d\n", status_reply->statusrep.ret);
        // io_csp_ftp_upload_cleanup(tspec);
        return MPL_ERR_FTP_BAD_RET;
    }

    /* Store the status. */
    tspec->status.complete = csp_ntoh32(status_reply->statusrep.complete);
    tspec->status.chunks = csp_ntoh32(status_reply->statusrep.total);
    tspec->status.entries = csp_ntoh16(status_reply->statusrep.entries);
    ftp_log_info("status.complete = %d\n", tspec->status.complete);
    ftp_log_info("status.chunks = %d\n", tspec->status.chunks);
    ftp_log_info("status.entries = %d\n", tspec->status.entries);
    for (int i = 0; i < tspec->status.entries; i++) {
        tspec->status.entry[i].next = csp_ntoh32(status_reply->statusrep.entry[i].next);
        tspec->status.entry[i].count = csp_ntoh32(status_reply->statusrep.entry[i].count);
    }

    return MPL_OK;

}



static mpl_status io_csp_ftp_data(io_csp_ftp_transfer_spec* tspec) {

    ftp_log_info("handling data stage...\n");

    if (!tspec) {
        return MPL_ERR_NULL;
    }

    /* Data packet. */
    ftp_packet_t packet;
    packet.type = FTP_DATA;

    /* Repeat for each status entry. */
    for (int entry = 0; entry < tspec->status.entries; entry++) {
        ftp_log_info("entry number %d/%d\n", entry + 1, tspec->status.entries);

        /* Repeat for the chunk count for this entry. */
        for (int i = 0; i < tspec->status.entry[entry].count; i++) {
            ftp_log_info("chunk number %d/%d\n", i + 1, tspec->status.entry[entry].count);

            /* Chunk number for this packet. */
            packet.data.chunk = tspec->status.entry[entry].next + i;

            if (ftell(tspec->fp) != packet.data.chunk * tspec->chunk_size) {
                if (fseek(tspec->fp, packet.data.chunk * tspec->chunk_size, SEEK_SET) != 0) {
                    ftp_log_error("fseek error at chunk %d: %s\n", packet.data.chunk, strerror(errno));
                    return MPL_ERR_FSEEK;
                }
            }

            /* Read a chunk from the file. */
            size_t read_bytes = fread(packet.data.bytes, 1, tspec->chunk_size, tspec->fp);
            if (read_bytes != tspec->chunk_size) {
                if (read_bytes < 0) {
                    return MPL_ERR_FREAD;
                }
                /* Clear the rest of the chunks. */
                memset(&packet.data.bytes[read_bytes], 0, sizeof(packet.data.bytes) - read_bytes);
            }

            /* The FTP client requires the chunk number in the data packet to be little-endian. */
           // packet.data.chunk = csp_htole32(packet.data.chunk);
            packet.data.chunk = csp_hton32(packet.data.chunk);
            /* Send a data packet. */
            int packet_len = sizeof(packet.type) + sizeof(packet.data.chunk) + tspec->chunk_size;
            if (csp_transaction_persistent(tspec->conn, tspec->timeout, &packet, packet_len, NULL, 0) != 1) {
                ftp_log_error("transaction error at chunk %d\n", csp_ntoh32(packet.data.chunk));
                return MPL_ERR_CSP_TRANS;
            }

        }

    }

    return MPL_OK;

}


// static mpl_status io_csp_ftp_upload(io_csp_ftp_transfer_spec* tspec, ftp_packet_t* request) {

//     mpl_status ret;
//     csp_packet_t* packet;
//     ftp_packet_t* ftp_packet;

//     ret = io_csp_ftp_reply_download_request(tspec, request);
//     csp_buffer_free(request);
//     if (ret != MPL_OK) {
//         return ret;
//     }

//     packet = csp_read(tspec->conn, tspec->timeout);
//     if (!packet) {
//         return MPL_ERR_CSP_READ;
//     }
//     ftp_packet = (ftp_packet_t*) packet->data;
//     if (ftp_packet->type == FTP_DONE) {
//         csp_buffer_free(packet);
//         return MPL_OK;
//     }

//     ret = io_csp_ftp_get_status(ftp_packet, tspec);
//     csp_buffer_free(packet);
//     if (ret != MPL_OK) {
//         return ret;
//     }

//     return io_csp_ftp_data(tspec);

// }




static mpl_status io_csp_ftp_reply_crc_request(io_csp_ftp_transfer_spec* tspec) {
    
    ftp_packet_t reply;
    char buf[135];

    if (!tspec) {
        return MPL_ERR_NULL;
    }

    if (!tspec->filespec.crc) {
        reply.crcrep.ret = GS_FTP_RET_IO;
    }
    else {
        reply.crcrep.ret = GS_FTP_RET_OK;
        reply.crcrep.crc = csp_hton32(tspec->filespec.crc);
    }

    reply.type = FTP_CRC_REPLY;
    int repsize = sizeof(reply.type) + sizeof(reply.crcrep);

    if (csp_transaction_persistent(tspec->conn, tspec->timeout, &reply, repsize, NULL, 0) != 1) {
        ftp_log_error("crc reply transaction error");
        return MPL_ERR_CSP_TRANS;
    }
    
    if (tspec->mapfp) {
        fclose(tspec->mapfp);
        sprintf(buf, "%s.map", tspec->filespec.filename);
        remove(buf);
    }

    return MPL_OK;

}


static mpl_status io_csp_ftp_reply_upload_request(io_csp_ftp_transfer_spec* tspec, ftp_packet_t* request) {

    ftp_packet_t reply;
    int repsize;

    if (!request) {
        return MPL_ERR_NULL;
    }

    if (tspec->busy) {
        ftp_log_error("ftp transfer unit busy\n");
        reply.uprep.ret = GS_FTP_RET_BUSY;
    }
    else {
        if (strlen(request->up.path) >= IO_CSP_FTP_MAX_FILENAME) {
            ftp_log_error("filename %s is too long (max %d, got %d)\n", 
                          request->up.path, IO_CSP_FTP_MAX_FILENAME, strlen(request->up.path));
            reply.uprep.ret = GS_FTP_RET_IO;
        }
        else {
            tspec->backend = request->up.backend; /* The backend type has a byte-length. */
            tspec->chunk_size = csp_hton16(request->up.chunk_size);
            tspec->filespec.size = csp_hton32(request->up.size);
            tspec->filespec.crc = csp_hton32(request->up.crc32);
            strcpy(tspec->filespec.filename, request->up.path);
            reply.uprep.ret = GS_FTP_RET_OK;
        }
    }

    reply.type = FTP_UPLOAD_REPLY;
    repsize = sizeof(reply.type) + sizeof(reply.uprep);
    if (csp_transaction_persistent(tspec->conn, tspec->timeout, &reply, repsize, NULL, 0) != 1) {
        ftp_log_error("csp_transaction_persistent timeout\n");
        return MPL_ERR_CSP_TRANS;
    }

    return MPL_OK;

}


static mpl_status io_csp_ftp_read_status(io_csp_ftp_transfer_spec* tspec) {

    FILE* mapfp;
    char mapfilename[IO_CSP_FTP_MAX_FILENAME + 4];
    
    sprintf(mapfilename, "%s.map", tspec->filespec.filename);

    /* Try opening the local target file. */
    tspec->fp = fopen(tspec->filespec.filename, "r+");
    
    /* If not exist, create one. */
    if (!tspec->fp) {
        tspec->fp = fopen(tspec->filespec.filename, "w+");
        
        if (!tspec->fp) {
            ftp_log_error("cannot open file %s: %s\n", tspec->filespec.filename, strerror(errno));
            return MPL_ERR_FOPEN;
        }

        /* Now try opening a bitmap. */
        mapfp = fopen(mapfilename, "r+");
        /* If not exist, create one and clear it. */
        if (!mapfp) {
            mapfp = fopen(mapfilename, "w+");
            if (!mapfp) {
                ftp_log_error("cannot create map %s: %s\n", tspec->filespec.filename, strerror(errno));
                return MPL_ERR_FOPEN;
            }
            for (int i = 0; i < tspec->status.chunks; i++) {
                if (fwrite(packet_missing, 1, 1, mapfp) != 1) {
                    ftp_log_error("error clearing map: %s\n", strerror(errno));
                    return MPL_ERR_WRITE;
                }
            }        
            tspec->status.entries = csp_hton16(1);
            tspec->status.complete = 0;
            tspec->status.entry[0].count = csp_hton32(tspec->status.chunks);
            tspec->status.entry[0].next = 0;

            fflush(mapfp);
            fsync(fileno(mapfp));

            tspec->mapfp = mapfp;
            return MPL_OK;
        }
    }
    else {
        /* The local file exists. */
        mapfp = fopen(mapfilename, "r+");
        
        /* We can't have the local file without a bitmap. */
        if (!mapfp) {
            ftp_log_error("local file %s exists but bitmap doesn't\n", tspec->filespec.filename);
            return MPL_ERR_FTP_EXISTS;
        }
    }

    /* Read chunk status. */
    tspec->status.entries = 0;
    unsigned count_in_entry = 0;
    unsigned next;
    bool s;

    for (unsigned i = 0; i < tspec->status.chunks; i++) {
        printf("i = %d, chunk=%d\n", i, tspec->status.chunks);
        char is_complete;

        if (ftell(mapfp) != i) {
            if (fseek(mapfp, i, SEEK_SET) != 0) {
                ftp_log_error("fseek error: %s\n", strerror(errno));
                return MPL_ERR_FSEEK;
            }
        }

        if (fread(&is_complete, 1, 1, mapfp) != 1) {
            ftp_log_error("fread error: %s\n", strerror(errno));
            return MPL_ERR_FREAD;
        }

        s = (is_complete == *packet_ok);

        if (s) {
            tspec->status.complete++;
        }

        if (tspec->status.entries < GS_FTP_STATUS_CHUNKS) {
            if (!s) {
                if (!count_in_entry) {
                    next = i;
                }
                count_in_entry++;
            }

            if (count_in_entry > 0 && (s || i == tspec->status.chunks - 1)) {
                tspec->status.entry[tspec->status.entries].next = next;
                tspec->status.entry[tspec->status.entries].count = count_in_entry;
                tspec->status.entries++;
            }
        }
    }

    tspec->mapfp = mapfp;
    return MPL_OK;

}



static mpl_status io_csp_ftp_receive_chunks(io_csp_ftp_transfer_spec* tspec) {

    ftp_packet_t    packet;
    csp_packet_t*   pkt;
    //int             packetsize;
    uint32          chunk;
    int             datalen;

    //packetsize = sizeof(packet.type) + sizeof(packet.data);

    while (1) {

        //printf("conn: %p, fp: %p, mapfp:%p\n", tspec->conn, tspec->fp, tspec->mapfp);

        // if (csp_transaction_persistent(tspec->conn, 5000, NULL, 0, &packet, -1) != 1) {
        //     ftp_log_error("csp_transaction_persistent timeout\n");
        //     fsync(fileno(tspec->fp));
        //     fsync(fileno(tspec->mapfp));
        //     return MPL_ERR_CSP_TRANS;
        // }

        pkt = csp_read(tspec->conn, tspec->timeout);
        if (!pkt) {
            printf("No read!!\n");
            return -1;
        }
        memcpy(&packet, pkt->data, pkt->length);
        csp_buffer_free(pkt);

        if (packet.type != FTP_DATA) {
            ftp_log_error("%d is not an ftp data packet type\n", packet.type);
            continue;
        }

        chunk = csp_letoh32(packet.data.chunk);
        if (chunk >= tspec->status.chunks) {
            ftp_log_error("bad chunk number %d (total %d)\n", chunk, tspec->status.chunks);
            continue;
        }
        datalen = (datalen == tspec->status.chunks - 1 ? tspec->filespec.size % tspec->chunk_size : tspec->chunk_size);
        if (datalen == 0) datalen = tspec->chunk_size;
        ftp_log_info("chunk number %d/%d (%d bytes)\n", chunk, tspec->status.chunks, datalen);


        /* Write to the local target. */
        if (ftell(tspec->fp) != tspec->chunk_size * chunk) {
            if (fseek(tspec->fp, tspec->chunk_size * chunk, SEEK_SET) != 0) {
                ftp_log_error("fseek error at chunk %d\n", chunk);
                return MPL_ERR_FSEEK;
            }
        }

        if (fwrite(packet.data.bytes, 1, datalen, tspec->fp) != datalen) {
            ftp_log_error("fwrite error at chunk %d (dlen %d)\n", chunk, datalen);
            return MPL_ERR_FWRITE;
        }
        fflush(tspec->fp);


        /* Update the bitmap. */
        if (ftell(tspec->mapfp) != chunk) {
            if (fseek(tspec->mapfp, chunk, SEEK_SET) != 0) {
                ftp_log_error("map fseek error at chunk %d\n", chunk);
                return MPL_ERR_FSEEK;
            }
        }

        if (fwrite(packet_ok, 1, 1, tspec->mapfp) != 1) {
            ftp_log_error("map fwrite error at chunk %d\n", chunk);
            return MPL_ERR_FWRITE;
        }
        fflush(tspec->mapfp);

        /* Last chunk. */
        if (chunk == tspec->status.chunks - 1) {
            break;
        }

    }

    fsync(fileno(tspec->fp));
    fsync(fileno(tspec->mapfp));

    return MPL_OK;

}



static mpl_status io_csp_ftp_reply_status_request(io_csp_ftp_transfer_spec* tspec) {

    ftp_packet_t reply;
    mpl_status ret;
    int repsize;

    tspec->status.chunks = (tspec->filespec.size + tspec->chunk_size - 1) / tspec->chunk_size;

    if ((ret = io_csp_ftp_read_status(tspec)) != MPL_OK) {
        reply.statusrep.ret = GS_FTP_RET_IO;
    }
    else {
        reply.statusrep.ret = GS_FTP_RET_OK;
        reply.statusrep.complete = csp_hton32(tspec->status.complete);
        reply.statusrep.total = csp_hton32(tspec->status.chunks);
        reply.statusrep.entries = csp_hton16(tspec->status.entries);
        for (int i = 0; i < tspec->status.entries; i++) {
            reply.statusrep.entry[i].count = csp_hton32(tspec->status.entry[i].count);
            reply.statusrep.entry[i].next = csp_hton32(tspec->status.entry[i].next);
        }
    }

    reply.type = FTP_STATUS_REPLY;
    repsize = sizeof(reply.type) + sizeof(reply.statusrep);
    if (csp_transaction_persistent(tspec->conn, tspec->timeout, &reply, repsize, NULL, 0) != 1) {
        ftp_log_error("csp_transaction_persistent error\n");
        return MPL_ERR_CSP_TRANS;
    }

    /* Data stage. */
    if (ret == MPL_OK) {
        return io_csp_ftp_receive_chunks(tspec);
    }

    return ret;

}



static void io_csp_ftp_done(io_csp_ftp_transfer_spec* tspec) {
    
    if (tspec->fp) {
        fflush(tspec->fp);
        fsync(fileno(tspec->fp));
        fclose(tspec->fp);
        tspec->fp = NULL;
    }

    if (tspec->mapfp) {
        fflush(tspec->mapfp);
        fsync(fileno(tspec->mapfp));
        fclose(tspec->mapfp);
        tspec->mapfp = NULL;
    }

    if (tspec->conn) {
        csp_close(tspec->conn);
        tspec->conn = NULL;
    }

    tspec->busy = false;

}



static void* io_csp_ftp_server_task(void* opaque_arg) {

    csp_packet_t* packet;
    csp_socket_t* socket;
    ftp_packet_t* ftp_packet;
    mpl_status ret;

    tspec.timeout = 1000;
    tspec.busy      = false;


    if ((socket = csp_socket(CSP_O_RDP | CSP_O_CRC32)) == NULL) {
        ftp_log_error("ftp socket creat error\n");
        return &ftp_task_canceled_socket;
    }

    if (csp_bind(socket, CSP_PORT_FTP) != 0) {
        ftp_log_error("bind error for ftp port %d\n", CSP_PORT_FTP);
        return &ftp_task_canceled_bind;
    }
    
    if (csp_listen(socket, 10) < 0) {
        ftp_log_error("listen error for ftp port %d\n", CSP_PORT_FTP);
        return NULL;
    }

    while (1) {

        if (!tspec.conn && (tspec.conn = csp_accept(socket, CSP_MAX_DELAY))) {
            ftp_log_info("incoming connection from node %d\n", csp_conn_src(tspec.conn));
        }

        if (!tspec.conn) {
            ftp_log_error("no connection accepted\n");
            continue;
        }

        while (1) {

            packet = csp_read(tspec.conn, CSP_FTP_READ_TIMEOUT);

            if (!packet) {
                break;
            }

            ftp_packet = (ftp_packet_t*) packet->data;

            switch (ftp_packet->type) {

                case FTP_DOWNLOAD_REQUEST:
                    ret = io_csp_ftp_reply_download_request(&tspec, ftp_packet);
                    if (ret != MPL_OK) {
                        ftp_log_error("io_csp_ftp_upload error: %d\n", ret);
                    }
                    break;

                case FTP_STATUS_REPLY:
                    ret = io_csp_ftp_process_status_reply(&tspec, ftp_packet);
                    if (ret != MPL_OK) {
                        ftp_log_error("io_csp_ftp_process_status_reply error: %d\n", ret);
                        break;
                    }
                    ret = io_csp_ftp_data(&tspec);
                    break;

                case FTP_UPLOAD_REQUEST:
                    ftp_log_info("upload request received\n");
                    if ((ret = io_csp_ftp_reply_upload_request(&tspec, ftp_packet)) != MPL_OK) {
                        ftp_log_error("io_csp_ftp_reply_upload_request error: %d\n", ret);
                    }
                    break;

                case FTP_STATUS_REQUEST:
                    ftp_log_info("upload status request received\n");
                    ret = io_csp_ftp_reply_status_request(&tspec);
                    break;

                case FTP_DONE:
                    ftp_log_info("data stage abort request\n");
                    io_csp_ftp_done(&tspec);
                    break;

                case FTP_CRC_REQUEST:
                    if ((ret = io_csp_ftp_reply_crc_request(&tspec)) != MPL_OK) {
                        ftp_log_error("crc reply error: %d\n", ret);
                    }
                    break;

                default:
                    ftp_log_error("Invalid ftp packet type: %d\n", ftp_packet->type);
                    break;
            }

            csp_buffer_free(packet);

        }

        if (tspec.conn) {
            csp_close(tspec.conn);
            tspec.conn = NULL;
        }

    }

    return &ftp_task_canceled;

}