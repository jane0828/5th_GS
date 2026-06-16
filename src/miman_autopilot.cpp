#include "miman_config.h"
#include "miman_coms.h"
#include "miman_csp.h"
#include "miman_imgui.h"
#include "miman_orbital.h"
#include "miman_radial.h"
#include "miman_model.h"
#include "miman_ftp.h"
#include "miman_ftprdp_integration.h"
#include "miman_autopilot.h"


extern StateCheckUnit State;
extern Setup * setup;
extern Console console;
extern pthread_t p_thread[16];
extern int NowTLE;
extern int NowFTP;
extern bool sgp4check;
extern int PingCounter;
extern int BeaconCounter;
extern CmdGenerator_GS * SatCMD[256];
static bool wdt_notsent;

static int start_ryu_ftp_thread(void *(*worker)(void *))
{
    miman_ftp_worker_arg_t * arg =
        miman_ftp_create_worker_arg(State.ftplistup[NowFTP].local_path,
                                    State.ftplistup[NowFTP].remote_path);
    if (arg == NULL) {
        console.AddLog("[ERROR]##[Auto]Failed to allocate Ryu FTP worker argument.");
        return -1;
    }

    int ret = pthread_create(&p_thread[8], NULL, worker, arg);
    if (ret != 0) {
        miman_ftp_destroy_worker_arg(arg);
        console.AddLog("[ERROR]##[Auto]Failed to start Ryu FTP thread. Retcode : %d", ret);
    }

    return ret;
}

// int FindValuablePath()
// {
//     int Valuable = 0;
//     for(int i = 0 ; i < 64; i++)
//     {
//         if(State.Satellites[GetNowTracking()]->MaxElevation(i) > 10.0f)
//         {
//             Valuable = i;
//             break;
//         }
//     }
//     return Valuable;
// }

int FindValuablePath(int satIndex)
{
    if (satIndex < 0 || satIndex >= SAT_MAX_NUM || !State.Satellites[satIndex]) return 0;
    int Valuable = 0;
    for (int i = 0; i < 64; i++) {
        if (State.Satellites[satIndex]->MaxElevation(i) > 10.0f) { Valuable = i; break; }
    }
    return Valuable;
}


void * AutoPilot(void *)
{
    console.AddLog("[Auto]Start AutoPilot.");
    int AutoRes = AutoPilot_task();
    if(AutoRes)
        console.AddLog("[ERROR]##[Auto]AutoPilot Stopped with Error Code : %d", AutoRes);
    if(AutoRes != -2)
    {
        State.Doppler = false;
        pthread_join(p_thread[7], NULL);
        console.AddLog("[OK]##[Auto]Stop Doppler Correction.");

        State.Engage = false;
        State.Doppler = false;
        pthread_join(p_thread[1], NULL);
        console.AddLog("[OK]##[Auto]Finish Engaging.");
        
        RotatorSetTo(0, 0);
        console.AddLog("[Auto]Beacon Success : %d, Ping Success : %d CMD Success : %d", BeaconCounter, PingCounter, State.CMDCount);
        BeaconCounter = 0;
        PingCounter = 0;
    }
    State.Successed = 0;
    return NULL;

}

static bool NowOn(int SatIndex, int NextValuablePath)
{
    if (SatIndex < 0 || SatIndex >= SAT_MAX_NUM) return false;
    if (NextValuablePath < 0 || NextValuablePath >= 64) return false;
    if (!State.Satellites[SatIndex]) return false;
    return (DateTime::Now(false) - State.Satellites[SatIndex]->_nextlos[NextValuablePath]).TotalSeconds() < 0;
}

