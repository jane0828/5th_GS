// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <stdint.h>
// #include <inttypes.h>
// #include <pthread.h>
// #include <signal.h>
// #include <unistd.h>
// #include <time.h>
// #include <sys/stat.h>
// #include <sys/time.h>
// #include <ctime>
// #include <cstdlib>
// #include <iostream>

// /* CSP */
// #include <csp/csp.h>
// #include <csp/csp_error.h>
// #include <csp/interfaces/csp_if_kiss.h>
// #include <csp/drivers/usart.h>
// #include <csp_port.h>
// #include <csp/arch/csp_queue.h>
// #include <csp/arch/csp_semaphore.h>
// #include <csp/arch/csp_malloc.h>
// #include <csp/arch/csp_time.h>
// #include <csp_conn.h>
// #include <csp_io.h>
// #include <csp/csp_endian.h>
// #include <csp/delay.h>

// /* Drivers / Util */
// #include <gs/util/log.h>
// #include <gs/ftp/client.h>
// #include <gs/ftp/types.h>
// #include <gs/util/crc32.h>
// #include <gs/util/string.h>
// #include <gs/util/crc32.h>
// #include <gs/util/clock.h>
// #include <gs/util/vmem.h>
// #include <ryu_ftp_client_adapter.h>
// #include <ryu_ftp_connection_diag.h>

// #include "miman_config.h"
// #include "miman_csp.h"
// #include "miman_coms.h"
// #include "miman_imgui.h"
// #include "miman_ftp.h"
// #include "miman_ftprdp_integration.h"

// extern Console console;
// extern StateCheckUnit State;
// extern pthread_t p_thread[16];
// extern pthread_mutex_t conn_lock;
// extern Setup * setup;

// static unsigned int ftp_chunk_size = 180;
// static unsigned int ftp_backend = 3; // Use file backend as standard
// static const char * const packet_missing = "-";
// static const char * const packet_ok = "+";

// char flistbuffer[16384];

// static pthread_mutex_t ftp_transfer_status_lock = PTHREAD_MUTEX_INITIALIZER;
// static uint32_t ftp_transfer_id_next = 1;
// static miman_ftp_transfer_status_t ftp_transfer_last_status;
// static const uint32_t MIMAN_FTP_WORKER_ARG_MAGIC = 0x46545052u;

// enum miman_ftp_engine_t {
//     MIMAN_FTP_ENGINE_OLD = 0,
//     MIMAN_FTP_ENGINE_RYU = 1,
// };

// enum miman_ftp_direction_t {
//     MIMAN_FTP_DIRECTION_DOWNLOAD = 0,
//     MIMAN_FTP_DIRECTION_UPLOAD = 1,
// };

// struct miman_ftp_worker_copy_t {
//     ftpinfo paths;
//     gs_ftp_settings_t settings;
//     miman_ftp_engine_t engine;
//     miman_ftp_direction_t direction;
//     uint32_t transfer_id;
// };

// struct miman_ftp_worker_arg {
//     uint32_t magic;
//     uint32_t magic_inverse;
//     ftpinfo paths;
// };

// struct miman_ftp_callback_context_t {
//     uint32_t transfer_id;
//     const char * engine_name;
//     const char * direction_name;
// };

// static const char * ftp_engine_name(miman_ftp_engine_t engine)
// {
//     return (engine == MIMAN_FTP_ENGINE_RYU) ? "RYU" : "OLD";
// }

// static const char * ftp_direction_name(miman_ftp_direction_t direction)
// {
//     return (direction == MIMAN_FTP_DIRECTION_UPLOAD) ? "UPLOAD" : "DOWNLOAD";
// }

// static uint32_t ftp_next_transfer_id(void)
// {
//     uint32_t transfer_id;

//     pthread_mutex_lock(&ftp_transfer_status_lock);
//     transfer_id = ftp_transfer_id_next++;
//     if (ftp_transfer_id_next == 0) {
//         ftp_transfer_id_next = 1;
//     }
//     pthread_mutex_unlock(&ftp_transfer_status_lock);

//     return transfer_id;
// }

// static void ftp_copy_paths(ftpinfo * destination, const ftpinfo * source)
// {
//     memset(destination, 0, sizeof(*destination));
//     memcpy(destination, source, sizeof(*destination));
//     destination->local_path[sizeof(destination->local_path) - 1] = '\0';
//     destination->remote_path[sizeof(destination->remote_path) - 1] = '\0';
// }

// miman_ftp_worker_arg_t * miman_ftp_create_worker_arg(const char * local_path,
//                                                      const char * remote_path)
// {
//     miman_ftp_worker_arg_t * arg =
//         (miman_ftp_worker_arg_t *) calloc(1, sizeof(*arg));
//     if (arg == NULL) {
//         return NULL;
//     }

//     arg->magic = MIMAN_FTP_WORKER_ARG_MAGIC;
//     arg->magic_inverse = ~MIMAN_FTP_WORKER_ARG_MAGIC;
//     if (local_path != NULL) {
//         snprintf(arg->paths.local_path, sizeof(arg->paths.local_path), "%s",
//                  local_path);
//     }
//     if (remote_path != NULL) {
//         snprintf(arg->paths.remote_path, sizeof(arg->paths.remote_path), "%s",
//                  remote_path);
//     }

//     return arg;
// }

