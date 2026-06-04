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

#include "miman_config.h"
#include "miman_csp.h"
#include "miman_coms.h"
#include "miman_imgui.h"
#include "miman_ftp.h"

extern Console console;
extern StateCheckUnit State;
extern pthread_t p_thread[16];
extern pthread_mutex_t conn_lock;
extern Setup * setup;


static unsigned int ftp_chunk_size = 185;
static unsigned int ftp_backend = 3; // Use file backend as standard
static const char * const packet_missing = "-";
static const char * const packet_ok = "+";

void ftp_callback(const gs_ftp_info_t * info)
{
    switch(info->type)
    {
        case GS_FTP_INFO_DL_FILE : {
            console.AddLog("[OK]##Received FTP Download Spec.");
            printftp("Received FTP Download Spec. Filesize : %u, CRC : %u.", info->u.file.size, info->u.file.crc);
            break;
        }
        case GS_FTP_INFO_UL_FILE : {
            console.AddLog("[OK]##Received FTP Upload Spec.");
            printftp("Received FTP Upload File Spec. Filesize : %u, CRC : %u.", info->u.file.size, info->u.file.crc);
            break;
        }
        case GS_FTP_INFO_CRC : {
            printftp("FTP Transaction Completed.");
            console.AddLog("Calculate FTP CRC. Remote : %u, Local : %u.", info->u.crc.remote, info->u.crc.local);
            break;
        }
        case GS_FTP_INFO_DL_COMPLETED : {
            console.AddLog("[OK]##Received FTP Status Request.");
            if(info->u.completed.completed_chunks != 0)
            {
                printftp("Restart FTP Downlink from chunk : %u, to total : %u", info->u.completed.completed_chunks, info->u.completed.total_chunks);
            }
            else
            {
                printftp("Start FTP Downlink. Total Chunk Size : %u", info->u.completed.total_chunks);
            }
            break;
        }
        case GS_FTP_INFO_UL_COMPLETED : {
            console.AddLog("[OK]##Received FTP Status Reply.");
            if(info->u.completed.completed_chunks != 0)
            {
                printftp("Restart FTP Uplink from chunk : %u, to total : %u", info->u.completed.completed_chunks, info->u.completed.total_chunks);
            }
            else
            {
                printftp("Start FTP Uplink. Total Chunk Size : %u", info->u.completed.total_chunks);
            }
            break;
        }
        case GS_FTP_INFO_DL_PROGRESS : {
            printftp("Download : %u/%u", info->u.progress.current_chunk, info->u.progress.total_chunks);
            break;
        }
        case GS_FTP_INFO_UL_PROGRESS : {
            printftp("Upload : %u/%u", info->u.progress.current_chunk, info->u.progress.total_chunks);
            break;
        }
        default : {
            console.AddLog("[ERROR]##Invalid Call to FTP Callback Function.");
        }
    }
}



ftpinfo *filelisthandler(filelist * list, uint16_t target, uint16_t status, int index)
{
    ftpinfo *nowftplistup = (ftpinfo *)malloc(sizeof(ftpinfo));
    char * filetype;
    
    char * filestatus;
    
    char * filelabel;
    char * fileconf;
    char * filename = (char *)malloc(sizeof(char) * 10);
    char filepath[256];
    char localfilepath[256];

    char * localfiletype;
    char * nowtime = (char *)malloc(sizeof(char) * 20);
    //filetype input
    switch(target) {
        case MIM_DLTYPE_HK : {
            filetype = "/mnt/ds/hk";
            filelabel = "hktlm";
            fileconf = ".hk";
            localfiletype = "data/hk";
            break;
        }
        case MIM_DLTYPE_AOD : {
            filetype = "/mnt/ds/aod";
            filelabel = "aodtlm";
            fileconf = ".aod";
            localfiletype = "data/aod";
            break;
        }
        case MIM_DLTYPE_LOG : {
            filetype = "/mnt/ds/log";
            filelabel = "logtlm";
            fileconf = ".log";
            localfiletype = "data/log";
            break;
        }
        case MIM_DLTYPE_SNSR : {
            filetype = "/mnt/ds/snsr";
            filelabel = "snsr";
            fileconf = ".raw";
            localfiletype = "data/snsr";
            break;
        }
        case MIM_DLTYPE_GPS : {
            filetype = "/mnt/ds/gps";
            filelabel = "raw";
            fileconf = ".raw";
            localfiletype = "data/gps";
            break;
        }
        case MIM_DLTYPE_CTRLO : {
            filetype = "/mnt/ds/ctrlo";
            filelabel = "raw";
            fileconf = ".raw";
            localfiletype = "data/ctrlo";
            break;
        }
    }
    //filestatus input
    switch(status) {
        case MIM_DLSTAT_NEW : {
            filestatus = "/";
            break;
        }
        case MIM_DLSTAT_OLD : {
            filestatus = "/sent/";
            break;
        }
        case MIM_DLSTAT_ING : {
            filestatus = "/input/";
            break;
        }
    }
    time_t secs = time(0);
    struct tm *local = localtime(&secs);
    sprintf(nowtime, "__%04d_%02d_%02d_%02d_%02d_%02d__", local->tm_year+1900, local->tm_mon+1, local->tm_mday,local->tm_hour, local->tm_min, local->tm_sec);


    sprintf(filename, "%08"PRIu32, list->file[index]);
    strcpy(filepath, filetype);
    strcat(filepath, filestatus);
    strcat(filepath, filelabel);
    strcat(filepath, filename);
    strcat(filepath, fileconf);

    strcpy(localfilepath, localfiletype);
    strcat(localfilepath, filestatus);
    strcat(localfilepath, nowtime);
    strcat(localfilepath, filelabel);
    strcat(localfilepath, filename);
    strcat(localfilepath, fileconf);
    memcpy(nowftplistup->name, filename, strlen(filename)+1);
    memcpy(nowftplistup->remote_path, filepath, strlen(filepath)+1);
    memcpy(nowftplistup->local_path, localfilepath, strlen(localfilepath)+1);
    //filename input
    return nowftplistup;
}

