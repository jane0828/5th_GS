#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <curl/curl.h>
#include <curl/easy.h>
#include <vector>
#include <list>
#include <pthread.h>
#include <Util.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

//For SGP4(Testing)
#include <CoordTopocentric.h>
#include <CoordGeodetic.h>
#include <Observer.h>
#include <SGP4.h>

//Custom Headers
#include "miman_orbital.h"
#include "miman_radial.h"
#include "miman_config.h"
#include "miman_imgui.h"
#include "miman_s_control.h"
#include <gs/util/crc32.h>

int rotator = -1;
char ControlBuf[32];
char ReadBuf[32];

Observer Yonsei(37.564021, 126.934600, 0.05);

extern StateCheckUnit State;
extern Console console;


Tle TLElistup[32768];
Tle FDSlistup;
int NowSat = -2;

struct Antenna Now;

float AZI_TOLERANCE = 3.0;
float ELE_TOLERANCE = 3.0;

struct termios new_rotator_tio;

extern Setup * setup;
extern int NowTLE;
extern Sband * sgs;

int init_rotator() {
	rotator = open(setup->Rotator_devname, O_RDWR | O_NOCTTY | O_NDELAY); // Or O_RDWR | O_NOCTTY? // @@ 로테이터 열기
    // open 함수 https://www.it-note.kr/19
    // O_RDWR은 파일 디스크립터인 fd를 읽기와 쓰기 모드로 열기 위한 지정이며
    // O_NOCCTY와 O_NONBLOCK는 시리얼 통신 장치에 맞추어 추가했습니다.

    if (rotator<0)
    {
		return -1;
	}
    // tcgetattr(rotator, &new_rotator_tio);
    tcflush(rotator, TCIOFLUSH);
    switch(setup->Rotator_baud)
    {
        case 4800 : new_rotator_tio.c_cflag = B4800; break;
        case 9600 : new_rotator_tio.c_cflag = B9600; break;
        default : break;
    }
    

    new_rotator_tio.c_cflag |= (CLOCAL | CREAD); 	// Receiver and local mode
    new_rotator_tio.c_cflag &= ~HUPCL;				// Do not change modem signals
    new_rotator_tio.c_cflag &= ~CSIZE;
    new_rotator_tio.c_cflag |= CS8;					// 8 bit // @@ S대역 포지셔너(로테이터) 컨트롤러이랑 같은 옵션이듯 // @@ 시리얼 통신 설정방법: https://softtone-someday.tistory.com/15, https://blog.naver.com/choi125496/130034222760
    new_rotator_tio.c_cflag &= ~CSTOPB;				// 1 stop bit
    new_rotator_tio.c_cflag &= ~PARENB;				// No parity check
    new_rotator_tio.c_cflag &= ~CRTSCTS;			// Enable hardware / software flow control

    new_rotator_tio.c_iflag &= ~IXON;				// No handshaking

    // Raw output
    new_rotator_tio.c_oflag &= ~OPOST; 				// Prevent special interpretation of output bytes (e.g. newline chars)

    // Canonical input
    new_rotator_tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);	// Non-canonical read

    // Initialize control characters
    new_rotator_tio.c_cc[VTIME]    = 5;
    new_rotator_tio.c_cc[VMIN]     = 1;		// Block 0.5(VTIME) sec after read first(VMIN) character

    tcflush(rotator, TCIOFLUSH);
    


    if (tcsetattr(rotator, TCSANOW, &new_rotator_tio) != 0)
    {
        console.AddLog("[ERROR]##Fail to configure rotator serial port\n");
        return -1;
    }
    	
    return 0;
}

void fin_rotator()
{
    if (rotator > 0)
    {
        tcflush(rotator, TCIOFLUSH);
        tcsetattr(rotator, TCSANOW, &new_rotator_tio);
        tcflush(rotator, TCIOFLUSH);
        close(rotator);
    }
}

void SatelliteObject::Initializer()
{
    try
    {
        
        this->start_date = DateTime::Now(true);
        this->end_date = DateTime(start_date.AddDays(7.0));
        this->_eci = this->sgp4.FindPosition(DateTime::Now(true));
        this->_eci_1secfuture = this->sgp4.FindPosition(DateTime::Now(true).AddSeconds(this->futuretime));
        this->topo = this->obs.GetLookAngle(this->_eci);
        this->_topo_1secfuture = this->obs.GetLookAngle(this->_eci_1secfuture);
        this->geo = this->_eci.ToGeodetic();
        this->GeneratePassList(180);
        for(int i = 0; i < 64; i++)
        {
            this->AOSeci[i] = this->sgp4.FindPosition(this->_nextaos[i]);
            this->AOStopo[i] = this->obs.GetLookAngle(this->AOSeci[i]);
        }
        
        
    }
    catch(const std::exception& e)
    {
        console.AddLog("[DEBUG]##%s\t%s\t",this->_name, e.what());
    }
}

