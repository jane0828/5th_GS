#include "ryu_ftp_client_adapter.h"

typedef struct {
    gs_ftp_info_callback_t callback;
    void * user_data;
} ryu_ftp_callback_bridge_t;

static void ryu_ftp_download_callback(const gs_ftp_info_t * source)
{
    ryu_ftp_callback_bridge_t * bridge = source->user_data;
    gs_ftp_info_t info = *source;

    info.user_data = bridge->user_data;
    switch (info.type) {
    case GS_FTP_INFO_UL_FILE:
        info.type = GS_FTP_INFO_DL_FILE;
        break;
    case GS_FTP_INFO_UL_COMPLETED:
        info.type = GS_FTP_INFO_DL_COMPLETED;
        break;
    case GS_FTP_INFO_UL_PROGRESS:
        info.type = GS_FTP_INFO_DL_PROGRESS;
        break;
    default:
        break;
    }

    bridge->callback(&info);
}

gs_error_t ryu_ftp_upload(const gs_ftp_settings_t * settings,
                          const char * local_url,
                          const char * remote_url,
                          gs_ftp_info_callback_t info_callback,
                          void * info_data,
                          bool use_rdp,
                          uint32_t interpacket_delay_ms)
{
    return ftpnew_upload(settings, local_url, remote_url, info_callback,
                         info_data, use_rdp, interpacket_delay_ms);
}

gs_error_t ryu_ftp_download(const gs_ftp_settings_t * settings,
                            const char * local_url,
                            const char * remote_url,
                            gs_ftp_info_callback_t info_callback,
                            void * info_data,
                            bool use_rdp)
{
    if (!info_callback) {
        return ftpnew_download(settings, local_url, remote_url, NULL, NULL,
                               use_rdp);
    }

    ryu_ftp_callback_bridge_t bridge = {
        .callback = info_callback,
        .user_data = info_data,
    };
    return ftpnew_download(settings, local_url, remote_url,
                           ryu_ftp_download_callback, &bridge, use_rdp);
}
