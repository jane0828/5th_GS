#ifndef RYU_FTP_COMPAT_H
#define RYU_FTP_COMPAT_H

/*
 * Build-only vocabulary bridge for the byte-identical Ryu source.
 * The public adapter remaps download callback events to GS_FTP_INFO_DL_*.
 */
#define GS_FTP_MAX_CHUNK_SIZE GS_FTP_CHUNK_SIZE
#define GS_FTP_INFO_FILE GS_FTP_INFO_UL_FILE
#define GS_FTP_INFO_COMPLETED GS_FTP_INFO_UL_COMPLETED
#define GS_FTP_INFO_PROGRESS GS_FTP_INFO_UL_PROGRESS

#endif