char * SatelliteObject::Name()
{
    return this->_name;
}

int SatelliteObject::Azimuth()
{
    return static_cast<int>(this->topo.azimuth * RAD_TO_DEG);
}

int SatelliteObject::Elevation()
{
    return static_cast<int>(this->topo.elevation * RAD_TO_DEG);
}

int SatelliteObject::Azimuth_1secfuture()
{
    return static_cast<int>(this->_topo_1secfuture.azimuth * RAD_TO_DEG);
}

int SatelliteObject::Elevation_1secfuture()
{
    return static_cast<int>(this->_topo_1secfuture.elevation * RAD_TO_DEG);
}

int SatelliteObject::AOS_Azimuth(int i)
{
    return static_cast<int>(this->AOStopo[i].azimuth * RAD_TO_DEG);
}

int SatelliteObject::AOS_Elevation(int i)
{
    return static_cast<int>(this->AOStopo[i].elevation * RAD_TO_DEG);
}

double SatelliteObject::MaxElevation(int i)
{
    return this->_max_elevation[i] * RAD_TO_DEG;
}

float SatelliteObject::DisplayPoint_Longitude()
{
    return (float)(this->geo.longitude * RAD_TO_DEG / 180);
}

float SatelliteObject::DisplayPoint_Latitude()
{
    return (float)(this->geo.latitude * RAD_TO_DEG / 90);
}

void SatelliteObject::Update()
{
    this->sgp4.SetTle(tle);
    this->_eci = this->sgp4.FindPosition(DateTime::Now(true));
    this->_eci_1secfuture = this->sgp4.FindPosition(DateTime::Now(true).AddSeconds(this->futuretime));
    this->topo = this->obs.GetLookAngle(this->_eci);
    this->_topo_1secfuture = this->obs.GetLookAngle(this->_eci_1secfuture);
    this->geo = this->_eci.ToGeodetic();
}

void SatelliteObject::GeneratePath()
{
    Eci ECIpath;
    CoordTopocentric TOPOpath;
    DateTime test_date = DateTime(this->_nextaos[0]);
    DateTime fin_date = DateTime(this->_nextlos[0]);
    TimeSpan step = fin_date - test_date;
    step = TimeSpan(step.Ticks() / 1023);
    this->CrossPath = 0;
    for(int i = 0; i < 1024; i++)
    {
        ECIpath = this->sgp4.FindPosition(test_date);
        TOPOpath = this->obs.GetLookAngle(ECIpath);
        path_az[i] = TOPOpath.azimuth;
        path_el[i] = TOPOpath.elevation;
        test_date = test_date + step;
    }
    // printf("Satellite : %s\n", this->Name());
    double cal;
    for(int i = 0; i < 1023; i++)
    {
        
        cal = abs(path_az[i + 1] - path_az[i]) * RAD_TO_DEG;
        // printf("Cal : %lf, az : %lf\n", cal, path_az[i]);
        if(cal > 90.0f)
        {
           
            //Counterclockwise Crossing
            if(this->path_az[0] * RAD_TO_DEG < 90 && this->path_az[1023] * RAD_TO_DEG > 90)
            {
                console.AddLog("Azimuth Crossing Path(Counterclockwise)");
                this->CrossPath = 1;
            }
                
            //Clockwise Crossing
            else if(this->path_az[0] * RAD_TO_DEG > 90 && this->path_az[1023] * RAD_TO_DEG < 90)
            {
                console.AddLog("Azimuth Crossing Path(Clockwise)");
                this->CrossPath = -1;

            }
            //Require Manual Control
            else
            {
                console.AddLog("This Path Requires manual control.");
                this->CrossPath = -2;
            }
            break;
        }
    }

}