// void miman_ftp_destroy_worker_arg(miman_ftp_worker_arg_t * arg)
// {
//     if (arg == NULL) {
//         return;
//     }

//     arg->magic = 0;
//     arg->magic_inverse = 0;
//     free(arg);
// }

// static bool ftp_is_owned_worker_arg(const void * param)
// {
//     const miman_ftp_worker_arg_t * arg =
//         (const miman_ftp_worker_arg_t *) param;

//     return (arg->magic == MIMAN_FTP_WORKER_ARG_MAGIC) &&
//            (arg->magic_inverse == ~MIMAN_FTP_WORKER_ARG_MAGIC);
// }

// static void ftp_store_transfer_status(const miman_ftp_worker_copy_t * job,
//                                       int return_code,
//                                       bool cleanup_done)
// {
//     pthread_mutex_lock(&ftp_transfer_status_lock);
//     ftp_transfer_last_status.transfer_id = job->transfer_id;
//     ftp_transfer_last_status.return_code = return_code;
//     ftp_transfer_last_status.cleanup_done = cleanup_done ? 1 : 0;
//     snprintf(ftp_transfer_last_status.engine,
//              sizeof(ftp_transfer_last_status.engine), "%s",
//              ftp_engine_name(job->engine));
//     snprintf(ftp_transfer_last_status.direction,
//              sizeof(ftp_transfer_last_status.direction), "%s",
//              ftp_direction_name(job->direction));
//     snprintf(ftp_transfer_last_status.local_path,
//              sizeof(ftp_transfer_last_status.local_path), "%s",
//              job->paths.local_path);
//     snprintf(ftp_transfer_last_status.remote_path,
//              sizeof(ftp_transfer_last_status.remote_path), "%s",
//              job->paths.remote_path);
//     pthread_mutex_unlock(&ftp_transfer_status_lock);
// }

// int miman_ftp_get_last_transfer_status(miman_ftp_transfer_status_t * status)
// {
//     if (status == NULL) {
//         return -1;
//     }

//     pthread_mutex_lock(&ftp_transfer_status_lock);
//     *status = ftp_transfer_last_status;
//     pthread_mutex_unlock(&ftp_transfer_status_lock);

//     return 0;
// }

// static bool ftp_copy_worker_param(void * param, miman_ftp_worker_copy_t * job,
//                                   miman_ftp_engine_t engine,
//                                   miman_ftp_direction_t direction)
// {
//     if ((param == NULL) || (job == NULL)) {
//         console.AddLog("[ERROR]##FTP_%s_%s missing worker argument.",
//                        ftp_engine_name(engine), ftp_direction_name(direction));
//         return false;
//     }

//     const ftpinfo * source = NULL;
//     miman_ftp_worker_arg_t * owned_arg = NULL;

//     if (ftp_is_owned_worker_arg(param)) {
//         owned_arg = (miman_ftp_worker_arg_t *) param;
//         source = &owned_arg->paths;
//     }
//     else {
//         source = (const ftpinfo *) param;
//         console.AddLog("[WARN]##FTP_%s_%s received legacy shared ftpinfo argument; "
//                        "pre-start pthread_create race remains until caller uses "
//                        "miman_ftp_create_worker_arg().",
//                        ftp_engine_name(engine), ftp_direction_name(direction));
//     }

//     memset(job, 0, sizeof(*job));
//     ftp_copy_paths(&job->paths, source);
//     if (owned_arg != NULL) {
//         miman_ftp_destroy_worker_arg(owned_arg);
//     }
//     job->settings.mode = GS_FTP_MODE_STANDARD;
//     job->settings.host = setup->obc_node;
//     job->settings.port = FTPRDP_PORT;
//     job->settings.timeout = 30000;
//     job->settings.chunk_size = State.chunk_sz;
//     job->engine = engine;
//     job->direction = direction;
//     job->transfer_id = ftp_next_transfer_id();
//     ftp_store_transfer_status(job, 0, false);

//     return true;
// }

// static void ftp_transfer_callback(const gs_ftp_info_t * info)
// {
//     if (info == NULL) {
//         console.AddLog("[ERROR]##FTP callback invoked with NULL info.");
//         return;
//     }

//     const miman_ftp_callback_context_t * context =
//         (const miman_ftp_callback_context_t *) info->user_data;

//     ftp_callback(info);

//     if (context == NULL) {
//         return;
//     }

//     switch (info->type) {
//         case GS_FTP_INFO_UL_PROGRESS:
//         case GS_FTP_INFO_DL_PROGRESS:
//             printftp("[FTP_%s_TEST][id=%u][%s] progress %u/%u chunk=%u",
//                      context->engine_name, context->transfer_id,
//                      context->direction_name,
//                      info->u.progress.current_chunk,
//                      info->u.progress.total_chunks,
//                      info->u.progress.chunk_size);
//             break;
//         case GS_FTP_INFO_CRC:
//             console.AddLog("[FTP_%s_TEST][id=%u][%s] CRC remote=%u local=%u",
//                            context->engine_name, context->transfer_id,
//                            context->direction_name,
//                            info->u.crc.remote, info->u.crc.local);
//             break;
//         default:
//             break;
//     }
// }

// static int ftp_run_transfer(const miman_ftp_worker_copy_t * job)
// {
//     miman_ftp_callback_context_t callback_context;
//     callback_context.transfer_id = job->transfer_id;
//     callback_context.engine_name = ftp_engine_name(job->engine);
//     callback_context.direction_name = ftp_direction_name(job->direction);