int AutoPilot_task()
{
    bool BeaconAvailable = false;
    bool PingAvailable = false;
    char NowTrackingNameBuffer[32];
    int NowTracking = GetNowTracking();
    if (NowTracking < 0 || NowTracking >= SAT_MAX_NUM || !State.Satellites[NowTracking]) {
        console.AddLog("[ERROR]##[Auto]Invalid tracking at start. trk=%d", NowTracking);
        State.Autopilot = false;
        return -2;
    }
    snprintf(NowTrackingNameBuffer, sizeof(NowTrackingNameBuffer), "%s",
            State.Satellites[NowTracking]->Name());
    int SatIndex = GetNowTracking();
    if (SatIndex < 0 || SatIndex >= SAT_MAX_NUM || !State.Satellites[SatIndex]) return -2;

    while(State.Autopilot)
    {

        bool BeaconAvailable = false;
        bool PingAvailable = false;
        bool CalibrationSuccess = false; // Add
        printf("Start AutoPilot.\n");
        int NextValuablePath = FindValuablePath(SatIndex);
        console.AddLog("[Auto]Start Autopilot. Next Valuable AOS : %d/%d %d:%d:%d",
                                                        State.Satellites[SatIndex]->_nextaos[NextValuablePath].AddHours(9).Month(), 
                                                        State.Satellites[SatIndex]->_nextaos[NextValuablePath].AddHours(9).Day(),
                                                        State.Satellites[SatIndex]->_nextaos[NextValuablePath].AddHours(9).Hour(),
                                                        State.Satellites[SatIndex]->_nextaos[NextValuablePath].AddHours(9).Minute(),
                                                        State.Satellites[SatIndex]->_nextaos[NextValuablePath].AddHours(9).Second());
        console.AddLog("[Auto]Max Elevation : %.2lf° Please Keep Devices On.", State.Satellites[SatIndex]->_max_elevation[NextValuablePath] * RAD_TO_DEG);
        // printf("%f\n", (State.Satellites[SatIndex]->_nextaos[NextValuablePath] -  DateTime::Now(true)).TotalMinutes());
        while(State.Autopilot)
        {
            if((State.Satellites[SatIndex]->_nextaos[NextValuablePath] -  DateTime::Now(true)).TotalMinutes() < 5.0f)
                break;
            usleep(20000); // 20 ms
        }
        PathGenerator(SatIndex);

        if(!State.Autopilot)
        {
            console.AddLog("[OK]##[Auto]Autopilot Stopped by User.");
            return 0;
        }

        State.Engage = true;
        pthread_create(&p_thread[1], NULL, Sattracker, NULL);
        console.AddLog("[OK]##[Auto]Start Engaging.");

        State.Doppler = true;
        pthread_create(&p_thread[7], NULL, Doppler, NULL);
        console.AddLog("[OK]##[Auto]Start Doppler Correction.");

        //Task
        //Check Beacon Received
        while(State.Autopilot && State.DoBeaconCheck)
        {
            if(BeaconCounter >= State.iteration)
            {
                BeaconAvailable = true;
                break;
            }
                
            if(!NowOn(SatIndex, NextValuablePath))
            {
                console.AddLog("[ERROR]##[Auto]Beacon Success is too hard. Beacon Success : %d", BeaconCounter);
                break;
            }
        }

        if(!State.Autopilot)
        {
            console.AddLog("[OK]##[Auto]Autopilot Stopped by User.");
            return 0;
        }


        if(State.AMPmode)
        {
    // --- DEBUG: show computed minutes-to-LOS ---
    double min_to_los =
        (State.Satellites[SatIndex]->_nextlos[NextValuablePath] - DateTime::Now(true)).TotalMinutes();

    // console.AddLog("[DBG][AMP] enter: SatIndex=%d Path=%d min_to_los=%.3f Autopilot=%d",
    //                SatIndex, NextValuablePath, min_to_los, (int)State.Autopilot);

    // throttle log inside loop (e.g., once per 1 second)
    DateTime last_log = DateTime::Now(false);

    while (State.Autopilot)
    {
        min_to_los =
            (State.Satellites[SatIndex]->_nextlos[NextValuablePath] - DateTime::Now(false)).TotalMinutes();

        if (min_to_los <= 15.0)
            break;

        // print at most once per 1 sec
        if ((DateTime::Now(false) - last_log).TotalSeconds() >= 1.0)
        {
            // console.AddLog("[DBG][AMP] waiting: min_to_los=%.3f (cond: %.3f > 15) now=%d/%d %02d:%02d:%02d",
            //                min_to_los, min_to_los,
            //                DateTime::Now(false).Month(), DateTime::Now(false).Day(),
            //                DateTime::Now(false).Hour(), DateTime::Now(false).Minute(), DateTime::Now(false).Second());
            last_log = DateTime::Now(false);
        }

        usleep(50000); // 50 ms
    }

    // console.AddLog("[DBG][AMP] exit-wait: min_to_los=%.3f Autopilot=%d",
                //    min_to_los, (int)State.Autopilot);

    if (!State.Autopilot)
        return 0;



            // while((State.Satellites[SatIndex]->_nextlos[NextValuablePath] - DateTime::Now(false)).TotalMinutes() > 15.0f)
            //     {
            //                     console.AddLog("[AUTO]##AMP Process running");
            //     if (!State.Autopilot) break;
            //     usleep(50000); // 50 ms
            //     }
            if(State.AmpTime > 0)
            {
                console.AddLog("[ERRPR]##Cannot Turn on AMP Now. Wati until the timer goes on.");
                console.AddLog("[ERROR]##If you don't want it, restart the software.");
                console.AddLog("[ERROR]##Remind the AMP would be turned off after the timer goes on");
                console.AddLog("[ERROR]##even if you restart the software.");
            while (State.AmpTime > 0 && State.Autopilot)
            {
                usleep(20000); // 20 ms
            }
            }
                
            pid_t pid = fork();
            if (pid == -1) {
                console.AddLog("[ERROR]##Failed to fork process");
            }
            else if (pid == 0) {
                // Child process
                execl("../amp/ampcontrol", "ampcontrol", NULL);
                console.AddLog("[OK]##Forked Safe Amplifier Process.");
            }
            else
            {
                console.AddLog("[OK]##Forked Safe Amplifier Process.(ON, Timer)");
                pthread_create(&p_thread[6], NULL, AmpTimer, NULL);
            }
        }
        
        /*-----------------Calibration Code-------------------*/
        if (State.DoBaudCalibration) {
            pthread_create(&p_thread[5],NULL,csp_baud_calibration,NULL);
        }
        if (State.DoFreqCalibration) {
            pthread_create(&p_thread[5],NULL,csp_freq_calibration,NULL);
        }
        /*-----------------------------------------------------*/

        //Ping Try
        while(State.Autopilot && State.DoPing && NowOn(SatIndex, NextValuablePath))
        {
            sleep(State.SpanTime);
            
            pthread_create(&p_thread[4], NULL, csp_ping_console, NULL);
            if(PingCounter >= 5)
            {
                PingAvailable = true;
                break;
            }
                
            if(!NowOn(SatIndex, NextValuablePath))
            {
                console.AddLog("[ERROR]##[Auto]Ping Success is too hard. Ping Success : %d", PingCounter);
                break;
            }
            pthread_join(p_thread[4], NULL);
        }


        if(!State.Autopilot)
        {
            console.AddLog("[OK]##[Auto]Autopilot Stopped by User.");
            return 0;
        }

        uint32_t now_total_rx_bytes = get_rx_bytes();
        wdt_notsent = true;   
        //Send Watchdog

if (wdt_notsent &&
    State.Autopilot &&
    State.DoCMD &&
    NowOn(SatIndex, NextValuablePath))
{
    /* ================= Get EPS HK 3형제 ==================-*/

for (int i = 0; i < 2 && State.Autopilot; i++)
{
    uint8_t cmd4[8] = {0x18,0x75,0xC0,0x00,0x00,0x01,0x14,0x47};

    packetsign* pkt =
        (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd4));

    pkt->Identifier = HVD_TEST;
    pkt->PacketType = MIM_PT_TMTC_TEST;
    pkt->Length     = sizeof(cmd4);
    memcpy(pkt->Data, cmd4, sizeof(cmd4));

    pthread_create(&p_thread[4], NULL,
                   task_uplink_onorbit, (void*)pkt);
    pthread_join(p_thread[4], NULL);
    free(pkt);

    console.AddLog("[Auto]EPS P60 DOCK GET TABLE HK Command Sent (%d/2)", i+1);
    sleep(10);
}

for (int i = 0; i < 2 && State.Autopilot; i++)
{
    uint8_t cmd5[8] = {0x18,0x75,0xC0,0x00,0x00,0x01,0x17,0x44};

    packetsign* pkt1 =
        (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd5));
    pkt1->Identifier = HVD_TEST;
    pkt1->PacketType = MIM_PT_TMTC_TEST;
    pkt1->Length     = sizeof(cmd5);
    memcpy(pkt1->Data, cmd5, sizeof(cmd5));

    pthread_create(&p_thread[4], NULL,
                   task_uplink_onorbit, (void*)pkt1);
    pthread_join(p_thread[4], NULL);
    free(pkt1);

    console.AddLog("[Auto]EPS P60 PDU GET TABLE HK Command Sent (%d/2)", i+1);
    sleep(10);
}

for (int i = 0; i < 2 && State.Autopilot; i++)
{
    uint8_t cmd6[8] = {0x18,0x75,0xC0,0x00,0x00,0x01,0x1a,0x00};

    packetsign* pkt2 =
        (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd6));
    pkt2->Identifier = HVD_TEST;
    pkt2->PacketType = MIM_PT_TMTC_TEST;
    pkt2->Length     = sizeof(cmd6);
    memcpy(pkt2->Data, cmd6, sizeof(cmd6));

    pthread_create(&p_thread[4], NULL,
                   task_uplink_onorbit, (void*)pkt2);
    pthread_join(p_thread[4], NULL);
    free(pkt2);

    console.AddLog("[Auto]EPS P60 ACU GET TABLE HK Command Sent (%d/2)", i+1);
    sleep(10);
}

    console.AddLog("[Auto]Start GND WDT reset sequence");

    /* ================= AX100 ================= */
    uint8_t cmd1[8] = {0x18,0x50,0xC0,0x00,0x00,0x01,0x06,0x70};

    for (int i = 0; i < 2 && State.Autopilot; i++)
    {
        packetsign* pkt =
            (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd1));

        pkt->Identifier = HVD_TEST;
        pkt->PacketType = MIM_PT_TMTC_TEST;
        pkt->Length     = sizeof(cmd1);
        memcpy(pkt->Data, cmd1, sizeof(cmd1));

        pthread_create(&p_thread[4], NULL,
                       task_uplink_onorbit, (void*)pkt);
        pthread_join(p_thread[4], NULL);
        free(pkt);

        console.AddLog("[Auto]AX100 GND WDT CMD sent (%d/2)", i+1);
        sleep(10);
    }

    if (!State.Autopilot) return 0;

    /* ================= EPS P60 DOCK ================= */
    uint8_t cmd2[12] =
        {0x18,0x75,0xC0,0x00,0x00,0x05,0x1e,0xa2,0xe8,0x03,0x00,0x00};

    for (int i = 0; i < 2 && State.Autopilot; i++)
    {
        packetsign* pkt =
            (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd2));

        pkt->Identifier = HVD_TEST;
        pkt->PacketType = MIM_PT_TMTC_TEST;
        pkt->Length     = sizeof(cmd2);
        memcpy(pkt->Data, cmd2, sizeof(cmd2));

        pthread_create(&p_thread[4], NULL,
                       task_uplink_onorbit, (void*)pkt);
        pthread_join(p_thread[4], NULL);
        free(pkt);

        console.AddLog("[Auto]EPS P60 DOCK GND WDT CMD sent (%d/2)", i+1);
        sleep(10);
    }

    console.AddLog("[Auto]GND WDT sequence completed");



    /* ================= Get EPS HK 3형제 ==================-*/
