/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#define GS_FTP_INTERNAL_USE 1

#include <gs/ftp/server.h>
#include <gs/ftp/client.h>
#include <gs/ftp/internal/types.h>
#include <csp/csp.h>
#include <csp/csp_endian.h>
#include <gs/util/string.h>
#include <gs/util/crc32.h>
#include <gs/util/thread.h>
#include "local.h"

static size_t stack_size = 16000;

GS_LOG_GROUP(gs_ftp_log, "ftp", GS_LOG_CAT_DEFAULT, LOG_DEFAULT_MASK);

#define FTP_MAX_BACKENDS 5

typedef enum {
    FTP_STATE_IDLE 	= 0,
    FTP_STATE_UPLOAD 	= 1,
    FTP_STATE_DOWNLOAD 	= 2,
} ftp_state_t;

typedef struct {
    gs_ftp_backend_type_t type;
    const gs_ftp_backend_t * backend;
} backend_t;

static backend_t backends[FTP_MAX_BACKENDS];

gs_error_t gs_ftp_server_register_backend(gs_ftp_backend_type_t type, const gs_ftp_backend_t *backend)
{
    gs_log_group_register(gs_ftp_log);

    for (unsigned int i = 0; i < FTP_MAX_BACKENDS; ++i) {
        if (backends[i].backend == NULL) {
            backends[i].type = type;
            backends[i].backend = backend;
            return GS_OK;
        }
    }

    log_warning("No free backend slots");
    return GS_ERROR_FULL;
}

const gs_ftp_backend_t * ftp_get_backend(gs_ftp_backend_type_t backend)
{
    for (unsigned int i = 0; i < FTP_MAX_BACKENDS; ++i) {
        if (backends[i].type == backend) {
            return backends[i].backend;
        }
    }
    return NULL;
}

