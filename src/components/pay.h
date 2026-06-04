#ifndef _PAY_H_
#define _PAY_H_


#include <stdint.h>


typedef struct {
    uint16_t Temperature;
    uint16_t ExposureTime;
} PAY_HkTlm_Pl_t;

typedef struct {
    uint32_t UnixTime;
    uint16_t Storage;
    uint16_t ImgIndex;
} PAY_HkTlm_PlBrd_t;

typedef struct {
    PAY_HkTlm_Pl_t Pl;
    PAY_HkTlm_PlBrd_t PlBrd;
} PAY_HkTlm_Payload_t;


#endif