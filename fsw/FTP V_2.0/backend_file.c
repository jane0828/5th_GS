/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <gs/ftp/server.h>
#include <gs/ftp/internal/types.h>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>

#include <gs/util/crc32.h>
#include <gs/util/string.h>
#include <gs/util/zip/zip.h>
#include "local.h"

#if GS_FTP_USE_GLOB
#include <libgen.h>
#include <glob.h>
#endif

#if (__linux__)
#include <dirent.h>
#else
#include <gs/storage/vfs/vfs.h> // -> GS_STORAGE_VFS
#endif

/* Chunk status markers */
static const char *packet_missing = "-";
static const char *packet_ok = "+";

struct ftp_file_state {
    /* Current bitmap and file */
    FILE *fd_file;
    FILE *fd_map;
    char *mem_map;

    /* Current file info */
    char file_name[GS_FTP_PATH_LENGTH + 1];
    uint32_t file_size;
    uint32_t file_chunk_size;
    uint32_t file_chunks;

    /* Directory pointer used in listing */
    DIR *dirp;
    char dirpath[GS_FTP_PATH_LENGTH + 1];

#if GS_FTP_USE_GLOB
    /* Globbing */
    glob_t globbuf;
    uint32_t globcount;
#endif
};

static gs_ftp_return_t ftp_file_timeout(void *state);