//     console.AddLog("[FTP_%s_TEST][id=%u][%s] begin local=%s remote=%s host=%u port=%u chunk=%u",
//                    callback_context.engine_name, job->transfer_id,
//                    callback_context.direction_name,
//                    job->paths.local_path, job->paths.remote_path,
//                    job->settings.host, job->settings.port,
//                    job->settings.chunk_size);

//     if (job->engine == MIMAN_FTP_ENGINE_RYU) {
//         ryu_ftp_diag_set_transfer(job->transfer_id,
//                                   callback_context.engine_name,
//                                   callback_context.direction_name);
//         ryu_ftp_diag_dump("baseline");

//         int status;
//         if (job->direction == MIMAN_FTP_DIRECTION_UPLOAD) {
//             status = (int) ryu_ftp_upload(&job->settings,
//                                           job->paths.local_path,
//                                           job->paths.remote_path,
//                                           ftp_transfer_callback,
//                                           &callback_context, true, 0);
//         }
//         else {
//             status = (int) ryu_ftp_download(&job->settings,
//                                             job->paths.local_path,
//                                             job->paths.remote_path,
//                                             ftp_transfer_callback,
//                                             &callback_context, true);
//         }

//         ryu_ftp_diag_dump("complete+0s");
// #ifdef FTP_RYU_CONN_DIAG
//         sleep(5);
//         ryu_ftp_diag_dump("complete+5s");
//         sleep(5);
//         ryu_ftp_diag_dump("complete+10s");
//         sleep(20);
//         ryu_ftp_diag_dump("complete+30s");
// #endif
//         ryu_ftp_diag_clear_transfer();
//         return status;
//     }

//     if (job->direction == MIMAN_FTP_DIRECTION_UPLOAD) {
//         return (int) gs_ftp_upload(&job->settings, job->paths.local_path,
//                                    job->paths.remote_path,
//                                    ftp_transfer_callback, &callback_context);
//     }

//     return (int) gs_ftp_download(&job->settings, job->paths.local_path,
//                                  job->paths.remote_path,
//                                  ftp_transfer_callback, &callback_context);
// }

// static void * ftp_transfer_onorbit(void * param,
//                                    miman_ftp_engine_t engine,
//                                    miman_ftp_direction_t direction)
// {
//     if (engine == MIMAN_FTP_ENGINE_OLD) {
//         ftp_avail();
//     }
//     bool dlstate = State.downlink_mode;
//     miman_ftp_worker_copy_t job;

//     if (!ftp_copy_worker_param(param, &job, engine, direction)) {
//         return NULL;
//     }

//     if((dlstate))
//         State.downlink_mode = false;
//     while(!State.uplink_mode)
//         continue;

//     printftp("Start FTP %s %s id=%u.",
//              ftp_engine_name(engine), ftp_direction_name(direction),
//              job.transfer_id);
//     State.downlink_mode = false;
//     State.ftp_mode = true;
//     int status = ftp_run_transfer(&job);
//     bool cleanup_done = false;

//     if (status != 0) {
//         console.AddLog("[FTP_%s_TEST][id=%u][%s][ERROR] retcode=%d",
//                        ftp_engine_name(engine), job.transfer_id,
//                        ftp_direction_name(direction), status);
//     }
//     else {
//         console.AddLog("[FTP_%s_TEST][id=%u][%s][OK] retcode=%d",
//                        ftp_engine_name(engine), job.transfer_id,
//                        ftp_direction_name(direction), status);
//     }

//     if (engine == MIMAN_FTP_ENGINE_OLD) {
//         ftp_abort();
//     }
//     cleanup_done = true;
//     State.ftp_mode = false;
//     ftp_store_transfer_status(&job, status, cleanup_done);
//     console.AddLog("[FTP_%s_TEST][id=%u][%s] cleanup_done=%d",
//                    ftp_engine_name(engine), job.transfer_id,
//                    ftp_direction_name(direction), cleanup_done ? 1 : 0);
//     if(dlstate)
//     {
//         State.downlink_mode = dlstate;
//     }
//     return NULL;
// }
// int ftp_list_callback(uint16_t entries, const gs_ftp_list_entry_t * listent, void * data)
// {
//     char pathbuf[256];
//     time_t tmtime = time(0);
//     struct tm * local = localtime(&tmtime);
//     sprintf(pathbuf, "./data/listup/Listup--%04d-%02d-%02d-%02d-", local->tm_year+1900, local->tm_mon+1, local->tm_mday,local->tm_hour);
//     strcat(pathbuf, State.gslistup->fpathbuf);
//     State.gslistup->fd = fopen(pathbuf, "wb");

//     memset(State.gslistup->flistbuf, 0, sizeof(State.gslistup->flistbuf));
//     if(listent->type == GS_FTP_LIST_FILE)
//         sprintf(State.gslistup->flistbuf, "Type : %s\tSize : %u\tName : %s\n", "F", listent->size, listent->path);
//     else
//         sprintf(State.gslistup->flistbuf, "Type : %s\tSize : %u\tName : %s\n", "D", listent->size, listent->path);
//     fprintf(State.gslistup->fd, State.gslistup->flistbuf);
//     strcat(State.gslistup->fdispbuf, State.gslistup->flistbuf);
//     fclose(State.gslistup->fd);
// }

