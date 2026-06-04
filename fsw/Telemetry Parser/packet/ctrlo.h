#pragma once


#include <stdint.h>
#include "components/ccsds.h"


typedef struct {
    float AttitudeSolution[7];
    float AngularVelocityError[3];
    float AngleError;
} ADCS_OutputTlm_Attitude_t;

typedef struct {
    float BestPosition[3];
    float BestVelocity[3];
    uint8_t PositionStdStatus;
    uint8_t VelocityStdStatus;
    uint8_t Spare[2];   /* 32-bit Alignment. */
} ADCS_OutputTlm_Orbit_t;

typedef struct {
    uint16_t IntegratorErrFlag;
    uint8_t ControlMode;
    uint8_t ControlModeUpdated;
    uint8_t AcuatorFlagUpdated;
    uint8_t FlagDetumbling;
    uint8_t FlagForcedSpinning;
    uint8_t FlagSensors;
    uint8_t FlagCss;
    uint8_t FlagDumping[3];
} ADCS_OutputTlm_ControlFlags_t;

typedef struct {
    ADCS_OutputTlm_Attitude_t       Attitude;
    ADCS_OutputTlm_Orbit_t          Orbit;
    ADCS_OutputTlm_ControlFlags_t   Flags;
} ADCS_OutputTlm_Payload_t;

typedef struct {
    ccsds_tlm_header_t tlmHeader;
    ADCS_OutputTlm_Payload_t ctrlo;
} ctrlo_packet_t;