/* Calculate CRC of current file */
static int file_crc(void *state, uint32_t *crc_arg)
{
    size_t bytes;
    char buf[512];
    struct ftp_file_state *file_state = (struct ftp_file_state *)state;

    if (!file_state)
        return GS_FTP_RET_INVAL;

    /* Flush file before calculating CRC */
    if (fflush(file_state->fd_file) != 0)
        log_error("Flush error");
    if (fsync(fileno(file_state->fd_file)) != 0)
        log_error("Fsync error");
    if (fseek(file_state->fd_file, 0, SEEK_SET) != 0)
        log_error("Fseek error");

    log_debug("File size to check %"PRIu32", state %p, fd %p", file_state->file_size, file_state, file_state->fd_file);

    uint32_t crc = gs_crc32_init();
    do {
        bytes = fread(buf, 1, sizeof(buf), file_state->fd_file);
        crc = gs_crc32_update(crc, buf, bytes);
    } while (bytes > 0);

    *crc_arg = crc = gs_crc32_finalize(crc);

    log_debug("Backend CRC 0x%"PRIx32" (%p), 0x%"PRIx32, *crc_arg, crc_arg, crc);

    return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_file_init(void **state)
{
    if (state) {
        struct ftp_file_state *file_state = calloc(1, sizeof(*file_state));

        if (file_state) {
            *state = file_state;
            return GS_FTP_RET_OK;
        }
    }

    return GS_FTP_RET_NOMEM;
}

static gs_ftp_return_t ftp_file_release(void *state)
{
    if (state) {
        free(state);
    }

    return GS_FTP_RET_OK;
}

#define MAKE_MAP_FILENAME(buf, filename)   snprintf(buf, sizeof(buf), "%s.map", filename)
#define MAKE_TMP_FILENAME(buf, filename)   snprintf(buf, sizeof(buf), "%s.tmp", filename)

static gs_ftp_return_t ftp_file_upload(void *state, char const *path, uint32_t memaddr, uint32_t size, uint32_t chunk_size)
{
    /* Map file name */
    bool new_file = false;

    struct ftp_file_state *file_state = (struct ftp_file_state*)state;

    if (!file_state)
        return GS_FTP_RET_INVAL;

    file_state->file_size = size;
    file_state->file_chunk_size = chunk_size;
    file_state->file_chunks = (size + chunk_size - 1) / chunk_size;
    GS_STRNCPY(file_state->file_name, path);

    /* Abort if a previous transfer was incomplete */
    if (file_state->fd_file || file_state->fd_map || file_state->mem_map) {
        log_warning("Aborting previous upload %p %p %p", file_state->fd_file, file_state->fd_map, file_state->mem_map);
        ftp_file_timeout(state);
    }

//    file_state->fd_file = fopen(path, "r+");
//    if (file_state->fd_file) {
//        log_error("upload file %s already exists", path);
//        fclose(file_state->fd_file);
//        file_state->fd_file = NULL;
//        return GS_FTP_RET_EXISTS;
//    }

    /* Try to create a new map file */
//    char tmp[GS_FTP_PATH_LENGTH + 10];
//    MAKE_TMP_FILENAME(tmp, path);

    /* Try to open file */
    file_state->fd_file = fopen(path, "r+");
    if (file_state->fd_file == NULL) {
        /* Create new file */
        new_file = true;
        file_state->fd_file = fopen(path, "w+");
        if (file_state->fd_file == NULL) {
            log_error("upload: failed to create data file: [%s]", path);
            return GS_FTP_RET_IO;
        }
    }

    /* Allocate memory map */
    if (file_state->mem_map == NULL) {
        file_state->mem_map = malloc(file_state->file_chunks);

        if (file_state->mem_map == NULL) {
            fclose(file_state->fd_file);
            file_state->fd_file = NULL;
            return GS_FTP_RET_NOSPC;
        }
    }

    /* Try to create a new map file */
    char map[GS_FTP_PATH_LENGTH + 10];
    MAKE_MAP_FILENAME(map, path);

    /* Check if file already exists */
    file_state->fd_map = fopen(map, "r+");
    if (file_state->fd_map == NULL || fread(file_state->mem_map, 1, file_state->file_chunks, file_state->fd_map) != file_state->file_chunks) {
        unsigned int i;

        /* Close file if already open but data could not be read */
        if (file_state->fd_map) {
            fclose(file_state->fd_map);
            file_state->fd_map = NULL;
        }

        /* Abort if data file exists but map doesn't */
        if (!file_state->fd_map && !new_file) {
            log_error("file %s already exists", path);
            fclose(file_state->fd_file);
            file_state->fd_file = NULL;
            free(file_state->mem_map);
            file_state->mem_map = NULL;
            return GS_FTP_RET_EXISTS;
        }

        /* Create new file */
        file_state->fd_map = fopen(map, "w+");
        if (file_state->fd_map == NULL) {
            log_error("Failed to create bitmap %s", map);
            fclose(file_state->fd_file);
            file_state->fd_file = NULL;
            free(file_state->mem_map);
            file_state->mem_map = NULL;
            return GS_FTP_RET_IO;
        }

        /* Clear contents */
        for (i = 0; i < file_state->file_chunks; i++) {
            if (fwrite(packet_missing, 1, 1, file_state->fd_map) < 1) {
                log_error("Failed to clear bitmap");
                fclose(file_state->fd_map);
                file_state->fd_map = NULL;
                fclose(file_state->fd_file);
                file_state->fd_file = NULL;
                free(file_state->mem_map);
                file_state->mem_map = NULL;
                return GS_FTP_RET_IO;
            }
            file_state->mem_map[i] = *packet_missing;
        }

        fflush(file_state->fd_map);
        fsync(fileno(file_state->fd_map));
    }

    return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_file_download(void *state, char const *path, uint32_t memaddr, uint32_t memsize, uint32_t chunk_size, uint32_t *size, uint32_t *crc)
{
    struct ftp_file_state *file_state = (struct ftp_file_state *)state;

    if (!file_state)
        return GS_FTP_RET_INVAL;

    /* Try to open file */
    file_state->fd_file = fopen(path, "r");
    if (file_state->fd_file == NULL)
        return GS_FTP_RET_NOENT;

    /* Read size */
    struct stat statbuf;
    if (stat(path, &statbuf) != 0)
        return GS_FTP_RET_IO;

    *size = statbuf.st_size;
    file_state->file_size = *size;
    file_state->file_chunk_size = chunk_size;
    file_state->file_chunks = (file_state->file_size + chunk_size - 1) / chunk_size;
    GS_STRNCPY(file_state->file_name, path);

    /* Calculate CRC */
    if (file_crc(state, crc) != 0)
        return GS_FTP_RET_IO;
    else
        return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_file_write_chunk(void *state, uint32_t chunk, uint8_t *data, uint32_t size)
{
    struct ftp_file_state *file_state = (struct ftp_file_state *)state;

    if (!file_state)
        return GS_FTP_RET_INVAL;

    /* Write to file */
    if ((unsigned int) ftell(file_state->fd_file) != chunk * file_state->file_chunk_size) {
        if (fseek(file_state->fd_file, chunk * file_state->file_chunk_size, SEEK_SET) != 0)
            goto chunk_write_error;
    }
    if (fwrite(data, 1, size, file_state->fd_file) != size)
        goto chunk_write_error;

#if (GS_STORAGE_VFS)
    fflush(file_state->fd_map);
    fsync(fileno(file_state->fd_map));
    /* Flush data file to prevent CRC error when uploading in multiple tasks*/
    fflush(file_state->fd_file);
#endif

    return GS_FTP_RET_OK;

chunk_write_error:
    log_error("Filesystem write error");
    fclose(file_state->fd_file);
    file_state->fd_file = NULL;
    return GS_FTP_RET_IO;
}

static gs_ftp_return_t ftp_file_read_chunk(void *state, uint32_t chunk, uint8_t *data, uint32_t size)
{
    struct ftp_file_state *file_state = (struct ftp_file_state *)state;

    if (!file_state)
        return GS_FTP_RET_INVAL;

    /* Read from file */
    if ((unsigned int) ftell(file_state->fd_file) != chunk * file_state->file_chunk_size) {
        if (fseek(file_state->fd_file, chunk * file_state->file_chunk_size, SEEK_SET) != 0)
            goto chunk_read_error;
    }
    if (fread(data, 1, size, file_state->fd_file) != size)
        goto chunk_read_error;

    return GS_FTP_RET_OK;

chunk_read_error:
    log_error("Filesystem read error (chunk %"PRIu32", size %"PRIu32", file chunk size %"PRIu32")",
              chunk, size, file_state->file_chunk_size);
    fclose(file_state->fd_file);
    file_state->fd_file = NULL;
    return GS_FTP_RET_IO;

}

static gs_ftp_return_t ftp_file_get_status(void *state, uint32_t chunk, int *status)
{
    struct ftp_file_state *file_state = (struct ftp_file_state *)state;

    if (!file_state || !status)
        return GS_FTP_RET_INVAL;

    *status = (file_state->mem_map[chunk] == *packet_ok);
    return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_file_set_status(void *state, uint32_t chunk)
{
    struct ftp_file_state *file_state = (struct ftp_file_state *)state;

    if (!file_state)
        return GS_FTP_RET_INVAL;

    /* Write to map file */
    if ((unsigned int) ftell(file_state->fd_map) != chunk) {
        if (fseek(file_state->fd_map, chunk, SEEK_SET) != 0)
            goto status_write_error;
    }
    if (fwrite(packet_ok, 1, 1, file_state->fd_map) != 1)
        goto status_write_error;

#if (GS_STORAGE_VFS)
    fflush(file_state->fd_map);
    fsync(fileno(file_state->fd_map));
#endif

    /* Mark in RAM map */
    file_state->mem_map[chunk] = *packet_ok;

    return GS_FTP_RET_OK;

status_write_error:
    log_error("Failed to write status for chunk %"PRIu32" %p", chunk, file_state->fd_map);
    fclose(file_state->fd_map);
    file_state->fd_map = NULL;
    return GS_FTP_RET_IO;
}

static gs_ftp_return_t ftp_file_get_crc(void *state, uint32_t *crc)
{
    return (crc != NULL && file_crc(state, crc) == 0) ? GS_FTP_RET_OK : GS_FTP_RET_INVAL;
}

static gs_ftp_return_t ftp_file_done(void *state)
{
    struct ftp_file_state *file_state = (struct ftp_file_state *)state;

    if (!file_state)
        return GS_FTP_RET_INVAL;

    /* Close files */
    if (file_state->fd_file) {
        fclose(file_state->fd_file);
        file_state->fd_file = NULL;
    }
    if (file_state->fd_map) {
        fclose(file_state->fd_map);
        file_state->fd_map = NULL;
    }
    if (file_state->mem_map) {
        free(file_state->mem_map);
        file_state->mem_map = NULL;
    }
    if (file_state->dirp) {
        closedir(file_state->dirp);
        file_state->dirp = NULL;
    }

    /* Generate map path */
    char map[GS_FTP_PATH_LENGTH + 10];
    MAKE_MAP_FILENAME(map, file_state->file_name);
    /* Remove map */
    remove(map);

//    /* Generate tmp path to rename file*/
//    char tmp[GS_FTP_PATH_LENGTH + 10];
//    MAKE_TMP_FILENAME(tmp, file_state->file_name);
//
//    /*Rename tmp file if it exists - ftp_done is called on both upload and download,
//    so we verify file existence before renaming*/
//    struct stat statbuf;
//    if(stat(tmp, &statbuf) == 0)
//    {
//        /* tmp file exists - rename it */
//        int status = rename(tmp, file_state->file_name);
//        if (status != 0) {
//            log_error("Failed renaming %s to %s %d", tmp, file_state->file_name, status);
//        }
//    }

    return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_file_abort(void *state)
{
    struct ftp_file_state *file_state = (struct ftp_file_state *)state;

    if (!file_state)
        return GS_FTP_RET_INVAL;

    /* Close files */
    if (file_state->fd_file) {
        fclose(file_state->fd_file);
        file_state->fd_file = NULL;
    }
    if (file_state->fd_map) {
        fclose(file_state->fd_map);
        file_state->fd_map = NULL;
    }
    if (file_state->mem_map) {
        free(file_state->mem_map);
        file_state->mem_map = NULL;
    }
    if (file_state->dirp) {
        closedir(file_state->dirp);
        file_state->dirp = NULL;
    }

    /* Generate map path */
    char map[GS_FTP_PATH_LENGTH + 10];
    MAKE_MAP_FILENAME(map, file_state->file_name);

    /* Remove map */
    if (remove(map) != 0) {
        log_error("Failed to remove %s", map);
        return GS_FTP_RET_IO;
    }

//    char tmp[GS_FTP_PATH_LENGTH + 10];
//    MAKE_TMP_FILENAME(tmp, file_state->file_name);
//    /* Remove file */
//    if (remove(tmp) != 0) {
//        log_error("Failed to remove %s", tmp);
//        return GS_FTP_RET_IO;
//    }

    return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_file_timeout(void *state)
{
    struct ftp_file_state *file_state = (struct ftp_file_state *)state;

    if (!file_state)
        return GS_FTP_RET_INVAL;

    /* Close files */
    if (file_state->fd_file) {
        fclose(file_state->fd_file);
        file_state->fd_file = NULL;
    }
    if (file_state->fd_map) {
        fclose(file_state->fd_map);
        file_state->fd_map = NULL;
    }
    if (file_state->mem_map) {
        free(file_state->mem_map);
        file_state->mem_map = NULL;
    }
    if (file_state->dirp) {
        closedir(file_state->dirp);
        file_state->dirp = NULL;
    }

    return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_file_remove(void *state, const char *path)
{
#if GS_FTP_USE_GLOB

    glob_t globbuf;
    int ret = glob(path, GLOB_TILDE_CHECK, NULL, &globbuf);

    for (unsigned int i = 0; i < globbuf.gl_pathc; i++) {
        log_debug("removing %s", globbuf.gl_pathv[i]);
        ret = remove(globbuf.gl_pathv[i]);
    }

    globfree(&globbuf);

    /* We assume that file does not exist on error */
    return ret ? GS_FTP_RET_NOENT : GS_FTP_RET_OK;

#else

    int ret = remove(path);

    /* We assume that file does not exist on error */
    return ret ? GS_FTP_RET_NOENT : GS_FTP_RET_OK;

#endif

}

static gs_ftp_return_t ftp_file_move(void *state, const char *from, const char *to)
{
    if (rename(from, to) != 0) {
        /* We assume that the file does not exist */
        log_warning("move: %s -> %s", from, to);
        return GS_FTP_RET_NOENT;
    }
    return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_file_copy(void *state, const char *from, const char *to)
{
    int in, out, fdfrom, fdto;
    char buf[512];

    fdfrom = open(from, O_RDONLY);
    if (fdfrom < 0)
        return GS_FTP_RET_IO;

    fdto = open(to, O_CREAT | O_TRUNC | O_WRONLY, 0755);
    if (fdto < 0) {
        close(fdfrom);
        return GS_FTP_RET_IO;
    }

    do {
        in = read(fdfrom, buf, sizeof(buf));
        if (in > 0) {
            out = write(fdto, buf, in);
            if (in != out) {
                close(fdfrom);
                close(fdto);
                return GS_FTP_RET_IO;
            }
        }
    } while (in > 1);

    close(fdfrom);
    close(fdto);

    return GS_FTP_RET_OK;
}

#if !GS_FTP_USE_GLOB
static bool valid_filename(const char * filename)
{
    if (filename && filename[0]) {
        if ((filename[0] != '.') ||
            ((strcmp(filename, ".") != 0) && (strcmp(filename, "..") != 0))) {
            return true;
        }
    }
    return false;
}
#endif

static gs_ftp_return_t ftp_file_list(void *state, const char *in_path, uint16_t *entries)
{
    struct ftp_file_state *file_state = (struct ftp_file_state *)state;

    if (!file_state || !in_path || !entries)
        return GS_FTP_RET_INVAL;

#if GS_FTP_USE_GLOB

    bool is_dir = false;
    struct stat stat_buf;

    /* Append * when listing full directory */
    int ret = stat(in_path, &stat_buf);
    if (ret == 0 && ((stat_buf.st_mode & S_IFMT) == S_IFDIR)) {
        is_dir = true;
    }
    char path[GS_FTP_PATH_LENGTH + 10];
    snprintf(path, sizeof(path), "%s%s", in_path, is_dir ? "/*" : "");

    ret = glob(path, GLOB_TILDE_CHECK, NULL, &file_state->globbuf);

    *entries = file_state->globbuf.gl_pathc;
    file_state->globcount = 0;

    if (ret == 0 || (ret == GLOB_NOMATCH && is_dir))
        return GS_FTP_RET_OK;

    /* We assume that file does not exist on error */
    globfree(&file_state->globbuf);
    return GS_FTP_RET_NOENT;

#else

    /* Copy entries */
    struct dirent *dent;
    *entries = 0;

    GS_STRNCPY(file_state->dirpath, in_path);

    file_state->dirp = opendir(file_state->dirpath);
    if (file_state->dirp == NULL) {
        return GS_FTP_RET_NOENT;
    }

    /* Count entries */
    while ((dent = readdir(file_state->dirp))) {
        if (valid_filename(dent->d_name) == true) {
            (*entries)++;
        }
    }

    /* Rewind for entry function */
    rewinddir(file_state->dirp);

    return GS_FTP_RET_OK;
#endif
}

static gs_ftp_return_t ftp_file_entry(void *state, gs_ftp_list_entry_t *ent)
{
    struct stat stat_buf;
    struct ftp_file_state *file_state = (struct ftp_file_state *)state;

    if (ent) {
        memset(ent, 0, sizeof(*ent));
    }

    if (!file_state) {
        return GS_FTP_RET_INVAL;
    }

#ifdef GS_FTP_USE_GLOB

    if (ent && (file_state->globcount < file_state->globbuf.gl_pathc)) {
        log_debug("list %s", file_state->globbuf.gl_pathv[file_state->globcount]);

        /* Name */
        GS_STRNCPY(ent->path, basename(file_state->globbuf.gl_pathv[file_state->globcount]));

        /* stats */
        const int res = stat(file_state->globbuf.gl_pathv[file_state->globcount], &stat_buf);
        if (res == 0) {
            ent->size = (uint32_t)stat_buf.st_size;
            ent->type = ((stat_buf.st_mode & S_IFMT) == S_IFDIR) ? GS_FTP_LIST_DIR : GS_FTP_LIST_FILE;
        }

        file_state->globcount++;
    } else {
        globfree(&file_state->globbuf);
        return GS_FTP_RET_NOENT;
    }

    return GS_FTP_RET_OK;

#else

    /* ent is allowed to be NULL, if an entry should be skipped */
    if (file_state->dirp == NULL) {
        return GS_FTP_RET_IO;
    }

    /* Loop through directories */
    if (ent) {
        struct dirent *dent;
        while ((dent = readdir(file_state->dirp))) {

            if (valid_filename(dent->d_name) == false) {
                continue;
            }

            /* Name */
            GS_STRNCPY(ent->path, dent->d_name);

            /* Size */
            char buf[GS_FTP_PATH_LENGTH + 1];
            if (file_state->dirpath[strlen(file_state->dirpath)-1] != '/') {
                gs_snprintf(buf, sizeof(buf), "%s/%s", file_state->dirpath, dent->d_name);
            } else {
                gs_snprintf(buf, sizeof(buf), "%s%s", file_state->dirpath, dent->d_name);
            }

            // stats
            const int res = stat(buf, &stat_buf);
            if (res == 0) {
                ent->size = (uint32_t)stat_buf.st_size;
                ent->type = ((stat_buf.st_mode & S_IFMT) == S_IFDIR) ? GS_FTP_LIST_DIR : GS_FTP_LIST_FILE;
            }

            return GS_FTP_RET_OK;
        }
    }

    closedir(file_state->dirp);
    file_state->dirp = NULL;
    return GS_FTP_RET_NOENT;

#endif
}

static gs_ftp_return_t ftp_file_mkfs(void * state, const char * path, uint8_t force)
{
    log_warning("mkfs: formatting [%s] partition (force %d)", path, force);

#if (GS_STORAGE_VFS)
    int ret = vfs_mkfs(path, force ? VFS_MKFS_FORCE : 0);
    if (ret == -ENODEV || ret == -ENOENT) {
        return GS_FTP_RET_NOENT;
    } else if (ret == -EBUSY) {
        return GS_FTP_RET_BUSY;
    } else if (ret == -EIO) {
        return GS_FTP_RET_IO;
    } else if (ret < 0) {
        // return GS_FTP_RET_INVAL;
    } else {
        return GS_FTP_RET_OK;
    }
#endif

    return GS_FTP_RET_INVAL;
}

static gs_ftp_return_t ftp_file_mkdir(void * state, const char * path, uint32_t mode)
{
    int ret = mkdir(path, mode);

    if (ret == -ENODEV || ret == -ENOENT)
        return GS_FTP_RET_NOENT;
    else if (ret == -EBUSY)
        return GS_FTP_RET_BUSY;
    else if (ret == -EIO)
        return GS_FTP_RET_IO;
    else if (ret < 0)
        return GS_FTP_RET_INVAL;

    return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_file_rmdir(void * state, const char * path)
{
    int ret = rmdir(path);

    if (ret == -ENODEV || ret == -ENOENT)
        return GS_FTP_RET_NOENT;
    else if (ret == -EBUSY)
        return GS_FTP_RET_BUSY;
    else if (ret == -EIO)
        return GS_FTP_RET_IO;
    else if (ret < 0)
        return GS_FTP_RET_INVAL;

    return GS_FTP_RET_OK;
}

static gs_ftp_return_t ftp_file_zip(void *state, const char *src, const char *dest, gs_ftp_zip_action_t action, uint32_t * comp_sz, uint32_t * decomp_sz)
{
    if((action == GS_FTP_ZIP) || (action == GS_FTP_UNZIP))
    {
        // Make sure source and destination are not the same
        if(!strcmp(src, dest))
        {
            return GS_FTP_RET_EXISTS;
        }

        // Make sure the source exist
        struct stat statbuf;
        if(stat(src, &statbuf) != 0)
        {
            return GS_FTP_RET_NOENT;
        }

        if(action == GS_FTP_ZIP) // Zip
        {
            *decomp_sz = (uint32_t)statbuf.st_size;
            if(gs_zip_compress_file(src, dest) != GS_OK)
            {
                log_error("Server: Compression failed");
                return GS_FTP_RET_IO;
            }
        }
        else // Unzip
        {
            *comp_sz = (uint32_t)statbuf.st_size;
            if(gs_zip_decompress_file(src, dest) != GS_OK)
            {
                log_error("Server: Decompression failed");
                return GS_FTP_RET_IO;
            }
        }

        if(stat(dest, &statbuf) != 0)
        {
            log_error("Server: Failed to create data file");
            return GS_FTP_RET_IO;
        }

        if(action == GS_FTP_ZIP)
        {
            *comp_sz = (uint32_t)statbuf.st_size;
        }
        else
        {
            *decomp_sz = (uint32_t)statbuf.st_size;
        }

        return GS_FTP_RET_OK;
    }

    return GS_FTP_RET_INVAL;
}

static const gs_ftp_backend_t backend_file = {
    .init        = ftp_file_init,
    .release     = ftp_file_release,
    .upload      = ftp_file_upload,
    .download    = ftp_file_download,
    .chunk_write = ftp_file_write_chunk,
    .chunk_read  = ftp_file_read_chunk,
    .status_get  = ftp_file_get_status,
    .status_set  = ftp_file_set_status,
    .list        = ftp_file_list,
    .entry       = ftp_file_entry,
    .remove      = ftp_file_remove,
    .move        = ftp_file_move,
    .copy        = ftp_file_copy,
    .mkfs        = ftp_file_mkfs,
    .mkdir       = ftp_file_mkdir,
    .rmdir       = ftp_file_rmdir,
    .crc         = ftp_file_get_crc,
    .zip         = ftp_file_zip,
    .abort       = ftp_file_abort,
    .done        = ftp_file_done,
    .timeout     = ftp_file_timeout,
};

const gs_ftp_backend_t * gs_ftp_server_backend_file(void)
{
    return &backend_file;
}
