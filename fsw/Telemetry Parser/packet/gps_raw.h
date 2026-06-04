#pragma once


#include <stdint.h>
#include "components/ccsds.h"


#define MAX_SAT_NUM             8


typedef struct {
    /* Reference week number in u32 to be aligned properly. */
    uint32_t          Week;
    int32_t           ms;
} GPS_HeaderReferenceTime;


/* RANGEGPSL1 log measurements. */
typedef struct {
    double                  Pseudorange;
    double                  CarrierPhase;
    float                   PseudorangeStd;
    float                   CarrierPhaseStd;
    float                   Doppler;
    float                   CNR;
    float                   Locktime;
    uint16_t                PRN;
    uint16_t                TrackingStatus;
} GPS_MeasurementsAux_RangeGPSL1_Comp_t;

typedef struct {
    GPS_HeaderReferenceTime Time;
    GPS_MeasurementsAux_RangeGPSL1_Comp_t Comp[MAX_SAT_NUM];
    uint8_t                 NumSat;
} GPS_MeasurementsAux_RangeGPSL1_t;


/* SATXYZ2 log measurements. */
typedef struct {
    double                  SatX;
    double                  SatY;
    double                  SatZ;
    double                  ClockCorrection;
    double                  IonosphereDelay;
    double                  TroposphereDelay;
    uint32_t                SatId;
} GPS_MeasurementsAux_SatXYZ2_Comp_t;

typedef struct {
    GPS_HeaderReferenceTime Time;
    GPS_MeasurementsAux_SatXYZ2_Comp_t Comp[MAX_SAT_NUM];
    uint8_t                 NumSat;
} GPS_MeasurementsAux_SatXYZ2_t;


/* Auxilary measurement package. */
typedef struct {
    GPS_MeasurementsAux_RangeGPSL1_t    RangeGPSL1;
    GPS_MeasurementsAux_SatXYZ2_t       SatXYZ2;
} GPS_MeasurementsAux_t_Payload_t;


typedef struct {
    ccsds_tlm_header_t              tlmHeader;
    GPS_MeasurementsAux_t_Payload_t gps;
} gps_packet_t;
