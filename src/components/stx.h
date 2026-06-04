#ifndef _STX_H_
#define _STX_H_


#include <stdint.h>


typedef struct {
    float PaTemperature;
    float OutputPower;
    float BatteryCurrent;
    float BatteryVoltage;
    uint8_t ResetCounter;
    uint8_t ControlMode;
    uint8_t Status;
    uint8_t Padding[1];
} STX_HkTlm_BoardTlm_t;

typedef struct {
    uint16_t TxReadyErrCount;
    uint16_t TxEndErrCount;
    uint16_t LastBufferOverruns;
    uint16_t LastBufferUnderruns;
} STX_HkTlm_TxStats_t;

typedef struct {
    STX_HkTlm_BoardTlm_t BrdTlm;
    STX_HkTlm_TxStats_t TxStats;
} STX_HkTlm_Payload_t;


#endif