static void * task_ftp(void * conn_param)
{
    csp_conn_t * conn = conn_param;

    const gs_ftp_backend_t * backend = NULL;

    ftp_state_t state = FTP_STATE_IDLE;
    ftp_upload_request_t ul;
    ftp_download_request_t dl;

    uint32_t ftp_size;
    uint32_t ftp_checksum;

    void * backend_state = NULL;
    csp_packet_t * packet = NULL;

    log_debug("starting FTP task with conn from %u", csp_conn_src(conn));

    while(1) {

        /* Read packet from stream - timeout should be at least RDP connection timeout */
        packet = csp_read(conn, 60000);
        if (!packet) {
            log_warning("Timeout during transfer, aborting");
            if (backend && backend->timeout)
                backend->timeout(backend_state);
            goto out;
        }

        /* Switch on packet type */
        ftp_packet_t * ftp_packet = (ftp_packet_t *) packet->data;

        log_trace("received packet of type %u in state %u",
                  ftp_packet->type, state);

        switch(ftp_packet->type) {

            case FTP_UPLOAD_REQUEST: {
                /* Validate state */
                if (state != FTP_STATE_IDLE) {
                    log_warning("Upload request received in state %"PRIu8, state);
                    goto out_free;
                }

                state = FTP_STATE_UPLOAD;

                /* Get file information, and store it in local structure */
                {
                    ftp_upload_request_t * upload = &ftp_packet->up;
                    upload->size = csp_ntoh32(upload->size);
                    upload->crc32 = csp_ntoh32(upload->crc32);
                    upload->chunk_size = csp_ntoh16(upload->chunk_size);
                    upload->mem_addr = csp_ntoh32(upload->mem_addr);
                    memcpy(&ul, upload, sizeof(ul));
                    ul.path[sizeof(ul.path) - 1] = 0; // ensure path is terminated
                }
                log_info("Upload begin: size %"PRIu32", path [%s], type %"PRIu8", addr 0x%"PRIX32" chunk size %u",
                         ul.size, ul.path, ul.backend, ul.mem_addr, ul.chunk_size);

                backend = ftp_get_backend(ul.backend);
                if (backend == NULL) {
                    log_error("No backend available for %u", ul.backend);
                    goto out_free;
                }

                /* Initialize backend state */
                if (backend->init) {
                    log_debug("Upload: Init backend");
                    if (backend->init(&backend_state) != GS_FTP_RET_OK) {
                        log_error("Could not init backend");
                        goto out_free;
                    }
                }

                ftp_packet->type = FTP_UPLOAD_REPLY;
                if (backend->upload) {
                    log_debug("Upload: Prepare backend upload");
                    ftp_packet->uprep.ret = backend->upload(backend_state,
                                                            ul.path, ul.mem_addr, ul.size, ul.chunk_size);
                } else {
                    ftp_packet->uprep.ret = GS_FTP_RET_NOTSUP;
                }

                packet->length = sizeof(gs_ftp_type_t) + sizeof(ftp_upload_reply_t);

                log_debug("Upload: Send reply");
                if (!csp_send(conn, packet, 0))
                    goto out_free;

                break;
            }

            case FTP_DOWNLOAD_REQUEST: {
                /* Validate state */
                if (state != FTP_STATE_IDLE) {
                    log_warning("Download request received in state %"PRIu8, state);
                    goto out_free;
                }

                state = FTP_STATE_DOWNLOAD;

                /* Get file information, and store it in local structure */
                {
                    ftp_download_request_t * download = (ftp_download_request_t *) &ftp_packet->down;
                    download->chunk_size = csp_ntoh16(download->chunk_size);
                    download->mem_addr = csp_ntoh32(download->mem_addr);
                    download->mem_size = csp_ntoh32(download->mem_size);
                    memcpy(&dl, download, sizeof(dl));
                    dl.path[sizeof(dl.path) - 1] = 0; // ensure path is terminated
                }
                log_info("Download begin: path [%s], type %"PRIu8", addr 0x%"PRIX32", size %"PRIu32", chunk size %u",
                         dl.path, dl.backend, dl.mem_addr, dl.mem_size, dl.chunk_size);

                backend = ftp_get_backend(dl.backend);
                if (backend == NULL) {
                    log_error("No backend available for type %u", dl.backend);
                    goto out_free;
                }

                /* Initialize backend state */
                if (backend->init) {
                    if (backend->init(&backend_state) != GS_FTP_RET_OK) {
                        log_error("Could not init backend");
                        goto out_free;
                    }
                }

                ftp_packet->type = FTP_DOWNLOAD_REPLY;

                if (backend->download) {
                    ftp_packet->downrep.ret = backend->download(backend_state, dl.path, dl.mem_addr, dl.mem_size, dl.chunk_size, &ftp_size, &ftp_checksum);
                    ftp_packet->downrep.size = csp_hton32(ftp_size);
                    ftp_packet->downrep.crc32 = csp_hton32(ftp_checksum);
                } else {
                    ftp_packet->downrep.ret = GS_FTP_RET_NOTSUP;
                }

                packet->length = sizeof(gs_ftp_type_t) + sizeof(ftp_download_reply_t);

                if (!csp_send(conn, packet, 0))
                    goto out_free;

                break;
            }

            case FTP_DATA: {
                /* Validate state */
                if (state != FTP_STATE_UPLOAD) {
                    log_warning("Data received in state %"PRIu8, state);
                    goto out_free;
                }

                if (backend == NULL)
                    goto out_free;

                ftp_data_t * data = (ftp_data_t *) &ftp_packet->data;

                /* NOTE: The network traffic is in this case Little-Endian!
                 * Therefore csp_letoh32 is used instead of csp_ntoh32 */
                data->chunk = csp_letoh32(data->chunk);

                if (data->chunk >= (ul.size + ul.chunk_size - 1)/ ul.chunk_size)
                    goto out_free;

                unsigned int remain = ul.size - data->chunk * ul.chunk_size;
                unsigned int size = remain > ul.chunk_size ? ul.chunk_size : remain;

                if (!backend->chunk_write ||
                    backend->chunk_write(backend_state, data->chunk, data->bytes, size) != GS_FTP_RET_OK)
                    goto out_free;

                if (!backend->status_set ||
                    backend->status_set(backend_state, data->chunk) != GS_FTP_RET_OK)
                    goto out_free;

                csp_buffer_free(packet);
                break;
            }

            case FTP_STATUS_REQUEST: {
                /* Validate state */
                if (state != FTP_STATE_UPLOAD) {
                    log_warning("Status request received in state %"PRIu8, state);
                    goto out_free;
                }

                if (backend == NULL)
                    goto out_free;

                ftp_status_reply_t * status = &ftp_packet->statusrep;
                memset(status, 0, sizeof(*status));
                ftp_packet->type = FTP_STATUS_REPLY;

                if (backend->status_get) {
                    /* Build status reply */
                    int next = 0;
                    int count = 0;
                    const int chunks = (ul.size + ul.chunk_size - 1) / ul.chunk_size;

                    for (int i = 0; i < chunks; i++) {
                        int s;

                        /* Read chunk status */
                        status->ret = backend->status_get(backend_state, i, &s);
                        if (status->ret != GS_FTP_RET_OK) {
                            log_error("Status get failed");
                            goto out_free;
                        }

                        /* Increase complete counter if chunk was received */
                        if (s) status->complete++;

                        /* Add chunk status to packet */
                        if (status->entries < GS_FTP_STATUS_CHUNKS) {
                            if (!s) {
                                if (!count) next = i;
                                count++;
                            }

                            if (count > 0 && (s || i == chunks - 1)) {
                                status->entry[status->entries].next = csp_hton32(next);
                                status->entry[status->entries].count = csp_hton32(count);
                                status->entries++;
                            }
                        }
                    }

                    status->entries = csp_hton16(status->entries);
                    status->complete = csp_hton32(status->complete);
                    status->total = csp_hton32(chunks);
                    status->ret = GS_FTP_RET_OK;
                } else {
                    status->ret = GS_FTP_RET_NOTSUP;
                }

                /* Send reply */
                packet->length = sizeof(gs_ftp_type_t) + sizeof(ftp_status_reply_t);
                if (!csp_send(conn, packet, 100))
                    goto out_free;

                break;
            }

            case FTP_STATUS_REPLY: {
                /* Validate state */
                if (state != FTP_STATE_DOWNLOAD) {
                    log_warning("Status reply received in state %"PRIu8, state);
                    goto out_free;
                }

                if (backend == NULL)
                    goto out_free;

                ftp_status_reply_t * status = (ftp_status_reply_t *) &ftp_packet->statusrep;

                status->entries = csp_ntoh16(status->entries);
                status->complete = csp_ntoh32(status->complete);
                status->total = csp_ntoh32(status->total);

                log_info("Transfer Status: %"PRIu32" of %"PRIu32" (%.1f%%)",
                         status->complete, status->total,
                         gs_ftp_percent_completed(status->complete, status->total));

                if (status->complete != status->total) {
                    unsigned int i, j;
                    for (i = 0; i < status->entries; i++) {
                        status->entry[i].next = csp_ntoh32(status->entry[i].next);
                        status->entry[i].count = csp_ntoh32(status->entry[i].count);

                        for (j = 0; j < status->entry[i].count; j++) {

                            uint32_t remain = ftp_size - (status->entry[i].next + j) * dl.chunk_size;
                            unsigned int size = remain > dl.chunk_size ? dl.chunk_size : remain;
                            unsigned int length = sizeof(gs_ftp_type_t) + sizeof(uint32_t) + size;

                            csp_packet_t * data_packet = csp_buffer_get(length);
                            if (data_packet == NULL)
                                goto out_free;

                            ftp_packet_t * ftp_data_packet = (void *) data_packet->data;
                            ftp_data_packet->type = FTP_DATA;
                            ftp_data_packet->data.chunk = status->entry[i].next + j;

                            /* Read chunk */
                            if (backend->chunk_read(backend_state, ftp_data_packet->data.chunk, ftp_data_packet->data.bytes, size) != GS_FTP_RET_OK) {
                                log_error("Failed to read chunk");
                                ftp_data_packet->type = GS_FTP_RET_IO;
                                packet->length = length;
                                if (!csp_send(conn, data_packet, 60000)) {
                                    csp_buffer_free(data_packet);
                                    goto out_free;
                                }
                                goto out;
                            }

                            /* Convert */
                            ftp_data_packet->data.chunk = csp_hton32(ftp_data_packet->data.chunk);

                            /* Send packet */
                            data_packet->length = length;
                            if (!csp_send(conn, data_packet, 60000)) {
                                csp_buffer_free(data_packet);
                                goto out_free;
                            }

                        }
                    }
                }

                csp_buffer_free(packet);
                break;
            }

            case FTP_CRC_REQUEST: {
                /* Validate state */
                if ((state != FTP_STATE_UPLOAD) && (state != FTP_STATE_DOWNLOAD)) {
                    log_warning("CRC request received in state %"PRIu8, state);
                    goto out_free;
                }

                if (backend == NULL)
                    goto out_free;

                ftp_crc_reply_t * crc = (ftp_crc_reply_t *) &ftp_packet->crcrep;

                /* Let the backend do the work */
                int ret = GS_FTP_RET_NOTSUP;
                if (backend->crc) {
                    log_debug("CRC request: Calculate CRC");
                    uint32_t c;
                    ret = backend->crc(backend_state, &c);
                    crc->crc = csp_hton32(c);
                }

                /* Send back reply */
                log_debug("CRC request: Send reply");
                ftp_packet->type = FTP_CRC_REPLY;
                ftp_packet->crcrep.ret = ret;
                packet->length = sizeof(gs_ftp_type_t) + sizeof(ftp_crc_reply_t);

                if (!csp_send(conn, packet, 0))
                    goto out_free;

                break;
            }

            case FTP_ABORT: {
                /* Validate state */
                if (state == FTP_STATE_IDLE) {
                    log_warning("FTP ABORT received in state %"PRIu8, state);
                    goto out_free;
                }

                if (backend == NULL)
                    goto out_free;

                /* Let the backend do the work */
                if (backend->abort)
                    backend->abort(backend_state);

                goto out_free;
            }

            case FTP_DONE: {
                /* Validate state */
                if (state == FTP_STATE_IDLE) {
                    log_warning("FTP DONE received in state %"PRIu8, state);
                    goto out_free;
                }

                if (backend == NULL)
                    goto out_free;

                /* Let the backend do the work */
                if (backend->done)
                    backend->done(backend_state);

                goto out_free;
            }

            case FTP_LIST_REQUEST: {
                /* Validate state */
                if (state != FTP_STATE_IDLE) {
                    log_warning("List request received in state %"PRIu8, state);
                    goto out_free;
                }

                uint16_t entry_count = 0;

                backend = ftp_get_backend(ftp_packet->list.backend);
                if (backend == NULL)
                    goto out_free;

                if (backend->init) {
                    if (backend->init(&backend_state) != GS_FTP_RET_OK) {
                        log_error("Could not init backend");
                        goto out_free;
                    }
                }

                ftp_packet->type = FTP_LIST_REPLY;
                packet->length = sizeof(gs_ftp_type_t) + sizeof(ftp_list_reply_t);

                if (backend && backend->list && backend->entry) {
                    /* Read entry count */
                    ftp_packet->list.path[sizeof(ftp_packet->list.path) - 1] = 0; // ensure path is terminated
                    gs_ftp_return_t ret = backend->list(backend_state, ftp_packet->list.path, &entry_count);
                    ftp_packet->listrep.ret = ret;
                    ftp_packet->listrep.entries = csp_hton16(entry_count);

                    if (!csp_send(conn, packet, 0))
                        goto out_free;

                    if (ret == GS_FTP_RET_OK) {
                        /* Send entries */
                        int i;
                        for (i = 0; i < entry_count; i++) {

                            csp_packet_t * entry = csp_buffer_get(sizeof(gs_ftp_type_t) + sizeof(gs_ftp_list_entry_t));
                            if (entry == NULL)
                                goto out;

                            entry->length = sizeof(gs_ftp_type_t) + sizeof(gs_ftp_list_entry_t);
                            ftp_packet_t * entry_packet = (void *) entry->data;
                            entry_packet->type = FTP_LIST_ENTRY;

                            /* Get next entry */
                            if (backend->entry(backend_state, &entry_packet->listent) != GS_FTP_RET_OK)
                                goto out;

                            /* Pack data */
                            entry_packet->listent.entry = csp_hton16(i);
                            entry_packet->listent.size = csp_hton32(entry_packet->listent.size);

                            if (!csp_send(conn, entry, 30000)) {
                                log_error("Data transaction failed");
                                csp_buffer_free(entry);
                                goto out;
                            }

                        }

                        /* We need to call entry once more to free the dirp */
                        backend->entry(backend_state, NULL);
                    }

                } else {
                    ftp_packet->listrep.ret = GS_FTP_RET_NOTSUP;

                    if (!csp_send(conn, packet, 0))
                        goto out_free;
                }

                goto out;
            }

            case FTP_MOVE_REQUEST: {
                /* Validate state */
                if (state != FTP_STATE_IDLE) {
                    log_warning("Remove request received in state %"PRIu8, state);
                    goto out_free;
                }

                backend = ftp_get_backend(ftp_packet->move.backend);

                if (backend && backend->move) {
                    ftp_packet->move.from[sizeof(ftp_packet->move.from) - 1] = 0; // ensure path is terminated
                    ftp_packet->move.to[sizeof(ftp_packet->move.to) - 1] = 0; // ensure path is terminated
                    ftp_packet->moverep.ret = backend->move(backend_state, ftp_packet->move.from, ftp_packet->move.to);
                } else {
                    ftp_packet->moverep.ret = GS_FTP_RET_NOTSUP;
                }

                /* Send back reply */
                ftp_packet->type = FTP_MOVE_REPLY;
                packet->length = sizeof(gs_ftp_type_t) + sizeof(ftp_move_reply_t);

                if (!csp_send(conn, packet, 0))
                    goto out_free;

                goto out;
            }

            case FTP_COPY_REQUEST: {
                /* Validate state */
                if (state != FTP_STATE_IDLE) {
                    log_warning("Copy request received in state %"PRIu8, state);
                    goto out_free;
                }

                backend = ftp_get_backend(ftp_packet->copy.backend);

                if (backend && backend->copy) {
                    char * from = ftp_packet->copy.from;
                    char * to   = ftp_packet->copy.to;
                    ftp_packet->copyrep.ret = backend->copy(backend_state, from, to);
                } else {
                    ftp_packet->copyrep.ret = GS_FTP_RET_NOTSUP;
                }

                /* Send back reply */
                ftp_packet->type = FTP_COPY_REPLY;
                packet->length = sizeof(gs_ftp_type_t) + sizeof(ftp_copy_reply_t);

                if (!csp_send(conn, packet, 0))
                    goto out_free;

                goto out;
            }

            case FTP_REMOVE_REQUEST: {
                /* Validate state */
                if (state != FTP_STATE_IDLE) {
                    log_warning("Remove request received in state %"PRIu8, state);
                    goto out_free;
                }

                backend = ftp_get_backend(ftp_packet->remove.backend);

                if (backend && backend->remove) {
                    ftp_packet->remove.path[sizeof(ftp_packet->remove.path) - 1] = 0; // ensure path is terminated
                    ftp_packet->removerep.ret = backend->remove(backend_state, ftp_packet->remove.path);
                } else {
                    ftp_packet->removerep.ret = GS_FTP_RET_NOTSUP;
                }

                /* Send back reply */
                ftp_packet->type = FTP_REMOVE_REPLY;
                packet->length = sizeof(gs_ftp_type_t) + sizeof(ftp_remove_reply_t);

                if (!csp_send(conn, packet, 0))
                    goto out_free;

                goto out;
            }

            case FTP_MKFS_REQUEST: {
                /* Validate state */
                if (state != FTP_STATE_IDLE) {
                    log_warning("MKFS received in state %"PRIu8, state);
                    goto out_free;
                }

                backend = ftp_get_backend(ftp_packet->mkfs.backend);

                if (backend && backend->mkfs) {
                    ftp_packet->mkfs.path[sizeof(ftp_packet->mkfs.path) - 1] = 0; // ensure path is terminated
                    ftp_packet->mkfsrep.ret = backend->mkfs(backend_state, ftp_packet->mkfs.path, ftp_packet->mkfs.force);
                } else {
                    ftp_packet->mkfsrep.ret = GS_FTP_RET_NOTSUP;
                }

                /* Send back reply */
                ftp_packet->type = FTP_MKFS_REPLY;
                packet->length = sizeof(gs_ftp_type_t) + sizeof(ftp_mkfs_reply_t);

                if (!csp_send(conn, packet, 0))
                    goto out_free;

                goto out;
            }

            case FTP_MKDIR_REQUEST: {
                /* Validate state */
                if (state != FTP_STATE_IDLE) {
                    log_warning("MKDIR received in state %"PRIu8, state);
                    goto out_free;
                }

                backend = ftp_get_backend(ftp_packet->mkdir.backend);

                if (backend && backend->mkdir) {
                    ftp_packet->mkdir.path[sizeof(ftp_packet->mkdir.path) - 1] = 0; // ensure path is terminated
                    ftp_packet->mkdirrep.ret = backend->mkdir(backend_state,
                                                              ftp_packet->mkdir.path,
                                                              0777); /* mode is sent from the client in host order (not network order)
                                                                        hence, mode doesn't work when client/server are different endians */
                } else {
                    ftp_packet->mkdirrep.ret = GS_FTP_RET_NOTSUP;
                }

                /* Send back reply */
                ftp_packet->type = FTP_MKDIR_REPLY;
                packet->length = sizeof(gs_ftp_type_t) + sizeof(ftp_mkdir_reply_t);

                if (!csp_send(conn, packet, 0))
                    goto out_free;

                goto out;
            }

            case FTP_RMDIR_REQUEST: {
                /* Validate state */
                if (state != FTP_STATE_IDLE) {
                    log_warning("RMDIR received in state %"PRIu8, state);
                    goto out_free;
                }

                backend = ftp_get_backend(ftp_packet->rmdir.backend);

                if (backend && backend->rmdir) {
                    ftp_packet->rmdir.path[sizeof(ftp_packet->rmdir.path) - 1] = 0;
                    ftp_packet->rmdirrep.ret = backend->rmdir(backend_state,
                                                              ftp_packet->rmdir.path);
                } else {
                    ftp_packet->rmdirrep.ret = GS_FTP_RET_NOTSUP;
                }

                /* Send back reply */
                ftp_packet->type = FTP_RMDIR_REPLY;
                packet->length = sizeof(gs_ftp_type_t) + sizeof(ftp_rmdir_reply_t);

                if (!csp_send(conn, packet, 0))
                    goto out_free;

                goto out;
            }

            case FTP_ZIP_REQUEST: {
                /* Validate state */
                if (state != FTP_STATE_IDLE) {
                    log_warning("ZIP received in state %"PRIu8, state);
                    goto out_free;
                }

                backend = ftp_get_backend(ftp_packet->zip.backend);

                uint32_t comp_sz = 0;
                uint32_t decomp_sz = 0;
                uint8_t action = ftp_packet->zip.action;
                if (backend && backend->zip) {
                    ftp_packet->ziprep.ret = backend->zip(backend_state, ftp_packet->zip.src, ftp_packet->zip.dest, action, &comp_sz, &decomp_sz);
                } else {
                    ftp_packet->ziprep.ret = GS_FTP_RET_NOTSUP;
                }
                ftp_packet->ziprep.action = action;
                ftp_packet->ziprep.comp_sz = csp_hton32(comp_sz);
                ftp_packet->ziprep.decomp_sz = csp_hton32(decomp_sz);

                /* Send back reply */
                ftp_packet->type = FTP_ZIP_REPLY;
                packet->length = sizeof(gs_ftp_type_t) + sizeof(ftp_zip_reply_t);

                if (!csp_send(conn, packet, 0))
                    goto out_free;

                goto out;
            }

            default:
                log_warning("FTP received invalid type %"PRIu8, ftp_packet->type);
                goto out_free;
        }
    }

out_free:
    csp_buffer_free(packet);
out:
    csp_close(conn);
    if (backend && backend->timeout)
        backend->timeout(backend_state);
    if (backend_state && backend && backend->release)
        backend->release(backend_state);

    log_debug("exit FTP task");

    gs_thread_exit(0);
}

gs_error_t gs_ftp_csp_service_handler_set_stack_size(size_t size)
{
    stack_size = size;
    return GS_OK;
}

gs_error_t gs_ftp_csp_service_handler(csp_conn_t * conn)
{
    if (conn == NULL) {
        log_warning("%s: NULL connection", __FUNCTION__);
        return GS_ERROR_ARG;
    }

    char name[20];
    snprintf(name, sizeof(name), "FTP-%u:%u", csp_conn_src(conn), csp_conn_sport(conn));
    gs_error_t error = gs_thread_create(name, task_ftp, conn, stack_size, GS_THREAD_PRIORITY_LOW, 0, NULL);
    if (error) {
        csp_close(conn);
    }
    return error;
}