// void * ftp_downlink_onorbit(void * param){
//     return ftp_transfer_onorbit(param, MIMAN_FTP_ENGINE_OLD,
//                                 MIMAN_FTP_DIRECTION_DOWNLOAD);
// }

// void * ftp_uplink_onorbit(void * param){
//     return ftp_transfer_onorbit(param, MIMAN_FTP_ENGINE_OLD,
//                                 MIMAN_FTP_DIRECTION_UPLOAD);
// }

// void * ftp_ryu_downlink_onorbit(void * param){
//     return ftp_transfer_onorbit(param, MIMAN_FTP_ENGINE_RYU,
//                                 MIMAN_FTP_DIRECTION_DOWNLOAD);
// }

// void * ftp_ryu_uplink_onorbit(void * param){
//     return ftp_transfer_onorbit(param, MIMAN_FTP_ENGINE_RYU,
//                                 MIMAN_FTP_DIRECTION_UPLOAD);
// }


// void * ftp_list_onorbit(void *){
//     gs_ftp_settings_t ftp_config;
//     ftp_config.mode = GS_FTP_MODE_STANDARD;
//     ftp_config.host = setup->obc_node;
//     ftp_config.port = FTPRDP_PORT;
//     ftp_config.timeout = setup->default_timeout + setup->guard_delay; //default timeout value
//     ftp_config.chunk_size = 200; //default chunk size value
//     int status = (int)gs_ftp_list(&ftp_config, State.gslistup->fpathbuf, ftp_list_callback, NULL);
    
//     if (status != 0) {
// 		console.AddLog("[ERROR]##Fail to call FTP list. Retcode : %d", status);
// 	}
//     else
//     {
//         console.AddLog("Call FTP list. Retcode : %d", status);
//         printf("%s", flistbuffer);
//     }
// }

// void * ftp_move_onorbit(void *){
//     gs_ftp_settings_t ftp_config;
//     ftp_config.mode = GS_FTP_MODE_STANDARD;
//     ftp_config.host = setup->obc_node;
//     ftp_config.port = FTPRDP_PORT;
//     ftp_config.timeout = setup->default_timeout + setup->guard_delay; //default timeout value
//     ftp_config.chunk_size = 200; //default chunk size value
//     int status = (int)gs_ftp_move(&ftp_config, State.gsmove->from, State.gsmove->to);

//     if (status != 0) {
// 		console.AddLog("[ERROR]##Fail to FTP move. Retcode : %d", status);
// 	}
//     else
//     {
//         console.AddLog("[OK]##FTP move done. Retcode : %d", status);
//     }
// }

// void * ftp_remove_onorbit(void *){
//     gs_ftp_settings_t ftp_config;
//     ftp_config.mode = GS_FTP_MODE_STANDARD;
//     ftp_config.host = setup->obc_node;
//     ftp_config.port = FTPRDP_PORT;
//     ftp_config.timeout = setup->default_timeout + setup->guard_delay; //default timeout value
//     ftp_config.chunk_size = 200; //default chunk size value
//     int status = (int)gs_ftp_remove(&ftp_config, State.gsremove->path);

//     if (status != 0) {
// 		console.AddLog("[ERROR]##Fail to FTP remove. Retcode : %d", status);
// 	}
//     else
//     {
//         console.AddLog("[OK]##FTP remove done. Retcode : %d", status);
//     }
// }

// void * ftp_copy_onorbit(void *){
//     gs_ftp_settings_t ftp_config;
//     ftp_config.mode = GS_FTP_MODE_STANDARD;
//     ftp_config.host = setup->obc_node;
//     ftp_config.port = FTPRDP_PORT;
//     ftp_config.timeout = setup->default_timeout + setup->guard_delay; //default timeout value
//     ftp_config.chunk_size = 200; //default chunk size value
//     int status = (int)gs_ftp_copy(&ftp_config, State.gscopy->from, State.gscopy->to);

//     if (status != 0) {
// 		console.AddLog("[ERROR]##Fail to FTP copy. Retcode : %d", status);
// 	}
//     else
//     {
//         console.AddLog("FTP copy done. Retcode : %d", status);
//     }
// }

// void * ftp_mkdir_onorbit(void *){
//     uint32_t mode = 0;
//     gs_ftp_settings_t ftp_config;
//     ftp_config.mode = GS_FTP_MODE_STANDARD;
//     ftp_config.host = setup->obc_node;
//     ftp_config.port = FTPRDP_PORT;
//     ftp_config.timeout = setup->default_timeout + setup->guard_delay; //default timeout value
//     ftp_config.chunk_size = 200; //default chunk size value
//     int status = (int)gs_ftp_mkdir(&ftp_config, State.gsmkdir->path, mode);

//     if (status != 0) {
// 		console.AddLog("[ERROR]##Fail to FTP make directory. Retcode : %d", status);
// 	}
//     else
//     {
//         console.AddLog("[OK]##FTP make directory done. Retcode : %d", status);
//     }
// }

// void * ftp_rmdir_onorbit(void *){
//     gs_ftp_settings_t ftp_config;
//     ftp_config.mode = GS_FTP_MODE_STANDARD;
//     ftp_config.host = setup->obc_node;
//     ftp_config.port = FTPRDP_PORT;
//     ftp_config.timeout = setup->default_timeout + setup->guard_delay; //default timeout value
//     ftp_config.chunk_size = 200; //default chunk size value
//     int status = (int)gs_ftp_rmdir(&ftp_config, State.gsrmdir->path);

