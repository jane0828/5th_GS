#ifndef GS_FTP_CLIENT_H
#define GS_FTP_CLIENT_H
/* Copyright (c) 2013-2018 GomSpace A/S. All rights reserved. */
/**
   @file

   FTP client interface.
*/

#include <gs/ftp/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   FTP list callback.

   @param[in] entries total number of entries.
   @param[in] entry list entry.
   @param[in] user_data user data provided in ftp_list().
*/
typedef int (*gs_ftp_list_callback_t)(uint16_t entries, const gs_ftp_list_entry_t *entry, void *user_data);

/**
   Indicates the type of info provided in gs_ftp_info_callback.
*/
typedef enum {
    GS_FTP_INFO_UL_FILE  = 0,   /**< File size and checksum */
    GS_FTP_INFO_DL_FILE,
    GS_FTP_INFO_CRC,         /**< CRC of remote and local file */
    GS_FTP_INFO_UL_COMPLETED,   /**< Completed and total chunks */
    GS_FTP_INFO_DL_COMPLETED,
    GS_FTP_INFO_UL_PROGRESS,    /**< Current chunk, total_chunks and chunk_size */
    GS_FTP_INFO_DL_PROGRESS,
} gs_ftp_info_type_t;

 /**
    Data passed back to gs_ftp_info_callback
 */
typedef struct {
    gs_ftp_info_type_t  type;       /**< Type of data */
    void               *user_data;  /**< User data provided when registering the callback */
    union {
        struct {
            uint32_t remote;        /**< CRC of remote file */
            uint32_t local;         /**< CRC of local file */
        }crc;
        struct{
            uint32_t size;          /**< size of file */
            uint32_t crc;           /**< CRC of file */
        }file;
        struct {
            uint32_t completed_chunks;     /**< Completed chunks */
            uint32_t total_chunks;         /**< Total chunks */
        } completed;
        struct {
            uint32_t current_chunk; /**< Current chunk */
            uint32_t total_chunks;  /**< Number of chunks in transfer */
            uint32_t chunk_size;    /**< Chunk size in bytes */
        }progress;
    }u;                             /**< Payload data */
} gs_ftp_info_t;

int ftp_abort();
int ftp_avail();
/**
   Info callback.

   Get Info during ftp upload / download process

   @param[in] type Type of info, can be one of FTP_INFO_FILE, FTP_INFO_CRC adn FTP_INFO_STATUS, FTP_INFO_PROGRESS
   @param[in] arg1 First info argument
   @param[in] arg2 Second info argument
   @param[in] arg3 Third info argument
   @param[in] user_data User data provided when registering the callback
*/
typedef void (*gs_ftp_info_callback_t)(const gs_ftp_info_t * type);


/**
   FTP mode.

   The FTP mode is solely used for compensating for \a none standard FTP settings and behaviors, e.g. legacy quirks.
*/
typedef enum {
    /**
       Standard FTP mode.
    */
    GS_FTP_MODE_STANDARD = 0,
    /**
       Legacy GATOSS mode.
       GATOSS uses a different FTP port, and different backend-type for files.
    */
    GS_FTP_MODE_GATOSS = 1,
} gs_ftp_mode_t;

/**
   FTP settings
*/
typedef struct {
    /**
       FTP mode.
       Default is #GS_FTP_MODE_STANDARD.
    */
    gs_ftp_mode_t mode;
    /**
       CSP address of host.
    */
    uint8_t host;
    /**
       CSP port on host.
       Use 0 (zero) for default port.
       @see gs_ftp_get_csp_port_force()
    */
    uint8_t port;
    /**
       Timeout in ms
       Use 0 (zero) for default timeout.
       @see gs_ftp_get_timeout_force()
    */
    uint32_t timeout;
    /**
       Chunk size in bytes
       Use 0 (zero) for default chunk size.
       @see gs_ftp_get_chunk_size_force()
    */
    uint32_t chunk_size;
} gs_ftp_settings_t;


/**
    Return default ftp settings

    @param[in] settings Pointer to settings struct
    @return_gs_error_t
*/
gs_error_t gs_ftp_default_settings(gs_ftp_settings_t * settings);


/**
   Upload to remote server.

   URLs can take the form:
   - file://\<path_to_file\>
   - mem://\<addr\>[++\<length\>]
   - \<path_to_file\>

   @param[in] settings FTP settings (host, port, ...)
   @param[in] local_url URL to the file to upload
   @param[in] remote_url url on the remote server
   @param[in] info_callback pointer to optional callback that is called during upload process
   @param[in] info_data pointer passed to info_callback

   @return_gs_error_t
*/
gs_error_t gs_ftp_upload(const gs_ftp_settings_t * settings, const char * local_url, const char * remote_url,
                         gs_ftp_info_callback_t info_callback, void * info_data);
gs_error_t gs_ftp_upload_force(const gs_ftp_settings_t * settings, const char * local_url, const char * remote_url,
                         gs_ftp_info_callback_t info_callback, void * info_data, int iteration);

/**
   Download from remote server.

   URLs can take the form:
   - file://\<path_to_file\>
   - mem://\<addr\>[++\<length\>]
   - \<path_to_file\>

   @param[in] settings FTP settings (host, port, ...)
   @param[in] local_url destination URL
   @param[in] remote_url source URL
   @param[in] info_callback pointer to optional callback that is called during upload process
   @param[in] info_data pointer passed to info_callback

   @return_gs_error_t
*/
gs_error_t gs_ftp_download(const gs_ftp_settings_t * settings, const char * local_url,  const char * remote_url,
                           gs_ftp_info_callback_t info_callback, void * info_data);
