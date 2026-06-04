#pragma once


#include <stdint.h>
#include "components/ccsds.h"


typedef struct {
    uint32_t Ticks;
    uint32_t TS;
    float CalibratedQuaternion_Qw;
    float CalibratedQuaternion_Qx;
    float CalibratedQuaternion_Qy;
    float CalibratedQuaternion_Qz;
    float TRACK_Confidence;
    float TRACK_Qw;
    float TRACK_Qx;
    float TRACK_Qy;
    float TRACK_Qz;
    float LISA_Qw;
    float LISA_Qx;
    float LISA_Qy;
    float LISA_Qz;
    float LISA_Percentageclose;
    uint32_t StableCount;
    uint8_t IsTrustworthy;
    uint8_t SolutionStrategy;
    uint8_t Padding[2];
} SNSR_STT_Measurements_t;

typedef struct {
    float AngleX[2];
    float AngleY[2];
    float SunDetection[2];
    uint8_t ErrorCode[2];
    uint8_t Padding[2];
} SNSR_FSS_Measurements_t;

typedef struct {
    float RawVoltageOut[5];
} SNSR_CSS_Measurements_t;

typedef struct {
    float Temperature[5];
} SNSR_TMP_Measurements_t;

typedef struct {
    float IMU_AngularVelocity[3];
    float IMU_Temperature;
} SNSR_IMU_Measurements_t;

typedef struct {
    float MMT_FieldOutputs[3];
} SNSR_MMT_Measurements_t;

typedef struct {
    SNSR_STT_Measurements_t STT;
    SNSR_FSS_Measurements_t FSS;
    SNSR_IMU_Measurements_t IMU;
    SNSR_MMT_Measurements_t MMT;
    SNSR_CSS_Measurements_t CSS;
    SNSR_TMP_Measurements_t TMP;
    bool FaultySTT;
    bool FaultyIMU;
    bool FaultyMMT;
    bool FaultyFSS[2];
} SNSR_Meas_Payload_t;

typedef struct {
    ccsds_tlm_header_t  tlmHeader;
    SNSR_Meas_Payload_t snsr;
} snsr_packet_t;