//     if (status != 0) {
// 		console.AddLog("[ERROR]##Fail to FTP remove directory. Retcode : %d", status);
// 	}
//     else
//     {
//         console.AddLog("[OK]##FTP remove directory done. Retcode : %d", status);
//     }
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <ctime>
#include <cstdlib>
#include <iostream>

/* CSP */
#include <csp/csp.h>
#include <csp/csp_error.h>
#include <csp/interfaces/csp_if_kiss.h>
#include <csp/drivers/usart.h>
#include <csp_port.h>
#include <csp/arch/csp_queue.h>
#include <csp/arch/csp_semaphore.h>
#include <csp/arch/csp_malloc.h>
#include <csp/arch/csp_time.h>
#include <csp_conn.h>
#include <csp_io.h>
#include <csp/csp_endian.h>
#include <csp/delay.h>

/* Drivers / Util */
#include <gs/util/log.h>
#include <gs/ftp/client.h>
#include <gs/ftp/types.h>
#include <gs/util/crc32.h>
#include <gs/util/string.h>
#include <gs/util/crc32.h>
#include <gs/util/clock.h>
#include <gs/util/vmem.h>
#include <ryu_ftp_client_adapter.h>
#include <ryu_ftp_connection_diag.h>

#include "miman_config.h"
#include "miman_csp.h"
#include "miman_coms.h"
#include "miman_imgui.h"
#include "miman_ftp.h"
#include "miman_ftprdp_integration.h"

extern Console console;
extern StateCheckUnit State;
extern pthread_t p_thread[16];
extern pthread_mutex_t conn_lock;
extern Setup * setup;

static unsigned int ftp_chunk_size = 180;
static unsigned int ftp_backend = 3; // Use file backend as standard
static const char * const packet_missing = "-";
static const char * const packet_ok = "+";

char flistbuffer[16384];

static pthread_mutex_t ftp_transfer_status_lock = PTHREAD_MUTEX_INITIALIZER;
static uint32_t ftp_transfer_id_next = 1;
static miman_ftp_transfer_status_t ftp_transfer_last_status;
static const uint32_t MIMAN_FTP_WORKER_ARG_MAGIC = 0x46545052u;

enum miman_ftp_engine_t {
    MIMAN_FTP_ENGINE_OLD = 0,
    MIMAN_FTP_ENGINE_RYU = 1,
};

enum miman_ftp_direction_t {
    MIMAN_FTP_DIRECTION_DOWNLOAD = 0,
    MIMAN_FTP_DIRECTION_UPLOAD = 1,
};

/*
 * FTP/RDP service ports are separated by FTP engine.
 *
 * OLD engine : CSP port 9
 * RYU engine : CSP port 19
 */
static const uint8_t FTP_RDP_PORT_OLD = 9;
static const uint8_t FTP_RDP_PORT_RYU = 19;

static uint8_t ftp_rdp_port_for_engine(miman_ftp_engine_t engine)
{
    switch (engine) {
        case MIMAN_FTP_ENGINE_RYU:
            return FTP_RDP_PORT_RYU;

        case MIMAN_FTP_ENGINE_OLD:
        default:
            return FTP_RDP_PORT_OLD;
    }
}

struct miman_ftp_worker_copy_t {
    ftpinfo paths;
    gs_ftp_settings_t settings;
    miman_ftp_engine_t engine;
    miman_ftp_direction_t direction;
    uint32_t transfer_id;
};

struct miman_ftp_worker_arg {
    uint32_t magic;
    uint32_t magic_inverse;
    ftpinfo paths;
};

struct miman_ftp_callback_context_t {
    uint32_t transfer_id;
    const char * engine_name;
    const char * direction_name;
};

static const char * ftp_engine_name(miman_ftp_engine_t engine)
{
    return (engine == MIMAN_FTP_ENGINE_RYU) ? "RYU" : "OLD";
}

static const char * ftp_direction_name(miman_ftp_direction_t direction)
{
    return (direction == MIMAN_FTP_DIRECTION_UPLOAD) ? "UPLOAD" : "DOWNLOAD";
}

static uint32_t ftp_next_transfer_id(void)
{
    uint32_t transfer_id;

    pthread_mutex_lock(&ftp_transfer_status_lock);
    transfer_id = ftp_transfer_id_next++;
    if (ftp_transfer_id_next == 0) {
        ftp_transfer_id_next = 1;
    }
    pthread_mutex_unlock(&ftp_transfer_status_lock);

    return transfer_id;
}

static void ftp_copy_paths(ftpinfo * destination, const ftpinfo * source)
{
    memset(destination, 0, sizeof(*destination));
    memcpy(destination, source, sizeof(*destination));
    destination->local_path[sizeof(destination->local_path) - 1] = '\0';
    destination->remote_path[sizeof(destination->remote_path) - 1] = '\0';
}

miman_ftp_worker_arg_t * miman_ftp_create_worker_arg(const char * local_path,
                                                     const char * remote_path)
{
    miman_ftp_worker_arg_t * arg =
        (miman_ftp_worker_arg_t *) calloc(1, sizeof(*arg));
    if (arg == NULL) {
        return NULL;
    }

    arg->magic = MIMAN_FTP_WORKER_ARG_MAGIC;
    arg->magic_inverse = ~MIMAN_FTP_WORKER_ARG_MAGIC;
    if (local_path != NULL) {
        snprintf(arg->paths.local_path, sizeof(arg->paths.local_path), "%s",
                 local_path);
    }
    if (remote_path != NULL) {
        snprintf(arg->paths.remote_path, sizeof(arg->paths.remote_path), "%s",
                 remote_path);
    }

    return arg;
}

