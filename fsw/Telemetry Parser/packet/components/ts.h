#ifndef _TS_H_
#define _TS_H_


#include <stdint.h>


typedef struct {
    uint16_t  CmdCounter;
    uint16_t  CmdErrCounter;
    uint16_t  CmdDropCounter;
    uint16_t  CmdInsertCounter;
} TS_HkTlm_Payload_t;


#endif
