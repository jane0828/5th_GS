/*******************************************************************************************
 * @file ftp_tools.h
 * 
 * @author Han-Gyeol Ryu (morimn21@gmail.com)
 * 
 * @brief Miscellaneous tools for debugging purposes.
 * 
 * @version 1.0
 * 
 * @date 2022-03-22
 * 
 * @copyright Copyright (c) 2021 Astrodynamics & Control Lab. Yonsei Univ.
 * 
 ******************************************************************************************/
#ifndef _FTP_TOOLS_H_
#define _FTP_TOOLS_H_

#include "cfe.h"

typedef enum {
    FTP_LOG_INFO        = 1,
    FTP_LOG_ERROR       = 2,
    FTP_LOG_NORMAL      = 3,
} ftp_log_type;


#ifdef MIM_FTP_DEBUG
#define ftp_log(str, mode, ...)         ftp_logger(str, __FUNCTION__, mode, ##__VA_ARGS__)
#define ftp_log_normal(str, ...)        ftp_logger(str, __FUNCTION__, FTP_LOG_NORMAL, ##__VA_ARGS__)
#define ftp_log_error(str, ...)         ftp_logger(str, __FUNCTION__, FTP_LOG_ERROR, ##__VA_ARGS__)
#define ftp_log_info(str, ...)          ftp_logger(str, __FUNCTION__, FTP_LOG_INFO, ##__VA_ARGS__)
#else
#define ftp_log(str, mode, ...)
#define ftp_log_normal(str, ...)
#define ftp_log_error(str, ...)
#define ftp_log_info(str, ...)
#endif

void ftp_logger(const char* str, const char* caller, ftp_log_type mode, ...);


#endif