for (int i = 0; i < 2 && State.Autopilot; i++)
{
    uint8_t cmd7[8] = {0x18,0x75,0xC0,0x00,0x00,0x01,0x14,0x47};

    packetsign* pkt3 =
        (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd7));
    pkt3->Identifier = HVD_TEST;
    pkt3->PacketType = MIM_PT_TMTC_TEST;
    pkt3->Length     = sizeof(cmd7);
    memcpy(pkt3->Data, cmd7, sizeof(cmd7));

    pthread_create(&p_thread[4], NULL,
                   task_uplink_onorbit, (void*)pkt3);
    pthread_join(p_thread[4], NULL);
    free(pkt3);

    console.AddLog("[Auto]EPS P60 DOCK GET TABLE HK Command Sent (%d/2)", i+1);
    sleep(10);
}
for (int i = 0; i < 2 && State.Autopilot; i++)
{
    uint8_t cmd8[8] = {0x18,0x75,0xC0,0x00,0x00,0x01,0x17,0x44};

    packetsign* pkt4 =
        (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd8));
    pkt4->Identifier = HVD_TEST;
    pkt4->PacketType = MIM_PT_TMTC_TEST;
    pkt4->Length     = sizeof(cmd8);
    memcpy(pkt4->Data, cmd8, sizeof(cmd8));

    pthread_create(&p_thread[4], NULL,
                   task_uplink_onorbit, (void*)pkt4);
    pthread_join(p_thread[4], NULL);
    free(pkt4);

    console.AddLog("[Auto]EPS P60 PDU GET TABLE HK Command Sent (%d/2)", i+1);
    sleep(10);
}
for (int i = 0; i < 2 && State.Autopilot; i++)
{
    uint8_t cmd9[8] = {0x18,0x75,0xC0,0x00,0x00,0x01,0x1a,0x00};

    packetsign* pkt5 =
        (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd9));
    pkt5->Identifier = HVD_TEST;
    pkt5->PacketType = MIM_PT_TMTC_TEST;
    pkt5->Length     = sizeof(cmd9);
    memcpy(pkt5->Data, cmd9, sizeof(cmd9));

    pthread_create(&p_thread[4], NULL,
                   task_uplink_onorbit, (void*)pkt5);
    pthread_join(p_thread[4], NULL);
    free(pkt5);

    console.AddLog("[Auto]EPS P60 ACU GET TABLE HK Command Sent (%d/2)", i+1);
    sleep(10);
}
// for (int i = 0; i < 1 && State.Autopilot; i++)
// {
//     uint8_t cmd10[11] = {0x18, 0x65, 0xc0, 0x0, 0x0, 0x4, 0x67, 0x20, 0x1, 0x0, 0x0};

