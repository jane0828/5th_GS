#pragma once
#ifndef _MIMAN_FTP_H_
#define _MIMAN_FTP_H_

int ftp_list_callback(uint16_t entries, const gs_ftp_list_entry_t * listent, void * data);
void ftp_callback(const gs_ftp_info_t * info);
void * ftp_uplink_onorbit(void * param);
void * ftp_uplink_force(void * param);
void * ftp_downlink_onorbit(void * param);
void * ftp_downlink_force(void * param);
void * ftp_list_onorbit(void * );
void * ftp_move_onorbit(void * );
void * ftp_remove_onorbit(void * );
void * ftp_copy_onorbit(void * );
void * ftp_mkdir_onorbit(void * );
void * ftp_rmdir_onorbit(void * );
ftpinfo* filelisthandler(filelist * list, uint16_t target, uint16_t status, int index = 0);


#endif