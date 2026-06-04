#ifndef _MPL_FTP_CRC_H_
#define _MPL_FTP_CRC_H_
#include "cfe.h"

uint32 io_crc_get_crc32(void* Data, int Size);

uint32 io_crc_get_file_crc32(FILE* fp);


#endif