bool SatelliteObject::GeneratePassList(const int time_step)
{
    bool found_aos = false;
    DateTime previous_date = DateTime(this->start_date);
    DateTime current_date = DateTime(this->start_date);

    for(int i = 0; i < 64; i++)
    {
        while (current_date < this->end_date)
        {
            bool end_of_pass = false;

            /*
            * calculate satellite position
            */
            Eci eci = this->sgp4.FindPosition(current_date);
            CoordTopocentric topo = this->obs.GetLookAngle(eci);

            if (!found_aos && topo.elevation > 0.0)
            {
                /*
                * aos hasnt occured yet, but the satellite is now above horizon
                * this must have occured within the last time_step
                */
                if (this->start_date == current_date)
                {
                    /*
                    * satellite was already above the horizon at the start,
                    * so use the start time
                    */
                    this->_nextaos[i] = this->start_date;
                }
                else
                {
                    /*
                    * find the point at which the satellite crossed the horizon
                    */
                    this->_nextaos[i] = this->FindCrossingPoint(this->obs, this->sgp4, previous_date, current_date, true);
                }
                found_aos = true;
            }
            else if (found_aos && topo.elevation < 0.0)
            {
                found_aos = false;
                /*
                * end of pass, so move along more than time_step
                */
                end_of_pass = true;
                /*
                * already have the aos, but now the satellite is below the horizon,
                * so find the los
                */
                this->_nextlos[i] = this->FindCrossingPoint(this->obs, this->sgp4, previous_date, current_date, false);

                this->_max_elevation[i] = this->FindMaxElevation(this->obs, this->sgp4, this->_nextaos[i], this->_nextlos[i]);
            }

            /*
            * save current time
            */
            previous_date = current_date;

            if (end_of_pass)
            {
                /*
                * at the end of the pass move the time along by 30mins
                */
                current_date = current_date + TimeSpan(0, 30, 0);
                found_aos = false;
                end_of_pass = false;
                break;
            }
            else
            {
                /*
                * move the time along by the time step value
                */
                current_date = current_date + TimeSpan(0, 0, time_step);
            }

            if (current_date > end_date)
            {
                /*
                * dont go past end time
                */
                current_date = this->end_date;
            }
        };

        if (found_aos)
        {
            /*
            * satellite still above horizon at end of search period, so use end
            * time as los
            */
            this->_nextlos[i] = this->end_date;
            this->_max_elevation[i] = this->FindMaxElevation(this->obs, this->sgp4, this->_nextaos[i], this->_nextlos[i]);
            return false;
        }
    }

    
}

double SatelliteObject::FindMaxElevation(Observer obs, SGP4& sgp4, const DateTime& aos, const DateTime& los)
{
    //Observer obs(user_geo);
    bool running;

    double time_step = (los - aos).TotalSeconds() / 9.0;
    DateTime current_time(aos); //! current time
    DateTime time1(aos); //! start time of search period
    DateTime time2(los); //! end time of search period
    double max_elevation; //! max elevation

    running = true;

    do
    {
        running = true;
        max_elevation = -99999999999999.0;
        while (running && current_time < time2)
        {
            /*
            * find position
            */
            Eci eci = sgp4.FindPosition(current_time);
            CoordTopocentric topo = obs.GetLookAngle(eci);

            if (topo.elevation > max_elevation)
            {
                /*
                * still going up
                */
                max_elevation = topo.elevation;
                /*
                * move time along
                */
                current_time = current_time.AddSeconds(time_step);
                if (current_time > time2)
                {
                    /*
                    * dont go past end time
                    */
                    current_time = time2;
                }
            }
            else
            {
                /*
                * stop
                */
                running = false;
            }
        }

        /*
        * make start time to 2 time steps back
        */
        time1 = current_time.AddSeconds(-2.0 * time_step);
        /*
        * make end time to current time
        */
        time2 = current_time;
        /*
        * current time to start time
        */
        current_time = time1;
        /*
        * recalculate time step
        */
        time_step = (time2 - time1).TotalSeconds() / 9.0;
    }
    while (time_step > 1.0);
        return max_elevation;
}

DateTime SatelliteObject::FindCrossingPoint(Observer obs, SGP4& sgp4, const DateTime& initial_time1, const DateTime& initial_time2, bool finding_aos)
{
    //Observer obs(user_geo);
    bool running;
    int cnt;

    DateTime time1(initial_time1);
    DateTime time2(initial_time2);
    DateTime middle_time;

    running = true;
    cnt = 0;
    while (running && cnt++ < 16)
    {
        middle_time = time1.AddSeconds((time2 - time1).TotalSeconds() / 2.0);
        /*
        * calculate satellite position
        */
        Eci eci = sgp4.FindPosition(middle_time);
        CoordTopocentric topo = obs.GetLookAngle(eci);

        if (topo.elevation > 0.0)
        {
            /*
            * satellite above horizon
            */
            if (finding_aos)
            {
                time2 = middle_time;
            }
            else
            {
                time1 = middle_time;
            }
        }
        else
        {
            if (finding_aos)
            {
                time1 = middle_time;
            }
            else
            {
                time2 = middle_time;
            }
        }

        if ((time2 - time1).TotalSeconds() < 1.0)
        {
            /*
            * two times are within a second, stop
            */
            running = false;
            /*
            * remove microseconds
            */
            int us = middle_time.Microsecond();
            middle_time = middle_time.AddMicroseconds(-us);
            /*
            * step back into the pass by 1 second
            */
            middle_time = middle_time.AddSeconds(finding_aos ? 1 : -1);
        }
    }

    /*
    * go back/forward 1second until below the horizon
    */
    running = true;
    cnt = 0;
    while (running && cnt++ < 6)
    {
        Eci eci = sgp4.FindPosition(middle_time);
        CoordTopocentric topo = obs.GetLookAngle(eci);
        if (topo.elevation > 0)
        {
            middle_time = middle_time.AddSeconds(finding_aos ? -1 : 1);
        }
        else
        {
            running = false;
        }
    }

    return middle_time;
}

