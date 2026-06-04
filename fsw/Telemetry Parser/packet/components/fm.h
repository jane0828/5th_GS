#ifndef _FM_H_
#define _FM_H_


#include <stdint.h>


typedef struct {
    uint32_t TimeSecond;
    uint32_t TimeSubsecond;
    uint16_t RebootCount;
    uint8_t CurrentMode;
    uint8_t CurrentSubmode;
    uint8_t PreviousMode;
    uint8_t PreviousSubmode;
    uint8_t LastRebootCause;
    uint8_t DeploymentPhase;
    uint32_t Reserved;
} FM_HkTlm_Payload_t;


#endif
