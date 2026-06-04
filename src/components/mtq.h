#ifndef _MTQ_H_
#define _MTQ_H_


#include <stdint.h>


typedef struct {
    uint8_t EnableStatus;
    int8_t Duty[3];
} MTQ_HkTlm_Payload_t;


#endif