TLEinfo * SetTLEinfo(const char * label)
{
    TLEinfo* res = (TLEinfo *)malloc(sizeof(TLEinfo));
    sprintf(res->remote, "https://celestrak.org/NORAD/elements/gp.php?GROUP=%s&FORMAT=tle", label);
    sprintf(res->local, "../bin/tle/%s.dat", label);
    sprintf(res->label, label);
    return res;
}

void Initialize_TLESettings()
{
    State.tleinfolistup[0] = SetTLEinfo("cubesat");
    State.tleinfolistup[1] = SetTLEinfo("last-30-days");
    State.tleinfolistup[2] = SetTLEinfo("weather");
    State.tleinfolistup[3] = SetTLEinfo("dmc");
    State.tleinfolistup[4] = SetTLEinfo("amateur");
    State.tleinfolistup[5] = SetTLEinfo("active");

    // printf("%s\t%s\t%s\n", State.tleinfolistup[0]->label, State.tleinfolistup[0]->remote, State.tleinfolistup[0]->local);
}

void Delete_TLESettings()
{
    for(int i = 0; i < sizeof(State.tleinfolistup); i++)
    {
        if(State.tleinfolistup[i] != NULL)
            free(State.tleinfolistup[i]);
        memset(State.tleinfolistup[i], 0, sizeof(State.tleinfolistup[i]));
    }
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

bool DownloadTLE(char *TLEURL, char *Local)
{
    //TLE Propagator Testing Part
    //TLE Download
    CURL *TLE_dcurl;
    FILE *TLE_fp;
    CURLcode TLEres;
    if (TLE_dcurl = curl_easy_init())
    {
        TLE_fp = fopen(Local, "wb");
        curl_easy_setopt(TLE_dcurl, CURLOPT_URL, TLEURL);
        curl_easy_setopt(TLE_dcurl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(TLE_dcurl, CURLOPT_WRITEDATA, TLE_fp);
        TLEres = curl_easy_perform(TLE_dcurl);

        curl_easy_cleanup(TLE_dcurl);
        fclose(TLE_fp);
        return true;
    }
    else
    {
        return false;
    }
}

void * TrackingSatellites(void *)
{
    while(State.TrackThread)
    {
        if(!State.NowTracking)
            continue;
        if((State.Fatellites != NULL) && (State.Fatellites->use != false))
        {
            try
            {
                State.Fatellites->Update();
            }
            catch(const std::exception& e)
            {
                console.AddLog("[ERROR]##Unable to Track Satellite : %s", State.Fatellites->Name());
                console.AddLog(e.what());
            }
        }
        for(int i = 0; i < sizeof(State.Satellites) / sizeof(SatelliteObject *); i++)
        {
            if((State.Satellites[i] == NULL) || (State.Satellites[i]->use == false))
                continue;
            else
            {
                if(State.Satellites[i]->cal = false)
                {
                    State.Satellites[State.loadindex]->Refresh(State.Satellites[i]->tle, Yonsei, false, true);
                    State.loadindex++;
                }
                try
                {
                    State.Satellites[i]->Update();
                }
                catch(const std::exception& e)
                {
                    console.AddLog("[ERROR]##Unable to Track Satellite : %s", State.Satellites[i]->Name());
                    console.AddLog(e.what());
                }
            }
        }
        usleep(10000);
    }
}

int ReadTLELines_Errorhandling(char * TLEFilename, char * trackingTLEname, bool fds, bool sgp4on)
{
    std::string TLEResultName;
    std::string TLEResultLine1;
    std::string TLEResultLine2;
    std::ifstream TLEDATA;
    TLEDATA.open(TLEFilename);
    State.loadindex = GetNowTracking();

    DateTime start_date = DateTime::Now(true);
    DateTime end_date(start_date.AddDays(7.0));

    while (getline(TLEDATA, TLEResultName) && getline(TLEDATA, TLEResultLine1) && getline(TLEDATA, TLEResultLine2))
    {
        
        TLEResultName.erase(std::find_if(TLEResultName.rbegin(), TLEResultName.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), TLEResultName.end());
        TLEResultLine1.erase(std::find_if(TLEResultLine1.rbegin(), TLEResultLine1.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), TLEResultLine1.end());
        TLEResultLine2.erase(std::find_if(TLEResultLine2.rbegin(), TLEResultLine2.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), TLEResultLine2.end());
        if(!strstr(TLEResultName.c_str(), trackingTLEname))
        {
            continue;
        }
        try
        {
            if(fds)
                FDSlistup = Tle(TLEResultName, TLEResultLine1, TLEResultLine2);
            else
            {
                TLElistup[State.loadindex] = Tle(TLEResultName, TLEResultLine1, TLEResultLine2);
                
            }
                
        }
        catch(const std::exception& e)
        {
            console.AddLog("[DEBUG]##TLE Load Error. Index : %d, Error : %s", State.loadindex, e.what());
        }
        try
        {
            if(fds)
            {
                if(strlen(State.Fatellites->Name()) != 0)
                    State.Fatellites->~SatelliteObject();
                State.Fatellites->Refresh(FDSlistup, Yonsei);
                break;
            }
            else
            {
                if(strlen(State.Satellites[State.loadindex]->Name()) != 0)
                    State.Satellites[State.loadindex]->~SatelliteObject();
                State.Satellites[State.loadindex]->Refresh(TLElistup[State.loadindex], Yonsei, false, sgp4on);
                State.loadindex++;
                // printf("Now : %d\n", State.loadindex);
            }
            
        }
        catch(const std::exception& e)
        {
            console.AddLog("[DEBUG]##SGP4 Error during importing. Skip this satellite and continue. Error : %s", e.what());
        }
        
    }
    TLEDATA.close();
    return State.loadindex;
}

int ReadTLELines(char *TLEFilename, bool fds, bool sgp4on)
{
    std::string TLEResultName;
    std::string TLEResultLine1;
    std::string TLEResultLine2;
    std::ifstream TLEDATA;
    TLEDATA.open(TLEFilename);
    State.loadindex = 0;

    DateTime start_date = DateTime::Now(true);
    DateTime end_date(start_date.AddDays(7.0));

    while (getline(TLEDATA, TLEResultName) && getline(TLEDATA, TLEResultLine1) && getline(TLEDATA, TLEResultLine2))
    {
        TLEResultName.erase(std::find_if(TLEResultName.rbegin(), TLEResultName.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), TLEResultName.end());
        TLEResultLine1.erase(std::find_if(TLEResultLine1.rbegin(), TLEResultLine1.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), TLEResultLine1.end());
        TLEResultLine2.erase(std::find_if(TLEResultLine2.rbegin(), TLEResultLine2.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), TLEResultLine2.end());
        try
        {
            if(fds)
                FDSlistup = Tle(TLEResultName, TLEResultLine1, TLEResultLine2);
            else
            {
                TLElistup[State.loadindex] = Tle(TLEResultName, TLEResultLine1, TLEResultLine2);
                
            }
                
        }
        catch(const std::exception& e)
        {
            console.AddLog("[DEBUG]##TLE Load Error. Index : %d, Error : %s", State.loadindex, e.what());
        }
        try
        {
            if(fds)
            {
                if(strlen(State.Fatellites->Name()) != 0)
                    State.Fatellites->~SatelliteObject();
                State.Fatellites->Refresh(FDSlistup, Yonsei);
                break;
            }
            else
            {
                if(strlen(State.Satellites[State.loadindex]->Name()) != 0)
                    State.Satellites[State.loadindex]->~SatelliteObject();
                State.Satellites[State.loadindex]->Refresh(TLElistup[State.loadindex], Yonsei, false, sgp4on);
                State.loadindex++;
                // printf("Now : %d\n", State.loadindex);
            }
            
        }
        catch(const std::exception& e)
        {
            console.AddLog("[DEBUG]##SGP4 Error during importing. Skip this satellite and continue. Error : %s", e.what());
        }
        
    }
    TLEDATA.close();
    return State.loadindex;
}


void * SatelliteInitialize(void * sgp4on)
{
    
    State.tleallindex = 32768;
    State.Display_TLE = false;
    State.Display_load = true;
    for(State.loadindex = 0; State.loadindex < sizeof(State.Satellites) / sizeof(SatelliteObject *); State.loadindex++)
    {
        
        if(State.Satellites[State.loadindex] != NULL)
        {
            if(strlen(State.Satellites[State.loadindex]->Name()) == 0)
                break;
            State.Satellites[State.loadindex]->~SatelliteObject();
        }
    }
    if((State.tleallindex = ReadTLELines(State.tleinfolistup[NowTLE]->local, false, *((bool *)sgp4on))) <= 0)
        console.AddLog("[ERROR]##Invalid type of TLE File. Please check again.");
    
    
    State.Display_load = false;
    State.Display_TLE = true;
    State.NowTracking = true;
    State.loadindex = 0;
}


void RotatorSetTo(int azimuth, int elevation)
{   
    State.RotatorReadReady = false;
    sprintf(ControlBuf, "W%03d %03d\r\n", azimuth, elevation);
    tcflush(rotator, TCIOFLUSH);
    int res = write(rotator, ControlBuf, 10);
    if(res > 0)
    {
        console.AddLog("Tracking : Az %d, El %d.", azimuth, elevation);
    }
    else
        console.AddLog("[ERROR]##Write to rotator failed.");
    
    sleep(1);
    State.RotatorReadReady = true;
}

void * RotatorEnter(void *)
{
    
    write(rotator, "\r\n", 2);
    int byte = 1;
    memset(ReadBuf, 0, strlen(ReadBuf));
    byte = read(rotator, ReadBuf, sizeof(ReadBuf));
    console.AddLog("[DEBUG]##Rotator : %s, Read Byte : %d", ReadBuf, byte);
    State.RotatorReading = true;
    tcflush(rotator, TCIOFLUSH);
}

void * RotatorReadInfo(void *)
{
    State.RotatorReading = true;
    int byte = 0;
    int read_count = 0;
    while(State.RotatorReading)
    {
        tcflush(rotator, TCIOFLUSH);
        memset(ReadBuf, 0, strlen(ReadBuf));

        if(write(rotator, "C2\r\n", 4) > 0)
        {
            State.RotatorReadReady = true;
        }
        else
        {
            console.AddLog("[ERROR]##Rotator Read Failed. Please Check Again.");
            State.RotatorReading = false;
            break;
        }

        State.RotatorReading = true;
        
        sleep(1);
        while(State.RotatorReading && State.RotatorReadReady)
        {
            byte = read(rotator, &ReadBuf[read_count], 1);
            if (byte < 1 && read_count > 3 && ReadBuf[read_count - 1] == '\n')
            {
                char azi[5] = {0}, ele[5] = {0};

                if (ReadBuf[3] == '-')	// AZ=-XXX
                {
                    // Sometime azi / ele can be negative value
                    // in that case, we need calibration

                    strncpy(azi, &ReadBuf[3], 4);
                    Now.az_now= atoi(azi);

                    fprintf(stderr, "Rotator azimuth calibration needed \n");

                    if (ReadBuf[12] == '-')		// AZ=-XXX  EL=-XXX
                    {
                        strncpy(ele, &ReadBuf[12], 4);
                        Now.el_now = atoi(ele);

                        fprintf(stderr, "Rotator elevation calibration needed \n");
                    }
                    else								// AZ=-XXX  EL=XXX
                    {
                        strncpy(ele, &ReadBuf[12], 3);
                        Now.el_now = atoi(ele);
                    }
                }
                else
                {
                    strncpy(azi, &ReadBuf[3], 3);
                    Now.az_now = atoi(azi);

                    if (ReadBuf[11] == '-')		// AZ=XXX  EL=-XXX
                    {
                        strncpy(ele, &ReadBuf[11], 4);
                        Now.el_now = atoi(ele);

                        fprintf(stderr, "Rotator elevation calibration needed \n");
                    }
                    else								// AZ=XXX  EL=XXX
                    {
                        strncpy(ele, &ReadBuf[11], 3);
                        Now.el_now = atoi(ele);
                    }
                }
                read_count = 0;
                sleep(1);
                State.RotatorReadReady = false;
            }
            else if (byte > 0)
            {
                read_count++;
            }
            else if (byte < 1)
            {
                console.AddLog("[ERROR]##Rotator Read Failed.");
                sleep(1);
                break;
            }
        }
    tcflush(rotator,TCIOFLUSH);
    }
    
    if(State.RotatorReading)
        State.RotatorReading = false;
    return NULL;
}

void * Doppler(void *) {
    int index = -2;
    uint32_t rxfreq, txfreq;
    double sol = 299792458;  // [m/s]
    double vel;
    double updated_freq;
    while(State.Doppler)
    {
        while(!State.RotatorReadReady)
            continue;
        index = GetNowTracking();
        // printf("%d\n", index);
        if (!State.Engage) {
            console.AddLog("[ERROR]##You tried doppler correction with no engaging");
            State.Doppler = false;
            break;
        }
        if(index < -1)
        {
            console.AddLog("[ERROR]##Please load TLE or FDS. Stop Doppler Correction.");
            State.Doppler = false;
            break;
        }
        else if (index == -1)
        {
            if(State.Fatellites == NULL)
            {
                State.Doppler = false;
                break;
            }
            vel= 1 - (State.Fatellites->topo.range_rate * 1000)/sol;
            updated_freq = vel * (setup->default_freq);
            
            rxfreq = uint32_t(static_cast<unsigned int>(updated_freq));
            txfreq = uint32_t(static_cast<unsigned int>((setup->default_freq) * 2 - updated_freq));
            console.AddLog("[DEBUG]##TX : %u, RX : %u", txfreq, rxfreq);

            //Set freq
            SetRxFreq(rxfreq);
            SetTxFreq(txfreq);
            int8_t rx_table_id = 1;
            int8_t tx_table_id = 5;
            // Save2FRAM((void *)&rx_table_id);
            // Save2FRAM((void *)&tx_table_id);
            usleep(DOPPLER_TIMESTEP);
            
            UpdateFreq();
        }
        else
        {
            if(State.Satellites[index] == NULL)
            {
                State.Doppler = false;
                break;
            }
            vel= 1 - (State.Satellites[index]->topo.range_rate * 1000)/sol;
            updated_freq = vel * (setup->default_freq);
            
            rxfreq = uint32_t(static_cast<unsigned int>(updated_freq));
            txfreq = uint32_t(static_cast<unsigned int>((setup->default_freq) * 2 - updated_freq));
            console.AddLog("[DEBUG]##TX : %u, RX : %u", txfreq, rxfreq);
            //Set freq
            SetRxFreq(rxfreq);
            SetTxFreq(txfreq);
            int8_t rx_table_id = 1;
            int8_t tx_table_id = 5;
            // Save2FRAM((void *)&rx_table_id);
            // Save2FRAM((void *)&tx_table_id);
            usleep(DOPPLER_TIMESTEP);
            UpdateFreq();
        }
        
    }
    
};

// void SetNowTracking(int index)
// {

//     if (index < -1 || index >= SAT_MAX_NUM || (index >= 0 && State.Satellites[index] == NULL)) {
//     console.AddLog("[ERROR]##[Track]SetNowTracking rejected. index=%d ptr=%p",
//                     index, (index >= 0 && index < SAT_MAX_NUM) ? State.Satellites[index] : NULL);
//     NowSat = -1;
//     return;
//     }
//     NowSat = index;
// }
void SetNowTracking_impl(int index, const char* file, int line)
{
    if (index < -1 || index >= SAT_MAX_NUM || (index >= 0 && State.Satellites[index] == NULL)) {
        console.AddLog("[ERROR]##[Track]SetNowTracking rejected. index=%d ptr=%p caller=%s:%d",
                       index, (index >= 0 && index < SAT_MAX_NUM) ? State.Satellites[index] : NULL,
                       file, line);
        NowSat = -1;
        return;
    }
    NowSat = index;
}

#define SetNowTracking(i) SetNowTracking_impl((i), __FILE__, __LINE__)

int GetNowTracking()
{
    return NowSat;
}

void * Sattracker(void *)
{
    State.Engage = true;

    while(State.Engage)
    {
        if(NowSat == -1)
        {
            if(State.Fatellites == NULL)
                continue;
            if(State.Fatellites->Elevation() < 0)
            {
                console.AddLog("[DEBUG]##Change to AOS.");
                Now.az_tag = State.Fatellites->AOS_Azimuth();
                Now.el_tag = State.Fatellites->AOS_Elevation();
            }
            else
            {
                Now.az_tag = State.Fatellites->Azimuth_1secfuture();
                Now.el_tag = State.Fatellites->Elevation_1secfuture();
            }
            if(Now.el_now > 80)
            {
                Now.el_tag = 80;
                State.Fatellites->futuretime = 6.0f;
            }
            else
                State.Fatellites->futuretime = 3.0f;
            
            if(State.Fatellites->CrossPath)
            {
                switch(State.Fatellites->CrossPath)
                {
                    case(1): {
                        
                        if(Now.az_tag < 90)
                            Now.az_tag += 360;
                        break;
                    }
                    case(-1): {
                        if(Now.az_tag < 90)
                            Now.az_tag += 360;
                        break;
                    }
                }
            }
            if ((abs(Now.az_tag - Now.az_now) >= AZI_TOLERANCE || abs(Now.el_tag - Now.el_now) >= ELE_TOLERANCE))
            {
                

                console.AddLog("[DEBUG]##Azimuth : %d, Elevation : %d\n", Now.az_tag, Now.el_tag);
                RotatorSetTo(Now.az_tag, Now.el_tag);
            }
            else
                console.AddLog("[DEBUG]##Tolerance Filtering...");
            sleep(1);
        }
        else
        {
            if(State.Satellites[NowSat] == NULL)
            {
                // console.AddLog("[ERROR]##Cannot Detect Tracking State.Satellites.");
                continue;
            }
        
            if(State.Satellites[NowSat]->Elevation() < 0)
            {
                console.AddLog("[DEBUG]##Change to AOS.");
                Now.az_tag = State.Satellites[NowSat]->AOS_Azimuth();
                Now.el_tag = State.Satellites[NowSat]->AOS_Elevation();
            }
            else
            {
                Now.az_tag = State.Satellites[NowSat]->Azimuth_1secfuture();
                Now.el_tag = State.Satellites[NowSat]->Elevation_1secfuture();
            }
            if(Now.el_now > 80)
            {
                Now.el_tag = 80;
                State.Satellites[NowSat]->futuretime = 6.0f;
            }
            else
                State.Satellites[NowSat]->futuretime = 3.0f;
            
            if(State.Satellites[NowSat]->CrossPath)
            {
                switch(State.Satellites[NowSat]->CrossPath)
                {
                    case(1): {
                        if(Now.az_tag < 90)
                            Now.az_tag += 360;
                        break;
                    }
                    case(-1): {
                        if(Now.az_tag < 90)
                            Now.az_tag += 360;
                        break;
                    }
                }
            }
            if ((abs(Now.az_tag - Now.az_now) >= AZI_TOLERANCE || abs(Now.el_tag - Now.el_now) >= ELE_TOLERANCE))
            {
                

                console.AddLog("[DEBUG]##Azimuth : %d, Elevation : %d\n", Now.az_tag, Now.el_tag);
                RotatorSetTo(Now.az_tag, Now.el_tag);
            }
            else
                console.AddLog("[DEBUG]##Tolerance Filtering...");
            sleep(1);
        }
        
    }
}

void FSWTleConverter(Tle& TleInfo, FSWTle * fswtle)
{

    bool in_degree = true;

    DateTime UTCtime = DateTime::Now(true);
    DateTime EpochInfo = TleInfo.Epoch();

    fswtle->fingerprint[0] = 1;
    fswtle->fingerprint[1] = 2;
    fswtle->fingerprint[2] = 0;
    fswtle->fingerprint[3] = 8;

    time_t now, standard;
    struct tm epochtime;
    epochtime.tm_year = 1980 - 1990;
    epochtime.tm_mon = 0;
    epochtime.tm_mday = 1;
    epochtime.tm_hour = 0;
    epochtime.tm_min = 0;
    epochtime.tm_sec = 0;
    epochtime.tm_isdst = 0;
    now = time(NULL);
    standard = mktime(&epochtime);
    fswtle->sec = static_cast<unsigned int>(difftime(standard, now));


    // fswtle->sec =
    //  (uint32_t)(UTCtime.Second() + 60 * UTCtime.Minute() + 60 * 60 * UTCtime.Hour() + 60 * 60 * 24 * UTCtime.DayOfYear(UTCtime.Year(), UTCtime.Month(), UTCtime.Day()));
    //fswtle->subsec = (uint32_t)(UTCtime.Microsecond()); //to b
    fswtle->micsec = (uint32_t)(UTCtime.Microsecond());
    //***************************TBD

    fswtle->epochYear = (uint32_t)(EpochInfo.Year());
    fswtle->epochDay = (uint32_t)(EpochInfo.DayOfYear(EpochInfo.Year(), EpochInfo.Month(), EpochInfo.Day()))-1;

    fswtle->INC = (double)(TleInfo.Inclination(in_degree));
    fswtle->RAAN = (double)(TleInfo.RightAscendingNode(in_degree));
    fswtle->ECC = (double)(TleInfo.Eccentricity());
    fswtle->AOP = (double)(TleInfo.ArgumentPerigee(in_degree));
    fswtle->MA = (double)(TleInfo.MeanAnomaly(in_degree));
    fswtle->MM = (double)(TleInfo.MeanMotion());
    fswtle->padding[0] = 0;
    fswtle->padding[1] = 0;
    fswtle->padding[2] = 0;
    fswtle->padding[3] = 0;
    fswtle->Checksum = gs_crc32(((uint8_t *)(&fswtle->sec)),72);

    console.AddLog("[DEBUG]##Ping Format - Time\nFingerprint : %u%u%u%u\tSecond : %u\tMicroSecond : %u\t EpochYear : %u\t, EpochDay : %u\n",
                    fswtle->fingerprint[0], fswtle->fingerprint[1], fswtle->fingerprint[2], fswtle->fingerprint[3],
                    fswtle->sec, fswtle->micsec, fswtle->epochYear, fswtle->epochDay);
    console.AddLog("[DEBUG]##Ping Format - TLE\nInclination : %f\t RAAN : %f\t Eccentricity :%f\t AOP : %f\t MA : %f\t MM : %f\t Checksum : %u\n",
                    fswtle->INC, fswtle->RAAN, fswtle->ECC, fswtle->AOP, fswtle->MA, fswtle->MM, fswtle->Checksum);
}