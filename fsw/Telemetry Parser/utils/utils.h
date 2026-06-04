/*******************************************************************************************
 * @file utils.h
 * 
 * @author Han-Gyeol Ryu (morimn21@gmail.com)
 * 
 * @brief Parser utilites.
 * 
 * @version 2.0
 * 
 * @date 2022-07-01
 * 
 * @copyright Copyright (c) 2021 Astrodynamics & Control Lab. Yonsei Univ.
 * 
 ******************************************************************************************/
#pragma once

#include <stdint.h>
#include <stdio.h>

uint32_t swap32(uint32_t u32);
uint16_t swap16(uint16_t u16);

typedef enum {
    LL_NORMAL   = 0,
    LL_ERROR    = 1,
    LL_WARN     = 2,
    LL_INFO     = 3,
} log_level; 

int Logger(const char* caller, log_level level, const char* str, ...);
int Logger(FILE* stream, const char* caller, log_level level, const char* str, ...);

#define LogError(str, ...)              Logger(__func__, LL_ERROR, str, ##__VA_ARGS__)
#define LogWarning(str, ...)            Logger(__func__, LL_WARN, str, ##__VA_ARGS__)
#define LogInfo(str, ...)               Logger(__func__, LL_INFO, str, ##__VA_ARGS__)
#define Log(str, ...)                   Logger(__func__, LL_INFO, str, ##__VA_ARGS__)

#define fLogError(fp, str, ...)         Logger(fp, __FUNCTION__, LL_ERROR, str, ##__VA_ARGS__)
#define fLogWarning(fp, str, ...)       Logger(fp, __FUNCTION__, LL_WARN, str, ##__VA_ARGS__)
#define fLogInfo(fp, str, ...)          Logger(fp, __FUNCTION__, LL_INFO, str, ##__VA_ARGS__)
#define fLog(fp, str, ...)              Logger(fp, __FUNCTION__, LL_INFO, str, ##__VA_ARGS__)