void miman_ftp_destroy_worker_arg(miman_ftp_worker_arg_t * arg)
{
    if (arg == NULL) {
        return;
    }

    arg->magic = 0;
    arg->magic_inverse = 0;
    free(arg);
}

static bool ftp_is_owned_worker_arg(const void * param)
{
    const miman_ftp_worker_arg_t * arg =
        (const miman_ftp_worker_arg_t *) param;

    return (arg->magic == MIMAN_FTP_WORKER_ARG_MAGIC) &&
           (arg->magic_inverse == ~MIMAN_FTP_WORKER_ARG_MAGIC);
}

static void ftp_store_transfer_status(const miman_ftp_worker_copy_t * job,
                                      int return_code,
                                      bool cleanup_done)
{
    pthread_mutex_lock(&ftp_transfer_status_lock);
    ftp_transfer_last_status.transfer_id = job->transfer_id;
    ftp_transfer_last_status.return_code = return_code;
    ftp_transfer_last_status.cleanup_done = cleanup_done ? 1 : 0;
    snprintf(ftp_transfer_last_status.engine,
             sizeof(ftp_transfer_last_status.engine), "%s",
             ftp_engine_name(job->engine));
    snprintf(ftp_transfer_last_status.direction,
             sizeof(ftp_transfer_last_status.direction), "%s",
             ftp_direction_name(job->direction));
    snprintf(ftp_transfer_last_status.local_path,
             sizeof(ftp_transfer_last_status.local_path), "%s",
             job->paths.local_path);
    snprintf(ftp_transfer_last_status.remote_path,
             sizeof(ftp_transfer_last_status.remote_path), "%s",
             job->paths.remote_path);
    pthread_mutex_unlock(&ftp_transfer_status_lock);
}

int miman_ftp_get_last_transfer_status(miman_ftp_transfer_status_t * status)
{
    if (status == NULL) {
        return -1;
    }

    pthread_mutex_lock(&ftp_transfer_status_lock);
    *status = ftp_transfer_last_status;
    pthread_mutex_unlock(&ftp_transfer_status_lock);

    return 0;
}

static bool ftp_copy_worker_param(void * param, miman_ftp_worker_copy_t * job,
                                  miman_ftp_engine_t engine,
                                  miman_ftp_direction_t direction)
{
    if ((param == NULL) || (job == NULL)) {
        console.AddLog("[ERROR]##FTP_%s_%s missing worker argument.",
                       ftp_engine_name(engine), ftp_direction_name(direction));
        return false;
    }

    const ftpinfo * source = NULL;
    miman_ftp_worker_arg_t * owned_arg = NULL;

    if (ftp_is_owned_worker_arg(param)) {
        owned_arg = (miman_ftp_worker_arg_t *) param;
        source = &owned_arg->paths;
    }
    else {
        source = (const ftpinfo *) param;
        console.AddLog("[WARN]##FTP_%s_%s received legacy shared ftpinfo argument; "
                       "pre-start pthread_create race remains until caller uses "
                       "miman_ftp_create_worker_arg().",
                       ftp_engine_name(engine), ftp_direction_name(direction));
    }

    memset(job, 0, sizeof(*job));
    ftp_copy_paths(&job->paths, source);
    if (owned_arg != NULL) {
        miman_ftp_destroy_worker_arg(owned_arg);
    }
    job->settings.mode = GS_FTP_MODE_STANDARD;
    job->settings.host = setup->obc_node;
    job->settings.port = ftp_rdp_port_for_engine(engine);
    job->settings.timeout = 30000;
    job->settings.chunk_size = State.chunk_sz;
    job->engine = engine;
    job->direction = direction;
    job->transfer_id = ftp_next_transfer_id();
    ftp_store_transfer_status(job, 0, false);

    return true;
}

static void ftp_transfer_callback(const gs_ftp_info_t * info)
{
    if (info == NULL) {
        console.AddLog("[ERROR]##FTP callback invoked with NULL info.");
        return;
    }

    const miman_ftp_callback_context_t * context =
        (const miman_ftp_callback_context_t *) info->user_data;

    ftp_callback(info);

    if (context == NULL) {
        return;
    }

    switch (info->type) {
        case GS_FTP_INFO_UL_PROGRESS:
        case GS_FTP_INFO_DL_PROGRESS:
            printftp("[FTP_%s_TEST][id=%u][%s] progress %u/%u chunk=%u",
                     context->engine_name, context->transfer_id,
                     context->direction_name,
                     info->u.progress.current_chunk,
                     info->u.progress.total_chunks,
                     info->u.progress.chunk_size);
            break;
        case GS_FTP_INFO_CRC:
            console.AddLog("[FTP_%s_TEST][id=%u][%s] CRC remote=%u local=%u",
                           context->engine_name, context->transfer_id,
                           context->direction_name,
                           info->u.crc.remote, info->u.crc.local);
            break;
        default:
            break;
    }
}

