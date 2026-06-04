#pragma once
#ifndef __MIMAN_ORBITAL_H_
#define __MIMAN_ORBITAL_H_
#include <string.h>
#include <iostream>
//For SGP4(Testing)
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>
#include <termios.h>
//Custom Headers
#include "miman_config.h"

#define RAD_TO_DEG 57.2958

//Tracking Satellite Lists

struct Engage{
    int NowSat;
    int rotatorinfo;
};

typedef struct{
    uint8_t fingerprint[4];
    uint32_t Checksum;
    
    uint32_t sec;
    uint32_t micsec;

    uint32_t epochYear;
    double epochDay;
    double INC;
    double RAAN;
    double ECC;
    double AOP;
    double MA;
    double MM;
    uint8_t padding[4];
}__attribute__((packed)) FSWTle;



struct Antenna {
    int az_tag = 0;
    int el_tag = 0;
    int az_now = 0;
    int el_now = 0;
};

//TLE Functions
int init_rotator();
void fin_rotator();
TLEinfo * SetTLEinfo(const char * label);
void Initialize_TLESettings();
void Delete_TLESettings();
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
bool DownloadTLE(char *TLEURL, char *Local);
// double FindMaxElevation(Observer obs, SGP4& sgp4, const DateTime& aos, const DateTime& los);
// DateTime FindCrossingPoint(Observer obs, SGP4& sgp4, const DateTime& initial_time1, const DateTime& initial_time2, bool finding_aos);
// Satellite GeneratePassList(Observer obs, SGP4& sgp4, const DateTime& start_time, const DateTime& end_time, const int time_step, Satellite Sat);
void * TrackingSatellites(void *);
int ReadTLELines_Errorhandling(char * TLEFilename, char * trackingTLEname, bool fds, bool sgp4on);
int ReadTLELines(char *TLEFilename, bool fds = false, bool sgp4on = false);
void AllocateSatelliteClass();
void * SatelliteInitialize(void * sgp4on);
void RotatorSetTo(int azimuth, int elevation);
void * RotatorEnter(void *);
void * RotatorReadInfo(void *);
void * Doppler(void *);
// void SetNowTracking(int index);
int GetNowTracking();
void * Sattracker(void * Nowengage);
void FSWTleConverter(Tle& TleInfo, FSWTle * fswtle);
void SetNowTracking_impl(int index, const char* file, int line);
#define SetNowTracking(i) SetNowTracking_impl((i), __FILE__, __LINE__)

#endif
