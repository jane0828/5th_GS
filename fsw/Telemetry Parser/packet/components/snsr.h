#ifndef _SNSR_H_
#define _SNSR_H_


#include <stdint.h>


typedef struct {
    uint8_t IsolationStatus;
    uint8_t Padding[1];
} SNSR_CSS_Telemetry_t;

typedef struct {
    float Temperature;
    uint16_t ReadWriteErrCounter;
    uint8_t Sleep;
    uint8_t Padding[1];
} SNSR_IMU_Telemetry_t;

typedef struct {
    uint16_t MfieldMeasErrCounter;
} SNSR_MMT_Telemetry_t;

typedef struct {
    float MCUTemperature;
    uint16_t ErrCounter;
    uint8_t Padding[2];
} SNSR_STT_Telemetry_t;

typedef struct {
    SNSR_STT_Telemetry_t STT;
    SNSR_IMU_Telemetry_t IMU;
    SNSR_MMT_Telemetry_t MMT;
    SNSR_CSS_Telemetry_t CSS;
} SNSR_HkTlm_Payload_t;


#endif