static int ftp_run_transfer(const miman_ftp_worker_copy_t * job)
{
    miman_ftp_callback_context_t callback_context;
    callback_context.transfer_id = job->transfer_id;
    callback_context.engine_name = ftp_engine_name(job->engine);
    callback_context.direction_name = ftp_direction_name(job->direction);

    console.AddLog("[FTP_%s_TEST][id=%u][%s] begin local=%s remote=%s host=%u port=%u chunk=%u",
                   callback_context.engine_name, job->transfer_id,
                   callback_context.direction_name,
                   job->paths.local_path, job->paths.remote_path,
                   job->settings.host, job->settings.port,
                   job->settings.chunk_size);

    if (job->engine == MIMAN_FTP_ENGINE_RYU) {
        ryu_ftp_diag_set_transfer(job->transfer_id,
                                  callback_context.engine_name,
                                  callback_context.direction_name);
        ryu_ftp_diag_dump("baseline");

        int status;
        if (job->direction == MIMAN_FTP_DIRECTION_UPLOAD) {
            status = (int) ryu_ftp_upload(&job->settings,
                                          job->paths.local_path,
                                          job->paths.remote_path,
                                          ftp_transfer_callback,
                                          &callback_context, true, 0);
        }
        else {
            status = (int) ryu_ftp_download(&job->settings,
                                            job->paths.local_path,
                                            job->paths.remote_path,
                                            ftp_transfer_callback,
                                            &callback_context, true);
        }

        ryu_ftp_diag_dump("complete+0s");
#ifdef FTP_RYU_CONN_DIAG
        sleep(5);
        ryu_ftp_diag_dump("complete+5s");
        sleep(5);
        ryu_ftp_diag_dump("complete+10s");
        sleep(20);
        ryu_ftp_diag_dump("complete+30s");
#endif
        ryu_ftp_diag_clear_transfer();
        return status;
    }

    if (job->direction == MIMAN_FTP_DIRECTION_UPLOAD) {
        return (int) gs_ftp_upload(&job->settings, job->paths.local_path,
                                   job->paths.remote_path,
                                   ftp_transfer_callback, &callback_context);
    }

    return (int) gs_ftp_download(&job->settings, job->paths.local_path,
                                 job->paths.remote_path,
                                 ftp_transfer_callback, &callback_context);
}

static void * ftp_transfer_onorbit(void * param,
                                   miman_ftp_engine_t engine,
                                   miman_ftp_direction_t direction)
{
    if (engine == MIMAN_FTP_ENGINE_OLD) {
        ftp_avail();
    }
    bool dlstate = State.downlink_mode;
    miman_ftp_worker_copy_t job;

    if (!ftp_copy_worker_param(param, &job, engine, direction)) {
        return NULL;
    }

    if((dlstate))
        State.downlink_mode = false;
    while(!State.uplink_mode)
        continue;

    printftp("Start FTP %s %s id=%u.",
             ftp_engine_name(engine), ftp_direction_name(direction),
             job.transfer_id);
    State.downlink_mode = false;
    State.ftp_mode = true;
    int status = ftp_run_transfer(&job);
    bool cleanup_done = false;

    if (status != 0) {
        console.AddLog("[FTP_%s_TEST][id=%u][%s][ERROR] retcode=%d",
                       ftp_engine_name(engine), job.transfer_id,
                       ftp_direction_name(direction), status);
    }
    else {
        console.AddLog("[FTP_%s_TEST][id=%u][%s][OK] retcode=%d",
                       ftp_engine_name(engine), job.transfer_id,
                       ftp_direction_name(direction), status);
    }

    if (engine == MIMAN_FTP_ENGINE_OLD) {
        ftp_abort();
    }
    cleanup_done = true;
    State.ftp_mode = false;
    ftp_store_transfer_status(&job, status, cleanup_done);
    console.AddLog("[FTP_%s_TEST][id=%u][%s] cleanup_done=%d",
                   ftp_engine_name(engine), job.transfer_id,
                   ftp_direction_name(direction), cleanup_done ? 1 : 0);
    if(dlstate)
    {
        State.downlink_mode = dlstate;
    }
    return NULL;
}
int ftp_list_callback(uint16_t entries, const gs_ftp_list_entry_t * listent, void * data)
{
    char pathbuf[256];
    time_t tmtime = time(0);
    struct tm * local = localtime(&tmtime);
    sprintf(pathbuf, "./data/listup/Listup--%04d-%02d-%02d-%02d-", local->tm_year+1900, local->tm_mon+1, local->tm_mday,local->tm_hour);
    strcat(pathbuf, State.gslistup->fpathbuf);
    State.gslistup->fd = fopen(pathbuf, "wb");

    memset(State.gslistup->flistbuf, 0, sizeof(State.gslistup->flistbuf));
    if(listent->type == GS_FTP_LIST_FILE)
        sprintf(State.gslistup->flistbuf, "Type : %s\tSize : %u\tName : %s\n", "F", listent->size, listent->path);
    else
        sprintf(State.gslistup->flistbuf, "Type : %s\tSize : %u\tName : %s\n", "D", listent->size, listent->path);
    fprintf(State.gslistup->fd, State.gslistup->flistbuf);
    strcat(State.gslistup->fdispbuf, State.gslistup->flistbuf);
    fclose(State.gslistup->fd);
}

void * ftp_downlink_onorbit(void * param){
    return ftp_transfer_onorbit(param, MIMAN_FTP_ENGINE_OLD,
                                MIMAN_FTP_DIRECTION_DOWNLOAD);
}