//     packetsign* pkt6 =
//         (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd10));
//     pkt6->Identifier = HVD_TEST;
//     pkt6->PacketType = MIM_PT_TMTC_TEST;
//     pkt6->Length     = sizeof(cmd10);
//     memcpy(pkt6->Data, cmd10, sizeof(cmd10));

//     pthread_create(&p_thread[4], NULL,
//                    task_uplink_onorbit, (void*)pkt6);
//     pthread_join(p_thread[4], NULL);
//     free(pkt6);

//     console.AddLog("[Auto]ADCS Commissioning Command Sent");
//     sleep(10);
// }

    console.AddLog("[Auto]AutoPilot sequence Ended");
    wdt_notsent = false;
}

        if(!State.Autopilot)
        {
            console.AddLog("[OK]##[Auto]Autopilot Stopped by User.");
            return 0;
        }

        bool ulsigdone = false;
        //UL SIGNAL
        if((State.DoFTPDL || State.DoFTPUL) && NowOn(SatIndex, NextValuablePath))
        {
            int NowBecCount = BeaconCounter;
        while (NowBecCount == BeaconCounter &&
            State.Autopilot &&
            NowOn(SatIndex, NextValuablePath))
        {
            usleep(5000); 
        }

            uint32_t initial = get_rx_bytes();
            DateTime inittime = DateTime::Now(false);
            
            while(State.Autopilot && !ulsigdone)
            {
                if((inittime - DateTime::Now()).TotalSeconds() > 30)
                    console.AddLog("[Auto]No Beacon for 30 seconds. Try Next Task.");
                console.AddLog("[Auto]Try to Send UL Signal");
                
                DateTime BeaconStandard = DateTime::Now(false);
                for(int i = 0; i < 10; i++)
                {
                    sleep(State.SpanTime);
                    if((DateTime::Now(false) - State.Satellites[SatIndex]->_nextlos[NextValuablePath]).TotalSeconds() > 0)
                    {
                        // console.AddLog("[ERROR]##[Auto]Failed to Send UL Signal.");
                        break;
                    }
                    
                    // MIM_TLEinfo = FSWTleConverter(TrackingTLE[NowSat]);
                    packetsign* TestPacket = (packetsign *)malloc(9);
                    TestPacket->Identifier = MIM_ID;
                    TestPacket->PacketType = MIM_PT_SIGNAL;
                    TestPacket->Length = 1;
                    memset(TestPacket->Data, 0, TestPacket->Length);
                    pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)TestPacket);
                    pthread_join(p_thread[4], NULL);
                }

                console.AddLog("Try to config total rx bytes.");
                int trying = 0;
                while(State.Autopilot && NowOn(SatIndex, NextValuablePath))
                {
                    if(gs_rparam_get_uint32(setup->ax100_node, 4, 0x003C, 0xB00B, setup->default_timeout, get_rx_bytes_address()) == GS_OK)
                    {
                        if((get_rx_bytes() - initial) % 16 != 0)
                        {
                            console.AddLog("[OK]##[Auto]UL Signal Send Done. Init : %u, Now : %u", initial, get_rx_bytes());
                            ulsigdone = true;
                            break;
                        }
                    }
                    else if (trying == 5)
                        break;
                    else
                    {
                        trying ++;
                        continue;
                    }
                        

                }
                
                if(!NowOn(SatIndex, NextValuablePath))
                {
                    console.AddLog("[ERROR]##[Auto]Failed to Send UL Signal.");
                    break;
                }
            }
        }

        

        if(!State.Autopilot)
        {
            console.AddLog("[OK]##[Auto]Autopilot Stopped by User.");
            return 0;
        }

        // //CMD
        // if(State.Autopilot && State.DoCMD && NowOn(SatIndex, NextValuablePath))
        // {
        //     for(int i = 0; i < 256; i++)
        //     {
        //         if(SatCMD[i] == NULL)
        //             continue;
        //         if(State.Successed > State.CMDCount)
        //         {
        //             State.CMDCount++;
        //             continue;
        //         }
        //         while(State.Autopilot)
        //         {
        //             sleep(State.SpanTime);
                    
        //             if(SatCMD[i]->Checksum)
        //                 SatCMD[i]->GenerateChecksum();
        //             packetsign * SendingCMD = SatCMD[i]->GenerateCMDPacket();
        //             pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)SendingCMD);
        //             pthread_join(p_thread[4], NULL);
        //             if(State.Successed < State.CMDCount)
        //             {
        //                 State.Successed = State.CMDCount;
        //                 break;
        //             }
        //             if(!NowOn(SatIndex, NextValuablePath))
        //             {
        //                 break;
        //             }
        //         }
        //         if(!NowOn(SatIndex, NextValuablePath))
        //         {
        //             break;
        //         }

        //     }
        //     if(!NowOn(SatIndex, NextValuablePath))
        //     {
        //         console.AddLog("[ERROR]##[Auto]Stop CMD after LOS. Success : %d", State.CMDCount);
        //         State.CMDCount = 0;
        //         break;
        //     }
        // }

        if(!State.Autopilot)
        {
            console.AddLog("[OK]##[Auto]Autopilot Stopped by User.");
            return 0;
        }

        //FTP DL
        //Temporary condition : max_el > 30.0deg
        while(State.Autopilot && State.DoFTPDL && NowOn(SatIndex, NextValuablePath))
        {
            if(State.ftp_mode)
                {usleep(10000); 
                    continue;
                }
            sleep(State.SpanTime);
            pthread_join(p_thread[8], NULL);
            if(!NowOn(SatIndex, NextValuablePath))
            {
                console.AddLog("[Auto]Stop FTP Download after LOS.");
                break;
            }
            if(State.ftp_version == 1)
                pthread_create(&p_thread[8], NULL, ftp_downlink_force, &State.ftplistup[NowFTP]);
            else if(State.ftp_version == 3)
                start_ryu_ftp_thread(ftp_ryu_downlink_onorbit);
            else
                pthread_create(&p_thread[8], NULL, ftp_downlink_onorbit, &State.ftplistup[NowFTP]);
            
        }

        if(!State.Autopilot)
        {
            console.AddLog("[OK]##[Auto]Autopilot Stopped by User.");
            return 0;
        }

        //FTP UL
        //Temporary condition : max_el > 30.0deg
        while(State.Autopilot && State.DoFTPUL && NowOn(SatIndex, NextValuablePath))
        {
            if(State.ftp_mode)
                continue;
            sleep(State.SpanTime);
            pthread_join(p_thread[8], NULL);
            if(!NowOn(SatIndex, NextValuablePath))
            {
                console.AddLog("[Auto]Stop FTP Upload after LOS.");
                break;
            }
            if(State.ftp_version == 1)
                pthread_create(&p_thread[8], NULL, ftp_uplink_force, &State.ftplistup[NowFTP]);
            else if(State.ftp_version == 3)
                start_ryu_ftp_thread(ftp_ryu_uplink_onorbit);
            else
                pthread_create(&p_thread[8], NULL, ftp_uplink_onorbit, &State.ftplistup[NowFTP]);
            
            
        }
        

        if(!State.Autopilot)
        {
            console.AddLog("[OK]##[Auto]Autopilot Stopped by User.");
            return 0;
        }

        //Wait Until LOS and return multhreads.
        pthread_join(p_thread[6], NULL);
        pthread_join(p_thread[8], NULL);
        pthread_join(p_thread[4], NULL);

        while(State.Autopilot && NowOn(SatIndex, NextValuablePath))
            usleep(20000);
        State.Doppler = false;
        pthread_join(p_thread[7], NULL);
        console.AddLog("[OK]##[Auto]Stop Doppler Correction.");

        State.Engage = false;
        State.Doppler = false;
        pthread_join(p_thread[1], NULL);
        console.AddLog("[OK]##[Auto]Finish Engaging.");
        
        RotatorSetTo(0, 0);
        console.AddLog("[Auto]Beacon Success : %d, Ping Success : %d CMD Success : %d", BeaconCounter, PingCounter, State.CMDCount);
        BeaconCounter = 0;
        PingCounter = 0;


        //TLE update시 주석 해제 필수
        console.AddLog("[Auto]Update TLE...");
        for(int i = 0; i < SAT_MAX_NUM; i++)
        {
            if (State.Satellites[i]) State.Satellites[i]->use = false;
        }
        if(DownloadTLE(State.tleinfolistup[NowTLE]->remote, State.tleinfolistup[NowTLE]->local))
        {
            State.NowTracking = false;
            
            console.AddLog("[OK]##[Auto]TLE \"%s\" Download Completed.", State.tleinfolistup[NowTLE]->label);

            // pthread_create(&p_thread[15], NULL, &SatelliteInitialize,  &sgp4check);
            // SatelliteInitialize(NULL);
            
        }
        else
        {
            console.AddLog("[ERROR]##[Auto]TLE Download failed. Please check again.");
            return -1;
        }
        // pthread_join(p_thread[15], NULL);
        printf("Updated TLE.\n");
        sleep(3);

        //AMP OFF
        pid_t pid_off = fork();

        if (pid_off == -1) {
            console.AddLog("[ERROR]##Failed to fork process");
        }
        else if (pid_off == 0) {
            // Child process
            execl("../amp/ampoff", "ampoff", NULL);
        }
        else
        {
            console.AddLog("[OK]##Forked Safe Amplifier Process. (OFF)");
        }
        State.AMPON = false;



        // TLE  업데이트시 주석 해ㅈ[ ㅍ;ㄹ수]
        csp_transaction(CSP_PRIO_HIGH, setup->gs100_node, AX100_PORT_GNDWDT_RESET, 1000, NULL, 0, NULL, 0) > 0;
        console.AddLog("Reset GS100 watchdog.");
        printf("Reset GS100 Watchdog.\n");
        sleep(3);
        int tlepopupindex = 0;
        
        printf("Search Samename Satellite.\n");

        State.tleallindex = SAT_MAX_NUM;
        if((State.tleallindex = ReadTLELines_Errorhandling(State.tleinfolistup[NowTLE]->local, NowTrackingNameBuffer,false, false)) <= 0)
            console.AddLog("[ERROR]##Invalid type of TLE File. Please check again.");
        State.Display_TLE = true;
       
        sleep(3);
        if (NowTracking < 0 || NowTracking >= SAT_MAX_NUM || !State.Satellites[NowTracking]) {
            console.AddLog("[ERROR]##[Auto]NowTracking invalid after TLE reload. idx=%d", NowTracking);
            return -2;
        }



        State.Satellites[NowTracking]->use = true;
        if(State.Satellites[NowTracking]->cal == false)
            State.Satellites[NowTracking]->Refresh(State.Satellites[NowTracking]->tle, State.Satellites[NowTracking]->obs, true, true);
        printf("Updated Propagator.\n");
        sleep(3);
        State.NowTracking = true;
        State.ModelRefreshRequired = true;
        printf("Model Refresh.\n");
        sleep(3);

        SetNowTracking(NowTracking);
        SatIndex = GetNowTracking();
        if (SatIndex < 0 || SatIndex >= SAT_MAX_NUM || !State.Satellites[SatIndex]) return -2;
        // BEFORE calling PathGenerator()
        if (!State.Autopilot) {
            console.AddLog("[OK]##[Auto]Autopilot Stopped by User (before PathGenerator).");
            return 0;
        }

        int trk = GetNowTracking();
        if (trk < 0 || trk >= SAT_MAX_NUM || State.Satellites[trk] == NULL) {
            console.AddLog("[ERROR]##[Auto]Invalid tracking index before PathGenerator. trk=%d ptr=%p",
                        trk, (trk >= 0 && trk < SAT_MAX_NUM) ? State.Satellites[trk] : NULL);
            return -2;
        }

        PathGenerator(trk);




        // PathGenerator(GetNowTracking());
        State.Display_load = false;
        State.Display_TLE = false;
        printf("Finish Autopilot.\n");
    }
    return 0;
}

