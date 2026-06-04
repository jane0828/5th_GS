#ifndef _EPS_H_
#define _EPS_H_


#include <stdint.h>


typedef struct {
    uint8_t Mode;
    uint8_t ResetCause;
    uint16_t ResetCounter;
    uint16_t OutputBusOnField;
    uint16_t OutputBusOverCurrentFaultField;
} EPS_HkTlm_System_t;

typedef struct {
    uint16_t BatteryStatus;
    int16_t BatteryInputVoltage;
    int16_t BatteryInputCurrent;
    int16_t BatteryTemperature;
} EPS_HkTlm_Battery_t;

typedef struct {
    int16_t MpptInputVoltageMB[3];
    int16_t MpptInputCurrentMB[3];
    int16_t ChannelCurrent3V3[4];
    int16_t ChannelCurrent5V[4];
    int16_t BoardSupplyVoltage;
    int16_t BoardTemperature;
} EPS_HkTlm_MotherBoard_t;

typedef struct {
    int16_t MpptInputVoltageDB[2];
    int16_t MpptInputCurrentDB[2];
    int16_t ChannelCurrent5V;
    int16_t ChannelCurrent12V;
} EPS_HkTlm_DaughterBoard_t;


typedef struct {
    EPS_HkTlm_System_t          System;
    EPS_HkTlm_Battery_t         Battery;
    EPS_HkTlm_MotherBoard_t     MotherBoard;
    EPS_HkTlm_DaughterBoard_t   DaughterBoard;
} EPS_HkTlm_Payload_t;


#endif