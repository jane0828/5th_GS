#ifndef _UTRX_H_
#define _UTRX_H_


#include <stdint.h>


typedef struct {
    uint32_t      TotalTxByte;
    uint32_t      TotalRxByte;
    uint32_t      LastContact;
    uint16_t      BootCount;
    uint16_t      BootCause;
    int16_t       LastRSSI;
    int16_t       LastRxErr;
    int16_t       BoardTemperature;
    uint8_t       ActiveConf;
    uint8_t       TxDuty;
} UTRX_HkTlm_AX100Tlm_t;

typedef struct {
    UTRX_HkTlm_AX100Tlm_t AX100;
} UTRX_HkTlm_Payload_t;


#endif