gs_error_t gs_ftp_download_force(const gs_ftp_settings_t * settings, const char * local_url,  const char * remote_url,
                           gs_ftp_info_callback_t info_callback, void * info_data, int iteration);

/**
   Move file on server.

   @param[in] settings FTP settings (host, port, ...)
   @param[in] from old name.
   @param[in] to new name.

   @return_gs_error_t
*/
gs_error_t gs_ftp_move(const gs_ftp_settings_t * settings, const char * from, const char * to);

/**
   Copy file on server.

   @param[in] settings FTP settings (host, port, ...)
   @param[in] from file to copy from.
   @param[in] to file to copy to

   @return_gs_error_t
*/
gs_error_t gs_ftp_copy(const gs_ftp_settings_t * settings, const char * from, const char * to);

/**
   Remove file on server.

   @param[in] settings FTP settings (host, port, ...)
   @param[in] path file to remove

   @return_gs_error_t
*/
gs_error_t gs_ftp_remove(const gs_ftp_settings_t * settings, const char * path);

/**
   Create directory on server.

   @param[in] settings FTP settings (host, port, ...)
   @param[in] path path to create.
   @param[in] mode standard linux mode.

   @return_gs_error_t
*/
gs_error_t gs_ftp_mkdir(const gs_ftp_settings_t * settings, const char *path, uint32_t mode);

/**
   Remove directory on server.

   @param[in] settings FTP settings (host, port, ...)
   @param[in] path path to remove.

   @return_gs_error_t
*/
gs_error_t gs_ftp_rmdir(const gs_ftp_settings_t * settings, const char *path);

/**
   List files on server on server.

   @param[in] settings FTP settings (host, port, ...)
   @param[in] path path to list.
   @param[in] callback results callback
   @param[in] user_data user_data provided to callback

   @return_gs_error_t
*/
gs_error_t gs_ftp_list(const gs_ftp_settings_t * settings, const char * path, gs_ftp_list_callback_t callback, void *user_data);

/**
   Compress / Decompress file on server.

   @param[in] settings FTP settings (host, port, ...)
   @param[in] src source path
   @param[in] dest destination
   @param[in] action 1=Compress, 2=Decompress
   @param[out] comp_sz Compressed size
   @param[out] decomp_sz Decompressed size

   @return_gs_error_t
*/
gs_error_t gs_ftp_zip(const gs_ftp_settings_t * settings, const char * src, const char * dest, uint8_t action,
                      uint32_t * comp_sz, uint32_t * decomp_sz);

/**
   Make file system on server.

   @param[in] settings FTP settings (host, port, ...)
   @param[in] path path of the filesystem.
   @param[in] force Force filesystem creation

   @return_gs_error_t
*/
gs_error_t gs_ftp_mkfs(const gs_ftp_settings_t * settings, const char *path, bool force);


/**
   Register FTP commands.
   @return_gs_error_t
*/
gs_error_t gs_ftp_register_commands(void);

/**
   Register FTP commands.
   @deprecated use gs_ftp_register_commands()
   @return_gs_error_t
*/
static inline gs_error_t ftp_register_commands(void)
{
    return gs_ftp_register_commands();
}

/**
   Convert string to mode.

   @param[in] string mode string.
   @param[out] return_mode converted FTP mode.
   @return_gs_error_t
*/
gs_error_t gs_ftp_string_to_mode(const char * string, gs_ftp_mode_t * return_mode);

/**
   Returns the mode as a string.
   @param[in] mode mode
   @return string representing the mode.
*/
const char * gs_ftp_mode_to_string(gs_ftp_mode_t mode);

/**
   Returns CSP port from settings or default if none specified.

   If \a settings specified, #gs_ftp_mode_t may change the returned port, if no port specified.

   @param[in] settings Pointer to settings struct. Use NULL for returning default value.
   @return CSP port.
*/
uint8_t gs_ftp_get_csp_port(const gs_ftp_settings_t * settings);
uint8_t gs_ftp_get_csp_port_force(const gs_ftp_settings_t * settings);

/**
   Returns timeout from settings or default if none specified.

   @param[in] settings Pointer to settings struct. Use NULL for returning default value.
   @return timeout (mS)
*/
uint32_t gs_ftp_get_timeout(const gs_ftp_settings_t * settings);
uint32_t gs_ftp_get_timeout_force(const gs_ftp_settings_t * settings);
    
/**
   Returns chunk size from settings or default if none specified.

   @param[in] settings Pointer to settings struct. Use NULL for returning default value.
   @return chunk size
*/
uint32_t gs_ftp_get_chunk_size(const gs_ftp_settings_t * settings);
uint32_t gs_ftp_get_chunk_size_force(const gs_ftp_settings_t * settings);

static gs_error_t ftp_status_request();
static gs_error_t ftp_status_reply();
static gs_error_t ftp_data(int count);
static gs_error_t ftp_crc();

//Custom
gs_error_t gs_ftp_upload_force_request(const gs_ftp_settings_t * settings, const char * local_url, const char * remote_url,
                         gs_ftp_info_callback_t info_callback, void * info_data);
void ftp_done_force(unsigned int remove_map);
void ftp_done(unsigned int remove_map);
gs_error_t gs_conn_create_force(const gs_ftp_settings_t * settings);

void ftp_close_force();
    
#ifdef __cplusplus
}
#endif
#endif
