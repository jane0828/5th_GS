#ifndef _RWA_H_
#define _RWA_H_


#include <stdint.h>


typedef struct {
    uint8_t WheelEnabled;
    uint8_t WheelErrFlags[3];
    int16_t WheelSpeed[3];
    int16_t WheelReferenceSpeed[3];
} RWA_HkTlm_WheelTlm_t;


typedef struct {
    RWA_HkTlm_WheelTlm_t WheelTlm;
} RWA_HkTlm_Payload_t;


#endif