int TLE_Autoupdate_Test()
{
    int NowTracking = GetNowTracking();
    char NowTrackingNameBuffer[32];
    printf("[Auto]Update TLE...\n");
    sprintf(NowTrackingNameBuffer, State.Satellites[NowTracking]->Name());
    for(int i = 0; i < SAT_MAX_NUM; i++)
    {
        if (State.Satellites[i]) State.Satellites[i]->use = false;
    }
    if(DownloadTLE(State.tleinfolistup[NowTLE]->remote, State.tleinfolistup[NowTLE]->local))
    {
        State.NowTracking = false;
        
        printf("[OK]##[Auto]TLE \"%s\" Download Completed.\n", State.tleinfolistup[NowTLE]->label);

        // pthread_create(&p_thread[15], NULL, &SatelliteInitialize,  &sgp4check);
        // SatelliteInitialize(NULL);
        
    }
    else
    {
        printf("[ERROR]##[Auto]TLE Download failed. Please check again.\n");
        return -1;
    }
    // pthread_join(p_thread[15], NULL);
    printf("Updated TLE.\n");
    sleep(3);

    printf("Skip GS100 Watchdog");
    // csp_transaction(CSP_PRIO_HIGH, setup->gs100_node, AX100_PORT_GNDWDT_RESET, 1000, NULL, 0, NULL, 0) > 0;
    // console.AddLog("Reset GS100 watchdog.");
    // printf("Reset GS100 Watchdog.\n");
    // sleep(3);
    int tlepopupindex = 0;
    
    printf("Search Samename Satellite.\n");

    State.tleallindex = 32768;
    if((State.tleallindex = ReadTLELines_Errorhandling(State.tleinfolistup[NowTLE]->local, NowTrackingNameBuffer,false, false)) <= 0)
        printf("[ERROR]##Invalid type of TLE File. Please check again.\n");
    State.Display_TLE = true;
    
    sleep(3);

    if (NowTracking < 0 || NowTracking >= SAT_MAX_NUM || !State.Satellites[NowTracking]) {
    console.AddLog("[ERROR]##[Auto]NowTracking invalid after TLE reload. idx=%d", NowTracking);
    return -2;
    }   

    State.Satellites[NowTracking]->use = true;
    if(State.Satellites[NowTracking]->cal == false)
        State.Satellites[NowTracking]->Refresh(State.Satellites[NowTracking]->tle, State.Satellites[NowTracking]->obs, true, true);
    printf("Updated Propagator.\n");
    sleep(3);
    State.NowTracking = true;
    State.ModelRefreshRequired = true;
    printf("Model Refresh.\n");
    sleep(3);
    SetNowTracking(NowTracking);
    PathGenerator(GetNowTracking());
    State.Display_load = false;
    State.Display_TLE = false;
    printf("Finish Autopilot.\n");
}