void * ftp_downlink_force(void * param){
#define CSP_USE_RDP
    ftp_avail();
    bool dlstate = State.downlink_mode;
    //This funcion must be on p_thread[4]
    if((dlstate))
        State.downlink_mode = false;
    while(!State.uplink_mode)
        continue;
    while(!State.RotatorReadReady)
        continue;
    
    //host : OBC adress
    //Port : 15
    ftpinfo * FTP = (ftpinfo *) param;
    // gs_ftp_info_callback_t = ftp_callback;
    printftp("Start FTP Downlink.");
    State.downlink_mode = false;
    gs_ftp_settings_t ftp_config;
    ftp_config.mode = GS_FTP_MODE_STANDARD;
    ftp_config.host = setup->obc_node;
    ftp_config.port = FTPFCD_PORT;
    ftp_config.timeout = 5000; //default timeout value
    ftp_config.chunk_size = State.chunk_sz; //default chunk size value
    State.ftp_mode = true;
    int status = (int)gs_ftp_download_force(&ftp_config, FTP->local_path, FTP->remote_path, ftp_callback, NULL, State.iteration);
    if (status != 0) {
		console.AddLog("[ERROR]##Fail to complete ftp_download. Retcode : %d", status);\
	}
    else
    {
        console.AddLog("[OK]##End FTP download. Retcode : %d", status);\
    }
    ftp_abort();
    State.ftp_mode = false;
    if(dlstate)
    {
        State.downlink_mode = dlstate;
    }
#undef CSP_USE_RDP
	return NULL;
}

void * ftp_uplink_force(void * param){
#define CSP_USE_RDP
    ftp_avail();
    bool dlstate = State.downlink_mode;
    //This funcion must be on p_thread[4]
    if((dlstate))
        State.downlink_mode = false;
    while(!State.uplink_mode)
        continue;
    while(!State.RotatorReadReady)
        continue;
    //host : OBC adress
    //Port : 15
    ftpinfo * FTP = (ftpinfo *) param;
    State.downlink_mode = false;
    gs_ftp_settings_t ftp_config;
    ftp_config.mode = GS_FTP_MODE_STANDARD;
    ftp_config.host = setup->obc_node;
    ftp_config.port = FTPFCD_PORT;
    ftp_config.timeout = 5000; //default timeout value
    ftp_config.chunk_size = State.chunk_sz; //default chunk size value
    State.ftp_mode = true;
    int status = 0;
    status = (int)gs_ftp_upload_force(&ftp_config, FTP->local_path, FTP->remote_path, ftp_callback, NULL, State.iteration);
    printftp("FTP task DONE.");
    
    if (status != 0) {
		console.AddLog("[ERROR]##Fail to complete ftp_upload. Retcode : %d", status);
	}
    else
    {
        console.AddLog("[OK]##End FTP upload. Retcode : %d", status);
    }
    ftp_abort();
    State.ftp_mode = false;
    if(dlstate)
    {
        State.downlink_mode = dlstate;
    }
#undef CSP_USE_RDP
	return NULL;
}


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

// #include "miman_config.h"
// #include "miman_csp.h"
// #include "miman_coms.h"
// #include "miman_imgui.h"
// #include "miman_ftp.h"

// extern Console console;
// extern StateCheckUnit State;
// extern pthread_t p_thread[16];
// extern pthread_mutex_t conn_lock;
// extern Setup * setup;