void * ftp_uplink_onorbit(void * param){
    return ftp_transfer_onorbit(param, MIMAN_FTP_ENGINE_OLD,
                                MIMAN_FTP_DIRECTION_UPLOAD);
}

void * ftp_ryu_downlink_onorbit(void * param){
    return ftp_transfer_onorbit(param, MIMAN_FTP_ENGINE_RYU,
                                MIMAN_FTP_DIRECTION_DOWNLOAD);
}

void * ftp_ryu_uplink_onorbit(void * param){
    return ftp_transfer_onorbit(param, MIMAN_FTP_ENGINE_RYU,
                                MIMAN_FTP_DIRECTION_UPLOAD);
}


void * ftp_list_onorbit(void *){
    gs_ftp_settings_t ftp_config;
    ftp_config.mode = GS_FTP_MODE_STANDARD;
    ftp_config.host = setup->obc_node;
    ftp_config.port = FTP_RDP_PORT_OLD;
    ftp_config.timeout = setup->default_timeout + setup->guard_delay; //default timeout value
    ftp_config.chunk_size = 200; //default chunk size value
    int status = (int)gs_ftp_list(&ftp_config, State.gslistup->fpathbuf, ftp_list_callback, NULL);
    
    if (status != 0) {
		console.AddLog("[ERROR]##Fail to call FTP list. Retcode : %d", status);
	}
    else
    {
        console.AddLog("Call FTP list. Retcode : %d", status);
        printf("%s", flistbuffer);
    }
}

void * ftp_move_onorbit(void *){
    gs_ftp_settings_t ftp_config;
    ftp_config.mode = GS_FTP_MODE_STANDARD;
    ftp_config.host = setup->obc_node;
    ftp_config.port = FTP_RDP_PORT_OLD;
    ftp_config.timeout = setup->default_timeout + setup->guard_delay; //default timeout value
    ftp_config.chunk_size = 200; //default chunk size value
    int status = (int)gs_ftp_move(&ftp_config, State.gsmove->from, State.gsmove->to);

    if (status != 0) {
		console.AddLog("[ERROR]##Fail to FTP move. Retcode : %d", status);
	}
    else
    {
        console.AddLog("[OK]##FTP move done. Retcode : %d", status);
    }
}

void * ftp_remove_onorbit(void *){
    gs_ftp_settings_t ftp_config;
    ftp_config.mode = GS_FTP_MODE_STANDARD;
    ftp_config.host = setup->obc_node;
    ftp_config.port = FTP_RDP_PORT_OLD;
    ftp_config.timeout = setup->default_timeout + setup->guard_delay; //default timeout value
    ftp_config.chunk_size = 200; //default chunk size value
    int status = (int)gs_ftp_remove(&ftp_config, State.gsremove->path);

    if (status != 0) {
		console.AddLog("[ERROR]##Fail to FTP remove. Retcode : %d", status);
	}
    else
    {
        console.AddLog("[OK]##FTP remove done. Retcode : %d", status);
    }
}

void * ftp_copy_onorbit(void *){
    gs_ftp_settings_t ftp_config;
    ftp_config.mode = GS_FTP_MODE_STANDARD;
    ftp_config.host = setup->obc_node;
    ftp_config.port = FTP_RDP_PORT_OLD;
    ftp_config.timeout = setup->default_timeout + setup->guard_delay; //default timeout value
    ftp_config.chunk_size = 200; //default chunk size value
    int status = (int)gs_ftp_copy(&ftp_config, State.gscopy->from, State.gscopy->to);

    if (status != 0) {
		console.AddLog("[ERROR]##Fail to FTP copy. Retcode : %d", status);
	}
    else
    {
        console.AddLog("FTP copy done. Retcode : %d", status);
    }
}

void * ftp_mkdir_onorbit(void *){
    uint32_t mode = 0;
    gs_ftp_settings_t ftp_config;
    ftp_config.mode = GS_FTP_MODE_STANDARD;
    ftp_config.host = setup->obc_node;
    ftp_config.port = FTP_RDP_PORT_OLD;
    ftp_config.timeout = setup->default_timeout + setup->guard_delay; //default timeout value
    ftp_config.chunk_size = 200; //default chunk size value
    int status = (int)gs_ftp_mkdir(&ftp_config, State.gsmkdir->path, mode);

    if (status != 0) {
		console.AddLog("[ERROR]##Fail to FTP make directory. Retcode : %d", status);
	}
    else
    {
        console.AddLog("[OK]##FTP make directory done. Retcode : %d", status);
    }
}

void * ftp_rmdir_onorbit(void *){
    gs_ftp_settings_t ftp_config;
    ftp_config.mode = GS_FTP_MODE_STANDARD;
    ftp_config.host = setup->obc_node;
    ftp_config.port = FTP_RDP_PORT_OLD;
    ftp_config.timeout = setup->default_timeout + setup->guard_delay; //default timeout value
    ftp_config.chunk_size = 200; //default chunk size value
    int status = (int)gs_ftp_rmdir(&ftp_config, State.gsrmdir->path);

    if (status != 0) {
		console.AddLog("[ERROR]##Fail to FTP remove directory. Retcode : %d", status);
	}
    else
    {
        console.AddLog("[OK]##FTP remove directory done. Retcode : %d", status);
    }
}