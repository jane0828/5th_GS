#ifndef RYU_FTP_CLIENT_ADAPTER_H
#define RYU_FTP_CLIENT_ADAPTER_H

#include <ftpnew_client.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 5th_GS integration entry points. Call these instead of the raw
 * ftpnew_upload()/ftpnew_download() functions when callbacks are used.
 */
gs_error_t ryu_ftp_upload(const gs_ftp_settings_t * settings,
                          const char * local_url,
                          const char * remote_url,
                          gs_ftp_info_callback_t info_callback,
                          void * info_data,
                          bool use_rdp,
                          uint32_t interpacket_delay_ms);

gs_error_t ryu_ftp_download(const gs_ftp_settings_t * settings,
                            const char * local_url,
                            const char * remote_url,
                            gs_ftp_info_callback_t info_callback,
                            void * info_data,
                            bool use_rdp);

#ifdef __cplusplus
}
#endif

#endif