// static unsigned int ftp_chunk_size = 185;
// static unsigned int ftp_backend = 3; // Use file backend as standard
// static const char * const packet_missing = "-";
// static const char * const packet_ok = "+";

// void ftp_callback(const gs_ftp_info_t * info)
// {
//     switch(info->type)
//     {
//         case GS_FTP_INFO_DL_FILE : {
//             console.AddLog("[OK]##Received FTP Download Spec.");
//             printftp("Received FTP Download Spec. Filesize : %u, CRC : %u.", info->u.file.size, info->u.file.crc);
//             break;
//         }
//         case GS_FTP_INFO_UL_FILE : {
//             console.AddLog("[OK]##Received FTP Upload Spec.");
//             printftp("Received FTP Upload File Spec. Filesize : %u, CRC : %u.", info->u.file.size, info->u.file.crc);
//             break;
//         }
//         case GS_FTP_INFO_CRC : {
//             printftp("FTP Transaction Completed.");
//             console.AddLog("Calculate FTP CRC. Remote : %u, Local : %u.", info->u.crc.remote, info->u.crc.local);
//             break;
//         }
//         case GS_FTP_INFO_DL_COMPLETED : {
//             console.AddLog("[OK]##Received FTP Status Request.");
//             if(info->u.completed.completed_chunks != 0)
//             {
//                 printftp("Restart FTP Downlink from chunk : %u, to total : %u", info->u.completed.completed_chunks, info->u.completed.total_chunks);
//             }
//             else
//             {
//                 printftp("Start FTP Downlink. Total Chunk Size : %u", info->u.completed.total_chunks);
//             }
//             break;
//         }
//         case GS_FTP_INFO_UL_COMPLETED : {
//             console.AddLog("[OK]##Received FTP Status Reply.");
//             if(info->u.completed.completed_chunks != 0)
//             {
//                 printftp("Restart FTP Uplink from chunk : %u, to total : %u", info->u.completed.completed_chunks, info->u.completed.total_chunks);
//             }
//             else
//             {
//                 printftp("Start FTP Uplink. Total Chunk Size : %u", info->u.completed.total_chunks);
//             }
//             break;
//         }
//         case GS_FTP_INFO_DL_PROGRESS : {
//             printftp("Download : %u/%u", info->u.progress.current_chunk, info->u.progress.total_chunks);
//             break;
//         }
//         case GS_FTP_INFO_UL_PROGRESS : {
//             printftp("Upload : %u/%u", info->u.progress.current_chunk, info->u.progress.total_chunks);
//             break;
//         }
//         default : {
//             console.AddLog("[ERROR]##Invalid Call to FTP Callback Function.");
//         }
//     }
// }



// ftpinfo *filelisthandler(filelist * list, uint16_t target, uint16_t status, int index)
// {
//     ftpinfo *nowftplistup = (ftpinfo *)malloc(sizeof(ftpinfo));
//     char * filetype;
    
//     char * filestatus;
    
//     char * filelabel;
//     char * fileconf;
//     char * filename = (char *)malloc(sizeof(char) * 10);
//     char filepath[256];
//     char localfilepath[256];

//     char * localfiletype;
//     char * nowtime = (char *)malloc(sizeof(char) * 20);
//     //filetype input
//     switch(target) {
//         case MIM_DLTYPE_HK : {
//             filetype = "/mnt/ds/hk";
//             filelabel = "hktlm";
//             fileconf = ".hk";
//             localfiletype = "data/hk";
//             break;
//         }
//         case MIM_DLTYPE_AOD : {
//             filetype = "/mnt/ds/aod";
//             filelabel = "aodtlm";
//             fileconf = ".aod";
//             localfiletype = "data/aod";
//             break;
//         }
//         case MIM_DLTYPE_LOG : {
//             filetype = "/mnt/ds/log";
//             filelabel = "logtlm";
//             fileconf = ".log";
//             localfiletype = "data/log";
//             break;
//         }
//         case MIM_DLTYPE_SNSR : {
//             filetype = "/mnt/ds/snsr";
//             filelabel = "snsr";
//             fileconf = ".raw";
//             localfiletype = "data/snsr";
//             break;
//         }
//         case MIM_DLTYPE_GPS : {
//             filetype = "/mnt/ds/gps";
//             filelabel = "raw";
//             fileconf = ".raw";
//             localfiletype = "data/gps";
//             break;
//         }
//         case MIM_DLTYPE_CTRLO : {
//             filetype = "/mnt/ds/ctrlo";
//             filelabel = "raw";
//             fileconf = ".raw";
//             localfiletype = "data/ctrlo";
//             break;
//         }
//     }
//     //filestatus input
//     switch(status) {
//         case MIM_DLSTAT_NEW : {
//             filestatus = "/";
//             break;
//         }
//         case MIM_DLSTAT_OLD : {
//             filestatus = "/sent/";
//             break;
//         }
//         case MIM_DLSTAT_ING : {
//             filestatus = "/input/";
//             break;
//         }
//     }
//     time_t secs = time(0);
//     struct tm *local = localtime(&secs);
//     sprintf(nowtime, "__%04d_%02d_%02d_%02d_%02d_%02d__", local->tm_year+1900, local->tm_mon+1, local->tm_mday,local->tm_hour, local->tm_min, local->tm_sec);


//     sprintf(filename, "%08"PRIu32, list->file[index]);
//     strcpy(filepath, filetype);
//     strcat(filepath, filestatus);
//     strcat(filepath, filelabel);
//     strcat(filepath, filename);
//     strcat(filepath, fileconf);

//     strcpy(localfilepath, localfiletype);
//     strcat(localfilepath, filestatus);
//     strcat(localfilepath, nowtime);
//     strcat(localfilepath, filelabel);
//     strcat(localfilepath, filename);
//     strcat(localfilepath, fileconf);
//     memcpy(nowftplistup->name, filename, strlen(filename)+1);
//     memcpy(nowftplistup->remote_path, filepath, strlen(filepath)+1);
//     memcpy(nowftplistup->local_path, localfilepath, strlen(localfilepath)+1);
//     //filename input
//     return nowftplistup;
// }

// void * ftp_downlink_force(void * param){
// #define CSP_USE_RDP
//     ftp_avail();
//     bool dlstate = State.downlink_mode;
//     //This funcion must be on p_thread[4]
//     if((dlstate))
//         State.downlink_mode = false;
//     while(!State.uplink_mode)
//         continue;
//     while(!State.RotatorReadReady)
//         continue;
    
//     //host : OBC adress
//     //Port : 15
//     ftpinfo * FTP = (ftpinfo *) param;
//     // gs_ftp_info_callback_t = ftp_callback;
//     printftp("Start FTP Downlink.");
//     State.downlink_mode = false;
//     gs_ftp_settings_t ftp_config;
//     ftp_config.mode = GS_FTP_MODE_STANDARD;
//     ftp_config.host = setup->obc_node;
//     ftp_config.port = FTPFCD_PORT;
//     ftp_config.timeout = 5000; //default timeout value
//     ftp_config.chunk_size = State.chunk_sz; //default chunk size value
//     State.ftp_mode = true;
//     int status = (int)gs_ftp_download_force(&ftp_config, FTP->local_path, FTP->remote_path, ftp_callback, NULL, State.iteration);
//     if (status != 0) {
// 		console.AddLog("[ERROR]##Fail to complete ftp_download. Retcode : %d", status);\
// 	}
//     else
//     {
//         console.AddLog("[OK]##End FTP download. Retcode : %d", status);\
//     }
//     ftp_abort();
//     State.ftp_mode = false;
//     if(dlstate)
//     {
//         State.downlink_mode = dlstate;
//     }
// #undef CSP_USE_RDP
// 	return NULL;
// }

// void * ftp_uplink_force(void * param){
// #define CSP_USE_RDP
//     ftp_avail();
//     bool dlstate = State.downlink_mode;
//     //This funcion must be on p_thread[4]
//     if((dlstate))
//         State.downlink_mode = false;
//     while(!State.uplink_mode)
//         continue;
//     while(!State.RotatorReadReady)
//         continue;
//     //host : OBC adress
//     //Port : 15
//     ftpinfo * FTP = (ftpinfo *) param;
//     State.downlink_mode = false;
//     gs_ftp_settings_t ftp_config;
//     ftp_config.mode = GS_FTP_MODE_STANDARD;
//     ftp_config.host = setup->obc_node;
//     ftp_config.port = FTPFCD_PORT;
//     ftp_config.timeout = 5000; //default timeout value
//     ftp_config.chunk_size = State.chunk_sz; //default chunk size value
//     State.ftp_mode = true;
//     int status = 0;
//     status = (int)gs_ftp_upload_force(&ftp_config, FTP->local_path, FTP->remote_path, ftp_callback, NULL, State.iteration);
//     printftp("FTP task DONE.");
    
//     if (status != 0) {
// 		console.AddLog("[ERROR]##Fail to complete ftp_upload. Retcode : %d", status);
// 	}
//     else
//     {
//         console.AddLog("[OK]##End FTP upload. Retcode : %d", status);
//     }
//     ftp_abort();
//     State.ftp_mode = false;
//     if(dlstate)
//     {
//         State.downlink_mode = dlstate;
//     }
// #undef CSP_USE_RDP
// 	return NULL;
// }