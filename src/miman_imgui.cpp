#include <pthread.h>
#include <inttypes.h>
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include <gs/param/rparam.h>
#include <csp/switch.h>
#include <csp/delay.h>

#include "miman_config.h"
#include "miman_imgui.h"
#include "miman_csp.h"
#include "miman_radial.h"
#include "miman_coms.h"
#include "miman_orbital.h"
#include "miman_model.h"
#include "miman_s_control.h"
#include "miman_ftp.h"
#include "miman_autopilot.h"
#include <mutex>

#define CMD_LABEL_MAX 330
extern FILE *log_ptr;
bool raw_data = true;
Console console;
bool p_open;
ImGuiWindowFlags mim_winflags = ImGuiWindowFlags_NoMove |
                                ImGuiWindowFlags_NoResize |
                                ImGuiWindowFlags_NoSavedSettings |
                                ImGuiWindowFlags_NoCollapse |
                                ImGuiWindowFlags_NoBringToFrontOnFocus|
                                ImGuiWindowFlags_HorizontalScrollbar|
                                ImGuiWindowFlags_NoTitleBar;
extern pthread_t p_thread[16];
extern Ptable_0 param0;
extern Ptable_1 param1;
extern Ptable_5 param5;
extern StateCheckUnit State;
extern Command * command;
extern Beacon * beacon;
extern MissionBeacon * missionbeacon;
extern GETFILEINFO * getfileinfo;
extern Event * event;
extern int BeaconCounter;
extern int PingCounter;
char callsignbuf[10];

int temptarget = 1;
int tempstatus = 1;
int tempfilenum = 1;
int tempoffset = 0;
int tempstep = 0;

extern dlreqdata * request;
static int freeze_cols = 1;
static int freeze_rows = 1;

bool booking;
uint32_t gen_msgid = 0;
uint16_t gen_fnccode = 0;

extern CmdGenerator_GS * SatCMD[256];
static bool show_rparamWindow         = false;      // rparam 가져오기 custom
static bool show_pingWindow           = false;      // ping 난사
static bool show_tmtcWindow           = false;      // 초기관제
static bool show_initialControlWindow = false;      // 초기관제
static const int INITIAL_CMD_INDICES[] = {
    0, 1, 2, 3, 4,   // no arg, u8, u16, u32, u64
    72, 73, 74, 76, 77, 79, 81, 82, 85, 86, 58,80, // ADCS sunpointing용
    155,    // Enable Beacon
    200,
    205,    // UANT burn channel
    160,    // UANT Get Status
    25,     // EPS Dock Get HK
    164, 165, 166, 167,   // Deploy SP
    204,       // FM Get File Info
    206,       // UTRX GNDWDT clear
    -1               // 끝 표시
};
ReportPacket_t g_last_report = {};                  // RPT


bool SchedulerState = false;
bool ChecksumState = true;
uint32_t ExecutionTimeBuf = 0;
uint32_t ExecutionWindowBuf = 0;
uint16_t EntryIDBuf = 0;
uint16_t GroupIDBuf = 0;
int NowCMD = 0;

extern struct Antenna Now;
int NowTLE = 0;
int Requested = -2;
extern int NowFTP;
extern Setup * setup;

Ptable_0 rparam0;
Ptable_1 rparam1;
Ptable_5 rparam5;
uint8_t remote_activeconf = 99;
int16_t remote_lastrssi = 999;
int16_t remote_rssibusy = 0;
uint16_t remote_bootcount = 0;

int Satellite_row;
char TrackingButtonTextBuffer[32];
char InfoButtonTextBuffer[32];

char Sendinglabelbuf[16];
char Deletelabelbuf[16];
uint8_t msgidbuf[2];
uint16_t msgidinfo;

char SelectButtonTextBuf[32];
char DeleteButtonTextBuf[32];
char SearchBuf[256];
int tlepopupindex;
bool sgp4check = false;
char FDSFilePath[256] = "../bin/fds/fds.dat";
char SatWindowLabelBuf[64];
char inputbuf[1024];
extern Sband * sgs;

char ComboboxLabel[32];
char ADCSCMDLabels[128][64];
char CICMDLabels[128][64];
char EPSCMDLabels[128][64];
char FMCMDLabels[128][64];
char GPSCMDLabels[128][64];
char MTQCMDLabels[128][64];
char PAYCMDLabels[128][64];
char RWACMDLabels[128][64];
char SNSRCMDLabels[128][64];
char STXCMDLabels[128][64];
char TOCMDLabels[128][64];
char UTRXCMDLabels[128][64];
char TSCMDLabels[128][64];
char ESCMDLabels[128][64];
char SCHCMDLabels[128][64];
char ECMCMDLabels[128][64];

char Templabels[CMD_LABEL_MAX][64];

char AmpTimeBuf[32];

bool show_typingWindow = false;
typedef struct CMDInfo {
    uint16_t msgid;
    uint16_t fnccode;
}__attribute__((packed));

//CMDInfo *info = new CMDInfo();

cmd_packet_t * shellcmd = new cmd_packet_t;
char repeated_label[64];

/********************************************5차 추가***************************************************/

static int current_sat_mode = 0; // 0: BEE1000, 1: BEE1012, 2: Uelysys
static int chosen_sat_mode = 0;
static const char* sat_names[3] = { "BEE1000", "BEE1012", "Uelysys" };
static const char* hmac_keys[3] = { "BEE1000", "BEE1012", "UELYSYS" };
static bool is_switching = false; // 상태 체크를 위함

static bool show_satellite_select = false;

static void* SwitchSatelliteTask(void* arg)
{
    extern Setup* setup;
    int mode = *(int*)arg;
    uint8_t target_node = setup->gs100_node; // 지상국의 gs100 노드

    char target_key[32] = { 0 };

    console.AddLog("Changing GS HMAC Key for %s...", sat_names[mode]); // 테스트 텍스트, 후에 수정가능

    // RX 업데이트
    gs_rparam_set_int8(target_node, 1, 0x0010, 0xB00B, 100, 1); //100은 수정 가능, 1은 테이블 번호 (rx table)
    gs_rparam_set_string(target_node, 1, 0x0020, 0xB00B, 100, target_key, strlen(target_key) + 1);

    // TX 업데이트
    gs_rparam_set_int8(target_node, 5, 0x0010, 0xB00B, 100, 1);
    gs_rparam_set_string(target_node, 5, 0x0020, 0xB00B, 100, target_key, strlen(target_key) + 1);

    console.AddLog("[OK]## Satellite Switched to: %s", sat_names[mode]);

    is_switching = false; // 통신이 끝나면 플래그 해제 (다시 버튼 클릭 가능)
    return NULL;
}

/***********************************************************************************************************/

void ImGui_ClearColorBuf(GLFWwindow * window, ImVec4 clear_color)
{
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
}


void ImGui_MainMenu()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("FDS Manager"))
            {
                State.Display_FDS = true;
            }
            if(ImGui::MenuItem("TLE Manager"))
            {
                State.Display_TLE = true;
            }
            if(ImGui::MenuItem("S-band Manager"))
            {
                State.Display_Sband = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Hardware"))
        {
            if(ImGui::MenuItem("GS100 Param 0"))
            {
                State.Display_paramt0 = true;
            }
            if(ImGui::MenuItem("GS100 Param 1"))
            {
                State.Display_paramt1 = true;
            }
            if(ImGui::MenuItem("GS100 Param 5"))
            {
                State.Display_paramt5 = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Test"))
        {
            if (ImGui::MenuItem("Scan AX100"))
            {
                
                pthread_create(&p_thread[4], NULL, csp_ping_scan, NULL); 
            }
            if (ImGui::MenuItem("Start Signaltest"))
            {
                pthread_create(&p_thread[10], NULL, &SignalTest, NULL); 
            }
            if (ImGui::MenuItem("Stop Signaltest"))
            {
                State.Signaltest = false;
                pthread_join(p_thread[10], NULL); 
            }
            if(State.Debugmode)
            {
                if (ImGui::MenuItem("Debug Mode Off"))
                {
                    csp_log_info("Set Debug : False.");
                    State.Debugmode = false;
                    csp_debug_set_level(CSP_INFO, false);
                    // csp_debug_toggle_level(CSP_BUFFER);
                    csp_debug_set_level(CSP_PACKET, false);
                    csp_debug_set_level(CSP_PROTOCOL, false);
                    // csp_debug_toggle_level(CSP_LOCK);
                    
                }
                    
            }
            else
            {
                if (ImGui::MenuItem("Debug Mode On"))
                {
                    State.Debugmode = true;
                    csp_debug_set_level(CSP_INFO, true);
                    // csp_debug_toggle_level(CSP_BUFFER);
                    csp_debug_set_level(CSP_PACKET, true);
                    csp_debug_set_level(CSP_PROTOCOL, true);
                    // csp_debug_toggle_level(CSP_LOCK);
                    csp_log_info("Set Debug : True.");
                }
                    
            }
            if (ImGui::MenuItem("TLE Update(Auto)"))
            {
                TLE_Autoupdate_Test();
            }
            if (ImGui::MenuItem("Segfault"))
            {
                int* ptr = nullptr;
                *ptr = 42;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void ImGui_ModelWindow(float fontscale)
{
    
    ImGui::Begin("Orbital Model", &p_open, mim_winflags);
    ImGui::SetWindowFontScale(fontscale);
    
    ImGui::SetWindowFontScale(1.0);
    ImGui::End();
}



void ImGui_TrackWindow(float fontscale)
{
    ImGui::Begin("Track Window", &p_open, mim_winflags);
    ImGui::SetWindowFontScale(fontscale * 0.8);

    ImGui::SameLine();
    Satellite_row = 0;

    // ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImVec4(0.698f, 0.847f, 0.949f, 1.0f)); // 표 1행 배경색 조절
    // ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));             // 표 1행 글자색 조절 (지금은 흰색)


    if (ImGui::BeginTable("##SatelliteListup", 8,
        ImGuiTableFlags_SizingStretchSame |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Borders |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Reorderable,
        ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y)))
    {
        ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("Next AOS");
        ImGui::TableSetupColumn("Next LOS");
        ImGui::TableSetupColumn("Max Elevation");
        ImGui::TableSetupColumn("Control");
        ImGui::TableSetupColumn("Azimuth");
        ImGui::TableSetupColumn("Elevation");
        ImGui::TableSetupColumn("Altitude");
        ImGui::TableHeadersRow();
        // ImGui::PopStyleColor(2);

        // --- Highlighting Fatellites row ---
        if (strlen(State.Fatellites->Name()) != 0)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.950f, 0.266f, 0.322f, 1.0f));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", State.Fatellites->Name());
            ImGui::TableNextColumn();
            ImGui::Text("%d/%d %d:%d:%d",
                State.Fatellites->_nextaos[0].AddHours(9).Month(),
                State.Fatellites->_nextaos[0].AddHours(9).Day(),
                State.Fatellites->_nextaos[0].AddHours(9).Hour(),
                State.Fatellites->_nextaos[0].AddHours(9).Minute(),
                State.Fatellites->_nextaos[0].AddHours(9).Second());

            ImGui::TableNextColumn();
            ImGui::Text("%d/%d %d:%d:%d",
                State.Fatellites->_nextlos[0].AddHours(9).Month(),
                State.Fatellites->_nextlos[0].AddHours(9).Day(),
                State.Fatellites->_nextlos[0].AddHours(9).Hour(),
                State.Fatellites->_nextlos[0].AddHours(9).Minute(),
                State.Fatellites->_nextlos[0].AddHours(9).Second());

            ImGui::TableNextColumn();
            ImGui::Text("%.2f", State.Fatellites->_max_elevation[0] * RAD_TO_DEG);

            ImGui::TableNextColumn();
            sprintf(InfoButtonTextBuffer, "Info##trackingwindow_Main");
            if (ImGui::Button(InfoButtonTextBuffer, ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
            {
                Requested = -1;
                State.Display_Satinfo = true;
            }

            ImGui::SameLine();
            if (GetNowTracking() == -1)
            {
                ImGui::Text("  On");
            }
            else
            
            {
                sprintf(TrackingButtonTextBuffer, "Track##trackingwindow_Main");
                if (ImGui::Button(TrackingButtonTextBuffer, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
                    SetNowTracking(-1);
            }

            ImGui::TableNextColumn();
            ImGui::Text("%.3lf°", State.Fatellites->topo.azimuth * RAD_TO_DEG);
            ImGui::TableNextColumn();
            ImGui::Text("%.3lf°", State.Fatellites->topo.elevation * RAD_TO_DEG);

            ImGui::TableNextColumn();
            ImGui::Text("%.2lfkm", State.Fatellites->geo.altitude);

            ImGui::PopStyleColor();  
        }

        // --- Normal satellite rows ---
        for (Satellite_row = 0; Satellite_row < sizeof(State.Satellites) / sizeof(SatelliteObject*); Satellite_row++)
        {
            if (strlen(State.Satellites[Satellite_row]->Name()) == 0) break;
            if (!State.Satellites[Satellite_row]->use) continue;

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", State.Satellites[Satellite_row]->Name());

            ImGui::TableNextColumn();
            ImGui::Text("%d/%d %d:%d:%d",
                State.Satellites[Satellite_row]->_nextaos[0].AddHours(9).Month(),
                State.Satellites[Satellite_row]->_nextaos[0].AddHours(9).Day(),
                State.Satellites[Satellite_row]->_nextaos[0].AddHours(9).Hour(),
                State.Satellites[Satellite_row]->_nextaos[0].AddHours(9).Minute(),
                State.Satellites[Satellite_row]->_nextaos[0].AddHours(9).Second());

            ImGui::TableNextColumn();
            ImGui::Text("%d/%d %d:%d:%d",
                State.Satellites[Satellite_row]->_nextlos[0].AddHours(9).Month(),
                State.Satellites[Satellite_row]->_nextlos[0].AddHours(9).Day(),
                State.Satellites[Satellite_row]->_nextlos[0].AddHours(9).Hour(),
                State.Satellites[Satellite_row]->_nextlos[0].AddHours(9).Minute(),
                State.Satellites[Satellite_row]->_nextlos[0].AddHours(9).Second());

            ImGui::TableNextColumn();
            ImGui::Text("%.2f", State.Satellites[Satellite_row]->_max_elevation[0] * RAD_TO_DEG);

            ImGui::TableNextColumn();
            sprintf(InfoButtonTextBuffer, "Info##trackingwindow%d", Satellite_row);
            if (ImGui::Button(InfoButtonTextBuffer, ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
            {
                Requested = Satellite_row;
                State.Display_Satinfo = true;
            }

            ImGui::SameLine();
            if (GetNowTracking() == Satellite_row)
            {
                ImGui::Text("  On");
            }
            else
            {
                sprintf(TrackingButtonTextBuffer, "Track##trackingwindow%d", Satellite_row);
                if (ImGui::Button(TrackingButtonTextBuffer, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
                    SetNowTracking(Satellite_row);
            }

            ImGui::TableNextColumn();
            ImGui::Text("%.3lf°", State.Satellites[Satellite_row]->topo.azimuth * RAD_TO_DEG);
            ImGui::TableNextColumn();
            ImGui::Text("%.3lf°", State.Satellites[Satellite_row]->topo.elevation * RAD_TO_DEG);

            ImGui::TableNextColumn();
            ImGui::Text("%.2lfkm", State.Satellites[Satellite_row]->geo.altitude);
        }

        ImGui::EndTable();

    }

    ImGui::End();
}

void ImGui_AutoPilotWindow(float fontscale)
{
    static bool p_open = true;

    ImGui::Begin("AutoPilot", &p_open, mim_winflags);
    ImGui::SetWindowFontScale(fontscale);

    // ===== Autopilot controls =====
    ImGui::Text("AutoPilot Controls");
    ImGui::Checkbox("Beacon Check", &State.DoBeaconCheck);
    // ImGui::SameLine();
    ImGui::Checkbox("Ping", &State.DoPing);
    ImGui::SameLine();
    ImGui::Checkbox("CMD", &State.DoCMD);

    // ImGui::Checkbox("FTP DL", &State.DoFTPDL);
    // ImGui::SameLine();
    // ImGui::Checkbox("FTP UL", &State.DoFTPUL);
    // ImGui::SameLine();
    ImGui::Checkbox("AMP", &State.AMPmode);

    // ImGui::Checkbox("Baud Calibration", &State.DoBaudCalibration);
    // ImGui::SameLine();
    // ImGui::Checkbox("Freq Calibration", &State.DoFreqCalibration);

    ImGui::Text("iteration");
    ImGui::SameLine();
    ImGui::InputInt("##AutoPilotIter", &State.iteration);

    ImGui::Text("Time Span");
    ImGui::SameLine();
    ImGui::InputInt("##AutoPilotSpan", &State.SpanTime);

    if (!State.Autopilot)
    {
        if (ImGui::Button("Start AutoPilot", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.8f)))
        {
            State.Autopilot = true;
            pthread_create(&p_thread[13], NULL, AutoPilot, NULL);
        }
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.650f, 0.266f, 0.322f, 1.0f));
        if (ImGui::Button("Stop AutoPilot", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.8f)))
        {
            State.Autopilot = false;
            pthread_join(p_thread[13], NULL);
        }
        ImGui::PopStyleColor();
    }

    // ===== Live status =====
    ImGui::Separator();
    ImGui::Text("Live Counters");
    ImGui::Text("BeaconCounter : %d", BeaconCounter);
    ImGui::Text("PingCounter   : %d", PingCounter);
    ImGui::Text("CMDCount      : %d", State.CMDCount);

    // ===== CMD Generator (simple) =====
    ImGui::Separator();
    ImGui::Text("CMD Generator");

    static uint32_t test_msgid;
    static uint16_t test_fnccode;
    static CFE_MSG_CommandHeader dummy_hdr;

    ImGui::InputScalar("MsgID",   ImGuiDataType_U32, &test_msgid,  NULL, NULL, "0x%08X");
    ImGui::InputScalar("FncCode", ImGuiDataType_U16, &test_fnccode, NULL, NULL, "%u");

    CMDDataGenerator(test_msgid, test_fnccode, &dummy_hdr, sizeof(dummy_hdr));

    ImGui::Separator();
    ImGui::Text("SatCMD Queue");
    for (int i = 0; i < 256; i++)
    {
        if (SatCMD[i] != NULL)
            ImGui::Text("SatCMD[%d] : READY", i);
    }

    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}





void ImGui_FrequencyWindow(float fontscale)
{   
    
    ImGui::Begin("Frequency Window", &p_open, mim_winflags);
    ImGui::SetWindowFontScale(fontscale);
    ImGui::Text("Center Frequency");
    ImGui::InputDouble("##centerfrequency(Hz)", &setup->default_freq, NULL, NULL);
    ImGui::SameLine();
    if(ImGui::Button("Set##SetDefaultFrequency", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
    {
        SetRxFreq(setup->default_freq);
        SetTxFreq(setup->default_freq);
        SetRxBaud(4800);
        SetTxBaud(4800);
        UpdateFreq();
        UpdateBaud();
    }

    if (ImGui::BeginTable("##TRXspectable", 2, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 3.8)))
    {
        ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
        ImGui::TableSetupColumn("RX",       ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide, 0.0f);
        ImGui::TableSetupColumn("TX",       ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide, 0.0f);
        ImGui::TableHeadersRow();
        ImGui::TableNextColumn();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%f MHz", ((float)param1.freq) / 1000000);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%f MHz", ((float)param5.freq) / 1000000);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%"PRIu32" bps", param1.baud);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%"PRIu32" bps", param5.baud);
        ImGui::EndTable();
    }
    ImGui::Text("Last RSSI(dBm) : ");
    ImGui::SameLine();
    ImGui::Text("%"PRId16"", Return_RSSI());
    if (ImGui::Button("Set to RX", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
    {
        switch_to_rx(setup->ax100_node);
    }
    ImGui::SameLine();
    if (ImGui::Button("Set to TX", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
    {
        switch_to_tx(setup->ax100_node);
    }

    if(State.Doppler == false)
    {

        if (ImGui::Button("Start Doppler Correction", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(State.GS100_Connection)
            {
                State.Doppler = true;
                pthread_create(&p_thread[7], NULL, Doppler, NULL);
                console.AddLog("[OK]##Start Doppler Correction.");
            }
            else
            {
                console.AddLog("[ERROR]##GS100 Connection Error - Cannot Start Doppler Correction.");
            }
            
        }
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.650f, 0.266f, 0.322f, 1.0f));
        if (ImGui::Button("Stop Doppler Correction", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            State.Doppler = false;
            pthread_join(p_thread[7], NULL);
        }
        ImGui::PopStyleColor();
    }
    ImGui::Text("Ping Size  ");
    ImGui::SameLine();
    ImGui::InputScalar("##pingsize", ImGuiDataType_U16, &setup->pingsize, NULL, NULL, "%u");
    ImGui::BeginChild("##DelaySettings", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true, mim_winflags);
    ImGui::Text("Default Delay");
    ImGui::SameLine();
    ImGui::InputScalar("##defaulttimeout", ImGuiDataType_U32, &setup->default_timeout, NULL, NULL, "%u");
    ImGui::Text("Guard Delay  ");
    ImGui::SameLine();
    ImGui::InputScalar("##guarddelay", ImGuiDataType_U32, &setup->guard_delay, NULL, NULL, "%u");
    ImGui::Text("Queue Delay  ");
    ImGui::SameLine();
    ImGui::InputScalar("##queuedelay", ImGuiDataType_U32, &setup->queue_delay, NULL, NULL, "%u");
    if(ImGui::Button("Apply##apply", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
    {
        set_serial_spec_micsec(setup->Transceiver_baud, setup->Switch_baud, 1000000, setup->queue_delay, setup->gs100_node, setup->default_timeout, setup->guard_delay);
        console.AddLog("Set Delay Spec : Default : %u, Guard : %u, Queue : %u", setup->default_timeout, setup->guard_delay, setup->queue_delay);
    }
    ImGui::EndChild();

    ImGui::SetWindowFontScale(1.0);
    ImGui::End();
}


void ImGui_RotatorWindow(float fontscale)
{
    ImGui::Begin("Rotator Window", &p_open, mim_winflags);
    ImGui::SetWindowFontScale(fontscale);
    if(State.Engage == false)
    {
        if (ImGui::Button("Start Rotator Engaging", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            State.Engage = true;
            pthread_create(&p_thread[1], NULL, Sattracker, NULL);
            console.AddLog("[OK]##Start Engaging.");
        }
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.650f, 0.266f, 0.322f, 1.0f));
        if (ImGui::Button("Stop Rotator Engaging", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            State.Engage = false;
            State.Doppler = false;
            pthread_join(p_thread[1], NULL);
            console.AddLog("[OK]##Finish Engaging.");
            console.AddLog("Beacon Success : %d, Ping Success : %d", BeaconCounter, PingCounter);
            BeaconCounter = 0;
            PingCounter = 0;
        }
        ImGui::PopStyleColor();
    }
    
    
    ImGui::Text("Rotator Manual Control");
    ImGui::Text("AZ : ");
    ImGui::SameLine();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputInt("##tg_az", &Now.az_tag);
    ImGui::PopItemWidth();
    ImGui::Text("EL : ");
    ImGui::SameLine();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputInt("##tg_el", &Now.el_tag);
    ImGui::PopItemWidth();
    if (ImGui::Button("Set", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
    {
        RotatorSetTo(Now.az_tag, Now.el_tag);
        //memset(&ControlBuf,0,sizeof(ControlBuf));
    }
    ImGui::SameLine();
    if (ImGui::Button("Init", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
    {
        RotatorSetTo(0, 0);
    }

    if (ImGui::BeginTable("##Rotspectable", 3, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 3.8)))
    {
        ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
        ImGui::TableSetupColumn("Band",         ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide, 0.0f);
        ImGui::TableSetupColumn("Azimuth",      ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide, 0.0f);
        ImGui::TableSetupColumn("Elevation",    ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide, 0.0f);
        ImGui::TableHeadersRow();
        ImGui::TableNextColumn();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("UHF");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%d °", Now.az_now);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%d °", Now.el_now);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("S-band");
        ImGui::TableSetColumnIndex(1);
        // ImGui::Text("%d °", Now.az_now);
        ImGui::TableSetColumnIndex(2);
        // ImGui::Text("%d °", Now.el_now);
        ImGui::EndTable();
    }


    if(ImGui::Button("Rotator On", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
    {
        init_rotator();
        pthread_create(&p_thread[2], NULL, RotatorReadInfo, NULL);
    }
    ImGui::SameLine();
    if(ImGui::Button("Rotator OFF", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
    {
        State.RotatorReading = false;
        pthread_join(p_thread[2], NULL);
        // fin_rotator();
    }
    if(ImGui::Button("Rotator IO Buffer Free", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
    {
        RotatorEnter(NULL);
    }
    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();

}

void StateWindowColumnManager(const char * info)
{
    ImGui::TableNextColumn();
    ImGui::Text(info);
    ImGui::TableNextColumn();
}

// 비콘 가독성 업!!
// static void BeaconSectionHeader(const char* title)
// {
//     ImGui::TableNextRow();
//     ImGui::TableSetColumnIndex(0);

//     ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1,
//                        ImGui::GetColorU32(ImVec4(0.25f, 0.25f, 0.35f, 0.4f)));


//     ImGui::PushFont(ImGui::GetFont());        
//     ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
//     ImGui::TextUnformatted(title);
//     ImGui::PopStyleColor();
//     ImGui::PopFont();

//     ImGui::TableSetColumnIndex(1);
//     ImGui::TextUnformatted("");
// }
static void BeaconSectionHeader(const char* title)
{
    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

    // Row background
    ImGui::TableSetBgColor(
        ImGuiTableBgTarget_RowBg0,
        ImGui::GetColorU32(ImVec4(0.25f, 0.25f, 0.35f, 0.6f))
    );

    ImGui::TableSetColumnIndex(0);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.6f, 1.0f));
    ImGui::TextUnformatted(title);
    ImGui::PopStyleColor();

    ImGui::TableSetColumnIndex(1);
    ImGui::TextUnformatted("");
}



void ImGui_BeaconWindow(float fontscale)
{
            
    ImGui::Begin("Beacon GUI", &p_open, mim_winflags);
    ImGui::SetWindowFontScale(fontscale);

    ImGui::BeginTabBar("##InfoDesk");
    if(ImGui::BeginTabItem("Beacon"))
    {

        if (ImGui::BeginTable("##BeaconTables", 2, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable))


    ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
    ImGui::TableSetupColumn("Parameter", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide, 0.0f);
    ImGui::TableSetupColumn("Data",      ImGuiTableColumnFlags_NoHide, 0.0f);
   
    ImGui::TableHeadersRow();


    if (!beacon)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Beacon");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("No data");
    }
    else
    {
        char buf[128];


        // =========================
        //  CCSDS HEADER
        // =========================
        BeaconSectionHeader("CCSDS Header");
        StateWindowColumnManager("CCSDS MsgID");
        ImGui::Text("0x%02X 0x%02X", beacon->CCSDS_MID[0], beacon->CCSDS_MID[1]);

        StateWindowColumnManager("CCSDS Sequence");
        ImGui::Text("0x%02X 0x%02X", beacon->CCSDS_Seq[0], beacon->CCSDS_Seq[1]);

        StateWindowColumnManager("CCSDS Length");
        ImGui::Text("0x%02X 0x%02X", beacon->CCSDS_Len[0], beacon->CCSDS_Len[1]);

        StateWindowColumnManager("CCSDS Time Code");
        ImGui::Text("0x%02X %02X %02X %02X %02X %02X",
                    beacon->CCSDS_TimeCode[0], beacon->CCSDS_TimeCode[1],
                    beacon->CCSDS_TimeCode[2], beacon->CCSDS_TimeCode[3],
                    beacon->CCSDS_TimeCode[4], beacon->CCSDS_TimeCode[5]);

        // =========================
        //  FSW - RPT
        // =========================


        BeaconSectionHeader("FSW - RPT");

        StateWindowColumnManager("RPT Command Counter");
        ImGui::Text("%" PRIu8, beacon->RPT_ErrCounter);

        StateWindowColumnManager("RPT Error Counter");
        ImGui::Text("%" PRIu8, beacon->RPT_ErrCounter);

        StateWindowColumnManager("RPT Report Count");
        ImGui::Text("%" PRIu8, beacon->RPT_ReportCnt);

        StateWindowColumnManager("RPT Critical Count");
        ImGui::Text("%" PRIu8, beacon->RPT_CriticalCnt);

        StateWindowColumnManager("RPT Boot Count");
        ImGui::Text("%" PRIu16, beacon->RPT_BootCount);

        StateWindowColumnManager("RPT Sc Time (sec)");
        ImGui::Text("%" PRIu32, beacon->RPT_ScTimeSec);

        StateWindowColumnManager("RPT Sc Time (subsec)");
        ImGui::Text("%" PRIu32, beacon->RPT_ScTimeSubsec);

        StateWindowColumnManager("RPT Sequence LSB");
        ImGui::Text("%" PRIu8, beacon->RPT_Sequence_LSB);

        // =========================
        //  COMS - STX (S-band Tx)
        // =========================
        BeaconSectionHeader("COMS - STX");
        StateWindowColumnManager("STX Symbol Rate");
        ImGui::Text("%" PRIu8, beacon->STX_symbol_rate);

        StateWindowColumnManager("STX Transmit Power");
        ImGui::Text("%" PRIu8, beacon->STX_transmit_power);

        StateWindowColumnManager("STX MODCOD");
        ImGui::Text("%" PRIu8, beacon->STX_modcod);

        StateWindowColumnManager("STX Roll-off");
        ImGui::Text("%" PRIu8, beacon->STX_roll_off);

        StateWindowColumnManager("STX Pilot Signal");
        ImGui::Text("%" PRIu8, beacon->STX_pilot_signal);

        StateWindowColumnManager("STX FEC Frame Size");
        ImGui::Text("%" PRIu8, beacon->STX_fec_frame_size);

        StateWindowColumnManager("STX Pre-Transmission Delay");
        ImGui::Text("%" PRIu16, beacon->STX_pretransmission_delay);

        StateWindowColumnManager("STX Center Frequency");
        ImGui::Text("%.3f", beacon->STX_center_frequency);

        StateWindowColumnManager("STX Modulator IF Type");
        ImGui::Text("%" PRIu8, beacon->STX_modulator_interface_type);

        StateWindowColumnManager("STX LVDS IO Type");
        ImGui::Text("%" PRIu8, beacon->STX_lvds_io_type);

        StateWindowColumnManager("STX System State");
        ImGui::Text("%" PRIu8, beacon->STX_SystemState);

        StateWindowColumnManager("STX Status Flag");
        ImGui::Text("%" PRIu8, beacon->STX_StatusFlag);

        // StateWindowColumnManager("STX CPU Temperature");
        // ImGui::Text("%.2f", beacon->STX_CpuTemp);

        // =========================
        //  COMS - UANT
        // =========================
        BeaconSectionHeader("COMS - UANT");
        StateWindowColumnManager("UANT1 Channels (0,1,Backup)");
        ImGui::Text("%" PRIu8 " %" PRIu8 " (Backup:%" PRIu8 ")",
                    beacon->UANT1_Chan0, beacon->UANT1_Chan1, beacon->UANT1_BackupActive);

        StateWindowColumnManager("UANT2 Channels (0,1,Backup)");
        ImGui::Text("%" PRIu8 " %" PRIu8 " (Backup:%" PRIu8 ")",
                    beacon->UANT2_Chan0, beacon->UANT2_Chan1, beacon->UANT2_BackupActive);

        // =========================
        //  COMS - UTRX
        // =========================
        BeaconSectionHeader("COMS - UTRX");
        StateWindowColumnManager("UTRX Active Config");
        ImGui::Text("%" PRIu8, beacon->UTRX_ActiveConf);

        StateWindowColumnManager("UTRX Boot Count");
        ImGui::Text("%" PRIu16, beacon->UTRX_BootCount);

        StateWindowColumnManager("UTRX Boot Cause");
        ImGui::Text("%" PRIu32, beacon->UTRX_BootCause);

        StateWindowColumnManager("UTRX Board Temperature");
        ImGui::Text("%" PRId16, beacon->UTRX_BoardTemp);

        // =========================
        //  PCDU - P60 Dock
        // =========================
        BeaconSectionHeader("EPS - P60 Dock");
        StateWindowColumnManager("P60 Dock Cout [0..2]");
        ImGui::Text("%" PRId16 " %" PRId16 " %" PRId16,
                    beacon->P60D_Cout[0], beacon->P60D_Cout[1], beacon->P60D_Cout[2]);

        StateWindowColumnManager("P60 Dock Vout [0..2]");
        ImGui::Text("%" PRIu16 " %" PRIu16 " %" PRIu16,
                    beacon->P60D_Vout[0], beacon->P60D_Vout[1], beacon->P60D_Vout[2]);

        StateWindowColumnManager("P60 Dock OutEn (bitmask)");
        ImGui::Text("0x%04X", beacon->P60D_OutEn);

        StateWindowColumnManager("P60 Dock Boot Cause");
        ImGui::Text("%" PRIu32, beacon->P60D_BootCause);

        StateWindowColumnManager("P60 Dock Boot Count");
        ImGui::Text("%" PRIu32, beacon->P60D_BootCount);

        StateWindowColumnManager("P60 Dock Batt Mode");
        ImGui::Text("%" PRIu8, beacon->P60D_BattMode);

        StateWindowColumnManager("P60 Dock Heater On");
        ImGui::Text("%" PRIu8, beacon->P60D_HeaterOn);

        StateWindowColumnManager("P60 Dock VBAT Voltage");
        ImGui::Text("%" PRIu16, beacon->P60D_VbatV);

        StateWindowColumnManager("P60 Dock VCC Current");
        ImGui::Text("%" PRId16, beacon->P60D_VccC);

        StateWindowColumnManager("P60 Dock Batt Voltage");
        ImGui::Text("%" PRIu16, beacon->P60D_BattV);

        StateWindowColumnManager("P60 Dock Batt Temp[0..1]");
        ImGui::Text("%" PRId16 " %" PRId16,
                    beacon->P60D_BattTemp[0], beacon->P60D_BattTemp[1]);

        StateWindowColumnManager("P60 Dock WDT CAN Left");
        ImGui::Text("%" PRIu32, beacon->P60D_WdtCanLeft);

        StateWindowColumnManager("P60 Dock Batt Charge Current");
        ImGui::Text("%" PRId16, beacon->P60D_BattChrg);

        StateWindowColumnManager("P60 Dock Batt Discharge Current");
        ImGui::Text("%" PRId16, beacon->P60D_BattDischrg);

        // =========================
        //  PCDU - P60 PDU
        // =========================
        BeaconSectionHeader("EPS - P60 PDU");
        StateWindowColumnManager("P60 PDU Cout[0..8]");
        ImGui::Text("%" PRId16 " %" PRId16 " %" PRId16 " %" PRId16 " %" PRId16 " %" PRId16 " %" PRId16 " %" PRId16 " %" PRId16,
                    beacon->P60P_Cout[0], beacon->P60P_Cout[1], beacon->P60P_Cout[2], beacon->P60P_Cout[3], beacon->P60P_Cout[4], beacon->P60P_Cout[5],
                beacon->P60P_Cout[6], beacon->P60P_Cout[7], beacon->P60P_Cout[8]);

        StateWindowColumnManager("P60 PDU Vout[0..8]");
        ImGui::Text("%" PRIu16 " %" PRIu16 " %" PRIu16 " %" PRIu16 " %" PRIu16 " %" PRIu16 " %" PRIu16 " %" PRIu16 " %" PRIu16,
                    beacon->P60P_Vout[0], beacon->P60P_Vout[1], beacon->P60P_Vout[2], beacon->P60P_Vout[3], beacon->P60P_Vout[4], beacon->P60P_Vout[5], 
                beacon->P60P_Vout[6], beacon->P60P_Vout[7], beacon->P60P_Vout[8]);

        StateWindowColumnManager("P60 PDU Vcc");
        ImGui::Text("%" PRId16, beacon->P60P_Vcc);

        StateWindowColumnManager("P60 PDU ConvEn");
        ImGui::Text("%" PRIu8, beacon->P60P_ConvEn);

        StateWindowColumnManager("P60 PDU OutEn (bitmask)");
        ImGui::Text("0x%04X", beacon->P60P_OutEn);

        // =========================
        //  PCDU - P60 ACU
        // =========================
        BeaconSectionHeader("EPS - P60 ACU");
        StateWindowColumnManager("P60 ACU Cin[0..5]");
        ImGui::Text("%" PRId16 " %" PRId16 " %" PRId16 " %" PRId16 " %" PRId16 " %" PRId16,
                    beacon->P60A_Cin[0], beacon->P60A_Cin[1],
                    beacon->P60A_Cin[2], beacon->P60A_Cin[3], 
                    beacon->P60A_Cin[4], beacon->P60A_Cin[5]);

        StateWindowColumnManager("P60 ACU Vin[0..5]");
        ImGui::Text("%" PRIu16 " %" PRIu16 " %" PRIu16 " %" PRIu16 " %" PRIu16 " %" PRIu16,
                    beacon->P60A_Vin[0], beacon->P60A_Vin[1],
                    beacon->P60A_Vin[2], beacon->P60A_Vin[3], 
                    beacon->P60A_Vin[4], beacon->P60A_Vin[5]);

        // =========================
        //  ADCS
        // =========================
        BeaconSectionHeader("ADCS");
        StateWindowColumnManager("ADCS Power State (bitfield)");
        ImGui::Text("0x%02X", beacon->ADCS_PowerState);

        StateWindowColumnManager("ADCS Control Mode");
        ImGui::Text("%" PRIu8, beacon->ADCS_ControlMode);

        StateWindowColumnManager("ADCS GYR0 Calib Rate X");
        ImGui::Text("%.6f", beacon->ADCS_GYR0CalibratedRateXComponent);

        StateWindowColumnManager("ADCS GYR0 Calib Rate Y");
        ImGui::Text("%.6f", beacon->ADCS_GYR0CalibratedRateYComponent);

        StateWindowColumnManager("ADCS GYR0 Calib Rate Z");
        ImGui::Text("%.6f", beacon->ADCS_GYR0CalibratedRateZComponent);
    }


        ImGui::EndTable();
        ImGui::EndTabItem();
        ImGui::SetWindowFontScale(1.0 * fontscale);
    }



    if(ImGui::BeginTabItem("Mission Beacon"))
    {

        if (ImGui::BeginTable("##MissionBeaconTables", 2, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable))


    ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
    ImGui::TableSetupColumn("Parameter", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide, 0.0f);
    ImGui::TableSetupColumn("Data",      ImGuiTableColumnFlags_NoHide, 0.0f);
   
    ImGui::TableHeadersRow();


    if (!missionbeacon)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Mission Beacon");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("No data");
    }
    else
    {
        char buf[128];

        // =========================
        //  CCSDS HEADER
        // =========================
        BeaconSectionHeader("Telemetry Header");
        StateWindowColumnManager("CCSDS MsgID");
        ImGui::Text("0x%02X 0x%02X", missionbeacon->CCSDS_MID[0], missionbeacon->CCSDS_MID[1]);

        StateWindowColumnManager("CCSDS Sequence");
        ImGui::Text("0x%02X 0x%02X", missionbeacon->CCSDS_Seq[0], missionbeacon->CCSDS_Seq[1]);

        StateWindowColumnManager("CCSDS Length");
        ImGui::Text("0x%02X 0x%02X", missionbeacon->CCSDS_Len[0], missionbeacon->CCSDS_Len[1]);

        StateWindowColumnManager("CCSDS Time Code");
        ImGui::Text("0x%02X %02X %02X %02X %02X %02X",
                    missionbeacon->CCSDS_TimeCode[0], missionbeacon->CCSDS_TimeCode[1],
                    missionbeacon->CCSDS_TimeCode[2], missionbeacon->CCSDS_TimeCode[3],
                    missionbeacon->CCSDS_TimeCode[4], missionbeacon->CCSDS_TimeCode[5]);


        // =========================
        //  SRL HouseKeeping
        // =========================
        BeaconSectionHeader("SRL HouseKeeping");

        StateWindowColumnManager("SRL Command Counter");
        ImGui::Text("%" PRIu8, missionbeacon->srlpayload.CommandCounter);

        StateWindowColumnManager("SRL Command Error Counter");
        ImGui::Text("%" PRIu8, missionbeacon->srlpayload.CommandErrorCounter);

        for (int i = 0; i < 4; i++) {
            char label[64];

            snprintf(label, sizeof(label), "IOHandleStatus[%d]", i);
            StateWindowColumnManager(label);
            ImGui::Text("%" PRIu8, missionbeacon->srlpayload.IOHandleStatus[i]);

            snprintf(label, sizeof(label), "IOHandleTxCount[%d]", i);
            StateWindowColumnManager(label);
            ImGui::Text("%" PRIu16, missionbeacon->srlpayload.IOHandleTxCount[i]);
        }


        // =========================
        //  RPT Payload (FSW Report summary)
        // =========================
        BeaconSectionHeader("RPT Payload Summary");

        StateWindowColumnManager("CmdCounter");
        ImGui::Text("%" PRIu8, missionbeacon->rptpayload.CmdCounter);

        StateWindowColumnManager("CmdErrCounter");
        ImGui::Text("%" PRIu8, missionbeacon->rptpayload.CmdErrCounter);

        BeaconSectionHeader("RPT Queue Info");
        StateWindowColumnManager("ReportQueueCnt");
        ImGui::Text("%" PRIu8, missionbeacon->rptpayload.ReportQueueCnt);

        StateWindowColumnManager("CriticalQueueCnt");
        ImGui::Text("%" PRIu8, missionbeacon->rptpayload.CriticalQueueCnt);


        // =========================
        //  Operation Data
        // =========================
        BeaconSectionHeader("RPT Operation Data");

        StateWindowColumnManager("BootCount");
        ImGui::Text("%" PRIu16, missionbeacon->rptpayload.BootCount);

        StateWindowColumnManager("TimeSec");
        ImGui::Text("%" PRIu32, missionbeacon->rptpayload.TimeSec);

        StateWindowColumnManager("TimeSubsec");
        ImGui::Text("%" PRIu32, missionbeacon->rptpayload.TimeSubsec);

        StateWindowColumnManager("Sequence");
        ImGui::Text("%" PRIu32, missionbeacon->rptpayload.Sequence);

        StateWindowColumnManager("ResetCause");
        ImGui::Text("0x%02X", (unsigned int)missionbeacon->rptpayload.ResetCause);


        // =========================
        // Mission Beacon Payload
        // =========================
        BeaconSectionHeader("Mission Beacon Payload");

        StateWindowColumnManager("Command Counter");
        ImGui::Text("%" PRIu8, missionbeacon->paybcnpayload.CommandCounter);

        StateWindowColumnManager("Command Error Counter");
        ImGui::Text("%" PRIu8, missionbeacon->paybcnpayload.CommandErrorCounter);

        StateWindowColumnManager("Payload System Status");
        ImGui::Text("%" PRIi8, missionbeacon->paybcnpayload.sys_status);

        StateWindowColumnManager("NTC 0 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->paybcnpayload.temp_ntc_0);

        StateWindowColumnManager("NTC 1 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->paybcnpayload.temp_ntc_1);

        StateWindowColumnManager("NTC 2 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->paybcnpayload.temp_ntc_2);

        StateWindowColumnManager("NTC 3 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->paybcnpayload.temp_ntc_3);

        StateWindowColumnManager("NTC 4 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->paybcnpayload.temp_ntc_4);

        StateWindowColumnManager("NTC 5 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->paybcnpayload.temp_ntc_5);

        StateWindowColumnManager("NTC 6 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->paybcnpayload.temp_ntc_6);

        StateWindowColumnManager("NTC 7 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->paybcnpayload.temp_ntc_7);

        StateWindowColumnManager("NTC 8 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->paybcnpayload.temp_ntc_8);

        StateWindowColumnManager("NTC 9 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->paybcnpayload.temp_ntc_9);

        StateWindowColumnManager("NTC 10 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->paybcnpayload.temp_ntc_10);

        StateWindowColumnManager("NTC 11 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->paybcnpayload.temp_ntc_11);


        // =========================
        //  Mission Housekeeping Payload
        // =========================
        BeaconSectionHeader("Mission Housekeeping Payload");

        StateWindowColumnManager("HK CommandCounter");
        ImGui::Text("%" PRIu8, missionbeacon->payhkpayload.CommandCounter);

        StateWindowColumnManager("HK CommandErrorCounter");
        ImGui::Text("%" PRIu8, missionbeacon->payhkpayload.CommandErrorCounter);

        StateWindowColumnManager("HK System Status");
        ImGui::Text("%" PRIi8, missionbeacon->payhkpayload.sys_status);

        StateWindowColumnManager("HK NTC 0 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->payhkpayload.temp_ntc_0);

        StateWindowColumnManager("HK NTC 1 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->payhkpayload.temp_ntc_1);

        StateWindowColumnManager("HK NTC 2 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->payhkpayload.temp_ntc_2);

        StateWindowColumnManager("HK NTC 3 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->payhkpayload.temp_ntc_3);

        StateWindowColumnManager("HK NTC 4 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->payhkpayload.temp_ntc_4);

        StateWindowColumnManager("HK NTC 5 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->payhkpayload.temp_ntc_5);

        StateWindowColumnManager("HK NTC 6 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->payhkpayload.temp_ntc_6);

        StateWindowColumnManager("HK NTC 7 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->payhkpayload.temp_ntc_7);

        StateWindowColumnManager("HK NTC 8 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->payhkpayload.temp_ntc_8);

        StateWindowColumnManager("HK NTC 9 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->payhkpayload.temp_ntc_9);

        StateWindowColumnManager("HK NTC 10 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->payhkpayload.temp_ntc_10);

        StateWindowColumnManager("HK NTC 11 Temperature");
        ImGui::Text("%" PRIi16, missionbeacon->payhkpayload.temp_ntc_11);

        StateWindowColumnManager("HK sen1_data_0");
        ImGui::Text("%" PRIu32, missionbeacon->payhkpayload.sen1_data_0);

        StateWindowColumnManager("HK sen1_data_1");
        ImGui::Text("%" PRIu32, missionbeacon->payhkpayload.sen1_data_1);



    }


        ImGui::EndTable();
        ImGui::EndTabItem();
        ImGui::SetWindowFontScale(1.0 * fontscale);
    }






        if (ImGui::BeginTabItem("Report"))
        {
            bool show_hexdump = false;
            ReportView_t view;
            {
                std::lock_guard<std::mutex> lk(g_report_view_mtx);
                view = g_report_view;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.650f, 0.266f, 0.322f, 1.0f));
            ImGui::Text("Total RX Bytes (Report) : %" PRIu32, get_rx_bytes());
            ImGui::PopStyleColor();

            ImGui::Separator();
            ImGui::SetWindowFontScale(0.9f * fontscale);

            ImGuiTableFlags rpt_flags =
                ImGuiTableFlags_SizingStretchSame |
                ImGuiTableFlags_ScrollY          |
                ImGuiTableFlags_RowBg            |
                ImGuiTableFlags_Borders          |
                ImGuiTableFlags_Resizable        |
                ImGuiTableFlags_Reorderable;

            if (ImGui::BeginTable("##ReportTable", 2, rpt_flags))
            {
                ImGui::TableSetupColumn("Parameter", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide, 0.0f);
                ImGui::TableSetupColumn("Data",      ImGuiTableColumnFlags_NoHide, 0.0f);
                ImGui::TableHeadersRow();

                auto ReportRow = [](const char* label, const char* fmt, auto value)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(label);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text(fmt, value);
                };

                if (!view.valid)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Report");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("No data");
                }
                else
                {
                    ImGui::TableNextRow();

                    BeaconSectionHeader("[CCSDS]");

                    ReportRow("CCSDS_Message_ID", "0x%04X", (unsigned int)view.CCMessage_ID);
                    ReportRow("CCSDS_Count",      "0x%04X", (unsigned int)view.CCCount);
                    ReportRow("CCSDS_Length",     "0x%04X", (unsigned int)view.CCLength);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("CCTime Code");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%02X %02X %02X %02X %02X %02X",
                                view.CCTime_code[0], view.CCTime_code[1],
                                view.CCTime_code[2], view.CCTime_code[3],
                                view.CCTime_code[4], view.CCTime_code[5]);

                    ImGui::TableNextRow();
                    BeaconSectionHeader("[Body]");

                    ReportRow("Reflected Msg Id",       "0x%04X", (unsigned int)view.reflected_msg_id);
                    ReportRow("Reflected Command Code",           "%u",     (unsigned int)view.reflected_cc);
                    ReportRow("Return Type",     "0x%02X", (unsigned int)view.ret_type);
                    ReportRow("Return Code",     "%d",     (int)view.ret_code);
                    ReportRow("Return Value Size", "%u",     (unsigned int)view.ret_val_size);

                    ImGui::TableNextRow();
                    BeaconSectionHeader("[Report Type]");
                    
                    /*****************************5차 추가**************************************************** */
                    auto HeaderRow = [](const char* title)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.80f, 0.80f, 1.0f));
                        ImGui::TextUnformatted(title);
                        ImGui::PopStyleColor();
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextUnformatted("");
                    };

                    auto RowFmt = [](const char* label, const char* fmt, auto v)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text(fmt, v);
                    };

                    auto RowArr_Hex = [](const char* label, const uint8_t* aa, int n, int break_after)
                    {
                        std::string s;
                        s.reserve(n * 4);
                        char tmp[16];

                        for (int i = 0; i < n; i++)
                        {
                            snprintf(tmp, sizeof(tmp), "%02X", aa[i]); 

                            if (i != 0)
                            {
                                if (break_after > 0 && (i % break_after) == 0) s.push_back('\n');
                                else s.push_back(' ');
                            }
                            s += tmp;
                        }

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::TextUnformatted(s.c_str());
                    };

                    /******************************************************************************************** */

                    switch (view.kind)
                    {
                        case REPORT_KIND_EPS_P60_PDU_GET_TABLE_HK:
                        {
                            auto &pl = view.u.eps_p60pdugettablehk;

                            auto HeaderRow = [](const char* title)
                            {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.80f, 0.80f, 1.0f));
                                ImGui::TextUnformatted(title);
                                ImGui::PopStyleColor();
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted("");
                            };

                            auto RowFmt = [](const char* label, const char* fmt, auto v)
                            {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::Text(fmt, v);
                            };

                            auto RowArr_i16 = [](const char* label, const int16_t* aa, int n, int break_after)
                            {
                                std::string s;
                                s.reserve(n * 8);
                                char tmp[32];

                                for (int i = 0; i < n; i++)
                                {
                                    snprintf(tmp, sizeof(tmp), "%" PRIi16, aa[i]);

                                    if (i != 0)
                                    {
                                        if (break_after > 0 && i == break_after + 1) s.push_back('\n');
                                        else s.push_back(' ');
                                    }
                                    s += tmp;
                                }

                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted(s.c_str());
                            };

                            auto RowArr_u16 = [](const char* label, const uint16_t* aa, int n, int break_after)
                            {
                                std::string s;
                                s.reserve(n * 8);
                                char tmp[32];

                                for (int i = 0; i < n; i++)
                                {
                                    snprintf(tmp, sizeof(tmp), "%" PRIu16, aa[i]);

                                    if (i != 0)
                                    {
                                        if (break_after > 0 && i == break_after + 1) s.push_back('\n');
                                        else s.push_back(' ');
                                    }
                                    s += tmp;
                                }

                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted(s.c_str());
                            };

                            auto RowArr_u8 = [](const char* label, const uint8_t* aa, int n, int break_after)
                            {
                                std::string s;
                                s.reserve(n * 4);
                                char tmp[16];

                                for (int i = 0; i < n; i++)
                                {
                                    snprintf(tmp, sizeof(tmp), "%" PRIu8, aa[i]);

                                    if (i != 0)
                                    {
                                        if (break_after > 0 && i == break_after + 1) s.push_back('\n');
                                        else s.push_back(' ');
                                    }
                                    s += tmp;
                                }

                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted(s.c_str());
                            };

                            auto RowArr_u32 = [](const char* label, const uint32_t* aa, int n, int break_after)
                            {
                                std::string s;
                                s.reserve(n * 12);
                                char tmp[32];

                                for (int i = 0; i < n; i++)
                                {
                                    snprintf(tmp, sizeof(tmp), "%" PRIu32, aa[i]);

                                    if (i != 0)
                                    {
                                        if (break_after > 0 && i == break_after + 1) s.push_back('\n');
                                        else s.push_back(' ');
                                    }
                                    s += tmp;
                                }

                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted(s.c_str());
                            };

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::TextUnformatted("Type");
                            ImGui::TableSetColumnIndex(1);
                            ImGui::TextUnformatted("EPS P60 PDU GET TABLE HK");

                            HeaderRow("[Report Data]");
                            RowArr_i16("c_out[0..8]",  pl.c_out, 9, 4);
                            RowArr_u16("v_out[0..8]",  pl.v_out, 9, 4);
                            RowArr_u8 ("out_en[0..8]", pl.out_en, 9, 4);
                            RowArr_u8 ("conv_en[0..2]", pl.conv_en, 3, 0);

                            HeaderRow("[Power]");
                            RowFmt("vcc",  "%" PRIu16, pl.vcc);
                            RowFmt("vbat", "%" PRIu16, pl.vbat);
                            RowFmt("temp", "%" PRIi16, pl.temp);

                            HeaderRow("[Boot/Time]");
                            RowFmt("bootcause", "%" PRIu32, pl.bootcause);
                            RowFmt("bootcnt",   "%" PRIu32, pl.bootcnt);
                            RowFmt("uptime",    "%" PRIu32, pl.uptime);
                            RowFmt("resetcause","0x%04X", (unsigned int)pl.resetcause);

                            HeaderRow("[Modes]");
                            RowFmt("batt_mode", "%" PRIu8, pl.batt_mode);

                            HeaderRow("[Latchups]");
                            RowArr_u16("latchup[0..8]", pl.latchup, 9, 4);

                            HeaderRow("[Device]");
                            RowArr_u8("device_type[0..7]",   pl.device_type, 8, 0);
                            RowArr_u8("device_status[0..7]", pl.device_status, 8, 0);

                            HeaderRow("[WDT Counters]");
                            RowFmt("wdt_cnt_gnd", "%" PRIu32, pl.wdt_cnt_gnd);
                            RowFmt("wdt_cnt_i2c", "%" PRIu32, pl.wdt_cnt_i2c);
                            RowFmt("wdt_cnt_can", "%" PRIu32, pl.wdt_cnt_can);
                            RowArr_u32("wdt_cnt_csp[0..1]", pl.wdt_cnt_csp, 2, 0);

                            HeaderRow("[WDT Left]");
                            RowFmt("wdt_gnd_left", "%" PRIu32, pl.wdt_gnd_left);
                            RowFmt("wdt_i2c_left", "%" PRIu32, pl.wdt_i2c_left);
                            RowFmt("wdt_can_left", "%" PRIu32, pl.wdt_can_left);
                            RowArr_u8("wdt_csp_left[0..1]", pl.wdt_csp_left, 2, 0);

                            break;
                        }

                        
                        case REPORT_KIND_EPS_P60_ACU_GET_TABLE_HK:
                        {
                            auto &pl = view.u.eps_p60acugettablehk;

                            auto HeaderRow = [](const char* title)
                            {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.80f, 0.80f, 1.0f));
                                ImGui::TextUnformatted(title);
                                ImGui::PopStyleColor();
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted("");
                            };

                            auto RowFmt = [](const char* label, const char* fmt, auto v)
                            {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::Text(fmt, v);
                            };

                            auto RowArr_i16 = [](const char* label, const int16_t* aa, int n, int break_after)
                            {
                                std::string s;
                                s.reserve(n * 8);
                                char tmp[32];

                                for (int i = 0; i < n; i++)
                                {
                                    snprintf(tmp, sizeof(tmp), "%" PRIi16, aa[i]);

                                    if (i != 0)
                                    {
                                        if (break_after > 0 && i == break_after + 1) s.push_back('\n');
                                        else s.push_back(' ');
                                    }
                                    s += tmp;
                                }

                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted(s.c_str());
                            };

                            auto RowArr_u16 = [](const char* label, const uint16_t* aa, int n, int break_after)
                            {
                                std::string s;
                                s.reserve(n * 8);
                                char tmp[32];

                                for (int i = 0; i < n; i++)
                                {
                                    snprintf(tmp, sizeof(tmp), "%" PRIu16, aa[i]);

                                    if (i != 0)
                                    {
                                        if (break_after > 0 && i == break_after + 1) s.push_back('\n');
                                        else s.push_back(' ');
                                    }
                                    s += tmp;
                                }

                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted(s.c_str());
                            };

                            auto RowArr_u8 = [](const char* label, const uint8_t* aa, int n, int break_after)
                            {
                                std::string s;
                                s.reserve(n * 4);
                                char tmp[16];

                                for (int i = 0; i < n; i++)
                                {
                                    snprintf(tmp, sizeof(tmp), "%" PRIu8, aa[i]);

                                    if (i != 0)
                                    {
                                        if (break_after > 0 && i == break_after + 1) s.push_back('\n');
                                        else s.push_back(' ');
                                    }
                                    s += tmp;
                                }

                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted(s.c_str());
                            };

                            auto RowArr_u32 = [](const char* label, const uint32_t* aa, int n, int break_after)
                            {
                                std::string s;
                                s.reserve(n * 12);
                                char tmp[32];

                                for (int i = 0; i < n; i++)
                                {
                                    snprintf(tmp, sizeof(tmp), "%" PRIu32, aa[i]);

                                    if (i != 0)
                                    {
                                        if (break_after > 0 && i == break_after + 1) s.push_back('\n');
                                        else s.push_back(' ');
                                    }
                                    s += tmp;
                                }

                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted(s.c_str());
                            };

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::TextUnformatted("Type");
                            ImGui::TableSetColumnIndex(1);
                            ImGui::TextUnformatted("EPS P60 ACU GET TABLE HK");

                            HeaderRow("[Inputs]");
                            RowArr_i16("c_in[0..5]", pl.c_in, 6, 2);
                            RowArr_u16("v_in[0..5]", pl.v_in, 6, 2);

                            HeaderRow("[Bus/Temp]");
                            RowFmt("vbat", "%" PRIu16, pl.vbat);
                            RowFmt("vcc",  "%" PRIu16, pl.vcc);
                            RowArr_i16("temp[0..2]", pl.temp, 3, 0);

                            HeaderRow("[MPPT]");
                            RowFmt("mppt_mode", "%" PRIu8, pl.mppt_mode);
                            RowArr_u16("vboost[0..5]", pl.vboost, 6, 2);
                            RowArr_u16("power[0..5]",  pl.power,  6, 2);

                            HeaderRow("[DAC]");
                            RowArr_u8 ("dac_en[0..2]",  pl.dac_en,  3, 0);
                            RowArr_u16("dac_val[0..5]", pl.dac_val, 6, 2);

                            HeaderRow("[Boot/Time]");
                            RowFmt("bootcause", "%" PRIu32, pl.bootcause);
                            RowFmt("bootcnt",   "%" PRIu32, pl.bootcnt);
                            RowFmt("uptime",    "%" PRIu32, pl.uptime);
                            RowFmt("resetcause","0x%04X", (unsigned int)pl.resetcause);

                            HeaderRow("[MPPT Timing]");
                            RowFmt("mppt_time",   "%" PRIu16, pl.mppt_time);
                            RowFmt("mppt_period", "%" PRIu16, pl.mppt_period);

                            HeaderRow("[Device]");
                            RowArr_u8("device_type[0..7]",   pl.device_type, 8, 0);
                            RowArr_u8("device_status[0..7]", pl.device_status, 8, 0);

                            HeaderRow("[WDT]");
                            RowFmt("wdt_cnt_gnd",  "%" PRIu32, pl.wdt_cnt_gnd);
                            RowFmt("wdt_gnd_left", "%" PRIu32, pl.wdt_gnd_left);

                            break;
                        }
















                        case REPORT_KIND_EPS_P60_DOCK_GET_TABLE_HK:
                        {
                            auto &pl = view.u.eps_p60dockgettablehk;

                            auto HeaderRow = [](const char* title)
                            {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.80f, 0.80f, 1.0f));
                                ImGui::TextUnformatted(title);
                                ImGui::PopStyleColor();
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted("");
                            };

                            auto RowFmt = [](const char* label, const char* fmt, auto v)
                            {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::Text(fmt, v);
                            };

                            auto RowText = [](const char* label, const char* text)
                            {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted(text);
                            };

                            auto RowArr_i16 = [](const char* label, const int16_t* aa, int n, int break_after)
                            {
                                std::string s;
                                s.reserve(n * 8);
                                char tmp[32];

                                for (int i = 0; i < n; i++)
                                {
                                    snprintf(tmp, sizeof(tmp), "%" PRIi16, aa[i]);

                                    if (i != 0)
                                    {
                                        if (break_after > 0 && i == break_after + 1) s.push_back('\n');
                                        else s.push_back(' ');
                                    }
                                    s += tmp;
                                }

                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted(s.c_str());
                            };

                            auto RowArr_u16 = [](const char* label, const uint16_t* aa, int n, int break_after)
                            {
                                std::string s;
                                s.reserve(n * 8);
                                char tmp[32];

                                for (int i = 0; i < n; i++)
                                {
                                    snprintf(tmp, sizeof(tmp), "%" PRIu16, aa[i]);

                                    if (i != 0)
                                    {
                                        if (break_after > 0 && i == break_after + 1) s.push_back('\n');
                                        else s.push_back(' ');
                                    }
                                    s += tmp;
                                }

                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted(s.c_str());
                            };

                            auto RowArr_u8 = [](const char* label, const uint8_t* aa, int n, int break_after)
                            {
                                std::string s;
                                s.reserve(n * 4);
                                char tmp[16];

                                for (int i = 0; i < n; i++)
                                {
                                    snprintf(tmp, sizeof(tmp), "%" PRIu8, aa[i]);

                                    if (i != 0)
                                    {
                                        if (break_after > 0 && i == break_after + 1) s.push_back('\n');
                                        else s.push_back(' ');
                                    }
                                    s += tmp;
                                }

                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted(s.c_str());
                            };

                            auto RowArr_u32 = [](const char* label, const uint32_t* aa, int n, int break_after)
                            {
                                std::string s;
                                s.reserve(n * 12);
                                char tmp[32];

                                for (int i = 0; i < n; i++)
                                {
                                    snprintf(tmp, sizeof(tmp), "%" PRIu32, aa[i]);

                                    if (i != 0)
                                    {
                                        if (break_after > 0 && i == break_after + 1) s.push_back('\n');
                                        else s.push_back(' ');
                                    }
                                    s += tmp;
                                }

                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::TextUnformatted(label);
                                ImGui::TableSetColumnIndex(1);
                                ImGui::TextUnformatted(s.c_str());
                            };

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::TextUnformatted("Type");
                            ImGui::TableSetColumnIndex(1);
                            ImGui::TextUnformatted("EPS P60 DOCK GET TABLE HK");

                            HeaderRow("[Report Data]");
                            RowArr_i16("c_out[0..12]",  pl.c_out, 13, 6);
                            RowArr_u16("v_out[0..12]",  pl.v_out, 13, 6);
                            RowArr_u8 ("out_en[0..12]", pl.out_en, 13, 6);

                            HeaderRow("[Temps]");
                            RowArr_i16("temp[0..1]", pl.temp, 2, 0);

                            HeaderRow("[Boot/Time]");
                            RowFmt("bootcause", "%" PRIu32, pl.bootcause);
                            RowFmt("bootcnt",   "%" PRIu32, pl.bootcnt);
                            RowFmt("uptime",    "%" PRIu32, pl.uptime);
                            RowFmt("resetcause","0x%04X", (unsigned int)pl.resetcause);

                            HeaderRow("[Modes/Flags]");
                            RowFmt("batt_mode",  "%" PRIu8, pl.batt_mode);
                            RowFmt("heater_on",  "%" PRIu8, pl.heater_on);
                            RowFmt("conv_5v_en", "%" PRIu8, pl.conv_5v_en);

                            HeaderRow("[Latchups]");
                            RowArr_u16("latchup[0..12]", pl.latchup, 13, 6);

                            HeaderRow("[Battery/Power]");
                            RowFmt("vbat_v", "%" PRIu16, pl.vbat_v);
                            RowFmt("vcc_c",  "%" PRIi16, pl.vcc_c);
                            RowFmt("batt_c", "%" PRIi16, pl.batt_c);
                            RowFmt("batt_v", "%" PRIu16, pl.batt_v);
                            RowArr_i16("batt_temp[0..1]", pl.batt_temp, 2, 0);

                            HeaderRow("[Device]");
                            RowArr_u8("device_type[0..7]",   pl.device_type, 8, 0);
                            RowArr_u8("device_status[0..7]", pl.device_status, 8, 0);
                            RowFmt("dearm_status", "%" PRIu8, pl.dearm_status);

                            HeaderRow("[WDT Counters]");
                            RowFmt("wdt_cnt_gnd", "%" PRIu32, pl.wdt_cnt_gnd);
                            RowFmt("wdt_cnt_i2c", "%" PRIu32, pl.wdt_cnt_i2c);
                            RowFmt("wdt_cnt_can", "%" PRIu32, pl.wdt_cnt_can);
                            RowArr_u32("wdt_cnt_csp[0..1]", pl.wdt_cnt_csp, 2, 0);

                            HeaderRow("[WDT Left]");
                            RowFmt("wdt_gnd_left", "%" PRIu32, pl.wdt_gnd_left);
                            RowFmt("wdt_i2c_left", "%" PRIu32, pl.wdt_i2c_left);
                            RowFmt("wdt_can_left", "%" PRIu32, pl.wdt_can_left);
                            RowArr_u8("wdt_csp_left[0..1]", pl.wdt_csp_left, 2, 0);

                            HeaderRow("[Battery Currents]");
                            RowFmt("batt_chrg",    "%" PRIi16, pl.batt_chrg);
                            RowFmt("batt_dischrg", "%" PRIi16, pl.batt_dischrg);

                            HeaderRow("[Deploy]");
                            RowFmt("ant6_depl", "%" PRIi8, pl.ant6_depl);
                            RowFmt("ar6_depl",  "%" PRIi8, pl.ar6_depl);

                            break;
                        }





                    case REPORT_KIND_ADCS_LOG_MASK:
                    {
                        ImGui::TextUnformatted("ADCS Log Inclusion Mask");

                        auto &pl = view.u.adcs_logmask;

                        ImGui::Separator();
                        ImGui::Text("Fast Inclusion Bitmask:");
                        for (int i = 0; i < 5; i++){
                            ImGui::Text("  [%d] : 0x%02X (byte)", i, pl.FastInclusionBitmask[i]);
                            uint8_t b = pl.FastInclusionBitmask[i];
                            ImGui::Text("[%d] 0x%02X  |  b7 b6 b5 b4 b3 b2 b1 b0 = %d%d%d%d%d%d%d%d",
                                i, b,
                                (b>>7)&1, (b>>6)&1, (b>>5)&1, (b>>4)&1,
                                (b>>3)&1, (b>>2)&1, (b>>1)&1, (b>>0)&1 );
                        }


                        ImGui::Separator();
                        ImGui::Text("Slow Inclusion Bitmask:");
                        for (int i = 0; i < 5; i++){
                            ImGui::Text("  [%d] : 0x%02X (byte)", i, pl.SlowInclusionBitmask[i]);
                            uint8_t b = pl.SlowInclusionBitmask[i];
                            ImGui::Text("[%d] 0x%02X  |  b7 b6 b5 b4 b3 b2 b1 b0 = %d%d%d%d%d%d%d%d",
                                i, b,
                                (b>>7)&1, (b>>6)&1, (b>>5)&1, (b>>4)&1,
                                (b>>3)&1, (b>>2)&1, (b>>1)&1, (b>>0)&1 );
                        }


                        break;
                    }


                    case REPORT_KIND_ADCS_UNSOLICIT_TLM_SETUP_TLM:
                    {
                        auto &pl = view.u.adcs_unsolicited_tlm_tlm;

                        ImGui::TextUnformatted("ADCS Unsolicited TLM Message Setup (TLM)");

                        ImGui::Separator();
                        ImGui::Text("[Return Interval Settings]");
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted("UART Return Interval");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", pl.UARTTlmReturnInterval);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted("UART2 Return Interval");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", pl.UART2TlmReturnInterval);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted("CAN Return Interval");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", pl.CANTlmRetrunInterval);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted("Reserved");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", pl.Reserved);

                        ImGui::Separator();


                        
                        auto PrintBitmask = [&](const char *title, const uint8_t mask[5])
                        {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::TextUnformatted(title);
                            ImGui::TableSetColumnIndex(1);

                            float line_h = ImGui::GetTextLineHeightWithSpacing();
                            float h = line_h * (5 + 1); // 5줄 + 여유

                            ImGui::BeginChild(title, ImVec2(0, h), true, ImGuiWindowFlags_NoScrollbar);

                            for (int i = 0; i < 5; i++)
                            {
                                uint8_t b = mask[i];

                                char bits[9];
                                for (int k = 0; k < 8; k++)
                                    bits[k] = ((b >> (7 - k)) & 1) ? '1' : '0';
                                bits[8] = 0;

                                ImGui::Text("idx %d: 0x%02X | %s", i, b, bits);
                            }

                            ImGui::EndChild();
                        };


                        PrintBitmask("UART ID Inclusion Bitmask",  pl.UARTTlmIDInclusionBitmask);
                        PrintBitmask("UART2 ID Inclusion Bitmask", pl.UART2TlmIDInclusionBitmask);
                        PrintBitmask("CAN ID Inclusion Bitmask",   pl.CANTlmIDInclusionBitmask);

                        break;
                    }
/************************************************************************************************************************************************* */                
                case REPORT_KIND_ADCS_GET_ERROR_LOG_SETTING:
                {
                    auto &pl = view.u.adcs_errorlogsetting;

                    ImGui::TextUnformatted("ADCS Error Log Setting");

                    ImGui::Separator();
                    ImGui::Text("[Settings]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Active State");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.ActiveState);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Buffer Full Action");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u (%s)", pl.BufferFullAction);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Reserved");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.Reserved);

                    break;
                }

                case REPORT_KIND_ADCS_GET_CURRENT_UNIX_TIME:
                {
                    auto &pl = view.u.adcs_currentunixtime;

                    ImGui::TextUnformatted("ADCS Current Unix Time");

                    ImGui::Separator();
                    ImGui::Text("[Time]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Current Unix Seconds [s]");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.CurrentUnixseconds);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Current Unix Nanoseconds [ns]");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.CurrentUnixNanoseconds);

                    // UTC 변환 표시
                    time_t t = static_cast<time_t>(pl.CurrentUnixseconds);
                    struct tm tm_utc;
                    gmtime_r(&t, &tm_utc);
                    char utc_str[64];
                    strftime(utc_str, sizeof(utc_str), "%Y-%m-%d %H:%M:%S UTC", &tm_utc);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("UTC Time");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s.%09u", utc_str, pl.CurrentUnixNanoseconds);

                    break;
                }

                case REPORT_KIND_ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC:
                {
                    auto &pl = view.u.adcs_persistconfigdiagnostic;

                    ImGui::TextUnformatted("ADCS Persist Config Diagnostic");

                    ImGui::Separator();
                    ImGui::Text("[Diagnostic]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("State");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.State);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Last Result");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.LastResult);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Timestamp [s]");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.Timestamp);

                    break;
                }

                case REPORT_KIND_ADCS_GET_COMMUNICATION_STATUS:
                {
                    auto &pl = view.u.adcs_communicationstatus;

                    ImGui::TextUnformatted("ADCS Communication Status");

                    ImGui::Separator();
                    ImGui::Text("[UART]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("UART TC Count");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.UART_TcCnt);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("UART TLM Count");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.UART_TlmCnt);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("UART SW Error");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.UART_ErrSW);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("UART HW Error");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.UART_ErrHW);

                    ImGui::Separator();
                    ImGui::Text("[UART2]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("UART2 TC Count");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.UART2_TcCnt);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("UART2 TLM Count");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.UART2_TlmCnt);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("UART2 SW Error");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.UART2_ErrSW);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("UART2 HW Error");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.UART2_ErrHW);

                    ImGui::Separator();
                    ImGui::Text("[CAN]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("CAN TC Count");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.CAN_TcCnt);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("CAN TLM Count");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.CAN_TlmCnt);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("CAN SW Error");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.CAN_ErrSW);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("CAN HW Error");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.CAN_ErrHW);

                    ImGui::Separator();
                    ImGui::Text("[I2C]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("I2C TC Count");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.I2C_TcCnt);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("I2C TLM Count");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.I2C_TlmCnt);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("I2C SW Error");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.I2C_ErrSW);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("I2C HW Error");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.I2C_ErrHW);

                    break;
                }

                case REPORT_KIND_ADCS_GET_CONTROL_ESTIMATION_MODE:
                {
                    auto &pl = view.u.adcs_controlestimationmode;

                    ImGui::TextUnformatted("ADCS Control & Estimation Mode");

                    ImGui::Separator();
                    ImGui::Text("[Mode]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Control Mode");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.ControlMode);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Main Estimator Mode");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.MainEstimatorMode);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Backup Estimator Mode");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.BackupEstimatorMode);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Control Timeout [s]");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.ControlTimeout);

                    break;
                }

                case REPORT_KIND_ADCS_GET_REFERENCE_IRC_VECTOR:
                {
                    auto &pl = view.u.adcs_referenceircvector;

                    ImGui::TextUnformatted("ADCS Reference IRC Vector (ECI)");

                    ImGui::Separator();
                    ImGui::Text("[ECI Pointing Vector]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("ECI Pointing Vector X");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.6f", pl.ECIPointingVectorX);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("ECI Pointing Vector Y");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.6f", pl.ECIPointingVectorY);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("ECI Pointing Vector Z");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.6f", pl.ECIPointingVectorZ);

                    break;
                }

                case REPORT_KIND_ADCS_GET_REFERENCE_LLH_TARGET:
                {
                    auto &pl = view.u.adcs_referencellhtarget;

                    ImGui::TextUnformatted("ADCS Reference LLH Target");

                    ImGui::Separator();
                    ImGui::Text("[LLH Target]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Latitude");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.6f", pl.Latitude);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Longitude");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.6f", pl.Longitude);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Altitude");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.3f", pl.Altitude);

                    break;
                }

                case REPORT_KIND_ADCS_GET_ORBIT_MODE :
                {
                    auto &pl = view.u.adcs_orbitmode;

                    ImGui::TextUnformatted("ADCS Orbit Mode");

                    ImGui::Separator();
                    ImGui::Text("[Otbit Mode]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("OrbitMode");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.OrbitMode);

                    break;
                }

                case REPORT_KIND_ADCS_GET_HEALTH_TLM_MMT:
                {
                    auto &pl = view.u.adcs_healthtlmmmt;

                    ImGui::TextUnformatted("ADCS Health TLM MMT");

                    // 일반 행 헬퍼
                    auto row_i16 = [](const char* label, int16_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%d", v);
                    };
                    auto row_u16 = [](const char* label, uint16_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };
                    auto row_u32 = [](const char* label, uint32_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };

                    auto row_flag = [](const char* label, uint8_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };

                    ImGui::Separator();
                    ImGui::Text("[Mag0 MCU]");
                    row_i16("Mag0 MCU Temperature", pl.Mag0MCUTemperature);
                    row_u16("Mag0 MCU Current",       pl.Mag0MCUCurrent);
                    row_u16("Mag0 MCU Voltage",       pl.Mag0MCUVoltage);

                    ImGui::Separator();
                    ImGui::Text("[Mag0 Sensor]");
                    row_i16("Mag0 Primary Temperature",   pl.Mag0PrimaryTemperature);
                    row_i16("Mag0 Redundant Temperature", pl.Mag0RedundantTemperature);

                    ImGui::Separator();
                    ImGui::Text("[Mag0 Burn]");
                    row_u32 ("Mag0 Burn Current",  pl.Mag0BurnCurrent);
                    row_flag("Mag0 Deploy Pin State",   pl.Mag0DeployPinState);
                    row_flag("Mag0 Burn Pin State",     pl.Mag0BurnPinState);
                    row_flag("Mag0 Burn Under Current", pl.Mag0BurnUnderCurrent);
                    row_flag("Mag0 Burn Over Current",  pl.Mag0BurnOverCurrent);
                    row_flag("Mag0 Deploy Timeout",     pl.Mag0DeployTimeout);

                    ImGui::Separator();
                    ImGui::Text("[Mag1 MCU]");
                    row_i16("Mag1 MCU Temperature", pl.Mag1MCUTemperature);
                    row_u16("Mag1 MCU Current",       pl.Mag1MCUCurrent);
                    row_u16("Mag1 MCU Voltage",       pl.Mag1MCUVoltage);

                    ImGui::Separator();
                    ImGui::Text("[Mag1 Sensor]");
                    row_i16("Mag1 Primary Temperature",   pl.Mag1PrimaryTemperature);
                    row_i16("Mag1 Redundant Temperature", pl.Mag1RedundantTemperature);

                    ImGui::Separator();
                    ImGui::Text("[Mag1 Burn]");
                    row_u32 ("Mag1 Burn Current",  pl.Mag1BurnCurrent);
                    row_flag("Mag1 Deploy Pin State",   pl.Mag1DeployPinState);
                    row_flag("Mag1 Burn Pin State",     pl.Mag1BurnPinState);
                    row_flag("Mag1 Burn Under Current", pl.Mag1BurnUnderCurrent);
                    row_flag("Mag1 Burn Over Current",  pl.Mag1BurnOverCurrent);
                    row_flag("Mag1 Deploy Timeout",     pl.Mag1DeployTimeout);

                    break;
                }

                case REPORT_KIND_ADCS_GET_RAW_CUBESENSE_SUN:
                {
                    auto &pl = view.u.adcs_rawcubesensesun;

                    ImGui::TextUnformatted("ADCS Raw CubeSense Sun");

                    // 헬퍼 람다들
                    auto row_i16 = [](const char* label, int16_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%d", v);
                    };
                    auto row_u8 = [](const char* label, uint8_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };
                    auto row_u32 = [](const char* label, uint32_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };

                    ImGui::Separator();
                    ImGui::Text("[Timestamp]");
                    row_u32("Time Second [s]",      pl.TimeSecond);
                    row_u32("Time NanoSecond [ns]", pl.TimeNanoSecond);

                    ImGui::Separator();
                    ImGui::Text("[FSS0]");
                    row_i16("FSS0 Alpha Angle",       pl.FSS0AlphaAngle);
                    row_i16("FSS0 Beta Angle",        pl.FSS0BetaAngle);
                    row_u8 ("FSS0 Capture Result",    pl.FSS0CaptureResult);
                    row_u8 ("FSS0 Detection Result",  pl.FSS0DetectionResult);

                    ImGui::Separator();
                    ImGui::Text("[FSS1]");
                    row_i16("FSS1 Alpha Angle",       pl.FSS1AlphaAngle);
                    row_i16("FSS1 Beta Angle",        pl.FSS1BetaAngle);
                    row_u8 ("FSS1 Capture Result",    pl.FSS1CaptureResult);
                    row_u8 ("FSS1 Detection Result",  pl.FSS1DetectionResult);

                    ImGui::Separator();
                    ImGui::Text("[FSS2]");
                    row_i16("FSS2 Alpha Angle",       pl.FSS2AlphaAngle);
                    row_i16("FSS2 Beta Angle",        pl.FSS2BetaAngle);
                    row_u8 ("FSS2 Capture Result",    pl.FSS2CaptureResult);
                    row_u8 ("FSS2 Detection Result",  pl.FSS2DetectionResult);

                    ImGui::Separator();
                    ImGui::Text("[FSS3]");
                    row_i16("FSS3 Alpha Angle",       pl.FSS3AlphaAngle);
                    row_i16("FSS3 Beta Angle",        pl.FSS3BetaAngle);
                    row_u8 ("FSS3 Capture Result",    pl.FSS3CaptureResult);
                    row_u8 ("FSS3 Detection Result",  pl.FSS3DetectionResult);

                    ImGui::Separator();
                    ImGui::Text("[Valid]");
                    row_u8("Valid Result", pl.ValidResult);

                    break;
                }
                
                case REPORT_KIND_ADCS_GET_REFERENCE_RPY_VALUES:
                {
                    auto &pl = view.u.adcs_referencerpyvalues;

                    ImGui::TextUnformatted("ADCS Reference RPY Values");

                    ImGui::Separator();
                    ImGui::Text("[Reference RPY]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Roll");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.6f", pl.Roll);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Pitch");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.6f", pl.Pitch);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Yaw");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%.6f", pl.Yaw);

                    break;
                }

                case REPORT_KIND_ADCS_GET_OPENLOOPCMD_MTQ:
                {
                    auto &pl = view.u.adcs_openloopcmdmtq;

                    ImGui::TextUnformatted("ADCS Open Loop Command MTQ");

                    ImGui::Separator();
                    ImGui::Text("[MTQ Command]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("MTQ0 Open Loop Cmd");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", pl.MTQ0_OpenLoopCmd);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("MTQ1 Open Loop Cmd");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", pl.MTQ1_OpenLoopCmd);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("MTQ2 Open Loop Cmd");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", pl.MTQ2_OpenLoopCmd);

                    break;
                }

                case REPORT_KIND_ADCS_GET_POWER_STATE:
                {
                    auto &pl = view.u.adcs_powerstate;

                    ImGui::TextUnformatted("ADCS Power State");

                    auto row_u8 = [](const char* label, uint8_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };

                    ImGui::Separator();
                    ImGui::Text("[Reaction Wheels]");
                    row_u8("RWL0", pl.RWL0);
                    row_u8("RWL1", pl.RWL1);
                    row_u8("RWL2", pl.RWL2);
                    row_u8("RWL3", pl.RWL3);

                    ImGui::Separator();
                    ImGui::Text("[Magnetometers]");
                    row_u8("MAG0", pl.MAG0);
                    row_u8("MAG1", pl.MAG1);

                    ImGui::Separator();
                    ImGui::Text("[Gyroscopes]");
                    row_u8("GYR0", pl.GYR0);
                    row_u8("GYR1", pl.GYR1);

                    ImGui::Separator();
                    ImGui::Text("[Fine Sun Sensors]");
                    row_u8("FSS0", pl.FSS0);
                    row_u8("FSS1", pl.FSS1);
                    row_u8("FSS2", pl.FSS2);
                    row_u8("FSS3", pl.FSS3);

                    ImGui::Separator();
                    ImGui::Text("[Horizon Sun Sensors]");
                    row_u8("HSS0", pl.HSS0);
                    row_u8("HSS1", pl.HSS1);

                    ImGui::Separator();
                    ImGui::Text("[Star Trackers]");
                    row_u8("STR0", pl.STR0);
                    row_u8("STR1", pl.STR1);

                    ImGui::Separator();
                    ImGui::Text("[External Sensors]");
                    row_u8("Ext Sensor 0", pl.ExtSensor0);
                    row_u8("Ext Sensor 1", pl.ExtSensor1);
                    row_u8("Ext GYR 0",    pl.ExtGYR0);
                    row_u8("Ext GYR 1",    pl.ExtGYR1);

                    break;
                }

                case REPORT_KIND_ADCS_GET_RUN_MODE:
                {
                    auto &pl = view.u.adcs_runmode;

                    ImGui::TextUnformatted("ADCS Run Mode");

                    ImGui::Separator();
                    ImGui::Text("[Run Mode]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Run Mode");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.RunMode);

                    break;
                }

                case REPORT_KIND_ADCS_GET_CONTROL_MODE:
                {
                    auto &pl = view.u.adcs_controlmode;

                    ImGui::TextUnformatted("ADCS Control Mode");

                    ImGui::Separator();
                    ImGui::Text("[Control Mode]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Control Mode");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.ControlMode);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Control Timeout");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.ControlTimeout);

                    break;
                }

                case REPORT_KIND_ADCS_GET_MAG0_MMT_CALIB_CONFIG:
                {
                    auto &pl = view.u.adcs_mag0mmtcalibconfig;

                    ImGui::TextUnformatted("ADCS Mag0 MMT Calibration Config");

                    auto row_i16 = [](const char* label, int16_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%d", v);
                    };

                    ImGui::Separator();
                    ImGui::Text("[Channel Offsets]");
                    row_i16("MMT Ch1 Offset", pl.MMT_Ch1Offset);
                    row_i16("MMT Ch2 Offset", pl.MMT_Ch2Offset);
                    row_i16("MMT Ch3 Offset", pl.MMT_Ch3Offset);

                    ImGui::Separator();
                    ImGui::Text("[Sensitivity Matrix - Diagonal]");
                    row_i16("MMT Sensitivity S11", pl.MMT_SensitivityMAT_S11);
                    row_i16("MMT Sensitivity S22", pl.MMT_SensitivityMAT_S22);
                    row_i16("MMT Sensitivity S33", pl.MMT_SensitivityMAT_S33);

                    ImGui::Separator();
                    ImGui::Text("[Sensitivity Matrix - Off-Diagonal]");
                    row_i16("MMT Sensitivity S12", pl.MMT_SensitivityMAT_S12);
                    row_i16("MMT Sensitivity S13", pl.MMT_SensitivityMAT_S13);
                    row_i16("MMT Sensitivity S21", pl.MMT_SensitivityMAT_S21);
                    row_i16("MMT Sensitivity S23", pl.MMT_SensitivityMAT_S23);
                    row_i16("MMT Sensitivity S31", pl.MMT_SensitivityMAT_S31);
                    row_i16("MMT Sensitivity S32", pl.MMT_SensitivityMAT_S32);

                    break;
                }

                case REPORT_KIND_ADCS_GET_MAG1_MMT_CALIB_CONFIG:
                {
                    auto &pl = view.u.adcs_mag1mmtcalibconfig;

                    ImGui::TextUnformatted("ADCS Mag1 MMT Calibration Config");

                    auto row_i16 = [](const char* label, int16_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%d", v);
                    };

                    ImGui::Separator();
                    ImGui::Text("[Channel Offsets]");
                    row_i16("MMT Ch1 Offset", pl.MMT_Ch1Offset);
                    row_i16("MMT Ch2 Offset", pl.MMT_Ch2Offset);
                    row_i16("MMT Ch3 Offset", pl.MMT_Ch3Offset);

                    ImGui::Separator();
                    ImGui::Text("[Sensitivity Matrix - Diagonal]");
                    row_i16("MMT Sensitivity S11", pl.MMT_SensitivityMAT_S11);
                    row_i16("MMT Sensitivity S22", pl.MMT_SensitivityMAT_S22);
                    row_i16("MMT Sensitivity S33", pl.MMT_SensitivityMAT_S33);

                    ImGui::Separator();
                    ImGui::Text("[Sensitivity Matrix - Off-Diagonal]");
                    row_i16("MMT Sensitivity S12", pl.MMT_SensitivityMAT_S12);
                    row_i16("MMT Sensitivity S13", pl.MMT_SensitivityMAT_S13);
                    row_i16("MMT Sensitivity S21", pl.MMT_SensitivityMAT_S21);
                    row_i16("MMT Sensitivity S23", pl.MMT_SensitivityMAT_S23);
                    row_i16("MMT Sensitivity S31", pl.MMT_SensitivityMAT_S31);
                    row_i16("MMT Sensitivity S32", pl.MMT_SensitivityMAT_S32);

                    break;
                }

                case REPORT_KIND_ADCS_GET_ESTIMATION_MODE:
                {
                    auto &pl = view.u.adcs_estimationmode;

                    ImGui::TextUnformatted("ADCS Estimation Mode");

                    ImGui::Separator();
                    ImGui::Text("[Mode]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Main Estimator Mode");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.MainEstimatorMode);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Backup Estimator Mode");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.BackupEstimatorMode);

                    break;
                }

                case REPORT_KIND_ADCS_GET_OPERATIONAL_STATE:
                {
                    auto &pl = view.u.adcs_operationalstate;

                    ImGui::TextUnformatted("ADCS Operational State");

                    ImGui::Separator();
                    ImGui::Text("[State]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Operational Mode");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.OperationalMode);

                    break;
                }

                case REPORT_KIND_ADCS_GET_RAW_CSS_SENSOR:
                {
                    auto &pl = view.u.adcs_rawcsssensor;

                    ImGui::TextUnformatted("ADCS Raw CSS Sensor");

                    auto row_u8 = [](const char* label, uint8_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };
                    auto row_u32 = [](const char* label, uint32_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };

                    ImGui::Separator();
                    ImGui::Text("[Timestamp]");
                    row_u32("Time Seconds",     pl.TimeSeconds);
                    row_u32("Time NanoSeconds", pl.TimeNanoSeconds);

                    ImGui::Separator();
                    ImGui::Text("[CSS Raw Values]");
                    row_u8("CSS0", pl.CSS0);
                    row_u8("CSS1", pl.CSS1);
                    row_u8("CSS2", pl.CSS2);
                    row_u8("CSS3", pl.CSS3);
                    row_u8("CSS4", pl.CSS4);
                    row_u8("CSS5", pl.CSS5);
                    row_u8("CSS6", pl.CSS6);
                    row_u8("CSS7", pl.CSS7);
                    row_u8("CSS8", pl.CSS8);
                    row_u8("CSS9", pl.CSS9);

                    ImGui::Separator();
                    ImGui::Text("[Valid]");
                    row_u8("CSS Valid Flag", pl.CSSValidFlag);

                    break;
                }

                case REPORT_KIND_ADCS_GET_RAW_GYR_SENSOR:
                {
                    auto &pl = view.u.adcs_rawgyrsensor;

                    ImGui::TextUnformatted("ADCS Raw GYR Sensor");

                    auto row_u8 = [](const char* label, uint8_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };
                    auto row_u32 = [](const char* label, uint32_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };
                    auto row_f32 = [](const char* label, float v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.6f", v);
                    };

                    ImGui::Separator();
                    ImGui::Text("[Timestamp]");
                    row_u32("Time Seconds",     pl.TimeSeconds);
                    row_u32("Time NanoSeconds", pl.TimeNanoSeconds);

                    ImGui::Separator();
                    ImGui::Text("[GYR0 Raw Rate]");
                    row_f32("GYR0 Raw Rate X", pl.GYR0RawRateX);
                    row_f32("GYR0 Raw Rate Y", pl.GYR0RawRateY);
                    row_f32("GYR0 Raw Rate Z", pl.GYR0RawRateZ);

                    ImGui::Separator();
                    ImGui::Text("[GYR1 Raw Rate]");
                    row_f32("GYR1 Raw Rate X", pl.GYR1RawRateX);
                    row_f32("GYR1 Raw Rate Y", pl.GYR1RawRateY);
                    row_f32("GYR1 Raw Rate Z", pl.GYR1RawRateZ);

                    ImGui::Separator();
                    ImGui::Text("[Valid]");
                    row_u8("GYR0 Valid Flag", pl.GYR0ValidFlag);
                    row_u8("GYR1 Valid Flag", pl.GYR1ValidFlag);

                    break;
                }

                case REPORT_KIND_ADCS_GET_CALIBRATED_GYR_SENSOR:
                {
                    auto &pl = view.u.adcs_calibratedgyrsensor;

                    ImGui::TextUnformatted("ADCS Calibrated GYR Sensor");

                    auto row_u8 = [](const char* label, uint8_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };
                    auto row_u32 = [](const char* label, uint32_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };
                    auto row_f32 = [](const char* label, float v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.6f", v);
                    };

                    ImGui::Separator();
                    ImGui::Text("[Timestamp]");
                    row_u32("Time Seconds",     pl.TimeSeconds);
                    row_u32("Time NanoSeconds", pl.TimeNanoSeconds);

                    ImGui::Separator();
                    ImGui::Text("[GYR0 Calibrated Rate]");
                    row_f32("GYR0 Calibrated Rate X", pl.GYR0CalibratedRateX);
                    row_f32("GYR0 Calibrated Rate Y", pl.GYR0CalibratedRateY);
                    row_f32("GYR0 Calibrated Rate Z", pl.GYR0CalibratedRateZ);

                    ImGui::Separator();
                    ImGui::Text("[GYR1 Calibrated Rate]");
                    row_f32("GYR1 Calibrated Rate X", pl.GYR1CalibratedRateX);
                    row_f32("GYR1 Calibrated Rate Y", pl.GYR1CalibratedRateY);
                    row_f32("GYR1 Calibrated Rate Z", pl.GYR1CalibratedRateZ);

                    ImGui::Separator();
                    ImGui::Text("[External GYR0 Calibrated Rate]");
                    row_f32("Ext GYR0 Calibrated Rate X", pl.ExtGYR0CalibratedRateX);
                    row_f32("Ext GYR0 Calibrated Rate Y", pl.ExtGYR0CalibratedRateY);
                    row_f32("Ext GYR0 Calibrated Rate Z", pl.ExtGYR0CalibratedRateZ);

                    ImGui::Separator();
                    ImGui::Text("[External GYR1 Calibrated Rate]");
                    row_f32("Ext GYR1 Calibrated Rate X", pl.ExtGYR1CalibratedRateX);
                    row_f32("Ext GYR1 Calibrated Rate Y", pl.ExtGYR1CalibratedRateY);
                    row_f32("Ext GYR1 Calibrated Rate Z", pl.ExtGYR1CalibratedRateZ);

                    ImGui::Separator();
                    ImGui::Text("[Valid]");
                    row_u8("GYR0 Valid Flag",     pl.GYR0ValidFlag);
                    row_u8("GYR1 Valid Flag",     pl.GYR1ValidFlag);
                    row_u8("Ext GYR0 Valid Flag", pl.EXTGYR0ValidFlag);
                    row_u8("Ext GYR1 Valid Flag", pl.EXTGYR1ValidFlag);

                    break;
                }

                case REPORT_KIND_ADCS_GET_MAG_SENSING_ELM_CONFIG:
                {
                    auto &pl = view.u.adcs_magsensingelmconfig;

                    ImGui::TextUnformatted("ADCS Mag Sensing Element Config");

                    ImGui::Separator();
                    ImGui::Text("[Sensing Element]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Mag0 Sensing Element");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.Mag0SensingElement);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Mag1 Sensing Element");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.Mag1SensingElement);

                    break;
                }

                case REPORT_KIND_ADCS_GET_TLM_LOG_INCLMASK:
                {
                    auto &pl = view.u.adcs_tlmloginclmask;

                    ImGui::TextUnformatted("ADCS TLM Log Inclusion Mask");

                    ImGui::Separator();
                    ImGui::Text("[Fast Inclusion Bitmask]");
                    for (int i = 0; i < 5; ++i) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Fast Inclusion Bitmask[%d]", i);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("0x%02X", pl.FastInclusionBitmask[i]);
                    }

                    ImGui::Separator();
                    ImGui::Text("[Slow Inclusion Bitmask]");
                    for (int i = 0; i < 5; ++i) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Slow Inclusion Bitmask[%d]", i);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("0x%02X", pl.SlowInclusionBitmask[i]);
                    }

                    break;
                }

                case REPORT_KIND_ADCS_GET_UNSOLICIT_TLM_MSG_SETUP:
                {
                    auto &pl = view.u.adcs_unsolicittlmmsgsetup;

                    ImGui::TextUnformatted("ADCS Unsolicited TLM Msg Setup");

                    ImGui::Separator();
                    ImGui::Text("[Return Intervals]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("UART TLM Return Interval");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.UARTTlmReturnInterval);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("UART2 TLM Return Interval");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.UART2TlmReturnInterval);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("CAN TLM Return Interval");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.CANTlmRetrunInterval);

                    ImGui::Separator();
                    ImGui::Text("[UART TLM ID Inclusion Bitmask]");
                    for (int i = 0; i < 5; ++i) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("UART TLM ID Bitmask[%d]", i);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("0x%02X", pl.UARTTlmIDInclusionBitmask[i]);
                    }

                    ImGui::Separator();
                    ImGui::Text("[UART2 TLM ID Inclusion Bitmask]");
                    for (int i = 0; i < 5; ++i) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("UART2 TLM ID Bitmask[%d]", i);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("0x%02X", pl.UART2TlmIDInclusionBitmask[i]);
                    }

                    ImGui::Separator();
                    ImGui::Text("[CAN TLM ID Inclusion Bitmask]");
                    for (int i = 0; i < 5; ++i) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("CAN TLM ID Bitmask[%d]", i);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("0x%02X", pl.CANTlmIDInclusionBitmask[i]);
                    }

                    break;
                }

                case REPORT_KIND_ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP:
                {
                    auto &pl = view.u.adcs_unsoliciteventmsgsetup;

                    ImGui::TextUnformatted("ADCS Unsolicited Event Msg Setup");

                    auto row_u8 = [](const char* label, uint8_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };

                    ImGui::Separator();
                    ImGui::Text("[UART Event Filter]");
                    row_u8("Info UART",     pl.InfoUART);
                    row_u8("Minor UART",    pl.MinorUART);
                    row_u8("Major UART",    pl.MajorUART);
                    row_u8("Critical UART", pl.CriticalUART);

                    ImGui::Separator();
                    ImGui::Text("[UART2 Event Filter]");
                    row_u8("Info UART2",     pl.InfoUART2);
                    row_u8("Minor UART2",    pl.MinorUART2);
                    row_u8("Major UART2",    pl.MajorUART2);
                    row_u8("Critical UART2", pl.CriticalUART2);

                    ImGui::Separator();
                    ImGui::Text("[CAN Event Filter]");
                    row_u8("Info CAN",     pl.InfoCAN);
                    row_u8("Minor CAN",    pl.MinorCAN);
                    row_u8("Major CAN",    pl.MajorCAN);
                    row_u8("Critical CAN", pl.CriticalCAN);

                    break;
                }

                case REPORT_KIND_ADCS_GET_EVENT_LOG_STATUS_RESPONSE:
                {
                    auto &pl = view.u.adcs_eventlogstatusresponse;

                    ImGui::TextUnformatted("ADCS Event Log Status Response");

                    auto row_u8 = [](const char* label, uint8_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };
                    auto row_u16 = [](const char* label, uint16_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };
                    auto row_u32 = [](const char* label, uint32_t v) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted(label);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", v);
                    };

                    ImGui::Separator();
                    ImGui::Text("[Entry Counts]");
                    row_u16("Num Queued Entry",   pl.NumQueuedEntry);
                    row_u16("Num Buffered Entry", pl.NumBufferedEntry);
                    row_u32("Num Entry",          pl.NumEntry);
                    row_u32("Num Empty Entry",    pl.NumEmptyEntry);

                    ImGui::Separator();
                    ImGui::Text("[Entry Timestamps]");
                    row_u32("Old Entry Unix Time",  pl.OldEntryUnixTime);
                    row_u32("Last Entry Unix Time", pl.LastEntryUnixTime);

                    ImGui::Separator();
                    ImGui::Text("[Event Severity Counts]");
                    row_u32("Num Critical EVS",      pl.NumCriticalEVS);
                    row_u32("Num Major Warning EVS", pl.NumMajorWarningEVS);
                    row_u32("Num Minor Warning EVS", pl.NumMinorWarningEVS);
                    row_u32("Num Info EVS",          pl.NumInfoEVS);

                    ImGui::Separator();
                    ImGui::Text("[State]");
                    row_u32("Write Count",     pl.WriteCnt);
                    row_u8 ("Read Que State", pl.ReadQueState);

                    break;
                }

                case REPORT_KIND_ADCS_GET_PORTMAP:
                {
                    auto &pl = view.u.adcs_portmap;

                    ImGui::TextUnformatted("ADCS Port Map");

                    // 노드 정보 표시 헬퍼 (4개 필드를 한 그룹으로)
                    auto node_group = [&](const char* group_label,
                                          uint8_t node_type, uint8_t abst_node_type,
                                          uint32_t serial_num, uint32_t address) {
                        ImGui::Separator();
                        ImGui::Text("[%s]", group_label);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted("Node Type");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", node_type);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted("Abstract Node Type");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", abst_node_type);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted("Serial Number");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%u", serial_num);

                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextUnformatted("Address");
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("0x%08X", address);
                    };

                    node_group("Sensor1", pl.NodeType_Sensor1, pl.AbstNodeType_Sensor1, pl.SerialNum_Sensor1, pl.Address_Sensor1);
                    node_group("Sensor2", pl.NodeType_Sensor2, pl.AbstNodeType_Sensor2, pl.SerialNum_Sensor2, pl.Address_Sensor2);
                    node_group("Sensor3", pl.NodeType_Sensor3, pl.AbstNodeType_Sensor3, pl.SerialNum_Sensor3, pl.Address_Sensor3);
                    node_group("Sensor4", pl.NodeType_Sensor4, pl.AbstNodeType_Sensor4, pl.SerialNum_Sensor4, pl.Address_Sensor4);
                    node_group("Sensor5", pl.NodeType_Sensor5, pl.AbstNodeType_Sensor5, pl.SerialNum_Sensor5, pl.Address_Sensor5);
                    node_group("Sensor6", pl.NodeType_Sensor6, pl.AbstNodeType_Sensor6, pl.SerialNum_Sensor6, pl.Address_Sensor6);
                    node_group("Sensor7", pl.NodeType_Sensor7, pl.AbstNodeType_Sensor7, pl.SerialNum_Sensor7, pl.Address_Sensor7);
                    node_group("Sensor8", pl.NodeType_Sensor8, pl.AbstNodeType_Sensor8, pl.SerialNum_Sensor8, pl.Address_Sensor8);

                    node_group("Wheel1", pl.NodeType_Wheel1, pl.AbstNodeType_Wheel1, pl.SerialNum_Wheel1, pl.Address_Wheel1);
                    node_group("Wheel2", pl.NodeType_Wheel2, pl.AbstNodeType_Wheel2, pl.SerialNum_Wheel2, pl.Address_Wheel2);
                    node_group("Wheel3", pl.NodeType_Wheel3, pl.AbstNodeType_Wheel3, pl.SerialNum_Wheel3, pl.Address_Wheel3);
                    node_group("Wheel4", pl.NodeType_Wheel4, pl.AbstNodeType_Wheel4, pl.SerialNum_Wheel4, pl.Address_Wheel4);

                    break;
                }

/***************************************************************************************************************************************************** */
                case REPORT_KIND_UANT_GET_STATUS_TLM:
                {
                    auto &pl = view.u.uant_getstatus;

                    ImGui::TextUnformatted("UANT ANT6 Release Status");

                    ImGui::Separator();
                    ImGui::Text("[Channel 0]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Channel 0 State");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u (%s)", pl.channel_0_state,
                                 pl.channel_0_state ? "Burning" : "Idle");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Channel 0 Status");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u (%s)", pl.channel_0_status,
                                 pl.channel_0_status ? "Released" : "Not released");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Channel 0 Burn Time Left [s]");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.channel_0_burn_time_left);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Channel 0 Burn Tries");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.channel_0_burn_tries);

                    ImGui::Separator();
                    ImGui::Text("[Channel 1]");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Channel 1 State");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u (%s)", pl.channel_1_state,
                                 pl.channel_1_state ? "Burning" : "Idle");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Channel 1 Status");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u (%s)", pl.channel_1_status,
                                 pl.channel_1_status ? "Released" : "Not released");

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Channel 1 Burn Time Left [s]");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.channel_1_burn_time_left);

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Channel 1 Burn Tries");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", pl.channel_1_burn_tries);

                    break;
                }

                /***************************************5차 추가************************************* */
                // 1. PAYUEL_ROMA Report
                case REPORT_KIND_PAYUEL_ROMA_NOOP: // CC 0, 1
                {
                    auto &pl = view.u.roma_noop;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("PAYUEL ROMA NOOP (CC 0)");

                    HeaderRow("[Counters Information]");
                    RowFmt("Command Counter", "%" PRIu8, pl.CmdCounter);
                    RowFmt("Error Counter",   "%" PRIu8, pl.ErrCounter);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_RESETCOUNTERS :
                {
                    auto &pl = view.u.roma_resetcounters;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("PAYUEL ROMA RESET COUNTERS (CC 1)");

                    HeaderRow("[Counters Information]");
                    RowFmt("Command Counter", "%" PRIu8, pl.CmdCounter);
                    RowFmt("Error Counter",   "%" PRIu8, pl.ErrCounter);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_COMMTEST: // CC 2
                {
                    auto &pl = view.u.roma_commtest;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("PAYUEL ROMA COMM TEST (CC 2)");

                    HeaderRow("[Comm Test Result]");
                    RowArr_Hex("RxBuf [16 Bytes]", pl.RxBuf, 16, 8);

                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_GETSPECIFICLINE: // CC 6, 8
                {
                    auto &pl = view.u.roma_getspecificline;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("PAYUEL ROMA GET SPECIFIC LINE (CC 6)");

                    HeaderRow("[Line Information]");
                    RowFmt("Line Number", "%" PRIu16, pl.line_no);
                    RowFmt("Info",        "%" PRIu8,  pl.info);
                    RowFmt("Length (len)","%" PRIu8,  pl.len);

                    HeaderRow("[Time & Increment]");
                    RowFmt("Time (Seconds)", "%" PRIu32, pl.time_s);
                    RowFmt("Time (ms)",      "%" PRIu16, pl.time_ms);
                    RowFmt("Increment",      "%" PRIu16, pl.incr);

                    HeaderRow("[Message]");
                    RowFmt("Line Data", "%s", pl.line);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_GETMULTIPLELINES: // CC 7, 9
                {
                    auto &pl = view.u.roma_getmultiplelines;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("PAYUEL ROMA GET MULTIPLE LINES (CC 7)");

                    HeaderRow("[Log Accumulator Data]");
                    RowFmt("Buffer Data", "%s", (const char*)pl.buffer);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_GETLATESTLINE: // CC 6, 8
                {
                    auto &pl = view.u.roma_getlatestline;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("PAYUEL ROMA GET LATEST LINE (CC 8)");

                    HeaderRow("[Line Information]");
                    RowFmt("Line Number", "%" PRIu16, pl.line_no);
                    RowFmt("Info",        "%" PRIu8,  pl.info);
                    RowFmt("Length (len)","%" PRIu8,  pl.len);

                    HeaderRow("[Time & Increment]");
                    RowFmt("Time (Seconds)", "%" PRIu32, pl.time_s);
                    RowFmt("Time (ms)",      "%" PRIu16, pl.time_ms);
                    RowFmt("Increment",      "%" PRIu16, pl.incr);

                    HeaderRow("[Message]");
                    RowFmt("Line Data", "%s", pl.line);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_GETLATESTNLINES: // CC 7, 9
                {
                    auto &pl = view.u.roma_getlatestNlines;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("PAYUEL ROMA GET MULTIPLE LINES (CC 9)");

                    HeaderRow("[Log Accumulator Data]");
                    RowFmt("Buffer Data", "%s", (const char*)pl.buffer);
                    break;
                }
                
                case REPORT_KIND_PAYUEL_ROMA_SETROUTEDEFAULT:
                {
                    auto &pl = view.u.roma_setroutedefault;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("PAYUEL ROMA COMMAND RESULT (CC %d)", view.reflected_cc);

                    HeaderRow("[Result Information]");
                    RowFmt("Result", "%" PRIu8, pl.result);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_RESETROUTE :
                {
                    auto &pl = view.u.roma_resetroute;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("PAYUEL ROMA COMMAND RESULT (CC %d)", view.reflected_cc);

                    HeaderRow("[Result Information]");
                    RowFmt("Result", "%" PRIu8, pl.result);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_LOADROUTE :
                {
                    auto &pl = view.u.roma_loadroute;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("PAYUEL ROMA COMMAND RESULT (CC %d)", view.reflected_cc);

                    HeaderRow("[Result Information]");
                    RowFmt("Result", "%" PRIu8, pl.result);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_SAVEROUTE :
                {
                    auto &pl = view.u.roma_saveroute;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("PAYUEL ROMA SAVE ROUTE (CC 19)");

                    HeaderRow("[Route Information]");
                    RowFmt("Route Data", "%s", pl.route);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_SENDROUTE :
                {
                    auto &pl = view.u.roma_sendroute;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("PAYUEL ROMA COMMAND RESULT (CC %d)", view.reflected_cc);

                    HeaderRow("[Result Information]");
                    RowFmt("Result", "%" PRIu8, pl.result);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_SETROUTE :
                {
                    auto &pl = view.u.roma_setroute;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("PAYUEL ROMA COMMAND RESULT (CC %d)", view.reflected_cc);

                    HeaderRow("[Result Information]");
                    RowFmt("Result", "%" PRIu8, pl.result);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_PARGET :
                {
                    auto &pl = view.u.roma_parget;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("PAYUEL ROMA PAR GET (CC 22)");

                    HeaderRow("[Parameter Information]");
                    RowFmt("Value", "%" PRId32, pl.value);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_PARSET :
                {
                    auto &pl = view.u.roma_parset;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("PAYUEL ROMA COMMAND RESULT (CC %d)", view.reflected_cc);

                    HeaderRow("[Result Information]");
                    RowFmt("Result", "%" PRIu8, pl.result);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_PARDEFAULTS :
                {
                    auto &pl = view.u.roma_pardefaults;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("PAYUEL ROMA COMMAND RESULT (CC %d)", view.reflected_cc);

                    HeaderRow("[Result Information]");
                    RowFmt("Result", "%" PRIu8, pl.result);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_PARSAVE :
                {
                    auto &pl = view.u.roma_parsave;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("PAYUEL ROMA COMMAND RESULT (CC %d)", view.reflected_cc);

                    HeaderRow("[Result Information]");
                    RowFmt("Result", "%" PRIu8, pl.result);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_PARRESTORE :
                {
                    auto &pl = view.u.roma_parrestore;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("PAYUEL ROMA COMMAND RESULT (CC %d)", view.reflected_cc);

                    HeaderRow("[Result Information]");
                    RowFmt("Result", "%" PRIu8, pl.result);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_PARLOAD :
                {
                    auto &pl = view.u.roma_parload;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("PAYUEL ROMA COMMAND RESULT (CC %d)", view.reflected_cc);

                    HeaderRow("[Result Information]");
                    RowFmt("Result", "%" PRIu8, pl.result);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_PARSETOOB :
                {
                    auto &pl = view.u.roma_parsetOOB;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("PAYUEL ROMA COMMAND RESULT (CC %d)", view.reflected_cc);

                    HeaderRow("[Result Information]");
                    RowFmt("Result", "%" PRIu8, pl.result);
                    break;
                }

                case REPORT_KIND_PAYUEL_ROMA_SENDCOMMAND :
                {
                    auto &pl = view.u.roma_sendcommand;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("PAYUEL ROMA COMMAND RESULT (CC %d)", view.reflected_cc);

                    HeaderRow("[Result Information]");
                    RowFmt("Result", "%" PRIu8, pl.result);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_NOOP: // CC 0
                {
                    auto &pl = view.u.lgpm_noop;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM NOOP (CC 0)");

                    HeaderRow("[Counters Information]");
                    RowFmt("Command Counter", "%" PRIu8, pl.CmdCounter);
                    RowFmt("Error Counter",   "%" PRIu8, pl.ErrCounter);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_RESETCOUNTERS: // CC 1
                {
                    auto &pl = view.u.lgpm_resetcounters;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM RESET COUNTERS (CC 1)");

                    HeaderRow("[Counters Information]");
                    RowFmt("Command Counter", "%" PRIu8, pl.CmdCounter);
                    RowFmt("Error Counter",   "%" PRIu8, pl.ErrCounter);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_MCU_ALIVE: // CC 2
                {
                    auto &pl = view.u.lgpm_mcualivecheck;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM MCU ALIVE CHECK (CC 2)");

                    HeaderRow("[Payload Info]");
                    RowFmt("MCU_ALIVE_CHECK", "%" PRIu8, pl.MCU_ALIVE_CHECK);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_3V3_PWR_ON: // CC 3
                {
                    auto &pl = view.u.lgpm_3v3pwron;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM 3V3 PWR ON (CC 3)");

                    HeaderRow("[Payload Info]");
                    RowFmt("PWR_ON_3V3", "%" PRIu8, pl.PWR_ON_3V3);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_3V3_PWR_OFF: // CC 4
                {
                    auto &pl = view.u.lgpm_3v3pwroff;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM 3V3 PWR OFF (CC 4)");

                    HeaderRow("[Payload Info]");
                    RowFmt("PWR_OFF_3V3", "%" PRIu8, pl.PWR_OFF_3V3);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_ON: // CC 5
                {
                    auto &pl = view.u.lgpm_mainboostswon;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM MAIN BOOST SW ON (CC 5)");

                    HeaderRow("[Payload Info]");
                    RowFmt("MAIN_BOOST_SW_ON", "%" PRIu8, pl.MAIN_BOOST_SW_ON);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_OFF: // CC 6
                {
                    auto &pl = view.u.lgpm_mainboostswoff;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM MAIN BOOST SW OFF (CC 6)");

                    HeaderRow("[Payload Info]");
                    RowFmt("MAIN_BOOST_SW_OFF", "%" PRIu8, pl.MAIN_BOOST_SW_OFF);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_ON: // CC 7
                {
                    auto &pl = view.u.lgpm_subboostswon;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM SUB BOOST SW ON (CC 7)");

                    HeaderRow("[Payload Info]");
                    RowFmt("SUB_BOOST_SW_ON", "%" PRIu8, pl.SUB_BOOST_SW_ON);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_OFF: // CC 8
                {
                    auto &pl = view.u.lgpm_subboostswoff;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM SUB BOOST SW OFF (CC 8)");

                    HeaderRow("[Payload Info]");
                    RowFmt("SUB_BOOST_SW_OFF", "%" PRIu8, pl.SUB_BOOST_SW_OFF);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_V28_MAIN_ON: // CC 9
                {
                    auto &pl = view.u.lgpm_v28mainon;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM V28 MAIN ON (CC 9)");

                    HeaderRow("[Payload Info]");
                    RowFmt("V28_MAIN_ON", "%" PRIu8, pl.V28_MAIN_ON);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_V28_MAIN_OFF: // CC 10
                {
                    auto &pl = view.u.lgpm_v28mainoff;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM V28 MAIN OFF (CC 10)");

                    HeaderRow("[Payload Info]");
                    RowFmt("V28_MAIN_OFF", "%" PRIu8, pl.V28_MAIN_OFF);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_V28_SUB_ON: // CC 11
                {
                    auto &pl = view.u.lgpm_v28subon;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM V28 SUB ON (CC 11)");

                    HeaderRow("[Payload Info]");
                    RowFmt("V28_SUB_ON", "%" PRIu8, pl.V28_SUB_ON);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_V28_SUB_OFF: // CC 12
                {
                    auto &pl = view.u.lgpm_v28suboff;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM V28 SUB OFF (CC 12)");

                    HeaderRow("[Payload Info]");
                    RowFmt("V28_SUB_OFF", "%" PRIu8, pl.V28_SUB_OFF);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_V12_MAIN_ON: // CC 13
                {
                    auto &pl = view.u.lgpm_v12mainon;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM V12 MAIN ON (CC 13)");

                    HeaderRow("[Payload Info]");
                    RowFmt("V12_MAIN_ON", "%" PRIu8, pl.V12_MAIN_ON);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_V12_MAIN_OFF: // CC 14
                {
                    auto &pl = view.u.lgpm_v12mainoff;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM V12 MAIN OFF (CC 14)");

                    HeaderRow("[Payload Info]");
                    RowFmt("V12_MAIN_OFF", "%" PRIu8, pl.V12_MAIN_OFF);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_PWR_SENSE_INFO: // CC 15
                {
                    auto &pl = view.u.lgpm_pwrsenseinfo;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM PWR SENSE INFO (CC 15)");

                    HeaderRow("[Payload Info]");
                    RowFmt("PWR_SENSE_INFO", "%" PRIu8, pl.PWR_SENSE_INFO);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_ON: // CC 16
                {
                    auto &pl = view.u.lgpm_pwrseqon;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM PWR SEQ ON (CC 16)");

                    HeaderRow("[Payload Info]");
                    RowFmt("PWR_SEQ_ON", "%" PRIu8, pl.PWR_SEQ_ON);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_OFF: // CC 17
                {
                    auto &pl = view.u.lgpm_pwrseqoff;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM PWR SEQ OFF (CC 17)");

                    HeaderRow("[Payload Info]");
                    RowFmt("PWR_SEQ_OFF", "%" PRIu8, pl.PWR_SEQ_OFF);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX1: // CC 18
                {
                    auto &pl = view.u.lgpm_rwacontrol_idx1;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM RWA CONTROL idx1 (CC 18)");

                    HeaderRow("[Payload Info]");
                    RowFmt("RWA_CONTROL", "%" PRIu8, pl.RWA_CONTROL);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX2: // CC 19
                {
                    auto &pl = view.u.lgpm_rwacontrol_idx2;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM RWA CONTROL idx2 (CC 19)");

                    HeaderRow("[Payload Info]");
                    RowFmt("RWA_CONTROL", "%" PRIu8, pl.RWA_CONTROL);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX3: // CC 20
                {
                    auto &pl = view.u.lgpm_rwacontrol_idx3;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM RWA CONTROL idx3 (CC 20)");

                    HeaderRow("[Payload Info]");
                    RowFmt("RWA_CONTROL", "%" PRIu8, pl.RWA_CONTROL);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_RWA_PWR_ON: // CC 21
                {
                    auto &pl = view.u.lgpm_rwapwron;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM RWA PWR ON (CC 21)");

                    HeaderRow("[Payload Info]");
                    RowFmt("RWA_PWR_ON", "%" PRIu8, pl.RWA_PWR_ON);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_RWA_PWR_OFF: // CC 22
                {
                    auto &pl = view.u.lgpm_rwapwroff;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM RWA PWR OFF (CC 22)");

                    HeaderRow("[Payload Info]");
                    RowFmt("RWA_PWR_OFF", "%" PRIu8, pl.RWA_PWR_OFF);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }

                case REPORT_KIND_PAYUEL_LGPM_RWA_SENSE_INFO: // CC 23
                {
                    auto &pl = view.u.lgpm_rwasenseinfo;
                    ImGui::TableNextRow(); 
                    ImGui::TableSetColumnIndex(0); 
                    ImGui::TextUnformatted("Type"); 
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted("LGPM RWA SENSE INFO (CC 23)");

                    HeaderRow("[Payload Info]");
                    RowFmt("RWA_SENSE_INFO", "%" PRIu8, pl.RWA_SENSE_INFO);
                    RowFmt("Execution Timestamp", "%" PRIu32, pl.Execution_Timestamp);
                    RowFmt("Reply Message", "%s", pl.Reply_Message);
                    RowFmt("CRC16", "0x%04X", pl.CRC16);
                    break;
                }



                /*************************************************************************************************** */



                    case REPORT_KIND_SC_GENERIC:
                    {
                        show_hexdump = true;
                        ImGui::Text("Generic payload (%u bytes)", view.ret_val_size);
                        break;      
                    }

                    
                    default:
                        show_hexdump = true;
                        ImGui::Text("Default type payload (%u bytes)", view.ret_val_size);
                        break;


                    }

                }

                ImGui::EndTable();
            }

            if (view.valid &&
                show_hexdump)
            {
                ImGui::Separator();
                ImGui::Text("Payload (Hex dump):");

                ImGui::BeginChild("##ReportPayload",
                                ImVec2(ImGui::GetContentRegionAvail().x,
                                        ImGui::GetContentRegionAvail().y),
                                true, ImGuiWindowFlags_HorizontalScrollbar);

                int bytes_per_line = 16;
                uint16_t dump_size = view.ret_val_size;
                if (dump_size > sizeof(view.u.generic.bytes))
                    dump_size = sizeof(view.u.generic.bytes);

                for (int i = 0; i < (int)dump_size; i++)
                {
                    ImGui::SameLine(0.0f, 0.0f);
                    ImGui::Text("%02X ", view.u.generic.bytes[i]);
                    if ((i + 1) % bytes_per_line == 0)
                        ImGui::NewLine();
                }

                if (dump_size % bytes_per_line != 0)
                    ImGui::NewLine();

                ImGui::EndChild();
            }

            ImGui::SetWindowFontScale(1.0f * fontscale);
            ImGui::EndTabItem();
        }



    if(ImGui::BeginTabItem("Event"))
    {

        if (ImGui::BeginTable("##EventTables", 2, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable))


    ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
    ImGui::TableSetupColumn("Parameter", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide, 0.0f);
    ImGui::TableSetupColumn("Data",      ImGuiTableColumnFlags_NoHide, 0.0f);
   
    ImGui::TableHeadersRow();


    if (!event)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Event");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("No data");
    }
    else
    {
        char buf[128];

        // =========================
        //  TLM Header
        // =========================
        BeaconSectionHeader("Event Header");

        StateWindowColumnManager("Telemetry MsgID");
        ImGui::Text("0x%04" PRIX16, (uint16_t)event->CCSDS_MsgId);

        StateWindowColumnManager("Telemetry Sequence");
        ImGui::Text("0x%04" PRIX16, (uint16_t)event->CCSDS_Seq);

        StateWindowColumnManager("Telemetry Length");
        ImGui::Text("0x%04" PRIX16, (uint16_t)event->CCSDS_Len);

        StateWindowColumnManager("Telemetry Time Code");
        ImGui::Text("0x%02" PRIX8 " %02" PRIX8 " %02" PRIX8 " %02" PRIX8 " %02" PRIX8 " %02" PRIX8,
                    (uint8_t)event->CCSDS_TimeCode[0], (uint8_t)event->CCSDS_TimeCode[1],
                    (uint8_t)event->CCSDS_TimeCode[2], (uint8_t)event->CCSDS_TimeCode[3],
                    (uint8_t)event->CCSDS_TimeCode[4], (uint8_t)event->CCSDS_TimeCode[5]);

        StateWindowColumnManager("Telemetry Padding");
        ImGui::Text("0x%08" PRIX32, (uint32_t)event->CCSDS_Padding);

        // =========================
        //  EVENT payload
        // =========================
        BeaconSectionHeader("Payload");

        StateWindowColumnManager("AppName");
        ImGui::Text("%.*s", (int)sizeof(event->AppName), event->AppName);

        StateWindowColumnManager("EventID");
        ImGui::Text("%" PRIu16 " (0x%04" PRIX16 ")", (uint16_t)event->EventID, (uint16_t)event->EventID);

        StateWindowColumnManager("EventType");
        ImGui::Text("%" PRIu16 " (0x%04" PRIX16 ")", (uint16_t)event->EventType, (uint16_t)event->EventType);

        StateWindowColumnManager("SpacecraftID");
        ImGui::Text("%" PRIu32 " (0x%08" PRIX32 ")", (uint32_t)event->SpacecraftID, (uint32_t)event->SpacecraftID);

        StateWindowColumnManager("ProcessorID");
        ImGui::Text("%" PRIu32 " (0x%08" PRIX32 ")", (uint32_t)event->ProcessorID, (uint32_t)event->ProcessorID);

        StateWindowColumnManager("Message");
        ImGui::Text("%.*s", (int)sizeof(event->Message), event->Message);

        StateWindowColumnManager("Spare1");
        ImGui::Text("0x%02" PRIX8, (uint8_t)event->Spare1);

        StateWindowColumnManager("Spare2");
        ImGui::Text("0x%02" PRIX8, (uint8_t)event->Spare2);
    }


        ImGui::EndTable();
        ImGui::EndTabItem();
        ImGui::SetWindowFontScale(1.0 * fontscale);
    }




    if(ImGui::BeginTabItem("Response"))
    {

        if (ImGui::BeginTable("##ResponseTables", 2, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable))


    ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
    ImGui::TableSetupColumn("Parameter", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide, 0.0f);
    ImGui::TableSetupColumn("Data",      ImGuiTableColumnFlags_NoHide, 0.0f);
   
    ImGui::TableHeadersRow();


    if (!getfileinfo)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Response");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("No data");
    }
    else
    {
        char buf[128];


        // =========================
        //  TLM Header
        // =========================
        BeaconSectionHeader("Telemetry Header");
        StateWindowColumnManager("Telemetry MsgID");
        ImGui::Text("0x%02X 0x%02X", getfileinfo->CCSDS_MID[0], getfileinfo->CCSDS_MID[1]);

        StateWindowColumnManager("Telemetry Sequence");
        ImGui::Text("0x%02X 0x%02X", getfileinfo->CCSDS_Seq[0], getfileinfo->CCSDS_Seq[1]);

        StateWindowColumnManager("Telemetry Length");
        ImGui::Text("0x%02X 0x%02X", getfileinfo->CCSDS_Len[0], getfileinfo->CCSDS_Len[1]);

        StateWindowColumnManager("Telemetry Time Code");
        ImGui::Text("0x%02X %02X %02X %02X %02X %02X",
                    getfileinfo->CCSDS_TimeCode[0], getfileinfo->CCSDS_TimeCode[1],
                    getfileinfo->CCSDS_TimeCode[2], getfileinfo->CCSDS_TimeCode[3],
                    getfileinfo->CCSDS_TimeCode[4], getfileinfo->CCSDS_TimeCode[5]);

        StateWindowColumnManager("Telemetry Padding");
        ImGui::Text("0x%02X %02X %02X %02X",
                    getfileinfo->padding[0], getfileinfo->padding[1],
                    getfileinfo->padding[2], getfileinfo->padding[3]);


        // =========================
        //  GETFILEINFO payload
        // =========================


        BeaconSectionHeader("Payload");
        StateWindowColumnManager("File Status");
        ImGui::Text("%" PRIu8, getfileinfo->FileStatus);

        StateWindowColumnManager("CRC Computed");
        ImGui::Text("%" PRIu8, getfileinfo->CRC_Computed);

        StateWindowColumnManager("Spare");
        ImGui::Text("0x%02X 0x%02X", getfileinfo->Spare[0], getfileinfo->Spare[1]);

        StateWindowColumnManager("CRC");
        ImGui::Text("%" PRIu32, getfileinfo->CRC);

        StateWindowColumnManager("File Size");
        ImGui::Text("%" PRIu32, getfileinfo->FileSize);

        StateWindowColumnManager("Last Modified Time");
        ImGui::Text("%" PRIu32, getfileinfo->LastModifiedTime);

        StateWindowColumnManager("Mode");
        ImGui::Text("%" PRIu32, getfileinfo->Mode);

        StateWindowColumnManager("Filename");
        ImGui::Text("%s", getfileinfo->Filename);

    }


        ImGui::EndTable();
        ImGui::EndTabItem();
        ImGui::SetWindowFontScale(1.0 * fontscale);
    }












        
    // if(ImGui::BeginTabItem("Telemetry"))
    // {
    //     ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.650f, 0.266f, 0.322f, 1.0f));
    //     ImGui::Text("Total RX Bytes(Beacon) : %"PRIu32, get_rx_bytes());
    //     ImGui::SetWindowFontScale(0.6 * fontscale);
    //     ImGui::PopStyleColor();

    //     ImGui::SetWindowFontScale(1.0 * fontscale);
    //     ImGui::EndTabItem();
    // }



    if(ImGui::BeginTabItem("AX100 RPARAM"))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.650f, 0.266f, 0.322f, 1.0f));
        ImGui::Text("Total RX Bytes(Beacon) : %"PRIu32, get_rx_bytes());
        ImGui::SetWindowFontScale(0.6 * fontscale);
        ImGui::PopStyleColor();
        
        ImGui::BeginChild("##AX100_shell_typeselect", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.8), true, NULL);
        
        


        if(get_boot_count() == 0)
            ImGui::Text("AX100 Boot Count   ");
        else
            ImGui::Text("AX100 Boot Count : %"PRIu16"", get_boot_count());
        if(ImGui::Button("Load##ax100_direct4_boot_count", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_get_uint16(setup->ax100_node, 4, 0x0020, 0xB00B, setup->default_timeout, get_boot_count_address()) == GS_OK)
                console.AddLog("[OK]##AX100 Boot Count : %u", get_boot_count());
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::Spacing();



        




        ImGui::Text("RSSI busy        ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputScalar("##ax100_direct4_rssi_busy", ImGuiDataType_S16, &remote_rssibusy, NULL, NULL, "%d");
        if(ImGui::Button("Load##ax100_direct4_rssi_busy", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
        {
            sleep(0.3);
            if(gs_rparam_get_int16(setup->ax100_node, 5, 0x0048, 0xB00B, setup->default_timeout, &remote_rssibusy) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK. rssi_busy : %"PRIu8, rparam0.csp_node);
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::SameLine();
        if(ImGui::Button("Save##ax100_direct4_rssi_busy", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            sleep(0.3);
            if(gs_rparam_set_int16(setup->ax100_node, 5, 0x0048, 0xB00B, setup->default_timeout, remote_rssibusy) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK.");
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::Spacing();


        if(get_rx_bytes() == 0)
            ImGui::Text("Total RX Bytes   ");
        else
            ImGui::Text("Total RX bytes : %"PRIu8"", get_rx_bytes());
        if(ImGui::Button("Load##ax100_direct4_total_rx_bytes", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_get_uint32(setup->ax100_node, 4, 0x003C, 0xB00B, setup->default_timeout, get_rx_bytes_address()) == GS_OK)
                console.AddLog("[OK]##Total RX Bytes : %u", get_rx_bytes());
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::Spacing();




        
        ImGui::Text("CAN enable       ");
        ImGui::SameLine();
        ImGui::Checkbox("##ax100_direct0_can_en", &rparam0.can_en);
        if(ImGui::Button("Load##ax100_direct0_can_en", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
        {
            sleep(0.3);
            if(gs_rparam_get_int8(setup->ax100_node, 0, rparam0.can_en_addr, 0xB00B, setup->default_timeout, (int8_t *)&rparam0.can_en) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK. can_en : %s", rparam0.can_en ? "true" : "false");
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::SameLine();
        if(ImGui::Button("Save##ax100_direct0_can_en", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            sleep(0.3);
            if(gs_rparam_set_int8(setup->ax100_node, 0, rparam0.can_en_addr, 0xB00B, setup->default_timeout, (bool)rparam0.can_en) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK.");
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::Spacing();




        ImGui::Text("TX inhibit       ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputScalar("##ax100_direct0_tx_inhibit", ImGuiDataType_U32, &rparam0.tx_inhibit, NULL, NULL, "%u");
        if(ImGui::Button("Load##ax100_direct0_tx_inhibit", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
        {
            sleep(0.3);
            if(gs_rparam_get_uint32(setup->ax100_node, 0, rparam0.tx_inhibit_addr, 0xB00B, setup->default_timeout, &rparam0.tx_inhibit) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK. tx_inhibit : %"PRIu32, rparam0.tx_inhibit);
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::SameLine();
        if(ImGui::Button("Save##ax100_direct0_tx_inhibit", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            sleep(0.3);
            if(gs_rparam_set_uint32(setup->ax100_node, 0, rparam0.tx_inhibit_addr, 0xB00B, setup->default_timeout, rparam0.tx_inhibit) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK.");
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
            
        }
        ImGui::Spacing();
        




        ImGui::Text("TX power         ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputScalar("##ax100_direct0_tx_pwr", ImGuiDataType_U8, &rparam0.tx_pwr, NULL, NULL, "%u");
        if(ImGui::Button("Load##ax100_direct0_tx_pwr", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_get_uint8(setup->ax100_node, 0, rparam0.tx_pwr_addr, 0xB00B, setup->default_timeout, &rparam0.tx_pwr) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK. tx_pwr : %"PRIu8, rparam0.tx_pwr);
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::SameLine();
        if(ImGui::Button("Save##ax100_direct0_tx_pwr", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_set_uint8(setup->ax100_node, 0, rparam0.tx_pwr_addr, 0xB00B, setup->default_timeout, rparam0.tx_pwr) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK.");
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }






        ImGui::Spacing();
        
        ImGui::Text("CSP rtable       ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##ax100_direct0_csp_rtable", rparam0.csp_rtable, sizeof(rparam0.csp_rtable));
        if(ImGui::Button("Load##ax100_direct0_csp_rtable", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_get_string(setup->ax100_node, 0, rparam0.csp_rtable_addr, 0xB00B, setup->default_timeout, rparam0.csp_rtable, sizeof(rparam0.csp_rtable)) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK. csp_rtable : %s", rparam0.csp_rtable);
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::SameLine();
        if(ImGui::Button("Save##ax100_direct0_csp_rtable", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_set_string(setup->ax100_node, 0, rparam0.csp_rtable_addr, 0xB00B, setup->default_timeout, rparam0.csp_rtable, sizeof(rparam0.csp_rtable)) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK.");
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::Spacing();
        
        if(remote_activeconf > 3)
            ImGui::Text("Active Conf      ");
        else
            ImGui::Text("Active Conf : %"PRIu8"", remote_activeconf);
        if(ImGui::Button("Load##ax100_direct0_active_conf", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_get_uint8(setup->ax100_node, 4, 0x0018, 0xB00B, setup->default_timeout, &remote_activeconf) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK. Active Conf : %"PRId8, remote_activeconf);
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
            
        }
        ImGui::Spacing();

        if(remote_lastrssi > 3)
            ImGui::Text("Last RSSI        ");
        else
            ImGui::Text("Last RSSI : %"PRId16"", remote_lastrssi);
        if(ImGui::Button("Load##ax100_direct0_last_rssi", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_get_int16(setup->ax100_node, 4, 0x0004, 0xB00B, setup->default_timeout, &remote_lastrssi) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK. Last RSSI : %"PRId16, remote_lastrssi);
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
            
        }
        ImGui::Spacing();

        ImGui::Text("RX guard         ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputScalar("##ax100_direct1_rx_guard", ImGuiDataType_U16, &rparam1.guard, NULL, NULL, "%u");
        if(ImGui::Button("Load##ax100_direct1_rx_guard", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_get_uint16(setup->ax100_node, 1, rparam1.guard_addr, 0xB00B, setup->default_timeout, &rparam1.guard) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK. rx_guard : %"PRIu16, rparam1.guard);
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::SameLine();
        if(ImGui::Button("Save##ax100_direct1_rx_guard", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_set_uint16(setup->ax100_node, 1, rparam1.guard_addr, 0xB00B, setup->default_timeout, rparam1.guard) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK.");
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::Spacing();
        
        ImGui::Text("TX guard         ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputScalar("##ax100_direct5_tx_guard", ImGuiDataType_U16, &rparam5.guard, NULL, NULL, "%u");
        if(ImGui::Button("Load##ax100_direct5_tx_guard", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_get_uint16(setup->ax100_node, 5, rparam5.guard_addr, 0xB00B, setup->default_timeout, &rparam5.guard) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK. tx_guard : %"PRIu16, rparam5.guard);
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::SameLine();
        if(ImGui::Button("Save##ax100_direct5_tx_guard", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_set_uint16(setup->ax100_node, 5, rparam5.guard_addr, 0xB00B, setup->default_timeout, rparam5.guard) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK.");
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::Spacing();

        ImGui::Text("RX Baudrate         ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputScalar("##ax100_direct1_rxbaud", ImGuiDataType_U32, &rparam1.baud, NULL, NULL, "%u");
        if(ImGui::Button("Load##ax100_direct1_rx_baud", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_get_uint32(setup->ax100_node, 1, rparam1.baud_addr, 0xB00B, setup->default_timeout, &rparam1.baud) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK. rx_baud : %"PRIu16, rparam1.baud);
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::SameLine();
        if(ImGui::Button("Save##ax100_direct1_rxbaud", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_set_uint32(setup->ax100_node, 1, rparam1.baud_addr, 0xB00B, setup->default_timeout, rparam1.baud) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK.");
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::Spacing();

        ImGui::Text("TX Baudrate         ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputScalar("##ax100_direct5_txbaud", ImGuiDataType_U32, &rparam5.baud, NULL, NULL, "%u");
        if(ImGui::Button("Load##ax100_direct5_txbaud", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_get_uint32(setup->ax100_node, 5, rparam5.baud_addr, 0xB00B, setup->default_timeout, &rparam5.baud) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK. tx_baud : %"PRIu16, rparam5.baud);
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::SameLine();
        if(ImGui::Button("Save##ax100_direct5_txbaud", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(gs_rparam_set_uint32(setup->ax100_node, 5, rparam5.baud_addr, 0xB00B, setup->default_timeout, rparam5.baud) == GS_OK)
                console.AddLog("[OK]##Remote Param Control OK.");
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::Spacing();

        if(ImGui::Button("Save to FRAM##ax100_direct", ImVec2(ImGui::GetContentRegionAvail().x * 0.33, ImGui::GetContentRegionAvail().y)))
        {
            if(gs_rparam_save(setup->ax100_node, setup->default_timeout, 0, 0) == GS_OK)
            {
                gs_rparam_save(setup->ax100_node, setup->default_timeout, 1, 1);
                gs_rparam_save(setup->ax100_node, setup->default_timeout, 5, 5);
                set_guard_spec_micsec(param5.baud, param1.baud, param5.guard, param1.guard, rparam5.guard, rparam1.guard);
                console.AddLog("[OK]##Remote Param Control OK.");
            }
                
            else
                console.AddLog("[ERROR]##Cannot Load Remote Parameter.");
        }
        ImGui::SameLine();
        if(ImGui::Button("Reset Watchdog##ax100_direct", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetContentRegionAvail().y)))
        {
            if(csp_transaction(CSP_PRIO_HIGH, setup->ax100_node, AX100_PORT_GNDWDT_RESET, 1000, NULL, 0, NULL, 0) > 0)
                console.AddLog("Remote Param Control OK.(No Reply)");
            else
                console.AddLog("[ERROR]##Remote Param Control Failed.(No Reply)");
        }
        ImGui::SameLine();
        if(ImGui::Button("Reboot AX100##ax100_direct", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y)))
        {
            csp_reboot(setup->ax100_node);
            console.AddLog("Remote Param Control OK.(No Reply)");
        }
        ImGui::EndChild();

        ImGui::BeginChild("##MEOW_shell_typeselect", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));
        ImGui::RadioButton("System CMD   ##shelltype_syscmd", &State.Shelltype, TYPE_SYSCMD);
        ImGui::SameLine();
        ImGui::RadioButton("Redirect CMD ##shelltype_rmdir ", &State.Shelltype, TYPE_SYSCMD_REDIR);
        ImGui::SameLine();
        ImGui::RadioButton("Remove Dir   ##shelltype_rmdirp", &State.Shelltype, TYPE_SET_REDIR_PATH);
        ImGui::SameLine();
        ImGui::RadioButton("DS Cleanup   ##shelltype_cleand", &State.Shelltype, TYPE_DS_CLEANUP);

        ImGui::RadioButton("HK Request   ##shelltype_requhk", &State.Shelltype, TYPE_HK_REQUEST);
        ImGui::SameLine();
        ImGui::RadioButton("Reserved     ##shelltype_reserv", &State.Shelltype, TYPE_RESERVED);
        ImGui::SameLine();
        ImGui::RadioButton("KILL         ##shelltype_killin", &State.Shelltype, TYPE_KILL);

        if(State.Shelltype != (int)shellcmd->type)
        {
            shellcmd->type = (uint8_t)State.Shelltype;
            memset(&shellcmd->required, 0, sizeof(shellcmd->required));
        }

        switch (State.Shelltype)
        {
        case TYPE_SYSCMD:
        {   
            ImGui::Text("Out U8   ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::InputScalar("##shell_syscmd_redirout", ImGuiDataType_U8, &(shellcmd->required.syscmd.redir_out), NULL, NULL, "%u");
            ImGui::Text("CMD      ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::InputText("##shell_syscmd_cmd", shellcmd->required.syscmd.cmd, sizeof(shellcmd->required.syscmd.cmd));
            break;
        }
        case TYPE_SYSCMD_REDIR:
        {   
            ImGui::Text("CMD      ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::InputText("##shell_syscmd_cmd1", shellcmd->required.syscmd_redir.cmd, sizeof(shellcmd->required.syscmd_redir.cmd));
            ImGui::Text("Redirect ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::InputText("##shell_syscmd_cmd2", shellcmd->required.syscmd_redir.redir_path, sizeof(shellcmd->required.syscmd_redir.redir_path));
            break;
        }
        case TYPE_SET_REDIR_PATH:
        {   
            ImGui::Text("Path     ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::InputText("##shell_syscmd_cmd4", shellcmd->required.set_redir.redir_path, sizeof(shellcmd->required.set_redir.redir_path));
            ImGui::Text("");
            break;
        }
        case TYPE_DS_CLEANUP:
        {   
            ImGui::Text("Path     ");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::InputScalar("##shell_syscmd_redirout5", ImGuiDataType_U8, &(shellcmd->required.ds_cleanup.opt), NULL, NULL, "%u");
            ImGui::Text("");
            break;
        }
        
        default:
        {
            ImGui::Text("ERROR!");
            break;
        }
        }

        if(ImGui::Button("Send##SendShellcmd", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 3)))
        {
            if(State.Debugmode)
            {
                printf("Input Shell Data : ");
                for(int i = 0; i < sizeof(cmd_packet_t); i++)
                    printf("%u\t", ((uint8_t *)shellcmd)[i]);
                printf("\n");
            }
            pthread_create(&p_thread[4], NULL, Direct_Shell, (void *)shellcmd); 
        }
        ImGui::EndChild();
        ImGui::SetWindowFontScale(1.0 * fontscale);
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::SetWindowFontScale(1.0);

    ImGui::End();
}

/*********************************5차 추가****************************************/
static void HMACWindow()
{
    static int mode_arg;

    ImGui::Text("Select Satellite");

    for (int i = 0; i < 3; i++)
    {
        if (ImGui::RadioButton(sat_names[i], &current_sat_mode, i))
        {
            mode_arg = current_sat_mode;
            pthread_join(p_thread[14], NULL);
            pthread_create(&p_thread[14], NULL, SwitchSatelliteTask, (void*)&mode_arg);
        }
    }
}
/*********************************************************************************/

static void PingWindow() {
    static uint8_t node = 0;

    ImGui::InputScalar("node", ImGuiDataType_U8, &node);

    if (ImGui::Button("Send Ping")) {

        pthread_join(p_thread[4], NULL);

        uint8_t *arg = (uint8_t*)malloc(sizeof(uint8_t));
        *arg = node; 

        pthread_create(&p_thread[4], NULL, csp_ping_everynode_console, (void*)arg);
    }
}


static void RparamWindow()
{
    static uint8_t node = 0;
    static uint8_t table_id = 0;
    static uint16_t addr = 0;
    static int type_index = 0;

    static uint8_t  value_u8 = 0;
    static uint16_t value_u16 = 0;
    static uint32_t value_u32 = 0;
    static int8_t   value_s8 = 0;
    static int16_t  value_s16 = 0;
    static int32_t  value_s32 = 0;
    static float    value_float = 0;
    // static char     value_char = 0;

    const char* type_names[] = { "uint8", "uint16", "uint32", "int8", "int16", "int32", "float"};

    ImGui::Text("Node");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputScalar("##rparam_node", ImGuiDataType_U8, &node);

    ImGui::Text("Table ID");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputScalar("##rparam_table", ImGuiDataType_U8, &table_id);


    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 50, 50, 255));
    ImGui::Text("!!!WARNING!!! Your Addr must be decimal, not hex");
    ImGui::PopStyleColor();

    ImGui::Text("Address");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputScalar("##rparam_addr", ImGuiDataType_U16, &addr);



    ImGui::Text("Type");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::BeginCombo("##rparam_type", type_names[type_index]))
    {
        for (int i = 0; i < IM_ARRAYSIZE(type_names); ++i)
        {
            bool is_selected = (type_index == i);
            if (ImGui::Selectable(type_names[i], is_selected))
                type_index = i;
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();

    if (type_index == 0)
    {
        ImGui::Text("Value (uint8)");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputScalar("##rparam_value_u8", ImGuiDataType_U8, &value_u8, NULL, NULL, "%u");
    }
    else if (type_index == 1)
    {
        ImGui::Text("Value (uint16)");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputScalar("##rparam_value_u16", ImGuiDataType_U16, &value_u16, NULL, NULL, "%u");
    }
    else if (type_index == 2)
    {
        ImGui::Text("Value (uint32)");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputScalar("##rparam_value_u32", ImGuiDataType_U32, &value_u32, NULL, NULL, "%u");
    }
        else if (type_index == 3)
    {
        ImGui::Text("Value (int8)");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputScalar("##rparam_value_s8", ImGuiDataType_S8, &value_s8, NULL, NULL, "%u");
    }
        else if (type_index == 4)
    {
        ImGui::Text("Value (int16)");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputScalar("##rparam_value_s16", ImGuiDataType_S16, &value_s16, NULL, NULL, "%u");
    }
        else if (type_index == 5)
    {
        ImGui::Text("Value (int32)");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputScalar("##rparam_value_s32", ImGuiDataType_S32, &value_s32, NULL, NULL, "%u");
    }
        else if (type_index == 6)
    {
        ImGui::Text("Value (float)");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputScalar("##rparam_value_float", ImGuiDataType_Float, &value_float, NULL, NULL, "%u");
    }


    float w = ImGui::GetContentRegionAvail().x;
    float h = ImGui::GetFontSize() * 1.5f;
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 215, 0, 255));
    ImGui::Text("Please Try: (get AX100 active_conf)");

    ImGui::Text("node = 5, table_id = 4, address = 24");
    ImGui::Text("Type: uint8");

    ImGui::PopStyleColor();
    if (ImGui::Button("Load##rparam_load", ImVec2(w * 0.5f, h)))
    {
        if (type_index == 0)
        {
            // eps 안불러와지면 0x0BB0 시도!
            if (gs_rparam_get_uint8(node, table_id, addr, 0xB00B, setup->default_timeout, &value_u8) == GS_OK)
                console.AddLog("[OK]##Rparam Load OK. node=%u table=%u addr=0x%04X value=%" PRIu8, node, table_id, addr, value_u8);
            else
                console.AddLog("[ERROR]##Rparam Load Failed.");
        }
        else if (type_index == 1)
        {
            if (gs_rparam_get_uint16(node, table_id, addr, 0xB00B, setup->default_timeout, &value_u16) == GS_OK)
                console.AddLog("[OK]##Rparam Load OK. node=%u table=%u addr=0x%04X value=%" PRIu16, node, table_id, addr, value_u16);
            else
                console.AddLog("[ERROR]##Rparam Load Failed.");
        }
        else if (type_index == 2)
        {
            if (gs_rparam_get_uint32(node, table_id, addr, 0xB00B, setup->default_timeout, &value_u32) == GS_OK)
                console.AddLog("[OK]##Rparam Load OK. node=%u table=%u addr=0x%04X value=%" PRIu32, node, table_id, addr, value_u32);
            else
                console.AddLog("[ERROR]##Rparam Load Failed.");
        }
        else if (type_index == 3)
        {
            if (gs_rparam_get_int8(node, table_id, addr, 0xB00B, setup->default_timeout, &value_s8) == GS_OK)
                console.AddLog("[OK]##Rparam Load OK. node=%u table=%u addr=0x%04X value=%" PRIi8, node, table_id, addr, value_s8);
            else
                console.AddLog("[ERROR]##Rparam Load Failed.");
        }

        else if (type_index == 4)
        {
            if (gs_rparam_get_int16(node, table_id, addr, 0xB00B, setup->default_timeout, &value_s16) == GS_OK)
                console.AddLog("[OK]##Rparam Load OK. node=%u table=%u addr=0x%04X value=%" PRIi16, node, table_id, addr, value_s16);
            else
                console.AddLog("[ERROR]##Rparam Load Failed.");
        }

        else if (type_index == 5)
        {
            if (gs_rparam_get_int32(node, table_id, addr, 0xB00B, setup->default_timeout, &value_s32) == GS_OK)
                console.AddLog("[OK]##Rparam Load OK. node=%u table=%u addr=0x%04X value=%" PRIi32, node, table_id, addr, value_s32);
            else
                console.AddLog("[ERROR]##Rparam Load Failed.");
        }

        else if (type_index == 6)
        {
            if (gs_rparam_get_float(node, table_id, addr, 0xB00B, setup->default_timeout, &value_float) == GS_OK)
                console.AddLog("[OK]##Rparam Load OK. node=%u table=%u addr=0x%04X value=%f", node, table_id, addr, value_float);
            else
                console.AddLog("[ERROR]##Rparam Load Failed.");
        }




    }

    ImGui::SameLine();

    if (ImGui::Button("Save##rparam_save", ImVec2(w * 0.5f, h)))
    {
        if (type_index == 0)
        {
            if (gs_rparam_set_uint8(node, table_id, addr, 0xB00B, setup->default_timeout, value_u8) == GS_OK)
                console.AddLog("[OK]##Rparam Save OK. node=%u table=%u addr=0x%04X value=%" PRIu8, node, table_id, addr, value_u8);
            else
                console.AddLog("[ERROR]##Rparam Save Failed.");
        }
        else if (type_index == 1)
        {
            if (gs_rparam_set_uint16(node, table_id, addr, 0xB00B, setup->default_timeout, value_u16) == GS_OK)
                console.AddLog("[OK]##Rparam Save OK. node=%u table=%u addr=0x%04X value=%" PRIu16, node, table_id, addr, value_u16);
            else
                console.AddLog("[ERROR]##Rparam Save Failed.");
        }
        else if (type_index == 2)
        {
            if (gs_rparam_set_uint32(node, table_id, addr, 0xB00B, setup->default_timeout, value_u32) == GS_OK)
                console.AddLog("[OK]##Rparam Save OK. node=%u table=%u addr=0x%04X value=%" PRIu32, node, table_id, addr, value_u32);
            else
                console.AddLog("[ERROR]##Rparam Save Failed.");
        }
    }
}


/*****************************************5차 추가************************************************/
struct SubsystemRange {
    int sub_index;
    int start;
    int end;
};

static const SubsystemRange FilterRules[] = {
    { 1, 0, 9 },      // Test & Debug
    { 2, 10, 46 },    // EPS range 1
    { 2, 209, 211 },  // EPS range 2
    { 2, 216, 216 },  // EPS single
    { 3, 47, 88 },    // ADCS
    { 4, 160, 163 },  // UANT
    { 5, 164, 167 },  // SP range 1
    { 5, 212, 212 },  // SP range 1
    { 6, 168, 180 },  // PAYUZUC
    { 7, 214, 215 },  // Cosmic
    { 8, 153, 159 },  // cFE range 1
    { 8, 207, 208 },  // cFE range 2
    { 8, 217, 221 },  // cFE range 3
    { 9, 136, 152 },  // SC APP
    { 10, 201, 204 }, // FTP range 1
    { 10, 213, 213 }, // FTP range 2
    { 11, 206, 206 }, // UTRX
    { 12, 230, 253 }, // LGPM (Case 230~253)
    { 13, 330, 342 }, // LTRX (Case 330~342) 
    { 14, 260, 277 }, // STX (Case 260~277)
    { 15, 280, 310 }, // PAYUEL_ROMA (Case 280~310)
    { 16, 320, 326 }, // PAYUEL_LGBAT (Case 320~326)
};
/*************************************************************************************************/

static void DrawCmdGeneratorBody(bool initial_mode)
{
    /******************************************5차 변경****************************************************/
    static int selected_index = 0;
    const char* index_names[] = {
        "All", "Test&Debug", "EPS", "ADCS", "UANT", "SP", "PAYUZUC", "Cosmic UEL Pi",
        "cFE", "SC APP", "FTP", "UTRX GNDWDT",
        "LGPM", "LTRX", "STX", "PAYUEL_ROMA", "PAYUEL_LGBAT"
    };

    ImGui::SetNextItemWidth(100.0f);
    if (ImGui::BeginCombo("##Subsystem", index_names[selected_index]))
    {
        for (int n = 0; n < IM_ARRAYSIZE(index_names); n++)
        {
            bool is_selected = (selected_index == n);
            if (ImGui::Selectable(index_names[n], is_selected))
                selected_index = n;

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();

    auto IsVisible = [&](int cmd_i) {
        if (selected_index == 0) return true; // "All" selected
        for (const auto& rule : FilterRules) {
            if (rule.sub_index == selected_index && cmd_i >= rule.start && cmd_i <= rule.end)
                return true;
        }
        return false;
        };
    const char* current_label =
        (gen_fnccode >= 0 && gen_fnccode < CMD_LABEL_MAX && strlen(Templabels[gen_fnccode]) > 0)
        ? Templabels[gen_fnccode]
        : "Select CMD type";

    if (ImGui::BeginCombo("##Select CMD type", current_label))
    {
        if (initial_mode)
        {

            for (int k = 0; INITIAL_CMD_INDICES[k] >= 0; ++k)
            {
                int i = INITIAL_CMD_INDICES[k];
                if (strlen(Templabels[i]) == 0 || !IsVisible(i))
                    continue;

                bool SelectedGen = (gen_fnccode == i);
                if (ImGui::Selectable(Templabels[i], SelectedGen))
                    gen_fnccode = i;

                if (SelectedGen)
                    ImGui::SetItemDefaultFocus();
            }
        }
        else
        {
            for (int i = 0; i < CMD_LABEL_MAX; i++)
            {
                if (strlen(Templabels[i]) == 0 || !IsVisible(i))
                    continue;

                bool SelectedGen = (gen_fnccode == i);
                if (ImGui::Selectable(Templabels[i], SelectedGen))
                {
                    if (gen_fnccode != i)
                        gen_fnccode = i;
                }
                if (SelectedGen)
                    ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::Checkbox("Write Raw Data", &raw_data);
    /**********************************************************************************************/

    switch (gen_fnccode)
    {
        case 0:
        {
            static uint16_t msgid = 0;
            static uint8_t fnccode = 0;
            ImGui::Text("For ADCS NoOp CMD, MsgID = 6245");
            ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

            ImGui::Checkbox("Schedule", &State.Scheduled);
            if (State.Scheduled)
            {
                ImGui::InputScalar("sch_u8ent", ImGuiDataType_U8, &State.entrynum);
                ImGui::InputScalar("sch_u32ent", ImGuiDataType_U32, &State.u32val);
            }

            if (ImGui::Button("Generate CMD"))
            {
                if (State.Scheduled)
                {
                    WriteSystemName(msgid);
                    pthread_join(p_thread[4], NULL);

                    typedef struct
                    {
                        uint8 entnum;
                        uint32 exenum;
                        packetsign TestPacket[];
                    } sch_cmd_t;

                    sch_cmd_t* schcmd;
                    schcmd = (sch_cmd_t*)malloc(5 + 2 + 2 + 4 + CFE_SB_CMD_HDR_SIZE);
                    schcmd->TestPacket->Identifier = HVD_TEST;
                    schcmd->TestPacket->PacketType = MIM_PT_TMTC_TEST;
                    schcmd->TestPacket->Length = CFE_SB_CMD_HDR_SIZE;
                    memset(schcmd->TestPacket->Data, 0, schcmd->TestPacket->Length);

                    uint8_t cmd2[CFE_SB_CMD_HDR_SIZE] = {0,};
                    msgid = htons(msgid);
                    memcpy(cmd2, &msgid, sizeof(uint16_t));
                    memcpy(cmd2 + 6, &fnccode, sizeof(uint8_t));
                    cmd2[2] = 0xc0;
                    cmd2[3] = 0x00;
                    cmd2[4] = 0x00;
                    cmd2[5] = 0x01;

                    uint16_t len = CFE_SB_CMD_HDR_SIZE;
                    const uint8_t* byteptr = cmd2;
                    uint8_t checksum = 0xFF;
                    while (len--)
                        checksum ^= *(byteptr++);
                    cmd2[7] = checksum;

                    memcpy(schcmd->TestPacket->Data, cmd2, sizeof(cmd2));
                    schcmd->entnum = State.entrynum;
                    schcmd->exenum = State.u32val;

                    for (int i = 0; i < schcmd->TestPacket->Length; i++)
                        printf("0x%x ", cmd2[i]);
                    printf("\n");

                    pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)schcmd);
                    msgid = htons(msgid);
                }
                else
                {
                    WriteSystemName(msgid);
                    pthread_join(p_thread[4], NULL);

                    packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + 8);
                    TestPacket->Identifier = HVD_TEST;
                    TestPacket->PacketType = MIM_PT_TMTC_TEST;
                    TestPacket->Length = 8;
                    memset(TestPacket->Data, 0, TestPacket->Length);

                    uint8_t cmd[8] = {0,};
                    msgid = htons(msgid);
                    memcpy(cmd, &msgid, sizeof(uint16_t));
                    memcpy(cmd + 6, &fnccode, sizeof(uint8_t));
                    cmd[2] = 0xc0;
                    cmd[3] = 0x00;
                    cmd[4] = 0x00;
                    cmd[5] = 0x01;

                    uint16_t len = 8;
                    const uint8_t* byteptr = cmd;
                    uint8_t checksum = 0xFF;
                    while (len--)
                        checksum ^= *(byteptr++);
                    cmd[7] = checksum;

                    memcpy(TestPacket->Data, cmd, sizeof(cmd));
                    for (int i = 0; i < TestPacket->Length; i++)
                        printf("0x%x ", cmd[i]);
                    printf("\n");

                    pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
                    msgid = htons(msgid);
                }
            }
            break;
        }
    case 1:
    {
        static uint16_t msgid = 0;
        static uint8_t fnccode = 0;
        static uint8_t arg = 0;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("arg u8", ImGuiDataType_U8, &arg);

        if (ImGui::Button("Generate CMD"))
        {
            WriteSystemName(msgid);
            pthread_join(p_thread[4], NULL);

            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + CFE_SB_CMD_HDR_SIZE + sizeof(uint8_t));
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length = CFE_SB_CMD_HDR_SIZE + sizeof(uint8_t);
            memset(TestPacket->Data, 0, TestPacket->Length);

            uint8_t cmd2[CFE_SB_CMD_HDR_SIZE + sizeof(uint8_t)] = {0,};
            msgid = htons(msgid);
            memcpy(cmd2, &msgid, sizeof(uint16_t));
            memcpy(cmd2 + 6, &fnccode, sizeof(uint8_t));
            memcpy(cmd2 + 8, &arg, sizeof(uint8_t));
            cmd2[2] = 0xc0;
            cmd2[3] = 0x00;
            cmd2[4] = 0x00;
            cmd2[5] = 0x02;

            uint16_t len = 9;
            const uint8_t* byteptr = cmd2;
            uint8_t checksum = 0xFF;
            while (len--)
                checksum ^= *(byteptr++);
            cmd2[7] = checksum;

            memcpy(TestPacket->Data, cmd2, sizeof(cmd2));
            for (int i = 0; i < TestPacket->Length; i++)
            {
                if (i < 8)
                    printf("0x%x ", cmd2[i]);
                else
                    printf("%u", cmd2[i]);
            }
            printf("\n");

            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
            msgid = htons(msgid);
        }
        break;
    }
    case 2:
    {
        static uint16_t msgid = 0;
        static uint8_t fnccode = 0;
        static uint16_t arg = 0;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("arg u16", ImGuiDataType_U16, &arg);

        if (ImGui::Button("Generate CMD"))
        {
            WriteSystemName(msgid);
            pthread_join(p_thread[4], NULL);

            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + CFE_SB_CMD_HDR_SIZE + sizeof(uint16_t));
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length = CFE_SB_CMD_HDR_SIZE + sizeof(uint8_t);
            memset(TestPacket->Data, 0, TestPacket->Length);

            uint8_t cmd2[CFE_SB_CMD_HDR_SIZE + sizeof(uint8_t)] = {0,};
            msgid = htons(msgid);
            memcpy(cmd2, &msgid, sizeof(uint16_t));
            memcpy(cmd2 + 6, &fnccode, sizeof(uint8_t));
            memcpy(cmd2 + 8, &arg, sizeof(uint16_t));
            cmd2[2] = 0xc0;
            cmd2[3] = 0x00;
            cmd2[4] = 0x00;
            cmd2[5] = 0x03;

            uint16_t len = 10;
            const uint8_t* byteptr = cmd2;
            uint8_t checksum = 0xFF;
            while (len--)
                checksum ^= *(byteptr++);
            cmd2[7] = checksum;

            memcpy(TestPacket->Data, cmd2, sizeof(cmd2));
            for (int i = 0; i < TestPacket->Length; i++)
            {
                if (i < 8)
                    printf("0x%x ", cmd2[i]);
                else
                    printf("%u", arg);
            }
            printf("\n");

            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
            msgid = htons(msgid);
        }
        break;
    }
    case 3:
    {
        static uint16_t msgid = 0;
        static uint8_t fnccode = 0;
        static uint32_t arg = 0;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("arg u32", ImGuiDataType_U32, &arg);

        if (ImGui::Button("Generate CMD"))
        {
            WriteSystemName(msgid);
            pthread_join(p_thread[4], NULL);

            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + CFE_SB_CMD_HDR_SIZE + sizeof(uint32_t));
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length = CFE_SB_CMD_HDR_SIZE + sizeof(uint32_t);
            memset(TestPacket->Data, 0, TestPacket->Length);

            uint8_t cmd2[CFE_SB_CMD_HDR_SIZE + sizeof(uint32_t)] = {0,};
            msgid = htons(msgid);
            memcpy(cmd2, &msgid, sizeof(uint16_t));
            memcpy(cmd2 + 6, &fnccode, sizeof(uint8_t));
            memcpy(cmd2 + 8, &arg, sizeof(uint32_t));
            cmd2[2] = 0xc0;
            cmd2[3] = 0x00;
            cmd2[4] = 0x00;
            cmd2[5] = 0x05;

            uint16_t len = 12;
            const uint8_t* byteptr = cmd2;
            uint8_t checksum = 0xFF;
            while (len--)
                checksum ^= *(byteptr++);
            cmd2[7] = checksum;

            memcpy(TestPacket->Data, cmd2, sizeof(cmd2));
            for (int i = 0; i < TestPacket->Length; i++)
                printf("0x%x ", cmd2[i]);
            printf("\n");

            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
            msgid = htons(msgid);
        }
        break;
    }
    case 4:
    {
        static uint16_t msgid = 0;
        static uint8_t fnccode = 0;
        static uint64_t arg = 0;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("arg u64", ImGuiDataType_U64, &arg);

        if (ImGui::Button("Generate CMD"))
        {
            if (State.Scheduled)
            {
                WriteSystemName(msgid);
                pthread_join(p_thread[4], NULL);

                typedef struct
                {
                    uint8 entnum;
                    uint32 exenum;
                    packetsign TestPacket[];
                } sch_cmd_t;

                sch_cmd_t* schcmd;
                schcmd = (sch_cmd_t*)malloc(5 + 2 + 2 + 4 + CFE_SB_CMD_HDR_SIZE + sizeof(uint64_t));
                schcmd->TestPacket->Identifier = HVD_TEST;
                schcmd->TestPacket->PacketType = MIM_PT_TMTC_TEST;
                schcmd->TestPacket->Length = CFE_SB_CMD_HDR_SIZE + sizeof(uint64_t);
                memset(schcmd->TestPacket->Data, 0, schcmd->TestPacket->Length);

                uint8_t cmd2[CFE_SB_CMD_HDR_SIZE + sizeof(uint64_t)] = {0,};
                msgid = htons(msgid);
                memcpy(cmd2, &msgid, sizeof(uint16_t));
                memcpy(cmd2 + 6, &fnccode, sizeof(uint8_t));
                memcpy(cmd2 + 8, &arg, sizeof(uint64_t));
                cmd2[2] = 0xc0;
                cmd2[3] = 0x00;
                cmd2[4] = 0x00;
                cmd2[5] = 0x09;

                uint16_t len = CFE_SB_CMD_HDR_SIZE + sizeof(uint64_t);
                const uint8_t* byteptr = cmd2;
                uint8_t checksum = 0xFF;
                while (len--)
                    checksum ^= *(byteptr++);
                cmd2[7] = checksum;

                memcpy(schcmd->TestPacket->Data, cmd2, sizeof(cmd2));
                schcmd->entnum = State.entrynum;
                schcmd->exenum = State.u32val;

                for (int i = 0; i < schcmd->TestPacket->Length; i++)
                    printf("0x%x ", cmd2[i]);
                printf("\n");

                pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)schcmd->TestPacket);
                msgid = htons(msgid);
            }
            else
            {
                WriteSystemName(msgid);
                pthread_join(p_thread[4], NULL);

                packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + CFE_SB_CMD_HDR_SIZE + sizeof(uint64_t));
                TestPacket->Identifier = HVD_TEST;
                TestPacket->PacketType = MIM_PT_TMTC_TEST;
                TestPacket->Length = CFE_SB_CMD_HDR_SIZE + sizeof(uint64_t);
                memset(TestPacket->Data, 0, TestPacket->Length);

                uint8_t cmd2[CFE_SB_CMD_HDR_SIZE + sizeof(uint64_t)] = {0,};
                msgid = htons(msgid);
                memcpy(cmd2, &msgid, sizeof(uint16_t));
                memcpy(cmd2 + 6, &fnccode, sizeof(uint8_t));
                memcpy(cmd2 + 8, &arg, sizeof(uint64_t));
                cmd2[2] = 0xc0;
                cmd2[3] = 0x00;
                cmd2[4] = 0x00;
                cmd2[5] = 0x09;

                uint16_t len = CFE_SB_CMD_HDR_SIZE + sizeof(uint64_t);
                const uint8_t* byteptr = cmd2;
                uint8_t checksum = 0xFF;
                while (len--)
                    checksum ^= *(byteptr++);
                cmd2[7] = checksum;

                memcpy(TestPacket->Data, cmd2, sizeof(cmd2));
                for (int i = 0; i < TestPacket->Length; i++)
                    printf("0x%x ", cmd2[i]);
                printf("\n");

                pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
                msgid = htons(msgid);
            }
        }
        break;
    }

    case 5:  {// s8
        static uint16_t msgid=0;
        static uint8_t fnccode=0;
        static int8_t arg=0;
        ImGui::InputScalar("msgid",ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode",ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("arg s8",ImGuiDataType_S8, &arg);
        if(ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            pthread_join(p_thread[4], NULL);
            packetsign * TestPacket = (packetsign *)malloc(2+2+4+CFE_SB_CMD_HDR_SIZE+sizeof(int8_t)); // 8 is for data
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length = CFE_SB_CMD_HDR_SIZE+sizeof(int8_t);
            memset(TestPacket->Data,0,TestPacket->Length);
            uint8_t cmd2[CFE_SB_CMD_HDR_SIZE+sizeof(int8_t)] = {0,};
            msgid = htons(msgid);
            memcpy(cmd2, &msgid, sizeof(uint16_t));
            memcpy(cmd2 + 7, &fnccode, sizeof(uint8_t));
            memcpy(cmd2 + 8, &arg, sizeof(int8_t));
            cmd2[2] = 0xc0;
            cmd2[3] = 0x00;
            cmd2[4] = 0x00;
            cmd2[5] = 0x02;
            uint16_t len = CFE_SB_CMD_HDR_SIZE+sizeof(int8_t);
            const uint8_t *byteptr = cmd2;
            uint8_t checksum = 0xFF;
            while (len--) {
                checksum ^= *(byteptr++);
            }
            cmd2[6]=checksum; // checksum
            memcpy(TestPacket->Data,cmd2,sizeof(cmd2));
            for(int i=0; i<TestPacket->Length; i++) {
                printf("0x%x ",cmd2[i]);
            } printf("\n");
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)TestPacket);
            msgid = htons(msgid);
        }
        break;
    }
    case 6:  {// s16
        static uint16_t msgid=0;
        static uint8_t fnccode=0;
        static int16_t arg=0;
        ImGui::InputScalar("msgid",ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode",ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("arg s16",ImGuiDataType_S16, &arg);
        if(ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            pthread_join(p_thread[4], NULL);
            packetsign * TestPacket = (packetsign *)malloc(2+2+4+CFE_SB_CMD_HDR_SIZE+sizeof(int16_t)); // 8 is for data
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length = CFE_SB_CMD_HDR_SIZE+sizeof(int16_t);
            memset(TestPacket->Data,0,TestPacket->Length);
            uint8_t cmd2[CFE_SB_CMD_HDR_SIZE+sizeof(int16_t)] = {0,};
            msgid = htons(msgid);
            memcpy(cmd2, &msgid, sizeof(uint16_t));
            memcpy(cmd2 + 7, &fnccode, sizeof(uint8_t));
            memcpy(cmd2 + 8, &arg, sizeof(int16_t));
            cmd2[2] = 0xc0;
            cmd2[3] = 0x00;
            cmd2[4] = 0x00;
            cmd2[5] = 0x03;
            uint16_t len = 10;
            const uint8_t *byteptr = cmd2;
            uint8_t checksum = 0xFF;
            while (len--) {
                checksum ^= *(byteptr++);
            }
            cmd2[6]=checksum; // checksum
            memcpy(TestPacket->Data,cmd2,sizeof(cmd2));
            for(int i=0; i<TestPacket->Length; i++) {
                printf("0x%x ",cmd2[i]);
            } printf("\n");
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)TestPacket);
            msgid = htons(msgid);
        }
        break;
    }
    case 7:  {// s32
        static uint16_t msgid=0;
        static uint8_t fnccode=0;
        static int32_t arg=0;
        ImGui::InputScalar("msgid",ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode",ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("arg s32",ImGuiDataType_S32, &arg);
        if(ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            pthread_join(p_thread[4], NULL);
            packetsign * TestPacket = (packetsign *)malloc(2+2+4+12); // 8 is for data
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length = 12;
            memset(TestPacket->Data,0,TestPacket->Length);
            uint8_t cmd2[12] = {0,};
            msgid = htons(msgid);
            memcpy(cmd2, &msgid, sizeof(uint16_t));
            memcpy(cmd2 + 7, &fnccode, sizeof(uint8_t));
            memcpy(cmd2 + 8, &arg, sizeof(int32_t));
            cmd2[2] = 0xc0;
            cmd2[3] = 0x00;
            cmd2[4] = 0x00;
            cmd2[5] = 0x05;
            uint16_t len = 12;
            const uint8_t *byteptr = cmd2;
            uint8_t checksum = 0xFF;
            while (len--) {
                checksum ^= *(byteptr++);
            }
            cmd2[6]=checksum; // checksum
            memcpy(TestPacket->Data,cmd2,sizeof(cmd2));
            for(int i=0; i<TestPacket->Length; i++) {
                printf("0x%x ",cmd2[i]);
            } printf("\n");
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)TestPacket);
            msgid = htons(msgid);
        }
        break;
    }
    case 8:  {// s64
        static uint16_t msgid=0;
        static uint8_t fnccode=0;
        static int64_t arg=0;
        ImGui::InputScalar("msgid",ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode",ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("arg s64",ImGuiDataType_S64, &arg);
        if(ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            pthread_join(p_thread[4], NULL);
            packetsign * TestPacket = (packetsign *)malloc(2+2+4+16); // 8 is for data
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length = 16;
            memset(TestPacket->Data,0,TestPacket->Length);
            uint8_t cmd2[16] = {0,};
            msgid = htons(msgid);
            memcpy(cmd2, &msgid, sizeof(uint16_t));
            memcpy(cmd2 + 7, &fnccode, sizeof(uint8_t));
            memcpy(cmd2 + 8, &arg, sizeof(int64_t));
            cmd2[2] = 0xc0;
            cmd2[3] = 0x00;
            cmd2[4] = 0x00;
            cmd2[5] = 0x09;
            uint16_t len = 16;
            const uint8_t *byteptr = cmd2;
            uint8_t checksum = 0xFF;
            while (len--) {
                checksum ^= *(byteptr++);
            }
            cmd2[6]=checksum; // checksum
            memcpy(TestPacket->Data,cmd2,sizeof(cmd2));
            for(int i=0; i<TestPacket->Length; i++) {
                printf("0x%x ",cmd2[i]);
            } printf("\n");
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)TestPacket);
            msgid = htons(msgid);
        }
        break;
    }
    case 9:  { // float
        static uint16_t msgid = 0;
        static uint8_t fnccode=0;
        static float arg =0;
        ImGui::InputScalar("msgid",ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode",ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("arg float",ImGuiDataType_Float, &arg);
        if(ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            pthread_join(p_thread[4], NULL);
            packetsign * TestPacket = (packetsign *)malloc(2+2+4+12); // 8 is for data
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length = 12;
            memset(TestPacket->Data,0,TestPacket->Length);
            uint8_t cmd2[12] = {0,};
            msgid = htons(msgid);
            memcpy(cmd2, &msgid, sizeof(uint16_t));
            memcpy(cmd2 + 7, &fnccode, sizeof(uint8_t));
            memcpy(cmd2 + 8, &arg, sizeof(float));
            cmd2[2] = 0xc0;
            cmd2[3] = 0x00;
            cmd2[4] = 0x00;
            cmd2[5] = 0x05;
            uint16_t len = 12;
            const uint8_t *byteptr = cmd2;
            uint8_t checksum = 0xFF;
            while (len--) {
                checksum ^= *(byteptr++);
            }
            cmd2[6]=checksum; // checksum
            memcpy(TestPacket->Data,cmd2,sizeof(cmd2));
            for(int i=0; i<TestPacket->Length; i++) {
                printf("0x%x ",cmd2[i]);
            } printf("\n");
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)TestPacket);
            msgid = htons(msgid);
        }
        break;
    }

    case 10: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_NOOP_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_NoopCmd_t) - 7};

            memcpy(command->epsnoopcmd.CmdHeader, &mid, 2);
            memcpy(command->epsnoopcmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsnoopcmd.CmdHeader + 4, len, 2);
            memcpy(command->epsnoopcmd.CmdHeader + 6, &fnccode, 1);

            command->epsnoopcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(EPS_NoopCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsnoopcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsnoopcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_NoopCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_NoopCmd_t);
            memcpy(pkt->Data, &command->epsnoopcmd, sizeof(EPS_NoopCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 11: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_RESET_COUNTERS_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_ResetCountersCmd_t) - 7};

            memcpy(command->epsresetcounterscmd.CmdHeader, &mid, 2);
            memcpy(command->epsresetcounterscmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsresetcounterscmd.CmdHeader + 4, len, 2);
            memcpy(command->epsresetcounterscmd.CmdHeader + 6, &fnccode, 1);

            command->epsresetcounterscmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(EPS_ResetCountersCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsresetcounterscmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsresetcounterscmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_ResetCountersCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_ResetCountersCmd_t);
            memcpy(pkt->Data, &command->epsresetcounterscmd, sizeof(EPS_ResetCountersCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 12: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_GET_COUNTERS_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_GetCountersCmd_t) - 7};

            memcpy(command->epsgetcounterscmd.CmdHeader, &mid, 2);
            memcpy(command->epsgetcounterscmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsgetcounterscmd.CmdHeader + 4, len, 2);
            memcpy(command->epsgetcounterscmd.CmdHeader + 6, &fnccode, 1);

            command->epsgetcounterscmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(EPS_GetCountersCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsgetcounterscmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsgetcounterscmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_GetCountersCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_GetCountersCmd_t);
            memcpy(pkt->Data, &command->epsgetcounterscmd, sizeof(EPS_GetCountersCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 13: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_GET_APPDATA_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_GetAppDataCmd_t) - 7};

            memcpy(command->epsgetappdatacmd.CmdHeader, &mid, 2);
            memcpy(command->epsgetappdatacmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsgetappdatacmd.CmdHeader + 4, len, 2);
            memcpy(command->epsgetappdatacmd.CmdHeader + 6, &fnccode, 1);

            command->epsgetappdatacmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(EPS_GetAppDataCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsgetappdatacmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsgetappdatacmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_GetAppDataCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_GetAppDataCmd_t);
            memcpy(pkt->Data, &command->epsgetappdatacmd, sizeof(EPS_GetAppDataCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 14: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_REPORT_APPDATA_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_ReportAppDataCmd_t) - 7};

            memcpy(command->epsreportappdatacmd.CmdHeader, &mid, 2);
            memcpy(command->epsreportappdatacmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsreportappdatacmd.CmdHeader + 4, len, 2);
            memcpy(command->epsreportappdatacmd.CmdHeader + 6, &fnccode, 1);

            command->epsreportappdatacmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(EPS_ReportAppDataCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsreportappdatacmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsreportappdatacmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_ReportAppDataCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_ReportAppDataCmd_t);
            memcpy(pkt->Data, &command->epsreportappdatacmd, sizeof(EPS_ReportAppDataCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 15: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_DOCK_SET_CHANNEL_SINGLE_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        ImGui::InputScalar("channel", ImGuiDataType_U8,
                           &command->epsp60docksetchannelsinglecmd.Payload.channel);
        ImGui::InputScalar("value", ImGuiDataType_U8,
                           &command->epsp60docksetchannelsinglecmd.Payload.value);
        ImGui::InputScalar("timeout", ImGuiDataType_U16,
                           &command->epsp60docksetchannelsinglecmd.Payload.timeout);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_Dock_SetChannelSingleCmd_t) - 7};

            memcpy(command->epsp60docksetchannelsinglecmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60docksetchannelsinglecmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60docksetchannelsinglecmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60docksetchannelsinglecmd.CmdHeader + 6, &fnccode, 1);

            command->epsp60docksetchannelsinglecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(EPS_P60_Dock_SetChannelSingleCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60docksetchannelsinglecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60docksetchannelsinglecmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_Dock_SetChannelSingleCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_Dock_SetChannelSingleCmd_t);
            memcpy(pkt->Data, &command->epsp60docksetchannelsinglecmd,
                   sizeof(EPS_P60_Dock_SetChannelSingleCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 16: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_DOCK_GET_CHANNEL_SINGLE_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        ImGui::InputScalar("channel", ImGuiDataType_U8,
                           &command->epsp60dockgetchannelsinglecmd.Payload.channel);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_Dock_GetChannelSingleCmd_t) - 7};

            memcpy(command->epsp60dockgetchannelsinglecmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60dockgetchannelsinglecmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60dockgetchannelsinglecmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60dockgetchannelsinglecmd.CmdHeader + 6, &fnccode, 1);

            command->epsp60dockgetchannelsinglecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(EPS_P60_Dock_GetChannelSingleCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60dockgetchannelsinglecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60dockgetchannelsinglecmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_Dock_GetChannelSingleCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_Dock_GetChannelSingleCmd_t);
            memcpy(pkt->Data, &command->epsp60dockgetchannelsinglecmd,
                   sizeof(EPS_P60_Dock_GetChannelSingleCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 17: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_DOCK_SET_CHANNELS_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        for (uint8_t i = 0; i < 13; i++) {
            char label[16];
            snprintf(label, sizeof(label), "ch[%u]", i);
            ImGui::InputScalar(label, ImGuiDataType_U8,
                               &command->epsp60docksetchannelscmd.Payload.channels[i]);
        }
        ImGui::InputScalar("timeout", ImGuiDataType_U16,
                           &command->epsp60docksetchannelscmd.Payload.timeout);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_Dock_SetChannelsCmd_t) - 7};

            memcpy(command->epsp60docksetchannelscmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60docksetchannelscmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60docksetchannelscmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60docksetchannelscmd.CmdHeader + 6, &fnccode, 1);

            command->epsp60docksetchannelscmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(EPS_P60_Dock_SetChannelsCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60docksetchannelscmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60docksetchannelscmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_Dock_SetChannelsCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_Dock_SetChannelsCmd_t);
            memcpy(pkt->Data, &command->epsp60docksetchannelscmd,
                   sizeof(EPS_P60_Dock_SetChannelsCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 18: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_DOCK_GET_CHANNELS_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_Dock_GetChannelsCmd_t) - 7};

            memcpy(command->epsp60dockgetchannelscmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60dockgetchannelscmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60dockgetchannelscmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60dockgetchannelscmd.CmdHeader + 6, &fnccode, 1);

            command->epsp60dockgetchannelscmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(EPS_P60_Dock_GetChannelsCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60dockgetchannelscmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60dockgetchannelscmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_Dock_GetChannelsCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_Dock_GetChannelsCmd_t);
            memcpy(pkt->Data, &command->epsp60dockgetchannelscmd,
                   sizeof(EPS_P60_Dock_GetChannelsCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 19: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_PDU_SET_CHANNEL_SINGLE_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        ImGui::InputScalar("channel", ImGuiDataType_U8,
                           &command->epsp60pdusetchannelsinglecmd.Payload.channel);
        ImGui::InputScalar("value", ImGuiDataType_U8,
                           &command->epsp60pdusetchannelsinglecmd.Payload.value);
        ImGui::InputScalar("timeout", ImGuiDataType_U32,
                           &command->epsp60pdusetchannelsinglecmd.Payload.timeout);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_PDU_SetChannelSingleCmd_t) - 7};

            memcpy(command->epsp60pdusetchannelsinglecmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60pdusetchannelsinglecmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60pdusetchannelsinglecmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60pdusetchannelsinglecmd.CmdHeader + 6, &fnccode, 1);

            command->epsp60pdusetchannelsinglecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(EPS_P60_PDU_SetChannelSingleCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60pdusetchannelsinglecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60pdusetchannelsinglecmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_PDU_SetChannelSingleCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_PDU_SetChannelSingleCmd_t);
            memcpy(pkt->Data, &command->epsp60pdusetchannelsinglecmd,
                   sizeof(EPS_P60_PDU_SetChannelSingleCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 20: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_PDU_GET_CHANNEL_SINGLE_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        ImGui::InputScalar("channel", ImGuiDataType_U8,
                           &command->epsp60pdugetchannelsinglecmd.Payload.channel);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_PDU_GetChannelSingleCmd_t) - 7};

            memcpy(command->epsp60pdugetchannelsinglecmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60pdugetchannelsinglecmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60pdugetchannelsinglecmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60pdugetchannelsinglecmd.CmdHeader + 6, &fnccode, 1);


            command->epsp60pdugetchannelsinglecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(EPS_P60_PDU_GetChannelSingleCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60pdugetchannelsinglecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60pdugetchannelsinglecmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_PDU_GetChannelSingleCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_PDU_GetChannelSingleCmd_t);
            memcpy(pkt->Data, &command->epsp60pdugetchannelsinglecmd,
                   sizeof(EPS_P60_PDU_GetChannelSingleCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 21: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_PDU_SET_CHANNELS_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        for (uint8_t i = 0; i < 9; i++) {
            char label[16];
            snprintf(label, sizeof(label), "ch[%u]", i);
            ImGui::InputScalar(label, ImGuiDataType_U8,
                               &command->epsp60pdusetchannelscmd.Payload.channels[i]);
        }
        ImGui::InputScalar("timeout", ImGuiDataType_U32,
                           &command->epsp60pdusetchannelscmd.Payload.timeout);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_PDU_SetChannelsCmd_t) - 7};

            memcpy(command->epsp60pdusetchannelscmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60pdusetchannelscmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60pdusetchannelscmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60pdusetchannelscmd.CmdHeader + 6, &fnccode, 1);

            command->epsp60pdusetchannelscmd.CmdHeader[7] = 0;

            uint16_t total = sizeof(EPS_P60_PDU_SetChannelsCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60pdusetchannelscmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60pdusetchannelscmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_PDU_SetChannelsCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_PDU_SetChannelsCmd_t);
            memcpy(pkt->Data, &command->epsp60pdusetchannelscmd,
                   sizeof(EPS_P60_PDU_SetChannelsCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 22: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_PDU_GET_CHANNELS_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_PDU_GetChannelsCmd_t) - 7};

            memcpy(command->epsp60pdugetchannelscmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60pdugetchannelscmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60pdugetchannelscmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60pdugetchannelscmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_PDU_GetChannelsCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60pdugetchannelscmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60pdugetchannelscmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_PDU_GetChannelsCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_PDU_GetChannelsCmd_t);
            memcpy(pkt->Data, &command->epsp60pdugetchannelscmd,
                   sizeof(EPS_P60_PDU_GetChannelsCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 23: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_ACU_SET_MPPT_MODE_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        ImGui::InputScalar("mode", ImGuiDataType_U8,
                           &command->epsp60acusetmpptmodecmd.Payload.mode);
        ImGui::InputScalar("timeout", ImGuiDataType_U32,
                           &command->epsp60acusetmpptmodecmd.Payload.timeout);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_ACU_SetMpptModeCmd_t) - 7};

            memcpy(command->epsp60acusetmpptmodecmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60acusetmpptmodecmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60acusetmpptmodecmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60acusetmpptmodecmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_ACU_SetMpptModeCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60acusetmpptmodecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60acusetmpptmodecmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_ACU_SetMpptModeCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_ACU_SetMpptModeCmd_t);
            memcpy(pkt->Data, &command->epsp60acusetmpptmodecmd,
                   sizeof(EPS_P60_ACU_SetMpptModeCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 24: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_ACU_GET_MPPT_MODE_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_ACU_GetMpptModeCmd_t) - 7};

            memcpy(command->epsp60acugetmpptmodecmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60acugetmpptmodecmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60acugetmpptmodecmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60acugetmpptmodecmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_ACU_GetMpptModeCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60acugetmpptmodecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60acugetmpptmodecmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_ACU_GetMpptModeCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_ACU_GetMpptModeCmd_t);
            memcpy(pkt->Data, &command->epsp60acugetmpptmodecmd,
                   sizeof(EPS_P60_ACU_GetMpptModeCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 25: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_DOCK_GET_TABLE_HK_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_Dock_GetTableHkCmd_t) - 7};

            memcpy(command->epsp60dockgettablehkcmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60dockgettablehkcmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60dockgettablehkcmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60dockgettablehkcmd.CmdHeader + 6, &fnccode, 1);


            command->epsp60dockgettablehkcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(EPS_P60_Dock_GetTableHkCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60dockgettablehkcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60dockgettablehkcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_Dock_GetTableHkCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_Dock_GetTableHkCmd_t);
            memcpy(pkt->Data, &command->epsp60dockgettablehkcmd,
                   sizeof(EPS_P60_Dock_GetTableHkCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 26: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_DOCK_GET_TABLE_CONF_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_Dock_GetTableConfCmd_t) - 7};

            memcpy(command->epsp60dockgettableconfcmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60dockgettableconfcmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60dockgettableconfcmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60dockgettableconfcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_Dock_GetTableConfCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60dockgettableconfcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60dockgettableconfcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_Dock_GetTableConfCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_Dock_GetTableConfCmd_t);
            memcpy(pkt->Data, &command->epsp60dockgettableconfcmd,
                   sizeof(EPS_P60_Dock_GetTableConfCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 27: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_DOCK_GET_TABLE_CAL_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_Dock_GetTableCalCmd_t) - 7};

            memcpy(command->epsp60dockgettablecalcmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60dockgettablecalcmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60dockgettablecalcmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60dockgettablecalcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_Dock_GetTableCalCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60dockgettablecalcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60dockgettablecalcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_Dock_GetTableCalCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_Dock_GetTableCalCmd_t);
            memcpy(pkt->Data, &command->epsp60dockgettablecalcmd,
                   sizeof(EPS_P60_Dock_GetTableCalCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 28: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_PDU_GET_TABLE_HK_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_PDU_GetTableHkCmd_t) - 7};

            memcpy(command->epsp60pdugettablehkcmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60pdugettablehkcmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60pdugettablehkcmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60pdugettablehkcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_PDU_GetTableHkCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60pdugettablehkcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60pdugettablehkcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_PDU_GetTableHkCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_PDU_GetTableHkCmd_t);
            memcpy(pkt->Data, &command->epsp60pdugettablehkcmd,
                   sizeof(EPS_P60_PDU_GetTableHkCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 29: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_PDU_GET_TABLE_CONF_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_PDU_GetTableConfCmd_t) - 7};

            memcpy(command->epsp60pdugettableconfcmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60pdugettableconfcmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60pdugettableconfcmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60pdugettableconfcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_PDU_GetTableConfCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60pdugettableconfcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60pdugettableconfcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_PDU_GetTableConfCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_PDU_GetTableConfCmd_t);
            memcpy(pkt->Data, &command->epsp60pdugettableconfcmd,
                   sizeof(EPS_P60_PDU_GetTableConfCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 30: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_PDU_GET_TABLE_CAL_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_PDU_GetTableCalCmd_t) - 7};

            memcpy(command->epsp60pdugettablecalcmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60pdugettablecalcmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60pdugettablecalcmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60pdugettablecalcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_PDU_GetTableCalCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60pdugettablecalcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60pdugettablecalcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_PDU_GetTableCalCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_PDU_GetTableCalCmd_t);
            memcpy(pkt->Data, &command->epsp60pdugettablecalcmd,
                   sizeof(EPS_P60_PDU_GetTableCalCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 31: {
        static uint16_t msgid = 0x1875;
        static uint8_t fnccode = EPS_P60_ACU_GET_TABLE_HK_CC;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t seq[2] = {0xC0, 0x00};
            uint8_t len[2] = {0x00, sizeof(EPS_P60_ACU_GetTableHkCmd_t) - 7};

            memcpy(command->epsp60acugettablehkcmd.CmdHeader, &mid, 2);
            memcpy(command->epsp60acugettablehkcmd.CmdHeader + 2, seq, 2);
            memcpy(command->epsp60acugettablehkcmd.CmdHeader + 4, len, 2);
            memcpy(command->epsp60acugettablehkcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_ACU_GetTableHkCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60acugettablehkcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            memcpy(command->epsp60acugettablehkcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P60_ACU_GetTableHkCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(EPS_P60_ACU_GetTableHkCmd_t);
            memcpy(pkt->Data, &command->epsp60acugettablehkcmd,
                   sizeof(EPS_P60_ACU_GetTableHkCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    // 32 : EPS_P60_ACU_GET_TABLE_CONF_CC  (No-Arg)
    case 32: {
        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_ACU_GET_TABLE_CONF_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00,
                               (uint8_t)(sizeof(EPS_P60_ACU_GetTableConfCmd_t) - 7)};

            memcpy(command->epsp60acugettableconfcmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60acugettableconfcmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60acugettableconfcmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60acugettableconfcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_ACU_GetTableConfCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60acugettableconfcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60acugettableconfcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 +
                                 sizeof(EPS_P60_ACU_GetTableConfCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_ACU_GetTableConfCmd_t);
            memcpy(pkt->Data, &command->epsp60acugettableconfcmd,
                   sizeof(EPS_P60_ACU_GetTableConfCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    // 33 : EPS_P60_ACU_GET_TABLE_CAL_CC  (No-Arg)
    case 33: {
        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_ACU_GET_TABLE_CAL_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00,
                               (uint8_t)(sizeof(EPS_P60_ACU_GetTableCalCmd_t) - 7)};

            memcpy(command->epsp60acugettablecalcmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60acugettablecalcmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60acugettablecalcmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60acugettablecalcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_ACU_GetTableCalCmd_t);
            const uint8_t *p = (const uint8_t *)&command->epsp60acugettablecalcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60acugettablecalcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 +
                                 sizeof(EPS_P60_ACU_GetTableCalCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_ACU_GetTableCalCmd_t);
            memcpy(pkt->Data, &command->epsp60acugettablecalcmd,
                   sizeof(EPS_P60_ACU_GetTableCalCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    // 34 : EPS_P60_DOCK_RESET_GND_WDT_CC
    case 34: {
        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_DOCK_RESET_GND_WDT_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("timeout (Dock)", ImGuiDataType_U32,
                           &command->epsp60dockresetgndwdtcmd.Payload.timeout);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(EPS_P60_Dock_ResetGndWdtCmd_t) - 7)
            };

            memcpy(command->epsp60dockresetgndwdtcmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60dockresetgndwdtcmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60dockresetgndwdtcmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60dockresetgndwdtcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_Dock_ResetGndWdtCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->epsp60dockresetgndwdtcmd;

            command->epsp60dockresetgndwdtcmd.CmdHeader[7] = 0;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60dockresetgndwdtcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(EPS_P60_Dock_ResetGndWdtCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_Dock_ResetGndWdtCmd_t);
            memcpy(pkt->Data, &command->epsp60dockresetgndwdtcmd,
                   sizeof(EPS_P60_Dock_ResetGndWdtCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    // 35 : EPS_P60_PDU_RESET_GND_WDT_CC
    case 35: {
        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_PDU_RESET_GND_WDT_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("timeout (PDU)", ImGuiDataType_U32,
                           &command->epsp60pduresetgndwdtcmd.Payload.timeout);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(EPS_P60_PDU_ResetGndWdtCmd_t) - 7)
            };

            memcpy(command->epsp60pduresetgndwdtcmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60pduresetgndwdtcmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60pduresetgndwdtcmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60pduresetgndwdtcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_PDU_ResetGndWdtCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->epsp60pduresetgndwdtcmd;

            command->epsp60pduresetgndwdtcmd.CmdHeader[7] = 0;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60pduresetgndwdtcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(EPS_P60_PDU_ResetGndWdtCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_PDU_ResetGndWdtCmd_t);
            memcpy(pkt->Data, &command->epsp60pduresetgndwdtcmd,
                   sizeof(EPS_P60_PDU_ResetGndWdtCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    // 36 : EPS_P60_ACU_RESET_GND_WDT_CC
    case 36: {
        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_ACU_RESET_GND_WDT_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("timeout (ACU)", ImGuiDataType_U32,
                           &command->epsp60acuresetgndwdtcmd.Payload.timeout);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(EPS_P60_ACU_ResetGndWdtCmd_t) - 7)
            };

            memcpy(command->epsp60acuresetgndwdtcmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60acuresetgndwdtcmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60acuresetgndwdtcmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60acuresetgndwdtcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_ACU_ResetGndWdtCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->epsp60acuresetgndwdtcmd;

            command->epsp60acuresetgndwdtcmd.CmdHeader[7] = 0;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60acuresetgndwdtcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(EPS_P60_ACU_ResetGndWdtCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_ACU_ResetGndWdtCmd_t);
            memcpy(pkt->Data, &command->epsp60acuresetgndwdtcmd,
                   sizeof(EPS_P60_ACU_ResetGndWdtCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    // 37 : EPS_P60_GET_PARAM_CC
    case 37: {
        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_GET_PARAM_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("node",     ImGuiDataType_U8,
                           &command->epsp60getparamcmd.Payload.node);
        ImGui::InputScalar("tableId",  ImGuiDataType_U8,
                           &command->epsp60getparamcmd.Payload.tableId);
        ImGui::InputScalar("addr",     ImGuiDataType_U16,
                           &command->epsp60getparamcmd.Payload.addr);
        ImGui::InputScalar("type",     ImGuiDataType_U8,
                           &command->epsp60getparamcmd.Payload.type);
        ImGui::InputScalar("size",     ImGuiDataType_U16,
                           &command->epsp60getparamcmd.Payload.size);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(EPS_P60_GetParamCmd_t) - 7)
            };

            memcpy(command->epsp60getparamcmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60getparamcmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60getparamcmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60getparamcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_GetParamCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->epsp60getparamcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60getparamcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(EPS_P60_GetParamCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_GetParamCmd_t);
            memcpy(pkt->Data, &command->epsp60getparamcmd,
                   sizeof(EPS_P60_GetParamCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    // 38 : EPS_P60_GET_PARAM_ARRAY_CC
    case 38: {
        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_GET_PARAM_ARRAY_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("node",     ImGuiDataType_U8,
                           &command->epsp60getparamarraycmd.Payload.node);
        ImGui::InputScalar("tableId",  ImGuiDataType_U8,
                           &command->epsp60getparamarraycmd.Payload.tableId);
        ImGui::InputScalar("addr",     ImGuiDataType_U16,
                           &command->epsp60getparamarraycmd.Payload.addr);
        ImGui::InputScalar("type",     ImGuiDataType_U8,
                           &command->epsp60getparamarraycmd.Payload.type);
        ImGui::InputScalar("size",     ImGuiDataType_U16,
                           &command->epsp60getparamarraycmd.Payload.size);
        ImGui::InputScalar("count",    ImGuiDataType_U16,
                           &command->epsp60getparamarraycmd.Payload.count);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(EPS_P60_GetParamArrayCmd_t) - 7)
            };

            memcpy(command->epsp60getparamarraycmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60getparamarraycmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60getparamarraycmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60getparamarraycmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_GetParamArrayCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->epsp60getparamarraycmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60getparamarraycmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(EPS_P60_GetParamArrayCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_GetParamArrayCmd_t);
            memcpy(pkt->Data, &command->epsp60getparamarraycmd,
                   sizeof(EPS_P60_GetParamArrayCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    // 39 : EPS_P60_SET_PARAM_CC
    case 39: {
        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_SET_PARAM_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("node",     ImGuiDataType_U8,
                           &command->epsp60setparamcmd.Payload.node);
        ImGui::InputScalar("tableId",  ImGuiDataType_U8,
                           &command->epsp60setparamcmd.Payload.tableId);
        ImGui::InputScalar("addr",     ImGuiDataType_U16,
                           &command->epsp60setparamcmd.Payload.addr);
        ImGui::InputScalar("type",     ImGuiDataType_U8,
                           &command->epsp60setparamcmd.Payload.type);
        ImGui::InputScalar("size",     ImGuiDataType_U16,
                           &command->epsp60setparamcmd.Payload.size);
        ImGui::InputScalar("timeout",  ImGuiDataType_U16,
                           &command->epsp60setparamcmd.Payload.timeout);

        for (uint8_t i = 0; i < sizeof(command->epsp60setparamcmd.Payload.data); ++i) {
            char label[32];
            snprintf(label, sizeof(label), "data[%u]", i);
            ImGui::InputScalar(label, ImGuiDataType_U8,
                               &command->epsp60setparamcmd.Payload.data[i]);
        }

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(EPS_P60_SetParamCmd_t) - 7)
            };

            memcpy(command->epsp60setparamcmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60setparamcmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60setparamcmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60setparamcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_SetParamCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->epsp60setparamcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60setparamcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(EPS_P60_SetParamCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_SetParamCmd_t);
            memcpy(pkt->Data, &command->epsp60setparamcmd,
                   sizeof(EPS_P60_SetParamCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    // 40 : EPS_P60_GET_TABLE_CC
    case 40: {
        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_GET_TABLE_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("node",     ImGuiDataType_U8,
                           &command->epsp60tablegetcmd.Payload.node);
        ImGui::InputScalar("tableId",  ImGuiDataType_U8,
                           &command->epsp60tablegetcmd.Payload.tableId);
        ImGui::InputScalar("rowCount", ImGuiDataType_U8,
                           &command->epsp60tablegetcmd.Payload.rowCount);
        ImGui::InputScalar("size",     ImGuiDataType_U16,
                           &command->epsp60tablegetcmd.Payload.size);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(EPS_P60_TableGetCmd_t) - 7)
            };

            memcpy(command->epsp60tablegetcmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60tablegetcmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60tablegetcmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60tablegetcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_TableGetCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->epsp60tablegetcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60tablegetcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(EPS_P60_TableGetCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_TableGetCmd_t);
            memcpy(pkt->Data, &command->epsp60tablegetcmd,
                   sizeof(EPS_P60_TableGetCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    // 41 : EPS_P60_LOAD_TABLE_CC
    case 41: {
        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_LOAD_TABLE_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("node",     ImGuiDataType_U8,
                           &command->epsp60tableloadcmd.Payload.node);
        ImGui::InputScalar("tableId",  ImGuiDataType_U8,
                           &command->epsp60tableloadcmd.Payload.tableId);
        ImGui::InputScalar("from",     ImGuiDataType_U8,
                           &command->epsp60tableloadcmd.Payload.from);
        ImGui::InputScalar("timeout",  ImGuiDataType_U16,
                           &command->epsp60tableloadcmd.Payload.timeout);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(EPS_P60_TableLoadCmd_t) - 7)
            };

            memcpy(command->epsp60tableloadcmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60tableloadcmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60tableloadcmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60tableloadcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_TableLoadCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->epsp60tableloadcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60tableloadcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(EPS_P60_TableLoadCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_TableLoadCmd_t);
            memcpy(pkt->Data, &command->epsp60tableloadcmd,
                   sizeof(EPS_P60_TableLoadCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    // 42 : EPS_P60_SAVE_TABLE_CC
    case 42: {
        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_SAVE_TABLE_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("node",     ImGuiDataType_U8,
                           &command->epsp60tablesavecmd.Payload.node);
        ImGui::InputScalar("tableId",  ImGuiDataType_U8,
                           &command->epsp60tablesavecmd.Payload.tableId);
        ImGui::InputScalar("to",       ImGuiDataType_U8,
                           &command->epsp60tablesavecmd.Payload.to);
        ImGui::InputScalar("timeout",  ImGuiDataType_U16,
                           &command->epsp60tablesavecmd.Payload.timeout);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(EPS_P60_TableSaveCmd_t) - 7)
            };

            memcpy(command->epsp60tablesavecmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60tablesavecmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60tablesavecmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60tablesavecmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_TableSaveCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->epsp60tablesavecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60tablesavecmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(EPS_P60_TableSaveCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_TableSaveCmd_t);
            memcpy(pkt->Data, &command->epsp60tablesavecmd,
                   sizeof(EPS_P60_TableSaveCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    // 43 : EPS_P60_TABLE_GET_STATIC_CC
    case 43: {
        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_TABLE_GET_STATIC_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("node",     ImGuiDataType_U8,
                           &command->epsp60tablegetstaticcmd.Payload.node);
        ImGui::InputScalar("tableId",  ImGuiDataType_U8,
                           &command->epsp60tablegetstaticcmd.Payload.tableId);
        ImGui::InputScalar("rowCount", ImGuiDataType_U8,
                           &command->epsp60tablegetstaticcmd.Payload.rowCount);
        ImGui::InputScalar("size",     ImGuiDataType_U16,
                           &command->epsp60tablegetstaticcmd.Payload.size);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(EPS_P60_TableGetStaticCmd_t) - 7)
            };

            memcpy(command->epsp60tablegetstaticcmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60tablegetstaticcmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60tablegetstaticcmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60tablegetstaticcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_TableGetStaticCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->epsp60tablegetstaticcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60tablegetstaticcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(EPS_P60_TableGetStaticCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_TableGetStaticCmd_t);
            memcpy(pkt->Data, &command->epsp60tablegetstaticcmd,
                   sizeof(EPS_P60_TableGetStaticCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    // 44 : EPS_P60_TABLE_DUMP_STATIC_CC
    case 44: {
        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_TABLE_DUMP_STATIC_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("size",    ImGuiDataType_U16,
                           &command->epsp60tabledumpstaticcmd.Payload.size);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(EPS_P60_TableDumpStaticCmd_t) - 7)
            };

            memcpy(command->epsp60tabledumpstaticcmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60tabledumpstaticcmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60tabledumpstaticcmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60tabledumpstaticcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_TableDumpStaticCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->epsp60tabledumpstaticcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60tabledumpstaticcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(EPS_P60_TableDumpStaticCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_TableDumpStaticCmd_t);
            memcpy(pkt->Data, &command->epsp60tabledumpstaticcmd,
                   sizeof(EPS_P60_TableDumpStaticCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    // 45 : EPS_P60_GET_DOCK_INFO_CC (No-Arg)
    case 45: {
        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_GET_DOCK_INFO_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(EPS_P60_GetDockInfoCmd_t) - 7)
            };

            memcpy(command->epsp60getdockinfocmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60getdockinfocmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60getdockinfocmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60getdockinfocmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_GetDockInfoCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->epsp60getdockinfocmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60getdockinfocmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(EPS_P60_GetDockInfoCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_GetDockInfoCmd_t);
            memcpy(pkt->Data, &command->epsp60getdockinfocmd,
                   sizeof(EPS_P60_GetDockInfoCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    // 46 : EPS_P60_RESET_CC (No-Arg)
    case 46: {
        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_RESET_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(EPS_P60_ResetCmd_t) - 7)
            };

            memcpy(command->epsp60resetcmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60resetcmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60resetcmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60resetcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_ResetCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->epsp60resetcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60resetcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(EPS_P60_ResetCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_ResetCmd_t);
            memcpy(pkt->Data, &command->epsp60resetcmd,
                   sizeof(EPS_P60_ResetCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }


        /********************************************************************/
    /*                       ADCS Commands (47~)                        */
    /********************************************************************/

    /* 47 : ADCS NoOp (CC 0) */
    case 47: {
        static uint16_t msgid   = ADCS_CMD_ID;      // 0x1865
        static uint8_t  fnccode = ADCS_NOOP_CC;     // 0

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid  = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(ADCS_NoopCmd_t) - 7};

            memcpy(command->adcsnoopcmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsnoopcmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsnoopcmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsnoopcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_NoopCmd_t);
            const uint8_t *p = (const uint8_t *)&command->adcsnoopcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsnoopcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(ADCS_NoopCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_NoopCmd_t);
            memcpy(pkt->Data, &command->adcsnoopcmd,
                   sizeof(ADCS_NoopCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 48 : ADCS Reset Counters (CC 1) */
    case 48: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_RESET_COUNTERS_CC;   // 1

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid  = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(ADCS_ResetCountersCmd_t) - 7};

            memcpy(command->adcsresetcounterscmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsresetcounterscmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsresetcounterscmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsresetcounterscmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_ResetCountersCmd_t);
            const uint8_t *p = (const uint8_t *)&command->adcsresetcounterscmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsresetcounterscmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(ADCS_ResetCountersCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_ResetCountersCmd_t);
            memcpy(pkt->Data, &command->adcsresetcounterscmd,
                   sizeof(ADCS_ResetCountersCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 49 : ADCS Reset App Cmd Counters (CC 2, no-arg로 사용) */
    case 49: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_RESET_APP_CMD_COUNTERS_CC;   // 2

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid  = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            /* no-arg 구조체: 헤더만 있는 타입으로 정의했다고 가정
               typedef struct { uint8_t CmdHeader[CFE_SB_CMD_HDR_SIZE]; } ADCS_ResetAppCmdCountersCmd_t;
            */
            uint8_t  len[2] = {0x00, sizeof(ADCS_ResetAppCmdCountersCmd_t) - 7};

            memcpy(command->adcsresetappcmdcounterscmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsresetappcmdcounterscmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsresetappcmdcounterscmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsresetappcmdcounterscmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_ResetAppCmdCountersCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->adcsresetappcmdcounterscmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsresetappcmdcounterscmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(ADCS_ResetAppCmdCountersCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_ResetAppCmdCountersCmd_t);
            memcpy(pkt->Data, &command->adcsresetappcmdcounterscmd,
                   sizeof(ADCS_ResetAppCmdCountersCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 50 : ADCS Reset Device Cmd Counters (CC 3, no-arg) */
    case 50: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_RESET_DEVICE_CMD_COUNTERS_CC;   // 3

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid  = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            /* no-arg 구조체:
               typedef struct { uint8_t CmdHeader[CFE_SB_CMD_HDR_SIZE]; } ADCS_ResetDeviceCmdCountersCmd_t;
            */
            uint8_t  len[2] = {0x00, sizeof(ADCS_ResetDeviceCmdCountersCmd_t) - 7};

            memcpy(command->adcsresetdevicecmdcounterscmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsresetdevicecmdcounterscmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsresetdevicecmdcounterscmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsresetdevicecmdcounterscmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_ResetDeviceCmdCountersCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->adcsresetdevicecmdcounterscmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsresetdevicecmdcounterscmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(ADCS_ResetDeviceCmdCountersCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_ResetDeviceCmdCountersCmd_t);
            memcpy(pkt->Data, &command->adcsresetdevicecmdcounterscmd,
                   sizeof(ADCS_ResetDeviceCmdCountersCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 51 : ADCS Set Communication Mode as CAN (CC 4, no-arg) */
    case 51: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_SET_COMMUNICATION_MODE_AS_CAN_CC;   // 4

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid  = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};

            /* no-arg struct:
               typedef struct { uint8_t CmdHeader[CFE_SB_CMD_HDR_SIZE]; } ADCS_SetCommunicationModeAsCanCmd_t;
            */
            uint8_t  len[2] = {0x00, sizeof(ADCS_SetCommunicationModeAsCanCmd_t) - 7};

            memcpy(command->adcssetcommunicationmodeascan.CmdHeader,     &mid, 2);
            memcpy(command->adcssetcommunicationmodeascan.CmdHeader + 2, seq,  2);
            memcpy(command->adcssetcommunicationmodeascan.CmdHeader + 4, len,  2);
            memcpy(command->adcssetcommunicationmodeascan.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_SetCommunicationModeAsCanCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->adcssetcommunicationmodeascan;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcssetcommunicationmodeascan.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(ADCS_SetCommunicationModeAsCanCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_SetCommunicationModeAsCanCmd_t);
            memcpy(pkt->Data, &command->adcssetcommunicationmodeascan,
                   sizeof(ADCS_SetCommunicationModeAsCanCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 52 : ADCS GPIO Enable High (CC 5, no-arg) */
    case 52: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_GPIO_ENABLE_HIGH_CC;   // 5

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(ADCS_GpioEnHighCmd_t) - 7};

            memcpy(command->adcsgpioenhighcmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsgpioenhighcmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsgpioenhighcmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsgpioenhighcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_GpioEnHighCmd_t);
            const uint8_t *p = (const uint8_t *)&command->adcsgpioenhighcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsgpioenhighcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(ADCS_GpioEnHighCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_GpioEnHighCmd_t);
            memcpy(pkt->Data, &command->adcsgpioenhighcmd,
                   sizeof(ADCS_GpioEnHighCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 53 : ADCS GPIO Enable Low (CC 6, no-arg) */
    case 53: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_GPIO_ENABLE_LOW_CC;   // 6

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(ADCS_GpioEnLowCmd_t) - 7};

            memcpy(command->adcsgpioenlowcmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsgpioenlowcmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsgpioenlowcmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsgpioenlowcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_GpioEnLowCmd_t);
            const uint8_t *p = (const uint8_t *)&command->adcsgpioenlowcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsgpioenlowcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(ADCS_GpioEnLowCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_GpioEnLowCmd_t);
            memcpy(pkt->Data, &command->adcsgpioenlowcmd,
                   sizeof(ADCS_GpioEnLowCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 54 : ADCS GPIO Boot High (CC 8, no-arg) */
    case 54: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_GPIO_BOOT_HIGH_CC;   // 8

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(ADCS_GpioBootHighCmd_t) - 7};

            memcpy(command->adcsgioboothighcmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsgioboothighcmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsgioboothighcmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsgioboothighcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_GpioBootHighCmd_t);
            const uint8_t *p = (const uint8_t *)&command->adcsgioboothighcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsgioboothighcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(ADCS_GpioBootHighCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_GpioBootHighCmd_t);
            memcpy(pkt->Data, &command->adcsgioboothighcmd,
                   sizeof(ADCS_GpioBootHighCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 55 : ADCS GPIO Boot Low (CC 9, no-arg) */
    case 55: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_GPIO_BOOT_LOW_CC;   // 9

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(ADCS_GpioBootLowCmd_t) - 7};

            memcpy(command->adcsgiobootlowcmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsgiobootlowcmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsgiobootlowcmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsgiobootlowcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_GpioBootLowCmd_t);
            const uint8_t *p = (const uint8_t *)&command->adcsgiobootlowcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsgiobootlowcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(ADCS_GpioBootLowCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_GpioBootLowCmd_t);
            memcpy(pkt->Data, &command->adcsgiobootlowcmd,
                   sizeof(ADCS_GpioBootLowCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 56 : ADCS Exit Bootloader (CC 10, no-arg) */
    case 56: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_EXIT_BOOTLOADER_CC;   // 10

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(ADCS_ExitBootLoaderCmd_t) - 7};

            memcpy(command->adcsexitbootloadercmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsexitbootloadercmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsexitbootloadercmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsexitbootloadercmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_ExitBootLoaderCmd_t);
            const uint8_t *p = (const uint8_t *)&command->adcsexitbootloadercmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsexitbootloadercmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(ADCS_ExitBootLoaderCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_ExitBootLoaderCmd_t);
            memcpy(pkt->Data, &command->adcsexitbootloadercmd,
                   sizeof(ADCS_ExitBootLoaderCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 57 : ADCS Reset (CC 11, no-arg) */
    case 57: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_SET_RESET_CC;   // 11

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(ADCS_ResetCmd_t) - 7};

            memcpy(command->adcsresetcmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsresetcmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsresetcmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsresetcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_ResetCmd_t);
            const uint8_t *p = (const uint8_t *)&command->adcsresetcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsresetcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(ADCS_ResetCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_ResetCmd_t);
            memcpy(pkt->Data, &command->adcsresetcmd,
                   sizeof(ADCS_ResetCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 58 : ADCS Set Current Unix Time (CC 12, payload) */
    case 58: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_SET_CURRENT_UNIX_TIME_CC;   // 12

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("Unix Seconds",     ImGuiDataType_U32,
                           &command->adcscurrentunixtimecmd.Payload.CurrentUnixseconds);
        ImGui::InputScalar("Unix Nanoseconds", ImGuiDataType_U32,
                           &command->adcscurrentunixtimecmd.Payload.CurrentUnixNanoseconds);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00,
                               sizeof(ADCS_CurrentUnixTimeCmd_t) - 7};

            memcpy(command->adcscurrentunixtimecmd.CmdHeader,     &mid, 2);
            memcpy(command->adcscurrentunixtimecmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcscurrentunixtimecmd.CmdHeader + 4, len,  2);
            memcpy(command->adcscurrentunixtimecmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_CurrentUnixTimeCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->adcscurrentunixtimecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcscurrentunixtimecmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(ADCS_CurrentUnixTimeCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_CurrentUnixTimeCmd_t);
            memcpy(pkt->Data, &command->adcscurrentunixtimecmd,
                   sizeof(ADCS_CurrentUnixTimeCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

/* 59 : ADCS Error Log Setting (CC 13, payload) */
case 59: {
    static uint16_t msgid   = ADCS_CMD_ID;
    static uint8_t  fnccode = ADCS_SET_ERROR_LOG_SETTING_CC;   // 13

    ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
    ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);


    uint8_t active_tmp =
        command->adcserrorlogsettingcmd.Payload.ActiveState ? 1 : 0;
    uint8_t buffer_tmp =
        command->adcserrorlogsettingcmd.Payload.BufferFullAction ? 1 : 0;

    ImGui::InputScalar("ActiveState",      ImGuiDataType_U8, &active_tmp);
    ImGui::InputScalar("BufferFullAction", ImGuiDataType_U8, &buffer_tmp);

    command->adcserrorlogsettingcmd.Payload.ActiveState      = active_tmp & 0x1;
    command->adcserrorlogsettingcmd.Payload.BufferFullAction = buffer_tmp & 0x1;

    if (ImGui::Button("Generate CMD")) {

        WriteSystemName(msgid);
        uint16_t mid = htons(msgid);
        uint8_t  seq[2] = {0xC0, 0x00};
        uint8_t  len[2] = {0x00,
                           sizeof(ADCS_ErrorLogSettingCmd_t) - 7};

        memcpy(command->adcserrorlogsettingcmd.CmdHeader,     &mid, 2);
        memcpy(command->adcserrorlogsettingcmd.CmdHeader + 2, seq,  2);
        memcpy(command->adcserrorlogsettingcmd.CmdHeader + 4, len,  2);
        memcpy(command->adcserrorlogsettingcmd.CmdHeader + 6, &fnccode, 1);

        uint16_t total = sizeof(ADCS_ErrorLogSettingCmd_t);
        const uint8_t *p =
            (const uint8_t *)&command->adcserrorlogsettingcmd;
        uint8_t crc = 0xFF;
        while (total--) crc ^= *(p++);

        memcpy(command->adcserrorlogsettingcmd.CmdHeader + 7, &crc, 1);

        packetsign *pkt =
            (packetsign *)malloc(2 + 2 + 4 +
                                 sizeof(ADCS_ErrorLogSettingCmd_t));
        pkt->Identifier = HVD_TEST;
        pkt->PacketType = MIM_PT_TMTC_TEST;
        pkt->Length     = sizeof(ADCS_ErrorLogSettingCmd_t);
        memcpy(pkt->Data, &command->adcserrorlogsettingcmd,
               sizeof(ADCS_ErrorLogSettingCmd_t));

        pthread_join(p_thread[4], NULL);
        pthread_create(&p_thread[4], NULL,
                       task_uplink_onorbit, (void *)pkt);
    }
    break;
}


    /* 60 : ADCS Persist Config (CC 14, no-arg) */
    case 60: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_SET_PERSIST_CONFIG_CC;   // 14

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00,
                               sizeof(ADCS_PersistConfigCmd_t) - 7};

            memcpy(command->adcspersistconfigcmd.CmdHeader,     &mid, 2);
            memcpy(command->adcspersistconfigcmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcspersistconfigcmd.CmdHeader + 4, len,  2);
            memcpy(command->adcspersistconfigcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_PersistConfigCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->adcspersistconfigcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcspersistconfigcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(ADCS_PersistConfigCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_PersistConfigCmd_t);
            memcpy(pkt->Data, &command->adcspersistconfigcmd,
                   sizeof(ADCS_PersistConfigCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 61 : ADCS Set Control Estimation Mode (CC 15) */
    case 61: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_SET_CONTROL_ESTIMATION_MODE_CC;   // 15

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("ControlMode",        ImGuiDataType_U8,
                           &command->adcscontrolestimationmodecmd.Payload.ControlMode);
        ImGui::InputScalar("MainEstimatorMode",  ImGuiDataType_U8,
                           &command->adcscontrolestimationmodecmd.Payload.MainEstimatorMode);
        ImGui::InputScalar("BackupEstimatorMode",ImGuiDataType_U8,
                           &command->adcscontrolestimationmodecmd.Payload.BackupEstimatorMode);
        ImGui::InputScalar("ControlTimeout",     ImGuiDataType_U16,
                           &command->adcscontrolestimationmodecmd.Payload.ControlTimeout);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid  = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00, (uint8_t)(sizeof(ADCS_ControlEstimationModeCmd_t) - 7)
            };

            memcpy(command->adcscontrolestimationmodecmd.CmdHeader,     &mid, 2);
            memcpy(command->adcscontrolestimationmodecmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcscontrolestimationmodecmd.CmdHeader + 4, len,  2);
            memcpy(command->adcscontrolestimationmodecmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_ControlEstimationModeCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->adcscontrolestimationmodecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcscontrolestimationmodecmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(
                2 + 2 + 4 + sizeof(ADCS_ControlEstimationModeCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_ControlEstimationModeCmd_t);
            memcpy(pkt->Data, &command->adcscontrolestimationmodecmd,
                   sizeof(ADCS_ControlEstimationModeCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 62 : ADCS Disable Mag/RWL/MntMng (CC 16) */
    case 62: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_SET_DISABLE_MAG_RWL_MNT_MNG_CC;   // 16

        ImGui::InputScalar("msgid",    ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode",  ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("Duration", ImGuiDataType_U16,
                           &command->adcsdisablemagrwlmntmngcmd.Payload.Duration);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid  = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00, (uint8_t)(sizeof(ADCS_DisableMagRwlMntMngCmd_t) - 7)
            };

            memcpy(command->adcsdisablemagrwlmntmngcmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsdisablemagrwlmntmngcmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsdisablemagrwlmntmngcmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsdisablemagrwlmntmngcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_DisableMagRwlMntMngCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->adcsdisablemagrwlmntmngcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsdisablemagrwlmntmngcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(
                2 + 2 + 4 + sizeof(ADCS_DisableMagRwlMntMngCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_DisableMagRwlMntMngCmd_t);
            memcpy(pkt->Data, &command->adcsdisablemagrwlmntmngcmd,
                   sizeof(ADCS_DisableMagRwlMntMngCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 63 : ADCS Set Reference IRC Vector (CC 17) */
    case 63: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_SET_REFERENCE_IRC_VECTOR_CC;   // 17

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("ECI X", ImGuiDataType_Float,
                           &command->adcsreferenceircvectorcmd.Payload.ECIPointingVectorX);
        ImGui::InputScalar("ECI Y", ImGuiDataType_Float,
                           &command->adcsreferenceircvectorcmd.Payload.ECIPointingVectorY);
        ImGui::InputScalar("ECI Z", ImGuiDataType_Float,
                           &command->adcsreferenceircvectorcmd.Payload.ECIPointingVectorZ);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid  = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00, (uint8_t)(sizeof(ADCS_ReferenceIRCVectorCmd_t) - 7)
            };

            memcpy(command->adcsreferenceircvectorcmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsreferenceircvectorcmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsreferenceircvectorcmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsreferenceircvectorcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_ReferenceIRCVectorCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->adcsreferenceircvectorcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsreferenceircvectorcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(
                2 + 2 + 4 + sizeof(ADCS_ReferenceIRCVectorCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_ReferenceIRCVectorCmd_t);
            memcpy(pkt->Data, &command->adcsreferenceircvectorcmd,
                   sizeof(ADCS_ReferenceIRCVectorCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 64 : ADCS Set Reference LLH Target (CC 18) */
    case 64: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_SET_REFERENCE_LLH_TARGET_CC;   // 18

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("Lat [deg]", ImGuiDataType_Float,
                           &command->adcsreferencellhtargetcmd.Payload.TargetLatitude);
        ImGui::InputScalar("Lon [deg]", ImGuiDataType_Float,
                           &command->adcsreferencellhtargetcmd.Payload.TargetLongiTude);
        ImGui::InputScalar("Alt [m]",   ImGuiDataType_Float,
                           &command->adcsreferencellhtargetcmd.Payload.TargetAltitude);

        if (ImGui::Button("Generate CMD")) {

            WriteSystemName(msgid);
            uint16_t mid  = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00, (uint8_t)(sizeof(ADCS_ReferenceLLHTargetCmd_t) - 7)
            };

            memcpy(command->adcsreferencellhtargetcmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsreferencellhtargetcmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsreferencellhtargetcmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsreferencellhtargetcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_ReferenceLLHTargetCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->adcsreferencellhtargetcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsreferencellhtargetcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(
                2 + 2 + 4 + sizeof(ADCS_ReferenceLLHTargetCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_ReferenceLLHTargetCmd_t);
            memcpy(pkt->Data, &command->adcsreferencellhtargetcmd,
                   sizeof(ADCS_ReferenceLLHTargetCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 65 : ADCS Set Orbit Mode (CC 19) */
    case 65: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_SET_ORBIT_MODE_CC;   // 19

        ImGui::InputScalar("msgid",    ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode",  ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("OrbitMode",ImGuiDataType_U8,
                           &command->adcsorbitmodecmd.Payload.OrbitMode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid  = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00, (uint8_t)(sizeof(ADCS_OrbitModeCmd_t) - 7)
            };

            memcpy(command->adcsorbitmodecmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsorbitmodecmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsorbitmodecmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsorbitmodecmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_OrbitModeCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->adcsorbitmodecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsorbitmodecmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(
                2 + 2 + 4 + sizeof(ADCS_OrbitModeCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_OrbitModeCmd_t);
            memcpy(pkt->Data, &command->adcsorbitmodecmd,
                   sizeof(ADCS_OrbitModeCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }
/* 66 : ADCS MAG Deploy Command (CC 20) */
case 66: {
    static uint16_t msgid   = ADCS_CMD_ID;
    static uint8_t  fnccode = ADCS_SET_MAG_DEPLOY_CMD_CC;   // 20

    static uint8_t deploy0 = 0;
    static uint8_t deploy1 = 0;


    deploy0 = command->adcsmagdeploycmd.Payload.DeployMAG0;
    deploy1 = command->adcsmagdeploycmd.Payload.DeployMAG1;

    ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
    ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

    ImGui::InputScalar("DeployMAG0", ImGuiDataType_U8, &deploy0);
    ImGui::InputScalar("DeployMAG1", ImGuiDataType_U8, &deploy1);

    if (ImGui::Button("Generate CMD")) {


        command->adcsmagdeploycmd.Payload.DeployMAG0 = deploy0 & 0x1;
        command->adcsmagdeploycmd.Payload.DeployMAG1 = deploy1 & 0x1;

        WriteSystemName(msgid);
        uint16_t mid  = htons(msgid);
        uint8_t  seq[2] = {0xC0, 0x00};
        uint8_t  len[2] = {
            0x00, (uint8_t)(sizeof(ADCS_MagDeployCmd_t) - 7)
        };

        memcpy(command->adcsmagdeploycmd.CmdHeader,     &mid, 2);
        memcpy(command->adcsmagdeploycmd.CmdHeader + 2, seq,  2);
        memcpy(command->adcsmagdeploycmd.CmdHeader + 4, len,  2);
        memcpy(command->adcsmagdeploycmd.CmdHeader + 6, &fnccode, 1);

        uint16_t total = sizeof(ADCS_MagDeployCmd_t);
        const uint8_t *p =
            (const uint8_t *)&command->adcsmagdeploycmd;
        uint8_t crc = 0xFF;
        while (total--) crc ^= *(p++);

        memcpy(command->adcsmagdeploycmd.CmdHeader + 7, &crc, 1);

        packetsign *pkt = (packetsign *)malloc(
            2 + 2 + 4 + sizeof(ADCS_MagDeployCmd_t));
        pkt->Identifier = HVD_TEST;
        pkt->PacketType = MIM_PT_TMTC_TEST;
        pkt->Length     = sizeof(ADCS_MagDeployCmd_t);
        memcpy(pkt->Data, &command->adcsmagdeploycmd,
               sizeof(ADCS_MagDeployCmd_t));

        pthread_join(p_thread[4], NULL);
        pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
    }
    break;
}

    /* 67 : ADCS Set Reference RPY Values (CC 21) */
    case 67: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_SET_REFERENCE_RPY_VALUES_CC;   // 21

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("Roll [deg]",  ImGuiDataType_Float,
                           &command->adcsreferencerpyvaluescmd.Payload.Roll);
        ImGui::InputScalar("Pitch [deg]", ImGuiDataType_Float,
                           &command->adcsreferencerpyvaluescmd.Payload.Pitch);
        ImGui::InputScalar("Yaw [deg]",   ImGuiDataType_Float,
                           &command->adcsreferencerpyvaluescmd.Payload.Yaw);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid  = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00, (uint8_t)(sizeof(ADCS_ReferenceRPYvaluesCmd_t) - 7)
            };

            memcpy(command->adcsreferencerpyvaluescmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsreferencerpyvaluescmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsreferencerpyvaluescmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsreferencerpyvaluescmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_ReferenceRPYvaluesCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->adcsreferencerpyvaluescmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsreferencerpyvaluescmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(
                2 + 2 + 4 + sizeof(ADCS_ReferenceRPYvaluesCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_ReferenceRPYvaluesCmd_t);
            memcpy(pkt->Data, &command->adcsreferencerpyvaluescmd,
                   sizeof(ADCS_ReferenceRPYvaluesCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 68 : ADCS Set Open Loop MTQ Command (CC 22) */
    case 68: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_SET_OPENLOOPCMD_MTQ_CC;   // 22

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("MTQ0 Cmd", ImGuiDataType_S16,
                           &command->adcsopenloopcmdmtqcmd.Payload.MTQ0_OpenLoopCmd);
        ImGui::InputScalar("MTQ1 Cmd", ImGuiDataType_S16,
                           &command->adcsopenloopcmdmtqcmd.Payload.MTQ1_OpenLoopCmd);
        ImGui::InputScalar("MTQ2 Cmd", ImGuiDataType_S16,
                           &command->adcsopenloopcmdmtqcmd.Payload.MTQ2_OpenLoopCmd);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid  = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00, (uint8_t)(sizeof(ADCS_OpenLoopCmdMTQCmd_t) - 7)
            };

            memcpy(command->adcsopenloopcmdmtqcmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsopenloopcmdmtqcmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsopenloopcmdmtqcmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsopenloopcmdmtqcmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_OpenLoopCmdMTQCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->adcsopenloopcmdmtqcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsopenloopcmdmtqcmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(
                2 + 2 + 4 + sizeof(ADCS_OpenLoopCmdMTQCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_OpenLoopCmdMTQCmd_t);
            memcpy(pkt->Data, &command->adcsopenloopcmdmtqcmd,
                   sizeof(ADCS_OpenLoopCmdMTQCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }


    case 69: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_SET_POWER_STATE_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputScalar("RWL0", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.RWL0);
        ImGui::InputScalar("RWL1", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.RWL1);
        ImGui::InputScalar("RWL2", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.RWL2);
        ImGui::InputScalar("RWL3", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.RWL3);

        ImGui::InputScalar("MAG0", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.MAG0);
        ImGui::InputScalar("MAG1", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.MAG1);

        ImGui::InputScalar("GYR0", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.GYR0);
        ImGui::InputScalar("GYR1", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.GYR1);

        ImGui::InputScalar("FSS0", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.FSS0);
        ImGui::InputScalar("FSS1", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.FSS1);
        ImGui::InputScalar("FSS2", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.FSS2);
        ImGui::InputScalar("FSS3", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.FSS3);

        ImGui::InputScalar("HSS0", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.HSS0);
        ImGui::InputScalar("HSS1", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.HSS1);

        ImGui::InputScalar("STR0", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.STR0);
        ImGui::InputScalar("STR1", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.STR1);

        ImGui::InputScalar("ExtSensor0", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.ExtSensor0);
        ImGui::InputScalar("ExtSensor1", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.ExtSensor1);

        ImGui::InputScalar("ExtGYR0", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.ExtGYR0);
        ImGui::InputScalar("ExtGYR1", ImGuiDataType_U8,
                           &command->adcspowerstatecmd.Payload.ExtGYR1);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid  = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00, (uint8_t)(sizeof(ADCS_PowerStateCmd_t) - 7)
            };

            memcpy(command->adcspowerstatecmd.CmdHeader,     &mid, 2);
            memcpy(command->adcspowerstatecmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcspowerstatecmd.CmdHeader + 4, len,  2);
            memcpy(command->adcspowerstatecmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_PowerStateCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->adcspowerstatecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcspowerstatecmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(
                2 + 2 + 4 + sizeof(ADCS_PowerStateCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_PowerStateCmd_t);
            memcpy(pkt->Data, &command->adcspowerstatecmd,
                   sizeof(ADCS_PowerStateCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }







    /* 70 : ADCS Set Run Mode (CC 24) */
    case 70: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_SET_RUN_MODE_CC;   // 24

        ImGui::InputScalar("msgid",    ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode",  ImGuiDataType_U8,  &fnccode);
        ImGui::InputScalar("RunMode",  ImGuiDataType_U8,
                           &command->adcsrunmodecmd.Payload.RunMode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid  = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00, (uint8_t)(sizeof(ADCS_RunModeCmd_t) - 7)
            };

            memcpy(command->adcsrunmodecmd.CmdHeader,     &mid, 2);
            memcpy(command->adcsrunmodecmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcsrunmodecmd.CmdHeader + 4, len,  2);
            memcpy(command->adcsrunmodecmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_RunModeCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->adcsrunmodecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcsrunmodecmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(
                2 + 2 + 4 + sizeof(ADCS_RunModeCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_RunModeCmd_t);
            memcpy(pkt->Data, &command->adcsrunmodecmd,
                   sizeof(ADCS_RunModeCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    /* 71 : ADCS Set Control Mode (CC 25) */
    case 71: {
        static uint16_t msgid   = ADCS_CMD_ID;
        static uint8_t  fnccode = ADCS_SET_CONTROL_MODE_CC;   // 25

        ImGui::InputScalar("msgid",        ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode",      ImGuiDataType_U8,  &fnccode);
        ImGui::InputScalar("ControlMode",  ImGuiDataType_U8,
                           &command->adcscontrolmodecmd.Payload.ControlMode);
        ImGui::InputScalar("ControlTimeout", ImGuiDataType_U16,
                           &command->adcscontrolmodecmd.Payload.Controltimeout);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid  = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00, (uint8_t)(sizeof(ADCS_ControlModeCmd_t) - 7)
            };

            memcpy(command->adcscontrolmodecmd.CmdHeader,     &mid, 2);
            memcpy(command->adcscontrolmodecmd.CmdHeader + 2, seq,  2);
            memcpy(command->adcscontrolmodecmd.CmdHeader + 4, len,  2);
            memcpy(command->adcscontrolmodecmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(ADCS_ControlModeCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->adcscontrolmodecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->adcscontrolmodecmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt = (packetsign *)malloc(
                2 + 2 + 4 + sizeof(ADCS_ControlModeCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(ADCS_ControlModeCmd_t);
            memcpy(pkt->Data, &command->adcscontrolmodecmd,
                   sizeof(ADCS_ControlModeCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }



    // case 72: {
    //     static uint16_t msgid   = ADCS_CMD_ID;
    //     static uint8_t  fnccode = ADCS_SET_WHL_CONFIG_CC;

    //     ImGui::InputScalar("msgid",   ImGuiDataType_U16,  &msgid);
    //     ImGui::InputScalar("fnccode", ImGuiDataType_U8,   &fnccode);

    //     ImGui::InputScalar("Rwl0Inertia",      ImGuiDataType_Float,
    //                        &command->adcswhlconfigcmd.Payload.Rwl0Inertia);
    //     ImGui::InputScalar("Rwl0MaxMomentum",  ImGuiDataType_Float,
    //                        &command->adcswhlconfigcmd.Payload.Rwl0MaxMomentum);
    //     ImGui::InputScalar("Rwl0MaxToque",     ImGuiDataType_Float,
    //                        &command->adcswhlconfigcmd.Payload.Rwl0MaxToque);

    //     ImGui::InputScalar("Rwl1Inertia",      ImGuiDataType_Float,
    //                        &command->adcswhlconfigcmd.Payload.Rwl1Inertia);
    //     ImGui::InputScalar("Rwl1MaxMomentum",  ImGuiDataType_Float,
    //                        &command->adcswhlconfigcmd.Payload.Rwl1MaxMomentum);
    //     ImGui::InputScalar("Rwl1MaxToque",     ImGuiDataType_Float,
    //                        &command->adcswhlconfigcmd.Payload.Rwl1MaxToque);

    //     ImGui::InputScalar("Rwl2Inertia",      ImGuiDataType_Float,
    //                        &command->adcswhlconfigcmd.Payload.Rwl2Inertia);
    //     ImGui::InputScalar("Rwl2MaxMomentum",  ImGuiDataType_Float,
    //                        &command->adcswhlconfigcmd.Payload.Rwl2MaxMomentum);
    //     ImGui::InputScalar("Rwl2MaxToque",     ImGuiDataType_Float,
    //                        &command->adcswhlconfigcmd.Payload.Rwl2MaxToque);

    //     ImGui::InputScalar("Rwl3Inertia",      ImGuiDataType_Float,
    //                        &command->adcswhlconfigcmd.Payload.Rwl3Inertia);
    //     ImGui::InputScalar("Rwl3MaxMomentum",  ImGuiDataType_Float,
    //                        &command->adcswhlconfigcmd.Payload.Rwl3MaxMomentum);
    //     ImGui::InputScalar("Rwl3MaxToque",     ImGuiDataType_Float,
    //                        &command->adcswhlconfigcmd.Payload.Rwl3MaxToque);

    //     ImGui::InputScalar("WheelRampTorque",  ImGuiDataType_Float,
    //                        &command->adcswhlconfigcmd.Payload.WheelRampTorque);

    //     ImGui::InputScalar("WheelScheme",      ImGuiDataType_U8,
    //                        &command->adcswhlconfigcmd.Payload.WheelScheme);
    //     ImGui::InputScalar("FailedWheelID",    ImGuiDataType_U8,
    //                        &command->adcswhlconfigcmd.Payload.FailedWheelID);

    //     ImGui::InputScalar("PyramidNominalMomentum",
    //                        ImGuiDataType_Float,
    //                        &command->adcswhlconfigcmd.Payload.PyramidNominalMomentum);
    //     ImGui::InputScalar("PyramidTiltAngle",
    //                        ImGuiDataType_Float,
    //                        &command->adcswhlconfigcmd.Payload.PyramidTiltAngle);

    //     if (ImGui::Button("Generate CMD")) {
    //         WriteSystemName(msgid);
    //         uint16_t mid  = htons(msgid);
    //         uint8_t  seq[2] = {0xC0, 0x00};
    //         uint8_t  len[2] = {
    //             0x00, (uint8_t)(sizeof(ADCS_WhlConfigCmd_t) - 7)
    //         };

    //         memcpy(command->adcswhlconfigcmd.CmdHeader,     &mid, 2);
    //         memcpy(command->adcswhlconfigcmd.CmdHeader + 2, seq,  2);
    //         memcpy(command->adcswhlconfigcmd.CmdHeader + 4, len,  2);
    //         memcpy(command->adcswhlconfigcmd.CmdHeader + 6, &fnccode, 1);

    //         uint16_t total = sizeof(ADCS_WhlConfigCmd_t);
    //         const uint8_t *p =
    //             (const uint8_t *)&command->adcswhlconfigcmd;
    //         uint8_t crc = 0xFF;
    //         while (total--) crc ^= *(p++);

    //         memcpy(command->adcswhlconfigcmd.CmdHeader + 7, &crc, 1);

    //         packetsign *pkt = (packetsign *)malloc(
    //             2 + 2 + 4 + sizeof(ADCS_WhlConfigCmd_t));
    //         pkt->Identifier = HVD_TEST;
    //         pkt->PacketType = MIM_PT_TMTC_TEST;
    //         pkt->Length     = sizeof(ADCS_WhlConfigCmd_t);
    //         memcpy(pkt->Data, &command->adcswhlconfigcmd,
    //                sizeof(ADCS_WhlConfigCmd_t));

    //         pthread_join(p_thread[4], NULL);
    //         pthread_create(&p_thread[4], NULL,
    //                        task_uplink_onorbit, (void *)pkt);
    //     }
    //     break;
    // }
        case 72: {

            static const uint16_t msgid   = ADCS_CMD_ID;
            static const uint8_t  fnccode = ADCS_SET_WHL_CONFIG_CC;


            static bool initialized = false;
            if (!initialized) {
                command->adcswhlconfigcmd.Payload.Rwl0Inertia        = 9.51015E-06 ;
                command->adcswhlconfigcmd.Payload.Rwl0MaxMomentum    = 0.00996 ;
                command->adcswhlconfigcmd.Payload.Rwl0MaxToque       = 0.002 ;

                command->adcswhlconfigcmd.Payload.Rwl1Inertia        = 9.51015E-06;
                command->adcswhlconfigcmd.Payload.Rwl1MaxMomentum    = 0.00996;
                command->adcswhlconfigcmd.Payload.Rwl1MaxToque       = 0.002;

                command->adcswhlconfigcmd.Payload.Rwl2Inertia        = 9.51015E-06;
                command->adcswhlconfigcmd.Payload.Rwl2MaxMomentum    = 0.00996;
                command->adcswhlconfigcmd.Payload.Rwl2MaxToque       = 0.002;

                command->adcswhlconfigcmd.Payload.Rwl3Inertia        = 9.51015E-06;
                command->adcswhlconfigcmd.Payload.Rwl3MaxMomentum    = 0.00996;
                command->adcswhlconfigcmd.Payload.Rwl3MaxToque       = 0.002;

                command->adcswhlconfigcmd.Payload.WheelRampTorque    = 0.0002;

                command->adcswhlconfigcmd.Payload.WheelScheme        = 3;
                command->adcswhlconfigcmd.Payload.FailedWheelID      = 0;

                command->adcswhlconfigcmd.Payload.PyramidNominalMomentum = 0.001;
                command->adcswhlconfigcmd.Payload.PyramidTiltAngle       = 26.565;

                initialized = true;
            }


            ImGui::Text("ADCS Wheel Config (fixed parameters)");
            ImGui::Text("MsgID   : 0x%04X", msgid);
            ImGui::Text("FncCode : %u", fnccode);
            ImGui::Separator();

            ImGui::Text("=== ADCS Wheel Config Parameters (Fixed) ===");
            ImGui::Separator();

            ImGui::Text("Rwl0Inertia          : %.6f", command->adcswhlconfigcmd.Payload.Rwl0Inertia);
            ImGui::Text("Rwl0MaxMomentum      : %.6f", command->adcswhlconfigcmd.Payload.Rwl0MaxMomentum);
            ImGui::Text("Rwl0MaxTorque        : %.6f", command->adcswhlconfigcmd.Payload.Rwl0MaxToque);

            ImGui::Text("Rwl1Inertia          : %.6f", command->adcswhlconfigcmd.Payload.Rwl1Inertia);
            ImGui::Text("Rwl1MaxMomentum      : %.6f", command->adcswhlconfigcmd.Payload.Rwl1MaxMomentum);
            ImGui::Text("Rwl1MaxTorque        : %.6f", command->adcswhlconfigcmd.Payload.Rwl1MaxToque);

            ImGui::Text("Rwl2Inertia          : %.6f", command->adcswhlconfigcmd.Payload.Rwl2Inertia);
            ImGui::Text("Rwl2MaxMomentum      : %.6f", command->adcswhlconfigcmd.Payload.Rwl2MaxMomentum);
            ImGui::Text("Rwl2MaxTorque        : %.6f", command->adcswhlconfigcmd.Payload.Rwl2MaxToque);

            ImGui::Text("Rwl3Inertia          : %.6f", command->adcswhlconfigcmd.Payload.Rwl3Inertia);
            ImGui::Text("Rwl3MaxMomentum      : %.6f", command->adcswhlconfigcmd.Payload.Rwl3MaxMomentum);
            ImGui::Text("Rwl3MaxTorque        : %.6f", command->adcswhlconfigcmd.Payload.Rwl3MaxToque);

            ImGui::Text("WheelRampTorque      : %.6f", command->adcswhlconfigcmd.Payload.WheelRampTorque);

            ImGui::Text("WheelScheme          : %u", command->adcswhlconfigcmd.Payload.WheelScheme);
            ImGui::Text("FailedWheelID        : %u", command->adcswhlconfigcmd.Payload.FailedWheelID);

            ImGui::Text("PyramidNominalMomentum : %.6f", command->adcswhlconfigcmd.Payload.PyramidNominalMomentum);
            ImGui::Text("PyramidTiltAngle       : %.6f", command->adcswhlconfigcmd.Payload.PyramidTiltAngle);

            ImGui::Separator();


            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid  = htons(msgid);
                uint8_t  seq[2] = { 0xC0, 0x00 };
                uint8_t  len[2] = {
                    0x00, (uint8_t)(sizeof(ADCS_WhlConfigCmd_t) - 7)
                };

                memcpy(command->adcswhlconfigcmd.CmdHeader,     &mid, 2);
                memcpy(command->adcswhlconfigcmd.CmdHeader + 2, seq,  2);
                memcpy(command->adcswhlconfigcmd.CmdHeader + 4, len,  2);
                memcpy(command->adcswhlconfigcmd.CmdHeader + 6, &fnccode, 1);

                command->adcswhlconfigcmd.CmdHeader[7] = 0;
                uint16_t total = sizeof(ADCS_WhlConfigCmd_t);
                const uint8_t *p =
                    (const uint8_t *)&command->adcswhlconfigcmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->adcswhlconfigcmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt = (packetsign *)malloc(
                    2 + 2 + 4 + sizeof(ADCS_WhlConfigCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(ADCS_WhlConfigCmd_t);
                memcpy(pkt->Data, &command->adcswhlconfigcmd,
                    sizeof(ADCS_WhlConfigCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }


        case 73: {
            static const uint16_t msgid   = ADCS_CMD_ID;
            static const uint8_t  fnccode = ADCS_SET_SATELLITE_CONFIG_CC;

            static bool initialized = false;
            if (!initialized) {
                ADCS_SatConfigCmd_Payload_t &p = command->adcssatconfigcmd.Payload;

                p.Ixx = 0.2252612;
                p.Iyy = 0.2369665;
                p.Izz = 0.1360058;
                p.Ixy = -0.0001218231;
                p.Ixz = 0.0002665824;
                p.Iyz = 0.001189326;

                p.SunPointingBodyVectorX = 0;
                p.SunPointingBodyVectorY = 0;
                p.SunPointingBodyVectorZ = -10000;

                p.TargetTrackingBodyVectorX = 0;
                p.TargetTrackingBodyVectorY = 0;
                p.TargetTrackingBodyVectorZ = 10000;

                p.SatTrackingBodyVectorX = 0;
                p.SatTrackingBodyVectorY = 0;
                p.SatTrackingBodyVectorZ = -10000;

                initialized = true;
            }

            ImGui::Text("MsgID   : 0x%04X", msgid);
            ImGui::Text("FncCode : %u",     fnccode);
            ImGui::Separator();

            ADCS_SatConfigCmd_Payload_t &p = command->adcssatconfigcmd.Payload;

            ImGui::Text("=== Satellite Inertia Matrix ===");
            ImGui::Text("Ixx : %.6f", p.Ixx);
            ImGui::Text("Iyy : %.6f", p.Iyy);
            ImGui::Text("Izz : %.6f", p.Izz);
            ImGui::Text("Ixy : %.6f", p.Ixy);
            ImGui::Text("Ixz : %.6f", p.Ixz);
            ImGui::Text("Iyz : %.6f", p.Iyz);

            ImGui::Separator();
            ImGui::Text("=== Sun Pointing Body Vector ===");
            ImGui::Text("X : %d", p.SunPointingBodyVectorX);
            ImGui::Text("Y : %d", p.SunPointingBodyVectorY);
            ImGui::Text("Z : %d", p.SunPointingBodyVectorZ);

            ImGui::Separator();
            ImGui::Text("=== Target Tracking Body Vector ===");
            ImGui::Text("X : %d", p.TargetTrackingBodyVectorX);
            ImGui::Text("Y : %d", p.TargetTrackingBodyVectorY);
            ImGui::Text("Z : %d", p.TargetTrackingBodyVectorZ);

            ImGui::Separator();
            ImGui::Text("=== Sat Tracking Body Vector ===");
            ImGui::Text("X : %d", p.SatTrackingBodyVectorX);
            ImGui::Text("Y : %d", p.SatTrackingBodyVectorY);
            ImGui::Text("Z : %d", p.SatTrackingBodyVectorZ);

            ImGui::Separator();

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid     = htons(msgid);
                uint8_t  seq[2]  = { 0xC0, 0x00 };
                uint8_t  len[2]  = {
                    0x00,
                    (uint8_t)(sizeof(ADCS_SatConfigCmd_t) - 7) 
                };

                memcpy(command->adcssatconfigcmd.CmdHeader,      &mid, 2);
                memcpy(command->adcssatconfigcmd.CmdHeader + 2,  seq,  2);
                memcpy(command->adcssatconfigcmd.CmdHeader + 4,  len,  2);
                memcpy(command->adcssatconfigcmd.CmdHeader + 6,  &fnccode, 1);


                command->adcssatconfigcmd.CmdHeader[7] = 0;
                uint16_t total = sizeof(ADCS_SatConfigCmd_t);
                const uint8_t *pp =
                    (const uint8_t *)&command->adcssatconfigcmd;
                uint8_t crc = 0xFF;
                while (total--) {
                    crc ^= *(pp++);
                }
                memcpy(command->adcssatconfigcmd.CmdHeader + 7, &crc, 1);


                packetsign *pkt = (packetsign *)malloc(
                    2 + 2 + 4 + sizeof(ADCS_SatConfigCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(ADCS_SatConfigCmd_t);
                memcpy(pkt->Data,
                    &command->adcssatconfigcmd,
                    sizeof(ADCS_SatConfigCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }

            break;
        }

        case 74: {
            static const uint16_t msgid   = ADCS_CMD_ID;
            static const uint8_t  fnccode = ADCS_SET_CONTROLLER_CONFIG_CC;

            static bool initialized = false;
            if (!initialized) {
                ADCS_ControllerConfig_Payload_t &p = command->adcscontrollerconfigcmd.Payload;

                p.DefaultControlMode             = 0;

                p.DetumblingDampingGain          = 0.1;
                p.SunSpinGainSunlit              = -0.2;
                p.SunSpinGainEclipse             = 0.02;
                p.DetumblingSpinGain             = 0.05;
                p.FastBDotGain                   = 1;
                p.YMomentumNutationDampingGain   = 0.5;
                p.YMomentumQuatGain              = 0.4;
                p.XAxisGGQuatGain                = 1;
                p.YAxisGGQuatGain                = 1;
                p.ZAxisGGQuatGain                = 1;
                p.WheelDesaturationGain          = 0.2;
                p.YMomentumProportionalGain      = 0.005;
                p.YMomentumDerivativeGain        = 0.048;
                p.RWheelProportionalGain         = 0.02;
                p.RWheelDerivativeGain           = 0.096;
                p.TrackingProportionalGain       = 0.08;
                p.TrackingDerivativeGain         = 0.192;
                p.TrackingIntegralGain           = 0.0004;
                p.ReferenceSpinRate              = -1;
                p.ReferenceWheelMomentum         = -0.004;
                p.YWheelBiasMomentum             = 0;
                p.ReferenceSpinRateRWspinControl = 0;
                p.SunKeepOutAngle                = 0;
                p.RollLimitAngle                 = 0;

                p.flags.YawCompensationForEarthRotation = 0;
                p.flags.EnableSunTrackingInEclipse      = 1;
                p.flags.EnableSunAvoidance              = 0;
                p.flags.Reserved                        = 0;

                initialized = true;
            }

            ImGui::Text("MsgID   : 0x%04X", msgid);
            ImGui::Text("FncCode : %u",     fnccode);
            ImGui::Separator();

            ADCS_ControllerConfig_Payload_t &p = command->adcscontrollerconfigcmd.Payload;

            ImGui::Text("=== Default Control Mode ===");
            ImGui::Text("DefaultControlMode : %u", p.DefaultControlMode);

            ImGui::Separator();
            ImGui::Text("=== Detumbling / Spin / B-dot Gains ===");
            ImGui::Text("DetumblingDampingGain        : %.6f", p.DetumblingDampingGain);
            ImGui::Text("SunSpinGainSunlit            : %.6f", p.SunSpinGainSunlit);
            ImGui::Text("SunSpinGainEclipse           : %.6f", p.SunSpinGainEclipse);
            ImGui::Text("DetumblingSpinGain           : %.6f", p.DetumblingSpinGain);
            ImGui::Text("FastBDotGain                 : %.6f", p.FastBDotGain);

            ImGui::Separator();
            ImGui::Text("=== Y-Momentum / Quat / GG Gains ===");
            ImGui::Text("YMomentumNutationDampingGain : %.6f", p.YMomentumNutationDampingGain);
            ImGui::Text("YMomentumQuatGain            : %.6f", p.YMomentumQuatGain);
            ImGui::Text("XAxisGGQuatGain              : %.6f", p.XAxisGGQuatGain);
            ImGui::Text("YAxisGGQuatGain              : %.6f", p.YAxisGGQuatGain);
            ImGui::Text("ZAxisGGQuatGain              : %.6f", p.ZAxisGGQuatGain);
            ImGui::Text("WheelDesaturationGain        : %.6f", p.WheelDesaturationGain);

            ImGui::Separator();
            ImGui::Text("=== Wheel / Tracking Gains ===");
            ImGui::Text("YMomentumProportionalGain    : %.6f", p.YMomentumProportionalGain);
            ImGui::Text("YMomentumDerivativeGain      : %.6f", p.YMomentumDerivativeGain);
            ImGui::Text("RWheelProportionalGain       : %.6f", p.RWheelProportionalGain);
            ImGui::Text("RWheelDerivativeGain         : %.6f", p.RWheelDerivativeGain);
            ImGui::Text("TrackingProportionalGain     : %.6f", p.TrackingProportionalGain);
            ImGui::Text("TrackingDerivativeGain       : %.6f", p.TrackingDerivativeGain);
            ImGui::Text("TrackingIntegralGain         : %.6f", p.TrackingIntegralGain);

            ImGui::Separator();
            ImGui::Text("=== Reference Values ===");
            ImGui::Text("ReferenceSpinRate              [deg/s] : %.6f", p.ReferenceSpinRate);
            ImGui::Text("ReferenceWheelMomentum         [Nms]   : %.6f", p.ReferenceWheelMomentum);
            ImGui::Text("YWheelBiasMomentum             [Nms]   : %.6f", p.YWheelBiasMomentum);
            ImGui::Text("ReferenceSpinRateRWspinControl [deg/s] : %.6f", p.ReferenceSpinRateRWspinControl);
            ImGui::Text("SunKeepOutAngle                [deg]   : %.6f", p.SunKeepOutAngle);
            ImGui::Text("RollLimitAngle                 [deg]   : %.6f", p.RollLimitAngle);

            ImGui::Separator();
            ImGui::Text("=== Flags ===");
            ImGui::Text("YawCompensationForEarthRotation : %u", p.flags.YawCompensationForEarthRotation);
            ImGui::Text("EnableSunTrackingInEclipse      : %u", p.flags.EnableSunTrackingInEclipse);
            ImGui::Text("EnableSunAvoidance              : %u", p.flags.EnableSunAvoidance);
            ImGui::Text("Reserved                        : %u", p.flags.Reserved);

            ImGui::Separator();

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid    = htons(msgid);
                uint8_t  seq[2] = { 0xC0, 0x00 };
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(ADCS_ControllerConfig_t) - 7)
                };

                memcpy(command->adcscontrollerconfigcmd.CmdHeader,      &mid, 2);
                memcpy(command->adcscontrollerconfigcmd.CmdHeader + 2,  seq,  2);
                memcpy(command->adcscontrollerconfigcmd.CmdHeader + 4,  len,  2);
                memcpy(command->adcscontrollerconfigcmd.CmdHeader + 6,  &fnccode, 1);

                command->adcscontrollerconfigcmd.CmdHeader[7] = 0;
                uint16_t total = sizeof(ADCS_ControllerConfig_t);
                const uint8_t *pp =
                    (const uint8_t *)&command->adcscontrollerconfigcmd;
                uint8_t crc = 0xFF;
                while (total--) {
                    crc ^= *(pp++);
                }
                memcpy(command->adcscontrollerconfigcmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt = (packetsign *)malloc(
                    2 + 2 + 4 + sizeof(ADCS_ControllerConfig_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(ADCS_ControllerConfig_t);
                memcpy(pkt->Data,
                    &command->adcscontrollerconfigcmd,
                    sizeof(ADCS_ControllerConfig_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }

            break;
        }

        case 75: { // ADCS Set Mag0 MMT Calib Config (CC 29)
            static uint16_t msgid   = ADCS_CMD_ID;
            static uint8_t  fnccode = ADCS_SET_MAG0_MMT_CALIB_CONFIG_CC;

            static int16_t MMT_Ch1Offset = 0;
            static int16_t MMT_Ch2Offset = -4;
            static int16_t MMT_Ch3Offset = 7;

            static int16_t MMT_SensitivityMAT_S11 = 1059;
            static int16_t MMT_SensitivityMAT_S22 = 1055;
            static int16_t MMT_SensitivityMAT_S33 = 1003;

            static int16_t MMT_SensitivityMAT_S12 = -2;
            static int16_t MMT_SensitivityMAT_S13 = -3;
            static int16_t MMT_SensitivityMAT_S21 = -2;

            static int16_t MMT_SensitivityMAT_S23 = 8;
            static int16_t MMT_SensitivityMAT_S31 = -3;
            static int16_t MMT_SensitivityMAT_S32 = 8;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ADCS_Mag0MMTCalibConfigCmd_Payload_t *pl =
                &command->adcsmag0mmtcalibconfigcmd.Payload;

            ImGui::Separator();
            ImGui::Text("ADCS Mag0 MMT Calib Config Payload");

            ImGui::InputScalar("MMT_Ch1Offset", ImGuiDataType_S16, &MMT_Ch1Offset);
            ImGui::InputScalar("MMT_Ch2Offset", ImGuiDataType_S16, &MMT_Ch2Offset);
            ImGui::InputScalar("MMT_Ch3Offset", ImGuiDataType_S16, &MMT_Ch3Offset);

            ImGui::InputScalar("MMT_SensitivityMAT_S11", ImGuiDataType_S16, &MMT_SensitivityMAT_S11);
            ImGui::InputScalar("MMT_SensitivityMAT_S22", ImGuiDataType_S16, &MMT_SensitivityMAT_S22);
            ImGui::InputScalar("MMT_SensitivityMAT_S33", ImGuiDataType_S16, &MMT_SensitivityMAT_S33);

            ImGui::InputScalar("MMT_SensitivityMAT_S12", ImGuiDataType_S16, &MMT_SensitivityMAT_S12);
            ImGui::InputScalar("MMT_SensitivityMAT_S13", ImGuiDataType_S16, &MMT_SensitivityMAT_S13);
            ImGui::InputScalar("MMT_SensitivityMAT_S21", ImGuiDataType_S16, &MMT_SensitivityMAT_S21);

            ImGui::InputScalar("MMT_SensitivityMAT_S23", ImGuiDataType_S16, &MMT_SensitivityMAT_S23);
            ImGui::InputScalar("MMT_SensitivityMAT_S31", ImGuiDataType_S16, &MMT_SensitivityMAT_S31);
            ImGui::InputScalar("MMT_SensitivityMAT_S32", ImGuiDataType_S16, &MMT_SensitivityMAT_S32);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                pl->MMT_Ch1Offset = MMT_Ch1Offset;
                pl->MMT_Ch2Offset = MMT_Ch2Offset;
                pl->MMT_Ch3Offset = MMT_Ch3Offset;

                pl->MMT_SensitivityMAT_S11 = MMT_SensitivityMAT_S11;
                pl->MMT_SensitivityMAT_S22 = MMT_SensitivityMAT_S22;
                pl->MMT_SensitivityMAT_S33 = MMT_SensitivityMAT_S33;

                pl->MMT_SensitivityMAT_S12 = MMT_SensitivityMAT_S12;
                pl->MMT_SensitivityMAT_S13 = MMT_SensitivityMAT_S13;
                pl->MMT_SensitivityMAT_S21 = MMT_SensitivityMAT_S21;

                pl->MMT_SensitivityMAT_S23 = MMT_SensitivityMAT_S23;
                pl->MMT_SensitivityMAT_S31 = MMT_SensitivityMAT_S31;
                pl->MMT_SensitivityMAT_S32 = MMT_SensitivityMAT_S32;

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(ADCS_Mag0MMTCalibConfigCmd_t) - 7)
                };

                memcpy(command->adcsmag0mmtcalibconfigcmd.CmdHeader,     &mid, 2);
                memcpy(command->adcsmag0mmtcalibconfigcmd.CmdHeader + 2, seq,  2);
                memcpy(command->adcsmag0mmtcalibconfigcmd.CmdHeader + 4, len,  2);
                memcpy(command->adcsmag0mmtcalibconfigcmd.CmdHeader + 6, &fnccode, 1);

                command->adcsmag0mmtcalibconfigcmd.CmdHeader[7] = 0;
                uint16_t total = sizeof(ADCS_Mag0MMTCalibConfigCmd_t);
                const uint8_t *p = (const uint8_t *)&command->adcsmag0mmtcalibconfigcmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);
                command->adcsmag0mmtcalibconfigcmd.CmdHeader[7] = crc;

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(ADCS_Mag0MMTCalibConfigCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(ADCS_Mag0MMTCalibConfigCmd_t);
                memcpy(pkt->Data, &command->adcsmag0mmtcalibconfigcmd,
                    sizeof(ADCS_Mag0MMTCalibConfigCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
            }

            ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                        command->adcsmag0mmtcalibconfigcmd.CmdHeader[0],
                        command->adcsmag0mmtcalibconfigcmd.CmdHeader[1],
                        command->adcsmag0mmtcalibconfigcmd.CmdHeader[2],
                        command->adcsmag0mmtcalibconfigcmd.CmdHeader[3],
                        command->adcsmag0mmtcalibconfigcmd.CmdHeader[4],
                        command->adcsmag0mmtcalibconfigcmd.CmdHeader[5],
                        command->adcsmag0mmtcalibconfigcmd.CmdHeader[6],
                        command->adcsmag0mmtcalibconfigcmd.CmdHeader[7]);

            ImGui::Text("Payload (UI values):");
            ImGui::Text("Offsets: Ch1=%d, Ch2=%d, Ch3=%d",
                        (int)MMT_Ch1Offset, (int)MMT_Ch2Offset, (int)MMT_Ch3Offset);
            ImGui::Text("S11=%d S22=%d S33=%d",
                        (int)MMT_SensitivityMAT_S11, (int)MMT_SensitivityMAT_S22, (int)MMT_SensitivityMAT_S33);
            ImGui::Text("S12=%d S13=%d S21=%d S23=%d S31=%d S32=%d",
                        (int)MMT_SensitivityMAT_S12, (int)MMT_SensitivityMAT_S13, (int)MMT_SensitivityMAT_S21,
                        (int)MMT_SensitivityMAT_S23, (int)MMT_SensitivityMAT_S31, (int)MMT_SensitivityMAT_S32);
            break;
        }






        case 76: {  
            static const uint16_t msgid   = ADCS_CMD_ID;
            static const uint8_t  fnccode = ADCS_SET_DEFAULT_MODE_CONFIG_CC;

            static bool initialized = false;
            if (!initialized) {

                ADCS_DefaultModeConfigCmd_Payload_t &p =
                    command->adcsdefaultmodeconfigcmd.Payload;

                p.DefaultRunMode                   = 1; 
                p.DefaultOperationalState          = 0;  
                p.DefaultControlModeInOpStateSafe  = 1;  
                p.DefaultControlModeInOpStateAuto  = 13; 


                initialized = true;
            }

            ImGui::Text("MsgID   : 0x%04X", msgid);
            ImGui::Text("FncCode : %u", fnccode);
            ImGui::Separator();

            ADCS_DefaultModeConfigCmd_Payload_t &p =
                command->adcsdefaultmodeconfigcmd.Payload;

            ImGui::Text("=== Default Mode Settings ===");
            ImGui::Text("DefaultRunMode                  : %u", p.DefaultRunMode);
            ImGui::Text("DefaultOperationalState         : %u", p.DefaultOperationalState);
            ImGui::Text("DefaultControlModeInOpStateSafe : %u", p.DefaultControlModeInOpStateSafe);
            ImGui::Text("DefaultControlModeInOpStateAuto : %u", p.DefaultControlModeInOpStateAuto);

            ImGui::Separator();

            if (ImGui::Button("Generate CMD")) {

                WriteSystemName(msgid);

                uint16_t mid    = htons(msgid);
                uint8_t  seq[2] = { 0xC0, 0x00 };
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(ADCS_DefaultModeConfigCmd_t) - 7)
                };

                memcpy(command->adcsdefaultmodeconfigcmd.CmdHeader,     &mid, 2);
                memcpy(command->adcsdefaultmodeconfigcmd.CmdHeader + 2, seq,  2);
                memcpy(command->adcsdefaultmodeconfigcmd.CmdHeader + 4, len,  2);
                memcpy(command->adcsdefaultmodeconfigcmd.CmdHeader + 6, &fnccode, 1);


                            command->adcsdefaultmodeconfigcmd.CmdHeader[7] = 0x00;

                uint16_t total = sizeof(ADCS_DefaultModeConfigCmd_t);
                const uint8_t *pp =
                    (const uint8_t *)&command->adcsdefaultmodeconfigcmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(pp++);
                memcpy(command->adcsdefaultmodeconfigcmd.CmdHeader + 7, &crc, 1);


                packetsign *pkt = (packetsign *)malloc(
                    2 + 2 + 4 + sizeof(ADCS_DefaultModeConfigCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(ADCS_DefaultModeConfigCmd_t);

                memcpy(pkt->Data,
                    &command->adcsdefaultmodeconfigcmd,
                    sizeof(ADCS_DefaultModeConfigCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }

            break;
        }

        case 77: {  
            static const uint16_t msgid   = ADCS_CMD_ID;
            static const uint8_t  fnccode = ADCS_SET_MOUNTING_CONFIG_CC;

            static bool initialized = false;
            if (!initialized) {

                ADCS_MountingConfigCmd_Payload_t &p =
                    command->adcsmountingconfigcmd.Payload;


                // ==== Stack & Actuators ====
                p.StackX_mounting = 3;
                p.StackY_mounting = 1;
                p.StackZ_mounting = 6;

                p.MTQ0_mounting = 2;
                p.MTQ1_mounting = 3;
                p.MTQ2_mounting = 5;

                p.Wheel0_mounting = 11;
                p.Wheel1_mounting = 12;
                p.Wheel2_mounting = 13;
                p.Wheel3_mounting = 14;

                // ==== Pyramid RWL angles (raw = deg * 100) ====
                p.PyramidRWL_alpha = 9000;
                p.PyramidRWL_beta  = 0;
                p.PyramidRWL_gamma = 18000;

                // ==== CSS Mountings ====
                p.CSS0_mounting = 5;
                p.CSS1_mounting = 0;
                p.CSS2_mounting = 0;
                p.CSS3_mounting = 0;
                p.CSS4_mounting = 0;
                p.CSS5_mounting = 1;
                p.CSS6_mounting = 3;
                p.CSS7_mounting = 2;
                p.CSS8_mounting = 4;
                p.CSS9_mounting = 6;

                // ==== FSS0..3 ====
                p.FSS0_alpha = 90; p.FSS0_beta = 180; p.FSS0_gamma = 0;
                p.FSS1_alpha = p.FSS1_beta = p.FSS1_gamma = 0;
                p.FSS2_alpha = p.FSS2_beta = p.FSS2_gamma = 0;
                p.FSS3_alpha = p.FSS3_beta = p.FSS3_gamma = 0;

                // ==== HSS0..1 ====
                p.HSS0_alpha = 0; p.HSS0_beta = -90; p.HSS0_gamma = 0;
                p.HSS1_alpha = p.HSS1_beta = p.HSS1_gamma = 0;

                // ==== MAG0..1 ====
                p.MAG0_alpha = 0; p.MAG0_beta = 0; p.MAG0_gamma = 0;
                p.MAG1_alpha = p.MAG1_beta = p.MAG1_gamma = 0;

                // ==== STR0..1 ====
                p.STR0_alpha = p.STR0_beta = p.STR0_gamma = 0;
                p.STR1_alpha = p.STR1_beta = p.STR1_gamma = 0;

                // ==== Ext Sensor 0/1 ====
                p.ExtSensor0_alpha = p.ExtSensor0_beta = p.ExtSensor0_gamma = 0;
                p.ExtSensor1_alpha = p.ExtSensor1_beta = p.ExtSensor1_gamma = 0;

                // ==== External Gyro axes ====
                p.ExtGyro0_axis1_mounting = 0;
                p.ExtGyro0_axis2_mounting = 0;
                p.ExtGyro0_axis3_mounting = 0;

                p.ExtGyro1_axis1_mounting = 0;
                p.ExtGyro1_axis2_mounting = 0;
                p.ExtGyro1_axis3_mounting = 0;

                // ============================================================
                initialized = true;
            }

            ImGui::Text("MsgID   : 0x%04X", msgid);
            ImGui::Text("FncCode : %u", fnccode);
            ImGui::Separator();

            ADCS_MountingConfigCmd_Payload_t &p =
                command->adcsmountingconfigcmd.Payload;

            ImGui::Text("=== Stack & Actuators ===");
            ImGui::Text("StackX: %u  StackY: %u  StackZ: %u", p.StackX_mounting, p.StackY_mounting, p.StackZ_mounting);
            ImGui::Text("MTQ0: %u  MTQ1: %u  MTQ2: %u", p.MTQ0_mounting, p.MTQ1_mounting, p.MTQ2_mounting);
            ImGui::Text("Wheels: %u %u %u %u", p.Wheel0_mounting, p.Wheel1_mounting, p.Wheel2_mounting, p.Wheel3_mounting);

            ImGui::Separator();
            ImGui::Text("=== Pyramid RWL Angles (deg = raw/100) ===");
            ImGui::Text("alpha: %d  beta: %d  gamma: %d", p.PyramidRWL_alpha, p.PyramidRWL_beta, p.PyramidRWL_gamma);

            ImGui::Separator();
            ImGui::Text("=== CSS Mountings ===");
            ImGui::Text("CSS0..9: %u %u %u %u %u %u %u %u %u %u",
                        p.CSS0_mounting, p.CSS1_mounting, p.CSS2_mounting, p.CSS3_mounting, p.CSS4_mounting,
                        p.CSS5_mounting, p.CSS6_mounting, p.CSS7_mounting, p.CSS8_mounting, p.CSS9_mounting);

            ImGui::Separator();
            ImGui::Text("=== FSS0..3 Angles ===");
            ImGui::Text("FSS0: %d %d %d", p.FSS0_alpha, p.FSS0_beta, p.FSS0_gamma);
            ImGui::Text("FSS1: %d %d %d", p.FSS1_alpha, p.FSS1_beta, p.FSS1_gamma);
            ImGui::Text("FSS2: %d %d %d", p.FSS2_alpha, p.FSS2_beta, p.FSS2_gamma);
            ImGui::Text("FSS3: %d %d %d", p.FSS3_alpha, p.FSS3_beta, p.FSS3_gamma);

            ImGui::Separator();
            ImGui::Text("=== HSS0..1 ===");
            ImGui::Text("HSS0: %d %d %d", p.HSS0_alpha, p.HSS0_beta, p.HSS0_gamma);
            ImGui::Text("HSS1: %d %d %d", p.HSS1_alpha, p.HSS1_beta, p.HSS1_gamma);

            ImGui::Separator();
            ImGui::Text("=== MAG0..1 ===");
            ImGui::Text("MAG0: %d %d %d", p.MAG0_alpha, p.MAG0_beta, p.MAG0_gamma);
            ImGui::Text("MAG1: %d %d %d", p.MAG1_alpha, p.MAG1_beta, p.MAG1_gamma);

            ImGui::Separator();
            ImGui::Text("=== STR0..1 ===");
            ImGui::Text("STR0: %d %d %d", p.STR0_alpha, p.STR0_beta, p.STR0_gamma);
            ImGui::Text("STR1: %d %d %d", p.STR1_alpha, p.STR1_beta, p.STR1_gamma);

            ImGui::Separator();
            ImGui::Text("=== External Sensor 0/1 ===");
            ImGui::Text("Ext0: %d %d %d", p.ExtSensor0_alpha, p.ExtSensor0_beta, p.ExtSensor0_gamma);
            ImGui::Text("Ext1: %d %d %d", p.ExtSensor1_alpha, p.ExtSensor1_beta, p.ExtSensor1_gamma);

            ImGui::Separator();
            ImGui::Text("=== External Gyro Mountings ===");
            ImGui::Text("Gyro0: %u %u %u", p.ExtGyro0_axis1_mounting, p.ExtGyro0_axis2_mounting, p.ExtGyro0_axis3_mounting);
            ImGui::Text("Gyro1: %u %u %u", p.ExtGyro1_axis1_mounting, p.ExtGyro1_axis2_mounting, p.ExtGyro1_axis3_mounting);

            ImGui::Separator();


            // ======================== Generate CMD ===========================
            if (ImGui::Button("Generate CMD")) {

                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t seq[2] = {0xC0, 0x00};
                uint8_t len[2] = {
                    0x00,
                    (uint8_t)(sizeof(ADCS_MountingConfigCmd_t) - 7)
                };

                memcpy(command->adcsmountingconfigcmd.CmdHeader,     &mid, 2);
                memcpy(command->adcsmountingconfigcmd.CmdHeader + 2, seq,  2);
                memcpy(command->adcsmountingconfigcmd.CmdHeader + 4, len,  2);
                memcpy(command->adcsmountingconfigcmd.CmdHeader + 6, &fnccode, 1);

                command->adcsmountingconfigcmd.CmdHeader[7] = 0;
                uint16_t total = sizeof(ADCS_MountingConfigCmd_t);
                const uint8_t *pp =
                    (const uint8_t *)&command->adcsmountingconfigcmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(pp++);
                memcpy(command->adcsmountingconfigcmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt = (packetsign *)malloc(
                    2 + 2 + 4 + sizeof(ADCS_MountingConfigCmd_t));

                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(ADCS_MountingConfigCmd_t);

                memcpy(pkt->Data,
                    &command->adcsmountingconfigcmd,
                    sizeof(ADCS_MountingConfigCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                                task_uplink_onorbit, (void *)pkt);
            }

            break;
        }


        case 79: {  
            static const uint16_t msgid   = ADCS_CMD_ID;
            static const uint8_t  fnccode = ADCS_SET_ESTIMATOR_CONFIG_CC;

            static bool initialized = false;
            if (!initialized) {

                ADCS_EstimatorConfigCmd_Payload_t &p =
                    command->adcsestimatorconfigcmd.Payload;


                p.DefaultMainEstimatorMode   = 0;  
                p.DefaultBackupEstimatorMode = 0; 

                p.MAGMeasurementNoise  =0.001;
                p.CSSMeasurementNoise  =10;
                p.FSSMeasurementNoise = 0.0001;
                p.HSSMeasurementNoise = 0.0001;
                p.STRMeasurementNoise  = 1E-8;

                p.MMTRKFSystemNoise    = 0.0002;
                p.EKFSystemNoise       = 1E-9;
                p.NutationEpsilonCorrection = 4.243392E-05;
                p.NutationPsiCorrection     = 1.607126E-05;

                // // ---- Bit-fields ----
                // p.UseFSSinEKF = 1;
                // p.UseCSSinEKF = 0;
                // p.UseHSSinEKF = 1;
                // p.UseSTRinEKF = 0;

                // p.TriadVector1 = 1;   // 4-bit
                // p.TriadVector2 = 0;   // 4-bit
                // p.Spare        = 0;   // unused padding
//revise
                // ---- Bit-fields ----
                p.UseFSSinEKF = 0;
                p.UseCSSinEKF = 0;
                p.UseHSSinEKF = 0;
                p.UseSTRinEKF = 0;

                p.TriadVector1 = 1;   // 4-bit
                p.TriadVector2 = 0;   // 4-bit


                // ============================================================

                initialized = true;
            }

            // GUI 출력 (read-only)
            ImGui::Text("MsgID   : 0x%04X", msgid);
            ImGui::Text("FncCode : %u", fnccode);
            ImGui::Separator();

            ADCS_EstimatorConfigCmd_Payload_t &p =
                command->adcsestimatorconfigcmd.Payload;

            ImGui::Text("=== Default Estimator Modes ===");
            ImGui::Text("Main   : %u", p.DefaultMainEstimatorMode);
            ImGui::Text("Backup : %u", p.DefaultBackupEstimatorMode);

            ImGui::Separator();
            ImGui::Text("=== Measurement Noise ===");
            ImGui::Text("MAG : %.6f", p.MAGMeasurementNoise);
            ImGui::Text("CSS : %.6f", p.CSSMeasurementNoise);
            ImGui::Text("FSS : %.6f", p.FSSMeasurementNoise);
            ImGui::Text("HSS : %.6f", p.HSSMeasurementNoise);
            ImGui::Text("STR : %.6f", p.STRMeasurementNoise);

            ImGui::Separator();
            ImGui::Text("=== System Noise ===");
            ImGui::Text("MMT-RKF : %.6f", p.MMTRKFSystemNoise);
            ImGui::Text("EKF     : %.6f", p.EKFSystemNoise);

            ImGui::Separator();
            ImGui::Text("=== Nutation Correction ===");
            ImGui::Text("Epsilon : %.6f", p.NutationEpsilonCorrection);
            ImGui::Text("Psi     : %.6f", p.NutationPsiCorrection);

            ImGui::Separator();
            ImGui::Text("=== Flags ===");
            ImGui::Text("UseFSSinEKF : %u", p.UseFSSinEKF);
            ImGui::Text("UseCSSinEKF : %u", p.UseCSSinEKF);
            ImGui::Text("UseHSSinEKF : %u", p.UseHSSinEKF);
            ImGui::Text("UseSTRinEKF : %u", p.UseSTRinEKF);

            ImGui::Separator();
            ImGui::Text("TriadVector1 : %u", p.TriadVector1);
            ImGui::Text("TriadVector2 : %u", p.TriadVector2);
            // ImGui::Text("Spare        : %u", p.Spare);

            ImGui::Separator();


            // ============================ Generate CMD ============================
            if (ImGui::Button("Generate CMD")) {

                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t seq[2] = {0xC0, 0x00};
                uint8_t len[2] = {
                    0x00,
                    (uint8_t)(sizeof(ADCS_EstimatorConfigCmd_t) - 7)
                };

                memcpy(command->adcsestimatorconfigcmd.CmdHeader,     &mid, 2);
                memcpy(command->adcsestimatorconfigcmd.CmdHeader + 2, seq,  2);
                memcpy(command->adcsestimatorconfigcmd.CmdHeader + 4, len,  2);
                memcpy(command->adcsestimatorconfigcmd.CmdHeader + 6, &fnccode, 1);


                command->adcsestimatorconfigcmd.CmdHeader[7] = 0x00;
                uint16_t total = sizeof(ADCS_EstimatorConfigCmd_t);
                const uint8_t *pp =
                    (const uint8_t *)&command->adcsestimatorconfigcmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(pp++);

                memcpy(command->adcsestimatorconfigcmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt = (packetsign *)malloc(
                    2 + 2 + 4 + sizeof(ADCS_EstimatorConfigCmd_t));

                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(ADCS_EstimatorConfigCmd_t);

                memcpy(pkt->Data,
                    &command->adcsestimatorconfigcmd,
                    sizeof(ADCS_EstimatorConfigCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                                task_uplink_onorbit, (void *)pkt);
            }

            break;
        }

case 80: {
    static const uint16_t msgid   = ADCS_CMD_ID;
    static const uint8_t  fnccode = ADCS_SET_SAT_ORBIT_PARAMS_CONFIG_CC;


    ADCS_SatOrbitParamConfigCmd_Payload_t &p =
        command->adcssatorbitparamconfigcmd.Payload;

    ImGui::Text("MsgID   : 0x%04X", msgid);
    ImGui::Text("FncCode : %u", fnccode);
    ImGui::Separator();

    ImGui::Text("=== Sat Orbit Params (double) ===");
    ImGui::InputDouble("Epoch",        &p.Epoch);
    ImGui::InputDouble("Inclination",  &p.Inclination);
    ImGui::InputDouble("RAAN",         &p.RAAN);
    ImGui::InputDouble("Eccentricity", &p.Eccentricity);
    ImGui::InputDouble("AOP",          &p.AOP);
    ImGui::InputDouble("MeanAnomaly",  &p.MeanAnomaly);
    ImGui::InputDouble("MeanMotion",   &p.MeanMotion);
    ImGui::InputDouble("B_StarDrag",   &p.B_StarDrag);

    ImGui::Separator();

    if (ImGui::Button("Generate CMD")) {
        WriteSystemName(msgid);

        uint16_t mid_be = htons(msgid);
        uint16_t ccsds_len_be = htons((uint16_t)(sizeof(ADCS_SatOrbitParamConfigCmd_t) - 7));

        memcpy(command->adcssatorbitparamconfigcmd.CmdHeader + 0, &mid_be, 2);
        command->adcssatorbitparamconfigcmd.CmdHeader[2] = 0xC0;
        command->adcssatorbitparamconfigcmd.CmdHeader[3] = 0x00;
        memcpy(command->adcssatorbitparamconfigcmd.CmdHeader + 4, &ccsds_len_be, 2);
        memcpy(command->adcssatorbitparamconfigcmd.CmdHeader + 6, &fnccode, 1);
        command->adcssatorbitparamconfigcmd.CmdHeader[7] = 0x00;

        uint16_t total = (uint16_t)sizeof(ADCS_SatOrbitParamConfigCmd_t);
        const uint8_t *pp = (const uint8_t *)&command->adcssatorbitparamconfigcmd;
        uint8_t crc = 0xFF;
        while (total--) crc ^= *(pp++);
        command->adcssatorbitparamconfigcmd.CmdHeader[7] = crc;

        packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(ADCS_SatOrbitParamConfigCmd_t));
        pkt->Identifier = HVD_TEST;
        pkt->PacketType = MIM_PT_TMTC_TEST;
        pkt->Length     = sizeof(ADCS_SatOrbitParamConfigCmd_t);

        memcpy(pkt->Data, &command->adcssatorbitparamconfigcmd, sizeof(ADCS_SatOrbitParamConfigCmd_t));

        pthread_join(p_thread[4], NULL);
        pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
    }

    break;
}




        case 81: { 
            static const uint16_t msgid   = ADCS_CMD_ID;
            static const uint8_t  fnccode = ADCS_SET_NODE_SELECTION_CONFIG_CC;

            static bool initialized = false;
            if (!initialized) {
                ADCS_NodeSelectionConfigCmd_Payload_t &p =
                    command->adcsnodeselectionconfigcmd.Payload;

                p.RSWSelectionFlags        = 15;
                p.MAGSelectionFlags        = 1;
                p.FSSSelectionFlags        = 1;
                p.HSSSelectionFlags        = 1;
                p.GYRSelectionFlags        = 12;
                p.STRSelectionFlags        = 0;
                p.GNSSSelectionFlags       = 1;
                p.ExtSensorSelectionFlags  = 0;

                initialized = true;
            }

            ADCS_NodeSelectionConfigCmd_Payload_t &p =
                command->adcsnodeselectionconfigcmd.Payload;


            ImGui::Text("MsgID   : 0x%04X", msgid);
            ImGui::Text("FncCode : %u",     fnccode);
            ImGui::Separator();

            ImGui::Text("=== Node Selection Flags ===");
            ImGui::Text("RSWSelectionFlags       : %d", p.RSWSelectionFlags);
            ImGui::Text("MAGSelectionFlags       : %d", p.MAGSelectionFlags);
            ImGui::Text("FSSSelectionFlags       : %d", p.FSSSelectionFlags);
            ImGui::Text("HSSSelectionFlags       : %d", p.HSSSelectionFlags);
            ImGui::Text("GYRSelectionFlags       : %d", p.GYRSelectionFlags);
            ImGui::Text("STRSelectionFlags       : %d", p.STRSelectionFlags);
            ImGui::Text("GNSSSelectionFlags      : %d", p.GNSSSelectionFlags);
            ImGui::Text("ExtSensorSelectionFlags : %d", p.ExtSensorSelectionFlags);

            ImGui::Separator();


            if (ImGui::Button("Generate CMD")) {

                WriteSystemName(msgid);


                uint16_t mid = htons(msgid);
                uint8_t seq[2] = {0xC0, 0x00};
                uint8_t len[2] = {
                    0x00,
                    (uint8_t)(sizeof(ADCS_NodeSelectionConfigCmd_t) - 7)
                };

                memcpy(command->adcsnodeselectionconfigcmd.CmdHeader,     &mid, 2);
                memcpy(command->adcsnodeselectionconfigcmd.CmdHeader + 2, seq,  2);
                memcpy(command->adcsnodeselectionconfigcmd.CmdHeader + 4, len,  2);
                memcpy(command->adcsnodeselectionconfigcmd.CmdHeader + 6, &fnccode, 1);
        command->adcsnodeselectionconfigcmd.CmdHeader[7] = 0x00;

                uint16_t total = sizeof(ADCS_NodeSelectionConfigCmd_t);
                const uint8_t *pp =
                    (const uint8_t *)&command->adcsnodeselectionconfigcmd;
                uint8_t crc = 0xFF;
                while (total--) {
                    crc ^= *(pp++);
                }
                memcpy(command->adcsnodeselectionconfigcmd.CmdHeader + 7, &crc, 1);


                packetsign *pkt = (packetsign *)malloc(
                    2 + 2 + 4 + sizeof(ADCS_NodeSelectionConfigCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(ADCS_NodeSelectionConfigCmd_t);

                memcpy(pkt->Data,
                    &command->adcsnodeselectionconfigcmd,
                    sizeof(ADCS_NodeSelectionConfigCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }

            break;
        }



        case 82: { 
            static const uint16_t msgid   = ADCS_CMD_ID;
            static const uint8_t  fnccode = ADCS_SET_MTQ_CONFIG_CC;

            static bool initialized = false;
            if (!initialized) {

                ADCS_MTQConfigCmd_Payload_t &p =
                    command->adcsmtqconfigcmd.Payload;


                p.MTQ0MaxDipoleMoment = 0.4;
                p.MTQ1MaxDipoleMoment = 0.4;
                p.MTQ2MaxDipoleMoment = 0.4;

                p.MaxMTQOnTime  = 800;
                p.MinMTQOnTime  = 10;

                p.MagneticControlFilterFactor = 0;
                // =====================================================

                initialized = true;
            }

            ADCS_MTQConfigCmd_Payload_t &p =
                command->adcsmtqconfigcmd.Payload;

     
            ImGui::Text("MsgID   : 0x%04X", msgid);
            ImGui::Text("FncCode : %u",     fnccode);
            ImGui::Separator();

            ImGui::Text("=== MTQ Maximum Dipole Moments ===");
            ImGui::Text("MTQ0 Max Dipole : %.4f", p.MTQ0MaxDipoleMoment);
            ImGui::Text("MTQ1 Max Dipole : %.4f", p.MTQ1MaxDipoleMoment);
            ImGui::Text("MTQ2 Max Dipole : %.4f", p.MTQ2MaxDipoleMoment);

            ImGui::Separator();
            ImGui::Text("=== MTQ On-Time Settings ===");
            ImGui::Text("Max MTQ On Time : %u ms", p.MaxMTQOnTime);
            ImGui::Text("Min MTQ On Time : %u ms", p.MinMTQOnTime);

            ImGui::Separator();
            ImGui::Text("=== Magnetic Control Filter ===");
            ImGui::Text("Filter factor : %.4f", p.MagneticControlFilterFactor);

            ImGui::Separator();

  
            if (ImGui::Button("Generate CMD")) {

                WriteSystemName(msgid);

                // CCSDS Header
                uint16_t mid = htons(msgid);
                uint8_t seq[2] = {0xC0, 0x00};
                uint8_t len[2] = {
                    0x00,
                    (uint8_t)(sizeof(ADCS_MTQConfigCmd_t) - 7)
                };

                memcpy(command->adcsmtqconfigcmd.CmdHeader,     &mid, 2);
                memcpy(command->adcsmtqconfigcmd.CmdHeader + 2, seq,  2);
                memcpy(command->adcsmtqconfigcmd.CmdHeader + 4, len,  2);
                memcpy(command->adcsmtqconfigcmd.CmdHeader + 6, &fnccode, 1);
        command->adcsmtqconfigcmd.CmdHeader[7] = 0x00;
                uint16_t total = sizeof(ADCS_MTQConfigCmd_t);
                const uint8_t *pp =
                    (const uint8_t *)&command->adcsmtqconfigcmd;
                uint8_t crc = 0xFF;
                while (total--) {
                    crc ^= *(pp++);
                }
                memcpy(command->adcsmtqconfigcmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt = (packetsign *)malloc(
                    2 + 2 + 4 + sizeof(ADCS_MTQConfigCmd_t));

                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(ADCS_MTQConfigCmd_t);

                memcpy(pkt->Data,
                    &command->adcsmtqconfigcmd,
                    sizeof(ADCS_MTQConfigCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }

            break;
        }

        case 85: { 

            static const uint16_t msgid   = ADCS_CMD_ID;
            static const uint8_t  fnccode = ADCS_SET_MAG_SENSING_ELM_CONFIG_CC;


            static bool initialized = false;
            if (!initialized) {

                ADCS_MagSensingElmConfigCmd_Payload_t &p =
                    command->adcsmagsensingelmconfigcmd.Payload;

                p.Mag0SensingElement = 0; 
                p.Mag1SensingElement = 0;  
                p.Spare = 0;              


                initialized = true;
            }

            // Payload alias
            ADCS_MagSensingElmConfigCmd_Payload_t &p =
                command->adcsmagsensingelmconfigcmd.Payload;


            ImGui::Text("MsgID   : 0x%04X", msgid);
            ImGui::Text("FncCode : %u", fnccode);
            ImGui::Separator();

            ImGui::Text("=== Magnetic Sensing Element Enable Flags ===");
            ImGui::Text("MAG0 Sensing Element : %u", p.Mag0SensingElement);
            ImGui::Text("MAG1 Sensing Element : %u", p.Mag1SensingElement);
            ImGui::Text("Spare Bits           : %u", p.Spare);
            ImGui::Separator();

            if (ImGui::Button("Generate CMD")) {

                WriteSystemName(msgid);

                // CCSDS Header (7바이트 + CRC)
                uint16_t mid = htons(msgid);
                uint8_t seq[2] = {0xC0, 0x00};
                uint8_t len[2] = {
                    0x00,
                    (uint8_t)(sizeof(ADCS_MagSensingElmConfigCmd_t) - 7)
                };

                memcpy(command->adcsmagsensingelmconfigcmd.CmdHeader,     &mid, 2);
                memcpy(command->adcsmagsensingelmconfigcmd.CmdHeader + 2, seq,  2);
                memcpy(command->adcsmagsensingelmconfigcmd.CmdHeader + 4, len,  2);
                memcpy(command->adcsmagsensingelmconfigcmd.CmdHeader + 6, &fnccode, 1);
        command->adcsmagsensingelmconfigcmd.CmdHeader[7] = 0x00;

                uint16_t total = sizeof(ADCS_MagSensingElmConfigCmd_t);
                const uint8_t *pp =
                    (const uint8_t *)&command->adcsmagsensingelmconfigcmd;

                uint8_t crc = 0xFF;
                while (total--) crc ^= *(pp++);
                memcpy(command->adcsmagsensingelmconfigcmd.CmdHeader + 7, &crc, 1);


                packetsign *pkt = (packetsign *)malloc(
                    2 + 2 + 4 + sizeof(ADCS_MagSensingElmConfigCmd_t));

                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(ADCS_MagSensingElmConfigCmd_t);

                memcpy(pkt->Data,
                    &command->adcsmagsensingelmconfigcmd,
                    sizeof(ADCS_MagSensingElmConfigCmd_t));

                // uplink thread 실행
                pthread_join(p_thread[4], NULL);
                pthread_create(
                    &p_thread[4], NULL,
                    task_uplink_onorbit, (void *)pkt
                );
            }

            break;
        }


        case 86: {
            static uint16_t msgid   = ADCS_CMD_ID;
            static uint8_t  fnccode = ADCS_SET_UNSOLICIT_TLM_MSG_SETUP_CC;

            static uint8_t uart_interval   = 0;
            static uint8_t uart2_interval  = 0;
            static uint8_t can_interval    = 0;
            static uint8_t reserved_field  = 0;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::Separator();
            ImGui::Text("Return Interval (0~15)");
            ImGui::InputScalar("UART Return Interval",  ImGuiDataType_U8, &uart_interval);
            ImGui::InputScalar("UART2 Return Interval", ImGuiDataType_U8, &uart2_interval);
            ImGui::InputScalar("CAN Return Interval",   ImGuiDataType_U8, &can_interval);
            ImGui::InputScalar("Reserved (4bit)",       ImGuiDataType_U8, &reserved_field);

            ADCS_UnsolicitTlmMsgSetupCmd_Payload_t &pl =
                command->adcsunsolicittlmmsgsetupcmd.Payload;

            ImGui::Separator();
            ImGui::Text("UART TLM ED Inclusion Bitmask");
            for (int i = 0; i < 5; i++) {
                ImGui::InputScalar(
                    (std::string("UART Bitmask[") + std::to_string(i) + "]").c_str(),
                    ImGuiDataType_U8,
                    &pl.UARTTlmEDInclusionBitmask[i]
                );
            }

            ImGui::Separator();
            ImGui::Text("UART2 TLM ED Inclusion Bitmask");
            for (int i = 0; i < 5; i++) {
                ImGui::InputScalar(
                    (std::string("UART2 Bitmask[") + std::to_string(i) + "]").c_str(),
                    ImGuiDataType_U8,
                    &pl.UART2TlmEDInclusionBitmask[i]
                );
            }

            ImGui::Separator();
            ImGui::Text("CAN TLM ED Inclusion Bitmask");
            for (int i = 0; i < 5; i++) {
                ImGui::InputScalar(
                    (std::string("CAN Bitmask[") + std::to_string(i) + "]").c_str(),
                    ImGuiDataType_U8,
                    &pl.CANTlmEDInclusionBitmask[i]
                );
            }

            if (ImGui::Button("Generate CMD")) {
                pl.UARTTlmReturnInterval  = uart_interval  & 0x0F;
                pl.UART2TlmReturnInterval = uart2_interval & 0x0F;
                pl.CANTlmRetrunInterval   = can_interval   & 0x0F;
                pl.Reserved               = reserved_field & 0x0F;

                WriteSystemName(msgid);
                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    static_cast<uint8_t>(sizeof(ADCS_UnsolicitTlmMsgSetupCmd_t) - 7)
                };

                memcpy(command->adcsunsolicittlmmsgsetupcmd.CmdHeader,     &mid, 2);
                memcpy(command->adcsunsolicittlmmsgsetupcmd.CmdHeader + 2, seq,  2);
                memcpy(command->adcsunsolicittlmmsgsetupcmd.CmdHeader + 4, len,  2);
                memcpy(command->adcsunsolicittlmmsgsetupcmd.CmdHeader + 6, &fnccode, 1);
        command->adcsunsolicittlmmsgsetupcmd.CmdHeader[7] = 0x00;

                uint16_t total = sizeof(ADCS_UnsolicitTlmMsgSetupCmd_t);
                const uint8_t *p =
                    (const uint8_t *)&command->adcsunsolicittlmmsgsetupcmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->adcsunsolicittlmmsgsetupcmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 +
                                        sizeof(ADCS_UnsolicitTlmMsgSetupCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(ADCS_UnsolicitTlmMsgSetupCmd_t);
                memcpy(pkt->Data, &command->adcsunsolicittlmmsgsetupcmd,
                    sizeof(ADCS_UnsolicitTlmMsgSetupCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }




        case 88: // ADCS2_COMM_01_CC
        {
            static uint16_t msgid   = ADCS_CMD_ID;
            static uint8_t  fnccode = 101;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("Flag_TLM Type", ImGuiDataType_U8,
                            &command->adcs101cmd.Payload.flag_tlmtype);

            ImGui::InputScalar("Flag_EST Mode", ImGuiDataType_U8,
                            &command->adcs101cmd.Payload.flag_estmode);

            ImGui::InputScalar("Flag_Cont Mode", ImGuiDataType_U8,
                            &command->adcs101cmd.Payload.flag_contmode);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(ADCS2_Comm01Cmd_t) - 7)
                };

                memcpy(command->adcs101cmd.CmdHeader,     &mid, 2);
                memcpy(command->adcs101cmd.CmdHeader + 2, seq,  2);
                memcpy(command->adcs101cmd.CmdHeader + 4, len,  2);
                memcpy(command->adcs101cmd.CmdHeader + 6, &fnccode, 1);

                uint16_t total = sizeof(ADCS2_Comm01Cmd_t);
                const uint8_t *p = (const uint8_t *)&command->adcs101cmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->adcs101cmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(ADCS2_Comm01Cmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(ADCS2_Comm01Cmd_t);
                memcpy(pkt->Data, &command->adcs101cmd,
                    sizeof(ADCS2_Comm01Cmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }


        // SC_NOOP_CC (0)
        case 136: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_NOOP_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(SC_NoopCmd_t) - 7)
                };

                memcpy(command->scnoopcmd.CmdHeader,     &mid, 2);
                memcpy(command->scnoopcmd.CmdHeader + 2, seq,  2);
                memcpy(command->scnoopcmd.CmdHeader + 4, len,  2);
                memcpy(command->scnoopcmd.CmdHeader + 6, &fnccode, 1);

                uint16_t total = sizeof(SC_NoopCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scnoopcmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scnoopcmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_NoopCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_NoopCmd_t);
                memcpy(pkt->Data, &command->scnoopcmd, sizeof(SC_NoopCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }

        // SC_RESET_COUNTERS_CC (1)
        case 137: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_RESET_COUNTERS_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(SC_ResetCountersCmd_t) - 7)
                };

                memcpy(command->scresetcounterscmd.CmdHeader,     &mid, 2);
                memcpy(command->scresetcounterscmd.CmdHeader + 2, seq,  2);
                memcpy(command->scresetcounterscmd.CmdHeader + 4, len,  2);
                memcpy(command->scresetcounterscmd.CmdHeader + 6, &fnccode, 1);

                uint16_t total = sizeof(SC_ResetCountersCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scresetcounterscmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scresetcounterscmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_ResetCountersCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_ResetCountersCmd_t);
                memcpy(pkt->Data, &command->scresetcounterscmd,
                    sizeof(SC_ResetCountersCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }

        // SC_START_ATS_CC (2)
        case 138: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_START_ATS_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("AtsNum", ImGuiDataType_U16,
                            &command->scstartatscmd.Payload.AtsNum);

            ImGui::InputScalar("Padding", ImGuiDataType_U16,
                            &command->scstartatscmd.Payload.Padding);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(SC_StartAtsCmd_t) - 7)
                };

                memcpy(command->scstartatscmd.CmdHeader,     &mid, 2);
                memcpy(command->scstartatscmd.CmdHeader + 2, seq,  2);
                memcpy(command->scstartatscmd.CmdHeader + 4, len,  2);
                memcpy(command->scstartatscmd.CmdHeader + 6, &fnccode, 1);

                uint16_t total = sizeof(SC_StartAtsCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scstartatscmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scstartatscmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_StartAtsCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_StartAtsCmd_t);
                memcpy(pkt->Data, &command->scstartatscmd,
                    sizeof(SC_StartAtsCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }

        // SC_STOP_ATS_CC (3)
        case 139: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_STOP_ATS_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(SC_StopAtsCmd_t) - 7)
                };

                memcpy(command->scstopatscmd.CmdHeader,     &mid, 2);
                memcpy(command->scstopatscmd.CmdHeader + 2, seq,  2);
                memcpy(command->scstopatscmd.CmdHeader + 4, len,  2);
                memcpy(command->scstopatscmd.CmdHeader + 6, &fnccode, 1);

                uint16_t total = sizeof(SC_StopAtsCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scstopatscmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scstopatscmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_StopAtsCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_StopAtsCmd_t);
                memcpy(pkt->Data, &command->scstopatscmd,
                    sizeof(SC_StopAtsCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }

        case 140: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_START_RTS_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("RtsNum",  ImGuiDataType_U16,
                            &command->scstartrtscmd.Payload.RtsNum);
            ImGui::InputScalar("Padding", ImGuiDataType_U16,
                            &command->scstartrtscmd.Payload.Padding);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid   = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {0x00, (uint8_t)(sizeof(SC_StartRtsCmd_t) - 7)};

                memcpy(command->scstartrtscmd.CmdHeader,     &mid, 2);
                memcpy(command->scstartrtscmd.CmdHeader + 2, seq,  2);
                memcpy(command->scstartrtscmd.CmdHeader + 4, len,  2);
                memcpy(command->scstartrtscmd.CmdHeader + 6, &fnccode, 1);

                uint16_t total = sizeof(SC_StartRtsCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scstartrtscmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scstartrtscmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_StartRtsCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_StartRtsCmd_t);
                memcpy(pkt->Data, &command->scstartrtscmd,
                    sizeof(SC_StartRtsCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }

        case 141: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_STOP_RTS_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("RtsNum",  ImGuiDataType_U16,
                            &command->scstoprtscmd.Payload.RtsNum);
            ImGui::InputScalar("Padding", ImGuiDataType_U16,
                            &command->scstoprtscmd.Payload.Padding);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid   = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {0x00, (uint8_t)(sizeof(SC_StopRtsCmd_t) - 7)};

                memcpy(command->scstoprtscmd.CmdHeader,     &mid, 2);
                memcpy(command->scstoprtscmd.CmdHeader + 2, seq,  2);
                memcpy(command->scstoprtscmd.CmdHeader + 4, len,  2);
                memcpy(command->scstoprtscmd.CmdHeader + 6, &fnccode, 1);
        command->scstoprtscmd.CmdHeader[7] = 0x00;

                uint16_t total = sizeof(SC_StopRtsCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scstoprtscmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scstoprtscmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_StopRtsCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_StopRtsCmd_t);
                memcpy(pkt->Data, &command->scstoprtscmd,
                    sizeof(SC_StopRtsCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }


        case 142: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_DISABLE_RTS_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("RtsNum",  ImGuiDataType_U16,
                            &command->scdisablertscmd.Payload.RtsNum);
            ImGui::InputScalar("Padding", ImGuiDataType_U16,
                            &command->scdisablertscmd.Payload.Padding);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid   = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {0x00, (uint8_t)(sizeof(SC_DisableRtsCmd_t) - 7)};

                memcpy(command->scdisablertscmd.CmdHeader,     &mid, 2);
                memcpy(command->scdisablertscmd.CmdHeader + 2, seq,  2);
                memcpy(command->scdisablertscmd.CmdHeader + 4, len,  2);
                memcpy(command->scdisablertscmd.CmdHeader + 6, &fnccode, 1);
        command->scdisablertscmd.CmdHeader[7] = 0x00;

                uint16_t total = sizeof(SC_DisableRtsCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scdisablertscmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scdisablertscmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_DisableRtsCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_DisableRtsCmd_t);
                memcpy(pkt->Data, &command->scdisablertscmd,
                    sizeof(SC_DisableRtsCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }

        case 143: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_ENABLE_RTS_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("RtsNum",  ImGuiDataType_U16,
                            &command->scenablertscmd.Payload.RtsNum);
            ImGui::InputScalar("Padding", ImGuiDataType_U16,
                            &command->scenablertscmd.Payload.Padding);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid   = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {0x00, (uint8_t)(sizeof(SC_EnableRtsCmd_t) - 7)};

                memcpy(command->scenablertscmd.CmdHeader,     &mid, 2);
                memcpy(command->scenablertscmd.CmdHeader + 2, seq,  2);
                memcpy(command->scenablertscmd.CmdHeader + 4, len,  2);
                memcpy(command->scenablertscmd.CmdHeader + 6, &fnccode, 1);

                command->scenablertscmd.CmdHeader[7] = 0;
                uint16_t total = sizeof(SC_EnableRtsCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scenablertscmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scenablertscmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_EnableRtsCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_EnableRtsCmd_t);
                memcpy(pkt->Data, &command->scenablertscmd,
                    sizeof(SC_EnableRtsCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }


        // SC_SWITCH_ATS_CC (8)
        case 144: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_SWITCH_ATS_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(SC_SwitchAtsCmd_t) - 7)
                };

                memcpy(command->scswitchatscmd.CmdHeader,     &mid, 2);
                memcpy(command->scswitchatscmd.CmdHeader + 2, seq,  2);
                memcpy(command->scswitchatscmd.CmdHeader + 4, len,  2);
                memcpy(command->scswitchatscmd.CmdHeader + 6, &fnccode, 1);

                uint16_t total = sizeof(SC_SwitchAtsCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scswitchatscmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scswitchatscmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_SwitchAtsCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_SwitchAtsCmd_t);
                memcpy(pkt->Data, &command->scswitchatscmd,
                    sizeof(SC_SwitchAtsCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }

        // SC_JUMP_ATS_CC (9)
        case 145: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_JUMP_ATS_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("NewTime", ImGuiDataType_U32,
                            &command->scjumpatscmd.Payload.NewTime);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(SC_JumpAtsCmd_t) - 7)
                };

                memcpy(command->scjumpatscmd.CmdHeader,     &mid, 2);
                memcpy(command->scjumpatscmd.CmdHeader + 2, seq,  2);
                memcpy(command->scjumpatscmd.CmdHeader + 4, len,  2);
                memcpy(command->scjumpatscmd.CmdHeader + 6, &fnccode, 1);

                uint16_t total = sizeof(SC_JumpAtsCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scjumpatscmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scjumpatscmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_JumpAtsCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_JumpAtsCmd_t);
                memcpy(pkt->Data, &command->scjumpatscmd,
                    sizeof(SC_JumpAtsCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }

        // SC_CONTINUE_ATS_ON_FAILURE_CC (10)
        case 146: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_CONTINUE_ATS_ON_FAILURE_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("ContinueState", ImGuiDataType_U8,
                            &command->sccontinueatsonfailurecmd.Payload.ContinueState);
            ImGui::InputScalar("Padding",       ImGuiDataType_U16,
                            &command->sccontinueatsonfailurecmd.Payload.Padding);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid   = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {0x00,
                    (uint8_t)(sizeof(SC_ContinueAtsOnFailureCmd_t) - 7)};

                memcpy(command->sccontinueatsonfailurecmd.CmdHeader,     &mid, 2);
                memcpy(command->sccontinueatsonfailurecmd.CmdHeader + 2, seq,  2);
                memcpy(command->sccontinueatsonfailurecmd.CmdHeader + 4, len,  2);
                memcpy(command->sccontinueatsonfailurecmd.CmdHeader + 6, &fnccode, 1);

                uint16_t total = sizeof(SC_ContinueAtsOnFailureCmd_t);
                const uint8_t *p =
                    (const uint8_t *)&command->sccontinueatsonfailurecmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->sccontinueatsonfailurecmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 +
                                        sizeof(SC_ContinueAtsOnFailureCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_ContinueAtsOnFailureCmd_t);
                memcpy(pkt->Data, &command->sccontinueatsonfailurecmd,
                    sizeof(SC_ContinueAtsOnFailureCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }

        // SC_APPEND_ATS_CC (11)
       
        case 147: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_APPEND_ATS_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("AtsNum",  ImGuiDataType_U16,
                            &command->scappendatscmd.Payload.AtsNum);
            ImGui::InputScalar("Padding", ImGuiDataType_U16,
                            &command->scappendatscmd.Payload.Padding);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid   = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {0x00, (uint8_t)(sizeof(SC_AppendAtsCmd_t) - 7)};

                memcpy(command->scappendatscmd.CmdHeader,     &mid, 2);
                memcpy(command->scappendatscmd.CmdHeader + 2, seq,  2);
                memcpy(command->scappendatscmd.CmdHeader + 4, len,  2);
                memcpy(command->scappendatscmd.CmdHeader + 6, &fnccode, 1);

                uint16_t total = sizeof(SC_AppendAtsCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scappendatscmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scappendatscmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_AppendAtsCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_AppendAtsCmd_t);
                memcpy(pkt->Data, &command->scappendatscmd,
                    sizeof(SC_AppendAtsCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }


        // SC_MANAGE_TABLE_CC (12)
        case 148: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_MANAGE_TABLE_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("Parameter", ImGuiDataType_U32,
                            &command->scmanagetablecmd.Payload.Parameter);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(SC_ManageTableCmd_t) - 7)
                };

                memcpy(command->scmanagetablecmd.CmdHeader,     &mid, 2);
                memcpy(command->scmanagetablecmd.CmdHeader + 2, seq,  2);
                memcpy(command->scmanagetablecmd.CmdHeader + 4, len,  2);
                memcpy(command->scmanagetablecmd.CmdHeader + 6, &fnccode, 1);

                uint16_t total = sizeof(SC_ManageTableCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scmanagetablecmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scmanagetablecmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_ManageTableCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_ManageTableCmd_t);
                memcpy(pkt->Data, &command->scmanagetablecmd,
                    sizeof(SC_ManageTableCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }

        // SC_START_RTS_GRP_CC (13)
        case 149: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_START_RTS_GRP_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("FirstRtsNum", ImGuiDataType_U16,
                            &command->scstartrtsgrpcmd.Payload.FirstRtsNum);
            ImGui::InputScalar("LastRtsNum", ImGuiDataType_U16,
                            &command->scstartrtsgrpcmd.Payload.LastRtsNum);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(SC_StartRtsGrpCmd_t) - 7)
                };

                memcpy(command->scstartrtsgrpcmd.CmdHeader,     &mid, 2);
                memcpy(command->scstartrtsgrpcmd.CmdHeader + 2, seq,  2);
                memcpy(command->scstartrtsgrpcmd.CmdHeader + 4, len,  2);
                memcpy(command->scstartrtsgrpcmd.CmdHeader + 6, &fnccode, 1);

                uint16_t total = sizeof(SC_StartRtsGrpCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scstartrtsgrpcmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scstartrtsgrpcmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_StartRtsGrpCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_StartRtsGrpCmd_t);
                memcpy(pkt->Data, &command->scstartrtsgrpcmd,
                    sizeof(SC_StartRtsGrpCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }

        // SC_STOP_RTS_GRP_CC (14)
        case 150: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_STOP_RTS_GRP_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("FirstRtsNum", ImGuiDataType_U16,
                            &command->scstoprtsgrpcmd.Payload.FirstRtsNum);
            ImGui::InputScalar("LastRtsNum", ImGuiDataType_U16,
                            &command->scstoprtsgrpcmd.Payload.LastRtsNum);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(SC_StopRtsGrpCmd_t) - 7)
                };

                memcpy(command->scstoprtsgrpcmd.CmdHeader,     &mid, 2);
                memcpy(command->scstoprtsgrpcmd.CmdHeader + 2, seq,  2);
                memcpy(command->scstoprtsgrpcmd.CmdHeader + 4, len,  2);
                memcpy(command->scstoprtsgrpcmd.CmdHeader + 6, &fnccode, 1);

                uint16_t total = sizeof(SC_StopRtsGrpCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scstoprtsgrpcmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scstoprtsgrpcmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_StopRtsGrpCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_StopRtsGrpCmd_t);
                memcpy(pkt->Data, &command->scstoprtsgrpcmd,
                    sizeof(SC_StopRtsGrpCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }

        // SC_DISABLE_RTS_GRP_CC (15)
        case 151: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_DISABLE_RTS_GRP_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("FirstRtsNum", ImGuiDataType_U16,
                            &command->scdisablertsgrpcmd.Payload.FirstRtsNum);
            ImGui::InputScalar("LastRtsNum", ImGuiDataType_U16,
                            &command->scdisablertsgrpcmd.Payload.LastRtsNum);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(SC_DisableRtsGrpCmd_t) - 7)
                };

                memcpy(command->scdisablertsgrpcmd.CmdHeader,     &mid, 2);
                memcpy(command->scdisablertsgrpcmd.CmdHeader + 2, seq,  2);
                memcpy(command->scdisablertsgrpcmd.CmdHeader + 4, len,  2);
                memcpy(command->scdisablertsgrpcmd.CmdHeader + 6, &fnccode, 1);

                uint16_t total = sizeof(SC_DisableRtsGrpCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scdisablertsgrpcmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scdisablertsgrpcmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_DisableRtsGrpCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_DisableRtsGrpCmd_t);
                memcpy(pkt->Data, &command->scdisablertsgrpcmd,
                    sizeof(SC_DisableRtsGrpCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }

        // SC_ENABLE_RTS_GRP_CC (16)
        case 152: {
            static uint16_t msgid   = SC_CMD_MID;
            static uint8_t  fnccode = SC_ENABLE_RTS_GRP_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("FirstRtsNum", ImGuiDataType_U16,
                            &command->scenablertsgrpcmd.Payload.FirstRtsNum);
            ImGui::InputScalar("LastRtsNum", ImGuiDataType_U16,
                            &command->scenablertsgrpcmd.Payload.LastRtsNum);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(SC_EnableRtsGrpCmd_t) - 7)
                };

                memcpy(command->scenablertsgrpcmd.CmdHeader,     &mid, 2);
                memcpy(command->scenablertsgrpcmd.CmdHeader + 2, seq,  2);
                memcpy(command->scenablertsgrpcmd.CmdHeader + 4, len,  2);
                memcpy(command->scenablertsgrpcmd.CmdHeader + 6, &fnccode, 1);

                command->scenablertsgrpcmd.CmdHeader[7] = 0;
                uint16_t total = sizeof(SC_EnableRtsGrpCmd_t);
                const uint8_t *p = (const uint8_t *)&command->scenablertsgrpcmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->scenablertsgrpcmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(SC_EnableRtsGrpCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(SC_EnableRtsGrpCmd_t);
                memcpy(pkt->Data, &command->scenablertsgrpcmd,
                    sizeof(SC_EnableRtsGrpCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL,
                            task_uplink_onorbit, (void *)pkt);
            }
            break;
        }

        case 153: { // TO_LAB NOOP
            static uint16_t msgid   = CFE_MISSION_TO_LAB_CMD_ID;
            static uint8_t  fnccode = TO_LAB_NOOP_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(TO_LAB_NoopCmd_t) - 7)
                };

                memcpy(command->tolabnoopcmd.CmdHeader,     &mid, 2);
                memcpy(command->tolabnoopcmd.CmdHeader + 2, seq,  2);
                memcpy(command->tolabnoopcmd.CmdHeader + 4, len,  2);
                memcpy(command->tolabnoopcmd.CmdHeader + 6, &fnccode, 1);

                command->tolabnoopcmd.CmdHeader[7] = 0;
                uint16_t total = sizeof(TO_LAB_NoopCmd_t);
                const uint8_t *p = (const uint8_t *)&command->tolabnoopcmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->tolabnoopcmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(TO_LAB_NoopCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(TO_LAB_NoopCmd_t);
                memcpy(pkt->Data, &command->tolabnoopcmd, sizeof(TO_LAB_NoopCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
            }

            ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                        command->tolabnoopcmd.CmdHeader[0],
                        command->tolabnoopcmd.CmdHeader[1],
                        command->tolabnoopcmd.CmdHeader[2],
                        command->tolabnoopcmd.CmdHeader[3],
                        command->tolabnoopcmd.CmdHeader[4],
                        command->tolabnoopcmd.CmdHeader[5],
                        command->tolabnoopcmd.CmdHeader[6],
                        command->tolabnoopcmd.CmdHeader[7]);
            break;
        }
case 154: { // TO_LAB Reset Status
    static uint16_t msgid   = CFE_MISSION_TO_LAB_CMD_ID;
    static uint8_t  fnccode = TO_LAB_RESET_STATUS_CC;

    ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
    ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

    if (ImGui::Button("Generate CMD")) {
        WriteSystemName(msgid);

        uint16_t mid = htons(msgid);
        uint8_t  seq[2] = {0xC0, 0x00};
        uint8_t  len[2] = {
            0x00,
            (uint8_t)(sizeof(TO_LAB_ResetCountersCmd_t) - 7)
        };

        memcpy(command->tolabresetcounterscmd.CmdHeader,     &mid, 2);
        memcpy(command->tolabresetcounterscmd.CmdHeader + 2, seq,  2);
        memcpy(command->tolabresetcounterscmd.CmdHeader + 4, len,  2);
        memcpy(command->tolabresetcounterscmd.CmdHeader + 6, &fnccode, 1);

        command->tolabresetcounterscmd.CmdHeader[7] = 0;
        uint16_t total = sizeof(TO_LAB_ResetCountersCmd_t);
        const uint8_t *p = (const uint8_t *)&command->tolabresetcounterscmd;
        uint8_t crc = 0xFF;
        while (total--) crc ^= *(p++);

        memcpy(command->tolabresetcounterscmd.CmdHeader + 7, &crc, 1);

        packetsign *pkt =
            (packetsign *)malloc(2 + 2 + 4 + sizeof(TO_LAB_ResetCountersCmd_t));
        pkt->Identifier = HVD_TEST;
        pkt->PacketType = MIM_PT_TMTC_TEST;
        pkt->Length     = sizeof(TO_LAB_ResetCountersCmd_t);
        memcpy(pkt->Data, &command->tolabresetcounterscmd,
               sizeof(TO_LAB_ResetCountersCmd_t));

        pthread_join(p_thread[4], NULL);
        pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
    }

    ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                command->tolabresetcounterscmd.CmdHeader[0],
                command->tolabresetcounterscmd.CmdHeader[1],
                command->tolabresetcounterscmd.CmdHeader[2],
                command->tolabresetcounterscmd.CmdHeader[3],
                command->tolabresetcounterscmd.CmdHeader[4],
                command->tolabresetcounterscmd.CmdHeader[5],
                command->tolabresetcounterscmd.CmdHeader[6],
                command->tolabresetcounterscmd.CmdHeader[7]);
    break;
}

case 155: { // TO_LAB Add Packet
    static uint16_t msgid   = CFE_MISSION_TO_LAB_CMD_ID;
    static uint8_t  fnccode = TO_LAB_ADD_PKT_CC;

    ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
    ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);


    TO_LAB_AddPacket_Payload_t *pl = &command->tolabaddpacketcmd.Payload;

    ImGui::Separator();
    ImGui::Text("TO_LAB_AddPacket Payload");

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.95f, 0.55f, 1.0f));

    ImGui::Text("To Enable Beacon, Enter:");
    ImGui::Text("Stream MsgId = 2074(hex 0x081A), QoS Pri = 0, QoS Rel = 0, Buflim = 4");

    ImGui::Text("To Subscribe TO Event Msg, Enter:");
    ImGui::Text("Stream MsgId = 2052(hex 0x0804), QoS Pri = 0, QoS Rel = 0, Buflim = 4");
    ImGui::Text("Stream MsgId = 2053(hex 0x0805), QoS Pri = 0, QoS Rel = 0, Buflim = 4");

    ImGui::PopStyleColor();

    ImGui::InputScalar("Stream MsgId (Atom)", ImGuiDataType_U32, &pl->Stream.Value);
    ImGui::InputScalar("QoS Priority",        ImGuiDataType_U8,  &pl->Flags.Priority);
    ImGui::InputScalar("QoS Reliability",     ImGuiDataType_U8,  &pl->Flags.Reliability);
    ImGui::InputScalar("BufLimit",            ImGuiDataType_U8,  &pl->BufLimit);

    if (ImGui::Button("Generate CMD")) {
        WriteSystemName(msgid);

        uint16_t mid = htons(msgid);
        uint8_t  seq[2] = {0xC0, 0x00};
        uint8_t  len[2] = {
            0x00,
            (uint8_t)(sizeof(TO_LAB_AddPacketCmd_t) - 7)
        };

        memcpy(command->tolabaddpacketcmd.CmdHeader,     &mid, 2);
        memcpy(command->tolabaddpacketcmd.CmdHeader + 2, seq,  2);
        memcpy(command->tolabaddpacketcmd.CmdHeader + 4, len,  2);
        memcpy(command->tolabaddpacketcmd.CmdHeader + 6, &fnccode, 1);


        command->tolabaddpacketcmd.CmdHeader[7] = 0;
        uint16_t total = sizeof(TO_LAB_AddPacketCmd_t);
        const uint8_t *p = (const uint8_t *)&command->tolabaddpacketcmd;
        uint8_t crc = 0xFF;
        while (total--) crc ^= *(p++);

        memcpy(command->tolabaddpacketcmd.CmdHeader + 7, &crc, 1);

        packetsign *pkt =
            (packetsign *)malloc(2 + 2 + 4 + sizeof(TO_LAB_AddPacketCmd_t));
        pkt->Identifier = HVD_TEST;
        pkt->PacketType = MIM_PT_TMTC_TEST;
        pkt->Length     = sizeof(TO_LAB_AddPacketCmd_t);
        memcpy(pkt->Data, &command->tolabaddpacketcmd,
               sizeof(TO_LAB_AddPacketCmd_t));

        pthread_join(p_thread[4], NULL);
        pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
    }

    ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                command->tolabaddpacketcmd.CmdHeader[0],
                command->tolabaddpacketcmd.CmdHeader[1],
                command->tolabaddpacketcmd.CmdHeader[2],
                command->tolabaddpacketcmd.CmdHeader[3],
                command->tolabaddpacketcmd.CmdHeader[4],
                command->tolabaddpacketcmd.CmdHeader[5],
                command->tolabaddpacketcmd.CmdHeader[6],
                command->tolabaddpacketcmd.CmdHeader[7]);

    ImGui::Text("Payload: Stream=0x%08X, Priority=%u, Reliability=%u, BufLimit=%u",
                pl->Stream.Value, pl->Flags.Priority, pl->Flags.Reliability, pl->BufLimit);
    break;
}





        case 156: { // TO_LAB Send Data Types
            static uint16_t msgid   = CFE_MISSION_TO_LAB_CMD_ID;
            static uint8_t  fnccode = TO_LAB_SEND_DATA_TYPES_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(TO_LAB_SendDataTypesCmd_t) - 7)
                };

                memcpy(command->tolabsenddatatypescmd.CmdHeader,     &mid, 2);
                memcpy(command->tolabsenddatatypescmd.CmdHeader + 2, seq,  2);
                memcpy(command->tolabsenddatatypescmd.CmdHeader + 4, len,  2);
                memcpy(command->tolabsenddatatypescmd.CmdHeader + 6, &fnccode, 1);


                command->tolabsenddatatypescmd.CmdHeader[7] = 0;
                uint16_t total = sizeof(TO_LAB_SendDataTypesCmd_t);
                const uint8_t *p = (const uint8_t *)&command->tolabsenddatatypescmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->tolabsenddatatypescmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(TO_LAB_SendDataTypesCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(TO_LAB_SendDataTypesCmd_t);
                memcpy(pkt->Data, &command->tolabsenddatatypescmd,
                    sizeof(TO_LAB_SendDataTypesCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
            }

            ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                        command->tolabsenddatatypescmd.CmdHeader[0],
                        command->tolabsenddatatypescmd.CmdHeader[1],
                        command->tolabsenddatatypescmd.CmdHeader[2],
                        command->tolabsenddatatypescmd.CmdHeader[3],
                        command->tolabsenddatatypescmd.CmdHeader[4],
                        command->tolabsenddatatypescmd.CmdHeader[5],
                        command->tolabsenddatatypescmd.CmdHeader[6],
                        command->tolabsenddatatypescmd.CmdHeader[7]);
            break;
        }



        case 157: { // TO_LAB Remove Packet
            static uint16_t msgid   = CFE_MISSION_TO_LAB_CMD_ID;
            static uint8_t  fnccode = TO_LAB_REMOVE_PKT_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            TO_LAB_RemovePacket_Payload_t *pl = &command->tolabremovepacketcmd.Payload;

            ImGui::Separator();
            ImGui::Text("TO_LAB_RemovePacket Payload");
            ImGui::InputScalar("Stream MsgId (Atom)", ImGuiDataType_U32, &pl->Stream.Value);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(TO_LAB_RemovePacketCmd_t) - 7)
                };

                memcpy(command->tolabremovepacketcmd.CmdHeader,     &mid, 2);
                memcpy(command->tolabremovepacketcmd.CmdHeader + 2, seq,  2);
                memcpy(command->tolabremovepacketcmd.CmdHeader + 4, len,  2);
                memcpy(command->tolabremovepacketcmd.CmdHeader + 6, &fnccode, 1);

                command->tolabremovepacketcmd.CmdHeader[7] = 0;
                uint16_t total = sizeof(TO_LAB_RemovePacketCmd_t);
                const uint8_t *p = (const uint8_t *)&command->tolabremovepacketcmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->tolabremovepacketcmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(TO_LAB_RemovePacketCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(TO_LAB_RemovePacketCmd_t);
                memcpy(pkt->Data, &command->tolabremovepacketcmd,
                    sizeof(TO_LAB_RemovePacketCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
            }

            ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                        command->tolabremovepacketcmd.CmdHeader[0],
                        command->tolabremovepacketcmd.CmdHeader[1],
                        command->tolabremovepacketcmd.CmdHeader[2],
                        command->tolabremovepacketcmd.CmdHeader[3],
                        command->tolabremovepacketcmd.CmdHeader[4],
                        command->tolabremovepacketcmd.CmdHeader[5],
                        command->tolabremovepacketcmd.CmdHeader[6],
                        command->tolabremovepacketcmd.CmdHeader[7]);

            ImGui::Text("Payload: Stream=0x%08X", pl->Stream.Value);
            break;
        }

        case 158: { // TO_LAB Remove All Packets
            static uint16_t msgid   = CFE_MISSION_TO_LAB_CMD_ID;
            static uint8_t  fnccode = TO_LAB_REMOVE_ALL_PKT_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(TO_LAB_RemoveAllCmd_t) - 7)
                };

                memcpy(command->tolabremoveallcmd.CmdHeader,     &mid, 2);
                memcpy(command->tolabremoveallcmd.CmdHeader + 2, seq,  2);
                memcpy(command->tolabremoveallcmd.CmdHeader + 4, len,  2);
                memcpy(command->tolabremoveallcmd.CmdHeader + 6, &fnccode, 1);

                command->tolabremoveallcmd.CmdHeader[7] = 0;
                uint16_t total = sizeof(TO_LAB_RemoveAllCmd_t);
                const uint8_t *p = (const uint8_t *)&command->tolabremoveallcmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->tolabremoveallcmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(TO_LAB_RemoveAllCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(TO_LAB_RemoveAllCmd_t);
                memcpy(pkt->Data, &command->tolabremoveallcmd,
                    sizeof(TO_LAB_RemoveAllCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
            }

            ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                        command->tolabremoveallcmd.CmdHeader[0],
                        command->tolabremoveallcmd.CmdHeader[1],
                        command->tolabremoveallcmd.CmdHeader[2],
                        command->tolabremoveallcmd.CmdHeader[3],
                        command->tolabremoveallcmd.CmdHeader[4],
                        command->tolabremoveallcmd.CmdHeader[5],
                        command->tolabremoveallcmd.CmdHeader[6],
                        command->tolabremoveallcmd.CmdHeader[7]);
            break;
        }

        case 159: { // TO_LAB Output Enable
            static uint16_t msgid   = CFE_MISSION_TO_LAB_CMD_ID;
            static uint8_t  fnccode = TO_LAB_OUTPUT_ENABLE_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  seq[2] = {0xC0, 0x00};
                uint8_t  len[2] = {
                    0x00,
                    (uint8_t)(sizeof(TO_LAB_EnableOutputCmd_t) - 7)
                };

                memcpy(command->tolabenableoutputcmd.CmdHeader,     &mid, 2);
                memcpy(command->tolabenableoutputcmd.CmdHeader + 2, seq,  2);
                memcpy(command->tolabenableoutputcmd.CmdHeader + 4, len,  2);
                memcpy(command->tolabenableoutputcmd.CmdHeader + 6, &fnccode, 1);

                command->tolabenableoutputcmd.CmdHeader[7] = 0;
                uint16_t total = sizeof(TO_LAB_EnableOutputCmd_t);
                const uint8_t *p = (const uint8_t *)&command->tolabenableoutputcmd;
                uint8_t crc = 0xFF;
                while (total--) crc ^= *(p++);

                memcpy(command->tolabenableoutputcmd.CmdHeader + 7, &crc, 1);

                packetsign *pkt =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(TO_LAB_EnableOutputCmd_t));
                pkt->Identifier = HVD_TEST;
                pkt->PacketType = MIM_PT_TMTC_TEST;
                pkt->Length     = sizeof(TO_LAB_EnableOutputCmd_t);
                memcpy(pkt->Data, &command->tolabenableoutputcmd,
                    sizeof(TO_LAB_EnableOutputCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
            }

            ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                        command->tolabenableoutputcmd.CmdHeader[0],
                        command->tolabenableoutputcmd.CmdHeader[1],
                        command->tolabenableoutputcmd.CmdHeader[2],
                        command->tolabenableoutputcmd.CmdHeader[3],
                        command->tolabenableoutputcmd.CmdHeader[4],
                        command->tolabenableoutputcmd.CmdHeader[5],
                        command->tolabenableoutputcmd.CmdHeader[6],
                        command->tolabenableoutputcmd.CmdHeader[7]);

            break;
        }

        case 160: { // UANT Get Status


            
            static uint16_t msgid   = UANT_APP_CMD_ID;
            static uint8_t  fnccode = UANT_APP_GET_STATUS_CC;
            ImGui::Text("MsgID   : 0x%04X", msgid);
            ImGui::Text("FncCode : %u",     fnccode);


    if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            pthread_join(p_thread[4], NULL);

            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + 8);
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length = 8;
            memset(TestPacket->Data, 0, TestPacket->Length);

            uint8_t cmd[8] = {0,};
            msgid = htons(msgid);
            memcpy(cmd, &msgid, sizeof(uint16_t));
            memcpy(cmd + 6, &fnccode, sizeof(uint8_t));
            cmd[2] = 0xc0;
            cmd[3] = 0x00;
            cmd[4] = 0x00;
            cmd[5] = 0x01;

            uint16_t len = 8;
            const uint8_t* byteptr = cmd;
            uint8_t checksum = 0xFF;
            while (len--)
                checksum ^= *(byteptr++);
            cmd[7] = checksum;

            memcpy(TestPacket->Data, cmd, sizeof(cmd));
            for (int i = 0; i < TestPacket->Length; i++)
                printf("0x%x ", cmd[i]);
            printf("\n");

            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
            
            msgid = htons(msgid);
    }


            break;
        }







        case 161: { // BEE-1000 UANT uantburnchannelcmd
            static uint16_t msgid   = UANT_APP_CMD_ID;
            static uint8_t  fnccode = UANT_APP_BURN_CHANNEL_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("addr u8",     ImGuiDataType_U8,  &command->uantburnchannelcmd.addr);
            ImGui::InputScalar("channel u8",  ImGuiDataType_U8,  &command->uantburnchannelcmd.channel);
            ImGui::InputScalar("duration u8", ImGuiDataType_U8,  &command->uantburnchannelcmd.duration);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  sequence[2] = {0xC0, 0x00};
                uint8_t  length[2]   = {0x00, (uint8_t)(sizeof(UANT_BurnChannelCmd_t) - 7)};

                memcpy(command->uantburnchannelcmd.CmdHeader,       &mid,      sizeof(uint16_t));
                memcpy(command->uantburnchannelcmd.CmdHeader + 2,   sequence,  sizeof(uint16_t));
                memcpy(command->uantburnchannelcmd.CmdHeader + 4,   length,    sizeof(uint16_t));
                memcpy(command->uantburnchannelcmd.CmdHeader + 6,   &fnccode,  sizeof(uint8_t));

                command->uantburnchannelcmd.CmdHeader[7] = 0x00;
                uint16_t len = sizeof(UANT_BurnChannelCmd_t);
                const uint8_t *byteptr = (const uint8_t *)&command->uantburnchannelcmd;
                uint8_t checksum = 0xFF;
                while (len--) {
                    checksum ^= *(byteptr++);
                }
                memcpy(command->uantburnchannelcmd.CmdHeader + 7, &checksum, sizeof(uint8_t));

                // 래핑해서 uplink
                packetsign *TestPacket =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(UANT_BurnChannelCmd_t));
                TestPacket->Identifier = HVD_TEST;
                TestPacket->PacketType = MIM_PT_TMTC_TEST;
                TestPacket->Length     = sizeof(UANT_BurnChannelCmd_t);
                memcpy(TestPacket->Data, &command->uantburnchannelcmd,
                    sizeof(UANT_BurnChannelCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)TestPacket);
            }

            ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                        command->uantburnchannelcmd.CmdHeader[0],
                        command->uantburnchannelcmd.CmdHeader[1],
                        command->uantburnchannelcmd.CmdHeader[2],
                        command->uantburnchannelcmd.CmdHeader[3],
                        command->uantburnchannelcmd.CmdHeader[4],
                        command->uantburnchannelcmd.CmdHeader[5],
                        command->uantburnchannelcmd.CmdHeader[6],
                        command->uantburnchannelcmd.CmdHeader[7]);

            ImGui::Text("Params: addr=%u, channel=%u, duration=%u",
                        command->uantburnchannelcmd.addr,
                        command->uantburnchannelcmd.channel,
                        command->uantburnchannelcmd.duration);

            break;
        }

        case 162: { // BEE-1000 UANT uantsetsettingscmd
            static uint16_t msgid   = UANT_APP_CMD_ID;
            static uint8_t  fnccode = UANT_APP_SET_SETTINGS_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("addr u8",               ImGuiDataType_U8,  &command->uantsetsettingscmd.addr);
            ImGui::InputScalar("MinutesUntilDeploy u16",ImGuiDataType_U16, &command->uantsetsettingscmd.MinutesUntilDeploy);
            ImGui::InputScalar("BackupActive u8",       ImGuiDataType_U8,  &command->uantsetsettingscmd.BackupActive);
            ImGui::InputScalar("MaxBurnDuration u8",    ImGuiDataType_U8,  &command->uantsetsettingscmd.MaxBurnDuration);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  sequence[2] = {0xC0, 0x00};
                uint8_t  length[2]   = {0x00, (uint8_t)(sizeof(UANT_SetSettingsCmd_t) - 7)};

                memcpy(command->uantsetsettingscmd.CmdHeader,       &mid,      sizeof(uint16_t));
                memcpy(command->uantsetsettingscmd.CmdHeader + 2,   sequence,  sizeof(uint16_t));
                memcpy(command->uantsetsettingscmd.CmdHeader + 4,   length,    sizeof(uint16_t));
                memcpy(command->uantsetsettingscmd.CmdHeader + 6,   &fnccode,  sizeof(uint8_t));

                // CRC 계산
                command->uantsetsettingscmd.CmdHeader[7] = 0x00;
                uint16_t len = sizeof(UANT_SetSettingsCmd_t);
                const uint8_t *byteptr = (const uint8_t *)&command->uantsetsettingscmd;
                uint8_t checksum = 0xFF;
                while (len--) {
                    checksum ^= *(byteptr++);
                }
                memcpy(command->uantsetsettingscmd.CmdHeader + 7, &checksum, sizeof(uint8_t));

                packetsign *TestPacket =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(UANT_SetSettingsCmd_t));
                TestPacket->Identifier = HVD_TEST;
                TestPacket->PacketType = MIM_PT_TMTC_TEST;
                TestPacket->Length     = sizeof(UANT_SetSettingsCmd_t);
                memcpy(TestPacket->Data, &command->uantsetsettingscmd,
                    sizeof(UANT_SetSettingsCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)TestPacket);
            }

            ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                        command->uantsetsettingscmd.CmdHeader[0],
                        command->uantsetsettingscmd.CmdHeader[1],
                        command->uantsetsettingscmd.CmdHeader[2],
                        command->uantsetsettingscmd.CmdHeader[3],
                        command->uantsetsettingscmd.CmdHeader[4],
                        command->uantsetsettingscmd.CmdHeader[5],
                        command->uantsetsettingscmd.CmdHeader[6],
                        command->uantsetsettingscmd.CmdHeader[7]);

            ImGui::Text("Params: addr=%u, MinutesUntilDeploy=%u, BackupActive=%u, MaxBurnDuration=%u",
                        command->uantsetsettingscmd.addr,
                        command->uantsetsettingscmd.MinutesUntilDeploy,
                        command->uantsetsettingscmd.BackupActive,
                        command->uantsetsettingscmd.MaxBurnDuration);

            break;
        }


        case 163: { // BEE-1000 UANT uantautodeploycmd
            static uint16_t msgid   = UANT_APP_CMD_ID;
            static uint8_t  fnccode = UANT_APP_AUTODEPLOY_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            ImGui::InputScalar("SecondsDelay u16", ImGuiDataType_U16, &command->uantautodeploycmd.SecondsDelay);
            ImGui::InputScalar("addrslave1 u8",    ImGuiDataType_U8,  &command->uantautodeploycmd.addrslave1);
            ImGui::InputScalar("addrslave2 u8",    ImGuiDataType_U8,  &command->uantautodeploycmd.addrslave2);

            if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);

                uint16_t mid = htons(msgid);
                uint8_t  sequence[2] = {0xC0, 0x00};
                uint8_t  length[2]   = {0x00, (uint8_t)(sizeof(UANT_AutoDeployCmd_t) - 7)};

                memcpy(command->uantautodeploycmd.CmdHeader,       &mid,      sizeof(uint16_t));
                memcpy(command->uantautodeploycmd.CmdHeader + 2,   sequence,  sizeof(uint16_t));
                memcpy(command->uantautodeploycmd.CmdHeader + 4,   length,    sizeof(uint16_t));
                memcpy(command->uantautodeploycmd.CmdHeader + 6,   &fnccode,  sizeof(uint8_t));

                // CRC 계산
                command->uantautodeploycmd.CmdHeader[7] = 0x00;
                uint16_t len = sizeof(UANT_AutoDeployCmd_t);
                const uint8_t *byteptr = (const uint8_t *)&command->uantautodeploycmd;
                uint8_t checksum = 0xFF;
                while (len--) {
                    checksum ^= *(byteptr++);
                }
                memcpy(command->uantautodeploycmd.CmdHeader + 7, &checksum, sizeof(uint8_t));

                packetsign *TestPacket =
                    (packetsign *)malloc(2 + 2 + 4 + sizeof(UANT_AutoDeployCmd_t));
                TestPacket->Identifier = HVD_TEST;
                TestPacket->PacketType = MIM_PT_TMTC_TEST;
                TestPacket->Length     = sizeof(UANT_AutoDeployCmd_t);
                memcpy(TestPacket->Data, &command->uantautodeploycmd,
                    sizeof(UANT_AutoDeployCmd_t));

                pthread_join(p_thread[4], NULL);
                pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)TestPacket);
            }

            ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                        command->uantautodeploycmd.CmdHeader[0],
                        command->uantautodeploycmd.CmdHeader[1],
                        command->uantautodeploycmd.CmdHeader[2],
                        command->uantautodeploycmd.CmdHeader[3],
                        command->uantautodeploycmd.CmdHeader[4],
                        command->uantautodeploycmd.CmdHeader[5],
                        command->uantautodeploycmd.CmdHeader[6],
                        command->uantautodeploycmd.CmdHeader[7]);

            ImGui::Text("Params: SecondsDelay=%u, addrslave1=%u, addrslave2=%u",
                        command->uantautodeploycmd.SecondsDelay,
                        command->uantautodeploycmd.addrslave1,
                        command->uantautodeploycmd.addrslave2);

            break;
        }





        case 164: { // SP1 Deploy


            
            static uint16_t msgid   = SP_CMD_ID;
            static uint8_t  fnccode = SP_Deploy1_CC;
            ImGui::Text("MsgID   : 0x%04X", msgid);
            ImGui::Text("FncCode : %u",     fnccode);


    if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            pthread_join(p_thread[4], NULL);

            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + 8);
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length = 8;
            memset(TestPacket->Data, 0, TestPacket->Length);

            uint8_t cmd[8] = {0,};
            msgid = htons(msgid);
            memcpy(cmd, &msgid, sizeof(uint16_t));
            memcpy(cmd + 6, &fnccode, sizeof(uint8_t));
            cmd[2] = 0xc0;
            cmd[3] = 0x00;
            cmd[4] = 0x00;
            cmd[5] = 0x01;

            uint16_t len = 8;
            const uint8_t* byteptr = cmd;
            uint8_t checksum = 0xFF;
            while (len--)
                checksum ^= *(byteptr++);
            cmd[7] = checksum;

            memcpy(TestPacket->Data, cmd, sizeof(cmd));
            for (int i = 0; i < TestPacket->Length; i++)
                printf("0x%x ", cmd[i]);
            printf("\n");

            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
            
            msgid = htons(msgid);
    }


            break;
        }

        
        case 165: { // SP2


            
            static uint16_t msgid   = SP_CMD_ID;
            static uint8_t  fnccode = SP_Deploy2_CC;
            ImGui::Text("MsgID   : 0x%04X", msgid);
            ImGui::Text("FncCode : %u",     fnccode);


    if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            pthread_join(p_thread[4], NULL);

            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + 8);
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length = 8;
            memset(TestPacket->Data, 0, TestPacket->Length);

            uint8_t cmd[8] = {0,};
            msgid = htons(msgid);
            memcpy(cmd, &msgid, sizeof(uint16_t));
            memcpy(cmd + 6, &fnccode, sizeof(uint8_t));
            cmd[2] = 0xc0;
            cmd[3] = 0x00;
            cmd[4] = 0x00;
            cmd[5] = 0x01;

            uint16_t len = 8;
            const uint8_t* byteptr = cmd;
            uint8_t checksum = 0xFF;
            while (len--)
                checksum ^= *(byteptr++);
            cmd[7] = checksum;

            memcpy(TestPacket->Data, cmd, sizeof(cmd));
            for (int i = 0; i < TestPacket->Length; i++)
                printf("0x%x ", cmd[i]);
            printf("\n");

            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
            
            msgid = htons(msgid);
    }


            break;
        }

        
        case 166: { // SP3


            
            static uint16_t msgid   = SP_CMD_ID;
            static uint8_t  fnccode = SP_Deploy3_CC;
            ImGui::Text("MsgID   : 0x%04X", msgid);
            ImGui::Text("FncCode : %u",     fnccode);


    if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            pthread_join(p_thread[4], NULL);

            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + 8);
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length = 8;
            memset(TestPacket->Data, 0, TestPacket->Length);

            uint8_t cmd[8] = {0,};
            msgid = htons(msgid);
            memcpy(cmd, &msgid, sizeof(uint16_t));
            memcpy(cmd + 6, &fnccode, sizeof(uint8_t));
            cmd[2] = 0xc0;
            cmd[3] = 0x00;
            cmd[4] = 0x00;
            cmd[5] = 0x01;

            uint16_t len = 8;
            const uint8_t* byteptr = cmd;
            uint8_t checksum = 0xFF;
            while (len--)
                checksum ^= *(byteptr++);
            cmd[7] = checksum;

            memcpy(TestPacket->Data, cmd, sizeof(cmd));
            for (int i = 0; i < TestPacket->Length; i++)
                printf("0x%x ", cmd[i]);
            printf("\n");

            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
            
            msgid = htons(msgid);
    }


            break;
        }

        
        case 167: { // SP4


            
            static uint16_t msgid   = SP_CMD_ID;
            static uint8_t  fnccode = SP_Deploy4_CC;
            ImGui::Text("MsgID   : 0x%04X", msgid);
            ImGui::Text("FncCode : %u",     fnccode);


    if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            pthread_join(p_thread[4], NULL);

            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + 8);
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length = 8;
            memset(TestPacket->Data, 0, TestPacket->Length);

            uint8_t cmd[8] = {0,};
            msgid = htons(msgid);
            memcpy(cmd, &msgid, sizeof(uint16_t));
            memcpy(cmd + 6, &fnccode, sizeof(uint8_t));
            cmd[2] = 0xc0;
            cmd[3] = 0x00;
            cmd[4] = 0x00;
            cmd[5] = 0x01;

            uint16_t len = 8;
            const uint8_t* byteptr = cmd;
            uint8_t checksum = 0xFF;
            while (len--)
                checksum ^= *(byteptr++);
            cmd[7] = checksum;

            memcpy(TestPacket->Data, cmd, sizeof(cmd));
            for (int i = 0; i < TestPacket->Length; i++)
                printf("0x%x ", cmd[i]);
            printf("\n");

            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
            
            msgid = htons(msgid);
    }


            break;
        }

    case 168: { // PAYUZUC NOOP
        static uint16_t msgid   = PAYUZUC_CMD_ID;
        static uint8_t  fnccode = PAYUZUC_NOOP_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);

            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(PAYUZUC_NoopCmd_t) - 7)
            };

            memcpy(command->payuzucnoopcmd.CmdHeader,     &mid, 2);
            memcpy(command->payuzucnoopcmd.CmdHeader + 2, seq,  2);
            memcpy(command->payuzucnoopcmd.CmdHeader + 4, len,  2);
            memcpy(command->payuzucnoopcmd.CmdHeader + 6, &fnccode, 1);

            command->payuzucnoopcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUZUC_NoopCmd_t);
            const uint8_t *p = (const uint8_t *)&command->payuzucnoopcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            command->payuzucnoopcmd.CmdHeader[7] = crc;

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUZUC_NoopCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUZUC_NoopCmd_t);
            memcpy(pkt->Data, &command->payuzucnoopcmd,
                   sizeof(PAYUZUC_NoopCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                    command->payuzucnoopcmd.CmdHeader[0],
                    command->payuzucnoopcmd.CmdHeader[1],
                    command->payuzucnoopcmd.CmdHeader[2],
                    command->payuzucnoopcmd.CmdHeader[3],
                    command->payuzucnoopcmd.CmdHeader[4],
                    command->payuzucnoopcmd.CmdHeader[5],
                    command->payuzucnoopcmd.CmdHeader[6],
                    command->payuzucnoopcmd.CmdHeader[7]);
        break;
    }

    case 169: { // PAYUZUC Reset Counters
        static uint16_t msgid   = PAYUZUC_CMD_ID;
        static uint8_t  fnccode = PAYUZUC_RESET_COUNTERS_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);

            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(PAYUZUC_ResetCountersCmd_t) - 7)
            };

            memcpy(command->payuzucresetcounterscmd.CmdHeader,     &mid, 2);
            memcpy(command->payuzucresetcounterscmd.CmdHeader + 2, seq,  2);
            memcpy(command->payuzucresetcounterscmd.CmdHeader + 4, len,  2);
            memcpy(command->payuzucresetcounterscmd.CmdHeader + 6, &fnccode, 1);

            command->payuzucresetcounterscmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUZUC_ResetCountersCmd_t);
            const uint8_t *p = (const uint8_t *)&command->payuzucresetcounterscmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            command->payuzucresetcounterscmd.CmdHeader[7] = crc;

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUZUC_ResetCountersCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUZUC_ResetCountersCmd_t);
            memcpy(pkt->Data, &command->payuzucresetcounterscmd,
                   sizeof(PAYUZUC_ResetCountersCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                    command->payuzucresetcounterscmd.CmdHeader[0],
                    command->payuzucresetcounterscmd.CmdHeader[1],
                    command->payuzucresetcounterscmd.CmdHeader[2],
                    command->payuzucresetcounterscmd.CmdHeader[3],
                    command->payuzucresetcounterscmd.CmdHeader[4],
                    command->payuzucresetcounterscmd.CmdHeader[5],
                    command->payuzucresetcounterscmd.CmdHeader[6],
                    command->payuzucresetcounterscmd.CmdHeader[7]);
        break;
    }

    case 170: { // PAYUZUC Ping
        static uint16_t msgid   = PAYUZUC_CMD_ID;
        static uint8_t  fnccode = PAYUZUC_PING_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        PAYUZUC_Ping_Payload_t *pl = &command->payuzucpingcmd.Payload;

        ImGui::Separator();
        ImGui::Text("PAYUZUC Ping Payload");
        ImGui::Text("PN: 0 = MCU only, 1 = MCU + Img Sensor");
        ImGui::InputScalar("PN", ImGuiDataType_U8, &pl->PN);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);

            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(PAYUZUC_PingCmd_t) - 7)
            };

            memcpy(command->payuzucpingcmd.CmdHeader,     &mid, 2);
            memcpy(command->payuzucpingcmd.CmdHeader + 2, seq,  2);
            memcpy(command->payuzucpingcmd.CmdHeader + 4, len,  2);
            memcpy(command->payuzucpingcmd.CmdHeader + 6, &fnccode, 1);

            command->payuzucpingcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUZUC_PingCmd_t);
            const uint8_t *p = (const uint8_t *)&command->payuzucpingcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            command->payuzucpingcmd.CmdHeader[7] = crc;

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUZUC_PingCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUZUC_PingCmd_t);
            memcpy(pkt->Data, &command->payuzucpingcmd,
                   sizeof(PAYUZUC_PingCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                    command->payuzucpingcmd.CmdHeader[0],
                    command->payuzucpingcmd.CmdHeader[1],
                    command->payuzucpingcmd.CmdHeader[2],
                    command->payuzucpingcmd.CmdHeader[3],
                    command->payuzucpingcmd.CmdHeader[4],
                    command->payuzucpingcmd.CmdHeader[5],
                    command->payuzucpingcmd.CmdHeader[6],
                    command->payuzucpingcmd.CmdHeader[7]);
        ImGui::Text("Payload: PN=%u", pl->PN);
        break;
    }

    case 171: { // PAYUZUC Set Mode
        static uint16_t msgid   = PAYUZUC_CMD_ID;
        static uint8_t  fnccode = PAYUZUC_SET_MODE_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        PAYUZUC_SetMode_Payload_t *pl = &command->payuzucsetmodecmd.Payload;

        ImGui::Separator();
        ImGui::Text("PAYUZUC Set Mode Payload");
        ImGui::Text("MD: 0 = Sleep, 1 = SD, 2 = HD");
        ImGui::InputScalar("MD", ImGuiDataType_U8, &pl->MD);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);

            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(PAYUZUC_SetModeCmd_t) - 7)
            };

            memcpy(command->payuzucsetmodecmd.CmdHeader,     &mid, 2);
            memcpy(command->payuzucsetmodecmd.CmdHeader + 2, seq,  2);
            memcpy(command->payuzucsetmodecmd.CmdHeader + 4, len,  2);
            memcpy(command->payuzucsetmodecmd.CmdHeader + 6, &fnccode, 1);

            command->payuzucsetmodecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUZUC_SetModeCmd_t);
            const uint8_t *p = (const uint8_t *)&command->payuzucsetmodecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            command->payuzucsetmodecmd.CmdHeader[7] = crc;

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUZUC_SetModeCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUZUC_SetModeCmd_t);
            memcpy(pkt->Data, &command->payuzucsetmodecmd,
                   sizeof(PAYUZUC_SetModeCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                    command->payuzucsetmodecmd.CmdHeader[0],
                    command->payuzucsetmodecmd.CmdHeader[1],
                    command->payuzucsetmodecmd.CmdHeader[2],
                    command->payuzucsetmodecmd.CmdHeader[3],
                    command->payuzucsetmodecmd.CmdHeader[4],
                    command->payuzucsetmodecmd.CmdHeader[5],
                    command->payuzucsetmodecmd.CmdHeader[6],
                    command->payuzucsetmodecmd.CmdHeader[7]);
        ImGui::Text("Payload: MD=%u", pl->MD);
        break;
    }

    case 172: { // PAYUZUC Memory Status (no-arg)
        static uint16_t msgid   = PAYUZUC_CMD_ID;
        static uint8_t  fnccode = PAYUZUC_MEMORY_STATUS_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);

            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(PAYUZUC_MemoryStatusCmd_t) - 7)
            };

            memcpy(command->payuzucmemorystatuscmd.CmdHeader,     &mid, 2);
            memcpy(command->payuzucmemorystatuscmd.CmdHeader + 2, seq,  2);
            memcpy(command->payuzucmemorystatuscmd.CmdHeader + 4, len,  2);
            memcpy(command->payuzucmemorystatuscmd.CmdHeader + 6, &fnccode, 1);

            command->payuzucmemorystatuscmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUZUC_MemoryStatusCmd_t);
            const uint8_t *p = (const uint8_t *)&command->payuzucmemorystatuscmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            command->payuzucmemorystatuscmd.CmdHeader[7] = crc;

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUZUC_MemoryStatusCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUZUC_MemoryStatusCmd_t);
            memcpy(pkt->Data, &command->payuzucmemorystatuscmd,
                   sizeof(PAYUZUC_MemoryStatusCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                    command->payuzucmemorystatuscmd.CmdHeader[0],
                    command->payuzucmemorystatuscmd.CmdHeader[1],
                    command->payuzucmemorystatuscmd.CmdHeader[2],
                    command->payuzucmemorystatuscmd.CmdHeader[3],
                    command->payuzucmemorystatuscmd.CmdHeader[4],
                    command->payuzucmemorystatuscmd.CmdHeader[5],
                    command->payuzucmemorystatuscmd.CmdHeader[6],
                    command->payuzucmemorystatuscmd.CmdHeader[7]);
        break;
    }

    case 173: { // PAYUZUC Set Exposure
        static uint16_t msgid   = PAYUZUC_CMD_ID;
        static uint8_t  fnccode = PAYUZUC_SET_EXPOSURE_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        PAYUZUC_SetExposure_Payload_t *pl = &command->payuzucsetexposurecmd.Payload;

        ImGui::Separator();
        ImGui::Text("PAYUZUC Set Exposure Payload");
        ImGui::InputScalar("EX1", ImGuiDataType_U8, &pl->EX1);
        ImGui::InputScalar("EX2", ImGuiDataType_U8, &pl->EX2);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);

            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(PAYUZUC_SetExposureCmd_t) - 7)
            };

            memcpy(command->payuzucsetexposurecmd.CmdHeader,     &mid, 2);
            memcpy(command->payuzucsetexposurecmd.CmdHeader + 2, seq,  2);
            memcpy(command->payuzucsetexposurecmd.CmdHeader + 4, len,  2);
            memcpy(command->payuzucsetexposurecmd.CmdHeader + 6, &fnccode, 1);

            command->payuzucsetexposurecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUZUC_SetExposureCmd_t);
            const uint8_t *p = (const uint8_t *)&command->payuzucsetexposurecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            command->payuzucsetexposurecmd.CmdHeader[7] = crc;

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUZUC_SetExposureCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUZUC_SetExposureCmd_t);
            memcpy(pkt->Data, &command->payuzucsetexposurecmd,
                   sizeof(PAYUZUC_SetExposureCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                    command->payuzucsetexposurecmd.CmdHeader[0],
                    command->payuzucsetexposurecmd.CmdHeader[1],
                    command->payuzucsetexposurecmd.CmdHeader[2],
                    command->payuzucsetexposurecmd.CmdHeader[3],
                    command->payuzucsetexposurecmd.CmdHeader[4],
                    command->payuzucsetexposurecmd.CmdHeader[5],
                    command->payuzucsetexposurecmd.CmdHeader[6],
                    command->payuzucsetexposurecmd.CmdHeader[7]);
        ImGui::Text("Payload: EX1=%u, EX2=%u", pl->EX1, pl->EX2);
        break;
    }

    case 174: { // PAYUZUC Capture
        static uint16_t msgid   = PAYUZUC_CMD_ID;
        static uint8_t  fnccode = PAYUZUC_CAPTURE_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        PAYUZUC_Capture_Payload_t *pl = &command->payuzuccapturecmd.Payload;

        ImGui::Separator();
        ImGui::Text("PAYUZUC Capture Payload");
        ImGui::InputScalar("MEM", ImGuiDataType_U8, &pl->MEM);
        ImGui::InputScalar("TST", ImGuiDataType_U8, &pl->TST);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);

            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(PAYUZUC_CaptureCmd_t) - 7)
            };

            memcpy(command->payuzuccapturecmd.CmdHeader,     &mid, 2);
            memcpy(command->payuzuccapturecmd.CmdHeader + 2, seq,  2);
            memcpy(command->payuzuccapturecmd.CmdHeader + 4, len,  2);
            memcpy(command->payuzuccapturecmd.CmdHeader + 6, &fnccode, 1);

            command->payuzuccapturecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUZUC_CaptureCmd_t);
            const uint8_t *p = (const uint8_t *)&command->payuzuccapturecmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            command->payuzuccapturecmd.CmdHeader[7] = crc;

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUZUC_CaptureCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUZUC_CaptureCmd_t);
            memcpy(pkt->Data, &command->payuzuccapturecmd,
                   sizeof(PAYUZUC_CaptureCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                    command->payuzuccapturecmd.CmdHeader[0],
                    command->payuzuccapturecmd.CmdHeader[1],
                    command->payuzuccapturecmd.CmdHeader[2],
                    command->payuzuccapturecmd.CmdHeader[3],
                    command->payuzuccapturecmd.CmdHeader[4],
                    command->payuzuccapturecmd.CmdHeader[5],
                    command->payuzuccapturecmd.CmdHeader[6],
                    command->payuzuccapturecmd.CmdHeader[7]);
        ImGui::Text("Payload: MEM=%u, TST=%u", pl->MEM, pl->TST);
        break;
    }

    case 175: { // PAYUZUC Download
        static uint16_t msgid   = PAYUZUC_CMD_ID;
        static uint8_t  fnccode = PAYUZUC_DOWNLOAD_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        PAYUZUC_Download_Payload_t *pl = &command->payuzucdownloadcmd.Payload;

        ImGui::Separator();
        ImGui::Text("PAYUZUC Download Payload");
        ImGui::InputScalar("MEM", ImGuiDataType_U8, &pl->MEM);
        ImGui::InputScalar("PRE", ImGuiDataType_U8, &pl->PRE);
        ImGui::InputScalar("LN1", ImGuiDataType_U8, &pl->LN1);
        ImGui::InputScalar("LN2", ImGuiDataType_U8, &pl->LN2);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);

            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(PAYUZUC_DownloadCmd_t) - 7)
            };

            memcpy(command->payuzucdownloadcmd.CmdHeader,     &mid, 2);
            memcpy(command->payuzucdownloadcmd.CmdHeader + 2, seq,  2);
            memcpy(command->payuzucdownloadcmd.CmdHeader + 4, len,  2);
            memcpy(command->payuzucdownloadcmd.CmdHeader + 6, &fnccode, 1);

            command->payuzucdownloadcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUZUC_DownloadCmd_t);
            const uint8_t *p = (const uint8_t *)&command->payuzucdownloadcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            command->payuzucdownloadcmd.CmdHeader[7] = crc;

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUZUC_DownloadCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUZUC_DownloadCmd_t);
            memcpy(pkt->Data, &command->payuzucdownloadcmd,
                   sizeof(PAYUZUC_DownloadCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                    command->payuzucdownloadcmd.CmdHeader[0],
                    command->payuzucdownloadcmd.CmdHeader[1],
                    command->payuzucdownloadcmd.CmdHeader[2],
                    command->payuzucdownloadcmd.CmdHeader[3],
                    command->payuzucdownloadcmd.CmdHeader[4],
                    command->payuzucdownloadcmd.CmdHeader[5],
                    command->payuzucdownloadcmd.CmdHeader[6],
                    command->payuzucdownloadcmd.CmdHeader[7]);
        ImGui::Text("Payload: MEM=%u, PRE=%u, LN1=%u, LN2=%u",
                    pl->MEM, pl->PRE, pl->LN1, pl->LN2);
        break;
    }

    case 176: { // PAYUZUC Read Register
        static uint16_t msgid   = PAYUZUC_CMD_ID;
        static uint8_t  fnccode = PAYUZUC_READ_REGISTER_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        PAYUZUC_ReadRegister_Payload_t *pl = &command->payuzucreadregistercmd.Payload;

        ImGui::Separator();
        ImGui::Text("PAYUZUC Read Register Payload");
        ImGui::InputScalar("AD1", ImGuiDataType_U8, &pl->AD1);
        ImGui::InputScalar("AD2", ImGuiDataType_U8, &pl->AD2);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);

            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(PAYUZUC_ReadRegisterCmd_t) - 7)
            };

            memcpy(command->payuzucreadregistercmd.CmdHeader,     &mid, 2);
            memcpy(command->payuzucreadregistercmd.CmdHeader + 2, seq,  2);
            memcpy(command->payuzucreadregistercmd.CmdHeader + 4, len,  2);
            memcpy(command->payuzucreadregistercmd.CmdHeader + 6, &fnccode, 1);

            command->payuzucreadregistercmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUZUC_ReadRegisterCmd_t);
            const uint8_t *p = (const uint8_t *)&command->payuzucreadregistercmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            command->payuzucreadregistercmd.CmdHeader[7] = crc;

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUZUC_ReadRegisterCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUZUC_ReadRegisterCmd_t);
            memcpy(pkt->Data, &command->payuzucreadregistercmd,
                   sizeof(PAYUZUC_ReadRegisterCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                    command->payuzucreadregistercmd.CmdHeader[0],
                    command->payuzucreadregistercmd.CmdHeader[1],
                    command->payuzucreadregistercmd.CmdHeader[2],
                    command->payuzucreadregistercmd.CmdHeader[3],
                    command->payuzucreadregistercmd.CmdHeader[4],
                    command->payuzucreadregistercmd.CmdHeader[5],
                    command->payuzucreadregistercmd.CmdHeader[6],
                    command->payuzucreadregistercmd.CmdHeader[7]);
        ImGui::Text("Payload: AD1=%u, AD2=%u", pl->AD1, pl->AD2);
        break;
    }

    case 177: { // PAYUZUC Write Register
        static uint16_t msgid   = PAYUZUC_CMD_ID;
        static uint8_t  fnccode = PAYUZUC_WRITE_REGISTER_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        PAYUZUC_WriteRegister_Payload_t *pl = &command->payuzucwriteregistercmd.Payload;

        ImGui::Separator();
        ImGui::Text("PAYUZUC Write Register Payload");
        ImGui::InputScalar("AD1", ImGuiDataType_U8, &pl->AD1);
        ImGui::InputScalar("AD2", ImGuiDataType_U8, &pl->AD2);
        ImGui::InputScalar("RG1", ImGuiDataType_U8, &pl->RG1);
        ImGui::InputScalar("RG2", ImGuiDataType_U8, &pl->RG2);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);

            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(PAYUZUC_WriteRegisterCmd_t) - 7)
            };

            memcpy(command->payuzucwriteregistercmd.CmdHeader,     &mid, 2);
            memcpy(command->payuzucwriteregistercmd.CmdHeader + 2, seq,  2);
            memcpy(command->payuzucwriteregistercmd.CmdHeader + 4, len,  2);
            memcpy(command->payuzucwriteregistercmd.CmdHeader + 6, &fnccode, 1);

            command->payuzucwriteregistercmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUZUC_WriteRegisterCmd_t);
            const uint8_t *p = (const uint8_t *)&command->payuzucwriteregistercmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            command->payuzucwriteregistercmd.CmdHeader[7] = crc;

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUZUC_WriteRegisterCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUZUC_WriteRegisterCmd_t);
            memcpy(pkt->Data, &command->payuzucwriteregistercmd,
                   sizeof(PAYUZUC_WriteRegisterCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                    command->payuzucwriteregistercmd.CmdHeader[0],
                    command->payuzucwriteregistercmd.CmdHeader[1],
                    command->payuzucwriteregistercmd.CmdHeader[2],
                    command->payuzucwriteregistercmd.CmdHeader[3],
                    command->payuzucwriteregistercmd.CmdHeader[4],
                    command->payuzucwriteregistercmd.CmdHeader[5],
                    command->payuzucwriteregistercmd.CmdHeader[6],
                    command->payuzucwriteregistercmd.CmdHeader[7]);
        ImGui::Text("Payload: AD1=%u, AD2=%u, RG1=%u, RG2=%u",
                    pl->AD1, pl->AD2, pl->RG1, pl->RG2);
        break;
    }

    case 178: { // PAYUZUC Download All
        static uint16_t msgid   = PAYUZUC_CMD_ID;
        static uint8_t  fnccode = PAYUZUC_DOWNLOAD_ALL_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        PAYUZUC_DownloadAll_Payload_t *pl = &command->payuzucdownloadallcmd.Payload;

        ImGui::Separator();
        ImGui::Text("PAYUZUC Download All Payload");
        ImGui::InputScalar("MEM",       ImGuiDataType_U8,  &pl->MEM);
        ImGui::InputScalar("PRE",       ImGuiDataType_U8,  &pl->PRE);
        ImGui::InputScalar("StartLine", ImGuiDataType_U16, &pl->StartLine);
        ImGui::InputScalar("LineNum",   ImGuiDataType_U16, &pl->LineNum);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);

            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(PAYUZUC_DownloadAllCmd_t) - 7)
            };

            memcpy(command->payuzucdownloadallcmd.CmdHeader,     &mid, 2);
            memcpy(command->payuzucdownloadallcmd.CmdHeader + 2, seq,  2);
            memcpy(command->payuzucdownloadallcmd.CmdHeader + 4, len,  2);
            memcpy(command->payuzucdownloadallcmd.CmdHeader + 6, &fnccode, 1);

            command->payuzucdownloadallcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUZUC_DownloadAllCmd_t);
            const uint8_t *p = (const uint8_t *)&command->payuzucdownloadallcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            command->payuzucdownloadallcmd.CmdHeader[7] = crc;

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUZUC_DownloadAllCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUZUC_DownloadAllCmd_t);
            memcpy(pkt->Data, &command->payuzucdownloadallcmd,
                   sizeof(PAYUZUC_DownloadAllCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                    command->payuzucdownloadallcmd.CmdHeader[0],
                    command->payuzucdownloadallcmd.CmdHeader[1],
                    command->payuzucdownloadallcmd.CmdHeader[2],
                    command->payuzucdownloadallcmd.CmdHeader[3],
                    command->payuzucdownloadallcmd.CmdHeader[4],
                    command->payuzucdownloadallcmd.CmdHeader[5],
                    command->payuzucdownloadallcmd.CmdHeader[6],
                    command->payuzucdownloadallcmd.CmdHeader[7]);
        ImGui::Text("Payload: MEM=%u, PRE=%u, StartLine=%u, LineNum=%u",
                    pl->MEM, pl->PRE, pl->StartLine, pl->LineNum);
        break;
    }

    case 179: { // PAYUZUC Mosaic
        static uint16_t msgid   = PAYUZUC_CMD_ID;
        static uint8_t  fnccode = PAYUZUC_MOSAIC_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        PAYUZUC_Mosaic_Payload_t *pl = &command->payuzucmosaiccmd.Payload;

        ImGui::Separator();
        ImGui::Text("PAYUZUC Mosaic Payload");
        ImGui::InputScalar("MEM", ImGuiDataType_U8, &pl->MEM);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);

            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(PAYUZUC_MosaicCmd_t) - 7)
            };

            memcpy(command->payuzucmosaiccmd.CmdHeader,     &mid, 2);
            memcpy(command->payuzucmosaiccmd.CmdHeader + 2, seq,  2);
            memcpy(command->payuzucmosaiccmd.CmdHeader + 4, len,  2);
            memcpy(command->payuzucmosaiccmd.CmdHeader + 6, &fnccode, 1);

            command->payuzucmosaiccmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUZUC_MosaicCmd_t);
            const uint8_t *p = (const uint8_t *)&command->payuzucmosaiccmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            command->payuzucmosaiccmd.CmdHeader[7] = crc;

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUZUC_MosaicCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUZUC_MosaicCmd_t);
            memcpy(pkt->Data, &command->payuzucmosaiccmd,
                   sizeof(PAYUZUC_MosaicCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                    command->payuzucmosaiccmd.CmdHeader[0],
                    command->payuzucmosaiccmd.CmdHeader[1],
                    command->payuzucmosaiccmd.CmdHeader[2],
                    command->payuzucmosaiccmd.CmdHeader[3],
                    command->payuzucmosaiccmd.CmdHeader[4],
                    command->payuzucmosaiccmd.CmdHeader[5],
                    command->payuzucmosaiccmd.CmdHeader[6],
                    command->payuzucmosaiccmd.CmdHeader[7]);
        ImGui::Text("Payload: MEM=%u", pl->MEM);
        break;
    }

    case 180: { // PAYUZUC Download All Child (CC 23, payload same as DownloadAll)
        static uint16_t msgid   = PAYUZUC_CMD_ID;
        static uint8_t  fnccode = PAYUZUC_DOWNLOAD_ALL_CHILD_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        PAYUZUC_DownloadAll_Payload_t *pl = &command->payuzucdownloadallchildcmd.Payload;

        ImGui::Separator();
        ImGui::Text("PAYUZUC Download All Child Payload");
        ImGui::InputScalar("MEM",       ImGuiDataType_U8,  &pl->MEM);
        ImGui::InputScalar("PRE",       ImGuiDataType_U8,  &pl->PRE);
        ImGui::InputScalar("StartLine", ImGuiDataType_U16, &pl->StartLine);
        ImGui::InputScalar("LineNum",   ImGuiDataType_U16, &pl->LineNum);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);

            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(PAYUZUC_DownloadAllCmd_t) - 7)
            };

            memcpy(command->payuzucdownloadallchildcmd.CmdHeader,     &mid, 2);
            memcpy(command->payuzucdownloadallchildcmd.CmdHeader + 2, seq,  2);
            memcpy(command->payuzucdownloadallchildcmd.CmdHeader + 4, len,  2);
            memcpy(command->payuzucdownloadallchildcmd.CmdHeader + 6, &fnccode, 1);

            command->payuzucdownloadallchildcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUZUC_DownloadAllCmd_t);
            const uint8_t *p = (const uint8_t *)&command->payuzucdownloadallchildcmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);
            command->payuzucdownloadallchildcmd.CmdHeader[7] = crc;

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUZUC_DownloadAllCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUZUC_DownloadAllCmd_t);
            memcpy(pkt->Data, &command->payuzucdownloadallchildcmd,
                   sizeof(PAYUZUC_DownloadAllCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                    command->payuzucdownloadallchildcmd.CmdHeader[0],
                    command->payuzucdownloadallchildcmd.CmdHeader[1],
                    command->payuzucdownloadallchildcmd.CmdHeader[2],
                    command->payuzucdownloadallchildcmd.CmdHeader[3],
                    command->payuzucdownloadallchildcmd.CmdHeader[4],
                    command->payuzucdownloadallchildcmd.CmdHeader[5],
                    command->payuzucdownloadallchildcmd.CmdHeader[6],
                    command->payuzucdownloadallchildcmd.CmdHeader[7]);
        ImGui::Text("Payload: MEM=%u, PRE=%u, StartLine=%u, LineNum=%u",
                    pl->MEM, pl->PRE, pl->StartLine, pl->LineNum);
        break;
    }











    case 201: { // ADDED FOR FTP TEST 1210, delete To_lab.so
        static uint16_t msgid = 0x1815;
        static uint8_t fnccode = 5;
        static char filename_buf[64] = "/cf/to_lab.so";

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);
        ImGui::InputText("Filename (char 64)", filename_buf, sizeof(filename_buf));


        if (ImGui::Button("Generate CMD")) {
            uint16_t msgid_be = htons(msgid);

            memset(command->todeletecmd.filename, 0, sizeof(command->todeletecmd.filename));
            strncpy(command->todeletecmd.filename, filename_buf, sizeof(command->todeletecmd.filename) - 1);

            uint8_t sequence[2] = {0xC0, 0x00};
            uint8_t  length[2]   = {0x00, (uint8_t)(sizeof(TO_DeleteCmd_t) - 7)};
            memcpy(command->todeletecmd.CmdHeader + 0, &msgid_be, sizeof(uint16_t));
            memcpy(command->todeletecmd.CmdHeader + 2, sequence, sizeof(uint16_t));
            memcpy(command->todeletecmd.CmdHeader + 4, length, sizeof(uint16_t));
            memcpy(command->todeletecmd.CmdHeader + 6, &fnccode, sizeof(uint8_t));


            command->todeletecmd.CmdHeader[7] = 0x00;

            pthread_join(p_thread[4], NULL);
            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + sizeof(TO_DeleteCmd_t));
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;

            TestPacket->Length = sizeof(TO_DeleteCmd_t);
            uint16_t len = sizeof(TO_DeleteCmd_t);


            const uint8_t* byteptr = reinterpret_cast<const uint8_t*>(&command->todeletecmd);

            uint8_t checksum = 0xFF;
            while (len--) checksum ^= *(byteptr++);
            memcpy(command->todeletecmd.CmdHeader + 7, &checksum, sizeof(uint8_t));
            memcpy(TestPacket->Data, &command->todeletecmd, sizeof(TO_DeleteCmd_t));
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
        }


        break;
    }










    case 202: { // ADDED FOR FTP TEST 1210, rename To_lab.so
        static uint16_t msgid = 0x1815;
        static uint8_t fnccode = 4;
        static char source_buf[64] = "/cf/to_lab23.so";
        static char target_buf[64] = "/cf/to_lab.so";

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);
        ImGui::InputText("Source (char 64)", source_buf, sizeof(source_buf));
        ImGui::InputText("Target (char 64)", target_buf, sizeof(target_buf));


        if (ImGui::Button("Generate CMD")) {
            uint16_t msgid_be = htons(msgid);
            memset(command->torenamecmd.source, 0, sizeof(command->torenamecmd.source));
            strncpy(command->torenamecmd.source, source_buf, sizeof(command->torenamecmd.source) - 1);

            memset(command->torenamecmd.target, 0, sizeof(command->torenamecmd.target));
            strncpy(command->torenamecmd.target, target_buf, sizeof(command->torenamecmd.target) - 1);

            uint8_t sequence[2] = {0xC0, 0x00};
            uint8_t  length[2]   = {0x00, (uint8_t)(sizeof(TO_RenameCmd_t) - 7)};
            memcpy(command->torenamecmd.CmdHeader + 0, &msgid_be, sizeof(uint16_t));
            memcpy(command->torenamecmd.CmdHeader + 2, sequence, sizeof(uint16_t));
            memcpy(command->torenamecmd.CmdHeader + 4, length, sizeof(uint16_t));
            memcpy(command->torenamecmd.CmdHeader + 6, &fnccode, sizeof(uint8_t));


            command->torenamecmd.CmdHeader[7] = 0x00;

            pthread_join(p_thread[4], NULL);
            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + sizeof(TO_RenameCmd_t));
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;

            TestPacket->Length = sizeof(TO_RenameCmd_t);
            uint16_t len = sizeof(TO_RenameCmd_t);


            const uint8_t* byteptr = reinterpret_cast<const uint8_t*>(&command->torenamecmd);

            uint8_t checksum = 0xFF;
            while (len--) checksum ^= *(byteptr++);
            memcpy(command->torenamecmd.CmdHeader + 7, &checksum, sizeof(uint8_t));
            memcpy(TestPacket->Data, &command->torenamecmd, sizeof(TO_RenameCmd_t));
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
        }


        break;
    }








    case 203: { 
        static uint16_t msgid = 0;
        static uint8_t  fnccode = 0;
        static char     name_buf[64] = "";


        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        ImGui::InputText("name", name_buf, sizeof(name_buf));
        ImGui::InputScalar("start_byte u32", ImGuiDataType_U32, &command->ftpsendfilecmd.start_byte);
        ImGui::InputScalar("end_byte u32",   ImGuiDataType_U32, &command->ftpsendfilecmd.end_byte);
        ImGui::InputScalar("interval u8",    ImGuiDataType_U8,  &command->ftpsendfilecmd.interval);

        ImGui::Separator();
        ImGui::Text("Padding (3 bytes)");
        ImGui::Separator();

        ImGui::InputScalar("padding[0] u8", ImGuiDataType_U8, &command->ftpsendfilecmd.padding[0]);
        ImGui::InputScalar("padding[1] u8", ImGuiDataType_U8, &command->ftpsendfilecmd.padding[1]);
        ImGui::InputScalar("padding[2] u8", ImGuiDataType_U8, &command->ftpsendfilecmd.padding[2]);

        if (ImGui::Button("Generate CMD")) {
            uint16_t msgid_be = htons(msgid);

            memset(command->ftpsendfilecmd.name, 0, sizeof(command->ftpsendfilecmd.name));
            strncpy(reinterpret_cast<char*>(command->ftpsendfilecmd.name), name_buf, sizeof(command->ftpsendfilecmd.name)-1);

            uint8_t sequence[2] = {0xC0, 0x00};
            uint8_t length[2]   = {0x00, 0x4D};

            memcpy(command->ftpsendfilecmd.CmdHeader + 0, &msgid_be, sizeof(uint16_t));
            memcpy(command->ftpsendfilecmd.CmdHeader + 2, sequence,  sizeof(uint16_t));
            memcpy(command->ftpsendfilecmd.CmdHeader + 4, length,    sizeof(uint16_t));
            memcpy(command->ftpsendfilecmd.CmdHeader + 6, &fnccode,  sizeof(uint8_t));

            command->ftpsendfilecmd.CmdHeader[7] = 0x00;

            pthread_join(p_thread[4], NULL);

            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + sizeof(FTP_sendfileCmd_t));
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length     = sizeof(FTP_sendfileCmd_t);



            
            uint16_t len = sizeof(FTP_sendfileCmd_t);
            const uint8_t* byteptr = reinterpret_cast<const uint8_t*>(&command->ftpsendfilecmd);
            uint8_t checksum = 0xFF;
            while (len--) checksum ^= *(byteptr++);

            memcpy(command->ftpsendfilecmd.CmdHeader + 7, &checksum, sizeof(uint8_t));

            memcpy(TestPacket->Data, &command->ftpsendfilecmd, sizeof(FTP_sendfileCmd_t));

            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);


        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
            command->ftpsendfilecmd.CmdHeader[0],
            command->ftpsendfilecmd.CmdHeader[1],
            command->ftpsendfilecmd.CmdHeader[2],
            command->ftpsendfilecmd.CmdHeader[3],
            command->ftpsendfilecmd.CmdHeader[4],
            command->ftpsendfilecmd.CmdHeader[5],
            command->ftpsendfilecmd.CmdHeader[6],
            command->ftpsendfilecmd.CmdHeader[7]);

        ImGui::Text("Params:");
        ImGui::Text("  name        = %s", name_buf);
        ImGui::Text("  start_byte  = %u", command->ftpsendfilecmd.start_byte);
        ImGui::Text("  end_byte    = %u", command->ftpsendfilecmd.end_byte);
        ImGui::Text("  interval    = %u", command->ftpsendfilecmd.interval);
        ImGui::Text("  padding[0..2] = {%u, %u, %u}",
            command->ftpsendfilecmd.padding[0],
            command->ftpsendfilecmd.padding[1],
            command->ftpsendfilecmd.padding[2]);

        break;
    }            





    case 204: { // FTP_filenameCmd
        static uint16_t msgid = 0x1815;
        static uint8_t fnccode = 10;
        static char path_buf[64] = "/cf/adcs.so";

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);
        ImGui::InputText("path", path_buf, sizeof(path_buf));
        ImGui::InputScalar("padding u32",   ImGuiDataType_U32, &command->ftpfilenamecmd.padding);


        if (ImGui::Button("Generate CMD")) {
            uint16_t msgid_be = htons(msgid);
            memset(command->ftpfilenamecmd.path, 0, sizeof(command->ftpfilenamecmd.path));
            strncpy(command->ftpfilenamecmd.path, path_buf, sizeof(command->ftpfilenamecmd.path) - 1);
            uint8_t sequence[2] = {0xC0, 0x00};
            uint8_t length[2] = {0x00, 0x45};
            memcpy(command->ftpfilenamecmd.CmdHeader + 0, &msgid_be, sizeof(uint16_t));
            memcpy(command->ftpfilenamecmd.CmdHeader + 2, sequence, sizeof(uint16_t));
            memcpy(command->ftpfilenamecmd.CmdHeader + 4, length, sizeof(uint16_t));
            memcpy(command->ftpfilenamecmd.CmdHeader + 6, &fnccode, sizeof(uint8_t));
            command->ftpfilenamecmd.CmdHeader[7] = 0x00;
            pthread_join(p_thread[4], NULL);
            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + sizeof(FTP_filenameCmd_t));
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length = sizeof(FTP_filenameCmd_t);
            uint16_t len = sizeof(FTP_filenameCmd_t);
            const uint8_t* byteptr = reinterpret_cast<const uint8_t*>(&command->ftpfilenamecmd);
            uint8_t checksum = 0xFF;
            while (len--) checksum ^= *(byteptr++);
            memcpy(command->ftpfilenamecmd.CmdHeader + 7, &checksum, sizeof(uint8_t));
            memcpy(TestPacket->Data, &command->ftpfilenamecmd, sizeof(FTP_filenameCmd_t));
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
            command->ftpfilenamecmd.CmdHeader[0],
            command->ftpfilenamecmd.CmdHeader[1],
            command->ftpfilenamecmd.CmdHeader[2],
            command->ftpfilenamecmd.CmdHeader[3],
            command->ftpfilenamecmd.CmdHeader[4],
            command->ftpfilenamecmd.CmdHeader[5],
            command->ftpfilenamecmd.CmdHeader[6],
            command->ftpfilenamecmd.CmdHeader[7]);

        ImGui::Text("Params: path = %s", path_buf);
        ImGui::Text("  padding    = %u", command->ftpfilenamecmd.padding);

        break;
    }


case 205: {
    static uint16_t msgid   = 0x1819;
    static uint8_t  fnccode = 0;
    static uint32_t payload = 0x0000081A;  
    static char     last_cmd_hex[128] = "";

    ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
    ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);
    ImGui::InputScalar("arg (uint32 LE)", ImGuiDataType_U32, &payload);

    if (ImGui::Button("Generate CMD"))
    {
        WriteSystemName(msgid);
        pthread_join(p_thread[4], NULL);


        uint8_t cmd[CFE_SB_CMD_HDR_SIZE + sizeof(uint32_t)] = {0};


        uint16_t mid_be = htons(msgid);
        memcpy(cmd, &mid_be, sizeof(uint16_t));   

        cmd[2] = 0xC0;
        cmd[3] = 0x00;


        uint16_t ccsds_len = htons(sizeof(cmd) - 7); 
        memcpy(cmd + 4, &ccsds_len, sizeof(uint16_t)); 

        // CC
        cmd[6] = fnccode;   // 0


        cmd[7] = 0x00;


        cmd[8]  = (uint8_t)( payload        & 0xFF);
        cmd[9]  = (uint8_t)((payload >> 8 ) & 0xFF);
        cmd[10] = (uint8_t)((payload >> 16) & 0xFF);
        cmd[11] = (uint8_t)((payload >> 24) & 0xFF);

        uint8_t checksum = 0xFF;
        for (size_t i = 0; i < sizeof(cmd); ++i)
            checksum ^= cmd[i];
        cmd[7] = checksum;


        char *ptr    = last_cmd_hex;
        size_t remain = sizeof(last_cmd_hex);
        int written = snprintf(
            ptr, remain,
            "%02X %02X %02X %02X %02X %02X %02X %02X\n"
            "%02X %02X %02X %02X",
            cmd[0], cmd[1], cmd[2], cmd[3],
            cmd[4], cmd[5], cmd[6], cmd[7],
            cmd[8], cmd[9], cmd[10], cmd[11]
        );
        (void)written; 


        packetsign* TestPacket =
            (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd));
        TestPacket->Identifier = HVD_TEST;
        TestPacket->PacketType = MIM_PT_TMTC_TEST;
        TestPacket->Length     = sizeof(cmd);   
        memcpy(TestPacket->Data, cmd, sizeof(cmd));

        pthread_create(&p_thread[4], NULL,
                       task_uplink_onorbit, (void*)TestPacket);
    }

    ImGui::Text("Last CMD:");
    ImGui::TextUnformatted(last_cmd_hex);
    break;
}


// UTRX GNDWDT clear

case 206: {
    static uint16_t msgid   = 0x1850;
    static uint8_t  fnccode = 6;

    ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
    ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

    if (ImGui::Button("Generate CMD"))
    {
        WriteSystemName(msgid);
        pthread_join(p_thread[4], NULL);


        uint8_t cmd[CFE_SB_CMD_HDR_SIZE] = {0};


        uint16_t mid_be = htons(msgid);
        memcpy(cmd, &mid_be, sizeof(uint16_t));   

        cmd[2] = 0xC0;
        cmd[3] = 0x00;


        uint16_t ccsds_len = htons(sizeof(cmd) - 7); 
        memcpy(cmd + 4, &ccsds_len, sizeof(uint16_t)); 

        // CC
        cmd[6] = fnccode;   // 0


        cmd[7] = 0x00;


        uint8_t checksum = 0xFF;
        for (size_t i = 0; i < sizeof(cmd); ++i)
            checksum ^= cmd[i];
        cmd[7] = checksum;


        packetsign* TestPacket =
            (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd));
        TestPacket->Identifier = HVD_TEST;
        TestPacket->PacketType = MIM_PT_TMTC_TEST;
        TestPacket->Length     = sizeof(cmd);   
        memcpy(TestPacket->Data, cmd, sizeof(cmd));

        pthread_create(&p_thread[4], NULL,
                       task_uplink_onorbit, (void*)TestPacket);
    }

    break;
}


        case 207: {
            // Add packet
            static uint16_t msgid   = CFE_MISSION_TO_LAB_CMD_ID;
            static uint8_t  fnccode = TO_LAB_ADD_PKT_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            if (ImGui::Button("Generate CMD")) {

  
        pthread_join(p_thread[4], NULL);


        uint8_t cmd[15] = {0};


        uint16_t mid_be = htons(msgid);
        memcpy(cmd, &mid_be, sizeof(uint16_t));   

        cmd[2] = 0xC0;
        cmd[3] = 0x00;


        uint16_t ccsds_len = htons(sizeof(cmd) - 7); 
        memcpy(cmd + 4, &ccsds_len, sizeof(uint16_t)); 

        // CC
        cmd[6] = fnccode;   // 0


        cmd[7] = 0x18;     // CRC


        cmd[8] = 0x1a;
        cmd[9] = 0x08;
        cmd[10] = 0x00;
        cmd[11] = 0x00;
        cmd[12] = 0x00;
        cmd[13] = 0x00;
        cmd[14] = 0x04;



        packetsign* TestPacket =
            (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd));
        TestPacket->Identifier = HVD_TEST;
        TestPacket->PacketType = MIM_PT_TMTC_TEST;
        TestPacket->Length     = sizeof(cmd);   
        memcpy(TestPacket->Data, cmd, sizeof(cmd));

        pthread_create(&p_thread[4], NULL,
                       task_uplink_onorbit, (void*)TestPacket);

        pthread_join(p_thread[4], NULL);




        sleep(1);

        uint8_t cmd2[12] = {0};


        cmd2[0] = 0x18;
        cmd2[1] = 0x19;

        cmd2[2] = 0xC0;
        cmd2[3] = 0x00;
        cmd2[4] = 0x00;
        cmd2[5] = 0x05;

        // CC
        cmd2[6] = 0x00;   // 0


        cmd2[7] = 0x29;     // CRC


        cmd2[8] = 0x1a;
        cmd2[9] = 0x08;
        cmd2[10] = 0x00;
        cmd2[11] = 0x00;




        packetsign* TestPacket1 =
            (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd2));
        TestPacket1->Identifier = HVD_TEST;
        TestPacket1->PacketType = MIM_PT_TMTC_TEST;
        TestPacket1->Length     = sizeof(cmd2);   
        memcpy(TestPacket1->Data, cmd2, sizeof(cmd2));

        pthread_create(&p_thread[4], NULL,
                       task_uplink_onorbit, (void*)TestPacket1);
        pthread_join(p_thread[4], NULL);


            }

            ImGui::Text("Letter to COSMIC... Please give us a reply...");




                    break;




        }
        case 208: {




            // Add packet
            static uint16_t msgid   = CFE_MISSION_TO_LAB_CMD_ID;
            static uint8_t  fnccode = TO_LAB_ADD_PKT_CC;

            ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
            ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

            if (ImGui::Button("Generate CMD")) {

  


        uint8_t cmd0[8] = {0};

            //mid
        cmd0[0] = 0x18;
        cmd0[1] = 0x23;
            //seq
        cmd0[2] = 0xC0;
        cmd0[3] = 0x00;
            //length
        cmd0[4] = 0x00;
        cmd0[5] = 0x01;

        // CC
        cmd0[6] = 27;   // 0

        uint8_t checksum = 0xFF;
        for (size_t i = 0; i < sizeof(cmd0); ++i)
            checksum ^= cmd0[i];
        cmd0[7] = checksum;






        packetsign* TestPacket0 =
            (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd0));
        TestPacket0->Identifier = HVD_TEST;
        TestPacket0->PacketType = MIM_PT_TMTC_TEST;
        TestPacket0->Length     = sizeof(cmd0);   
        memcpy(TestPacket0->Data, cmd0, sizeof(cmd0));

        pthread_create(&p_thread[4], NULL,
                       task_uplink_onorbit, (void*)TestPacket0);
        pthread_join(p_thread[4], NULL);


            sleep(0.1);

        pthread_join(p_thread[4], NULL);


        uint8_t cmd[15] = {0};


        uint16_t mid_be = htons(msgid);
        memcpy(cmd, &mid_be, sizeof(uint16_t));   

        cmd[2] = 0xC0;
        cmd[3] = 0x00;


        uint16_t ccsds_len = htons(sizeof(cmd) - 7); 
        memcpy(cmd + 4, &ccsds_len, sizeof(uint16_t)); 

        // CC
        cmd[6] = fnccode;   // 0


        cmd[7] = 0x18;     // CRC


        cmd[8] = 0x1a;
        cmd[9] = 0x08;
        cmd[10] = 0x00;
        cmd[11] = 0x00;
        cmd[12] = 0x00;
        cmd[13] = 0x00;
        cmd[14] = 0x04;



        packetsign* TestPacket =
            (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd));
        TestPacket->Identifier = HVD_TEST;
        TestPacket->PacketType = MIM_PT_TMTC_TEST;
        TestPacket->Length     = sizeof(cmd);   
        memcpy(TestPacket->Data, cmd, sizeof(cmd));

        pthread_create(&p_thread[4], NULL,
                       task_uplink_onorbit, (void*)TestPacket);

        pthread_join(p_thread[4], NULL);




        sleep(0.1);

        uint8_t cmd2[12] = {0};


        cmd2[0] = 0x18;
        cmd2[1] = 0x19;

        cmd2[2] = 0xC0;
        cmd2[3] = 0x00;
        cmd2[4] = 0x00;
        cmd2[5] = 0x05;

        // CC
        cmd2[6] = 0x00;   // 0


        cmd2[7] = 0x29;     // CRC


        cmd2[8] = 0x1a;
        cmd2[9] = 0x08;
        cmd2[10] = 0x00;
        cmd2[11] = 0x00;




        packetsign* TestPacket1 =
            (packetsign*)malloc(2 + 2 + 4 + sizeof(cmd2));
        TestPacket1->Identifier = HVD_TEST;
        TestPacket1->PacketType = MIM_PT_TMTC_TEST;
        TestPacket1->Length     = sizeof(cmd2);   
        memcpy(TestPacket1->Data, cmd2, sizeof(cmd2));

        pthread_create(&p_thread[4], NULL,
                       task_uplink_onorbit, (void*)TestPacket1);
        pthread_join(p_thread[4], NULL);


            }

            ImGui::Text("TO SET DUAL EMISSION");
            ImGui::Text("->");
            ImGui::Text("TO LAB ADD PACKET");

            ImGui::Text("->");
            ImGui::Text("HK1 SEND COMBINED PACKET");


                    break;
        }

case 209: { // EPS P31U SET CONFIG2 (CC 54)
    static uint16_t msgid   = 6257;
    static uint8_t  fnccode = 54;

    ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
    ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

    ImGui::Separator();
    ImGui::Text("EPS P31U SET CONFIG2 Payload");

    ImGui::InputScalar("batt_maxvoltage u16",      ImGuiDataType_U16, &command->epsp31usetconfig2.batt_maxvoltage);
    ImGui::InputScalar("batt_safevoltage u16",     ImGuiDataType_U16, &command->epsp31usetconfig2.batt_safevoltage);
    ImGui::InputScalar("batt_criticalvoltage u16", ImGuiDataType_U16, &command->epsp31usetconfig2.batt_criticalvoltage);
    ImGui::InputScalar("batt_normalvoltage u16",   ImGuiDataType_U16, &command->epsp31usetconfig2.batt_normalvoltage);

    ImGui::InputScalar("reserved1_0 u32", ImGuiDataType_U32, &command->epsp31usetconfig2.reserved1_0);
    ImGui::InputScalar("reserved1_1 u32", ImGuiDataType_U32, &command->epsp31usetconfig2.reserved1_1);

    ImGui::InputScalar("reserved2_0 u8", ImGuiDataType_U8, &command->epsp31usetconfig2.reserved2_0);
    ImGui::InputScalar("reserved2_1 u8", ImGuiDataType_U8, &command->epsp31usetconfig2.reserved2_1);
    ImGui::InputScalar("reserved2_2 u8", ImGuiDataType_U8, &command->epsp31usetconfig2.reserved2_2);
    ImGui::InputScalar("reserved2_3 u8", ImGuiDataType_U8, &command->epsp31usetconfig2.reserved2_3);

    if (ImGui::Button("Generate CMD")) {
        WriteSystemName(msgid);

        uint16_t mid = htons(msgid);
        uint8_t  seq[2] = {0xC0, 0x00};
        uint8_t  len[2] = {0x00, (uint8_t)(sizeof(EPS_P31U_SETCONFIG2) - 7)};

        memcpy(command->epsp31usetconfig2.CmdHeader,     &mid, 2);
        memcpy(command->epsp31usetconfig2.CmdHeader + 2, seq,  2);
        memcpy(command->epsp31usetconfig2.CmdHeader + 4, len,  2);
        memcpy(command->epsp31usetconfig2.CmdHeader + 6, &fnccode, 1);

        command->epsp31usetconfig2.CmdHeader[7] = 0;
        uint16_t total = sizeof(EPS_P31U_SETCONFIG2);
        const uint8_t *p = (const uint8_t *)&command->epsp31usetconfig2;
        uint8_t crc = 0xFF;
        while (total--) crc ^= *(p++);
        command->epsp31usetconfig2.CmdHeader[7] = crc;

        packetsign *pkt =
            (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P31U_SETCONFIG2));
        pkt->Identifier = HVD_TEST;
        pkt->PacketType = MIM_PT_TMTC_TEST;
        pkt->Length     = sizeof(EPS_P31U_SETCONFIG2);
        memcpy(pkt->Data, &command->epsp31usetconfig2, sizeof(EPS_P31U_SETCONFIG2));

        pthread_join(p_thread[4], NULL);
        pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
    }

    ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                command->epsp31usetconfig2.CmdHeader[0],
                command->epsp31usetconfig2.CmdHeader[1],
                command->epsp31usetconfig2.CmdHeader[2],
                command->epsp31usetconfig2.CmdHeader[3],
                command->epsp31usetconfig2.CmdHeader[4],
                command->epsp31usetconfig2.CmdHeader[5],
                command->epsp31usetconfig2.CmdHeader[6],
                command->epsp31usetconfig2.CmdHeader[7]);

    ImGui::Text("Payload: max=%u safe=%u critical=%u normal=%u r1_0=%u r1_1=%u r2={%u,%u,%u,%u}",
                command->epsp31usetconfig2.batt_maxvoltage,
                command->epsp31usetconfig2.batt_safevoltage,
                command->epsp31usetconfig2.batt_criticalvoltage,
                command->epsp31usetconfig2.batt_normalvoltage,
                command->epsp31usetconfig2.reserved1_0,
                command->epsp31usetconfig2.reserved1_1,
                command->epsp31usetconfig2.reserved2_0,
                command->epsp31usetconfig2.reserved2_1,
                command->epsp31usetconfig2.reserved2_2,
                command->epsp31usetconfig2.reserved2_3);
    break;
}

case 210: { // EPS P31U CONFIG2 (CC 55)
    static uint16_t msgid   = 6257;
    static uint8_t  fnccode = 55;

    ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
    ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

    ImGui::Separator();
    ImGui::Text("EPS P31U CONFIG2 Payload");
    ImGui::InputScalar("cmd u8", ImGuiDataType_U8, &command->epsp31uconfig2.cmd);

    if (ImGui::Button("Generate CMD")) {
        WriteSystemName(msgid);

        uint16_t mid = htons(msgid);
        uint8_t  seq[2] = {0xC0, 0x00};
        uint8_t  len[2] = {0x00, (uint8_t)(sizeof(EPS_P31U_CONFIG2) - 7)};

        memcpy(command->epsp31uconfig2.CmdHeader,     &mid, 2);
        memcpy(command->epsp31uconfig2.CmdHeader + 2, seq,  2);
        memcpy(command->epsp31uconfig2.CmdHeader + 4, len,  2);
        memcpy(command->epsp31uconfig2.CmdHeader + 6, &fnccode, 1);

        command->epsp31uconfig2.CmdHeader[7] = 0;
        uint16_t total = sizeof(EPS_P31U_CONFIG2);
        const uint8_t *p = (const uint8_t *)&command->epsp31uconfig2;
        uint8_t crc = 0xFF;
        while (total--) crc ^= *(p++);
        command->epsp31uconfig2.CmdHeader[7] = crc;

        packetsign *pkt =
            (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P31U_CONFIG2));
        pkt->Identifier = HVD_TEST;
        pkt->PacketType = MIM_PT_TMTC_TEST;
        pkt->Length     = sizeof(EPS_P31U_CONFIG2);
        memcpy(pkt->Data, &command->epsp31uconfig2, sizeof(EPS_P31U_CONFIG2));

        pthread_join(p_thread[4], NULL);
        pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
    }

    ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                command->epsp31uconfig2.CmdHeader[0],
                command->epsp31uconfig2.CmdHeader[1],
                command->epsp31uconfig2.CmdHeader[2],
                command->epsp31uconfig2.CmdHeader[3],
                command->epsp31uconfig2.CmdHeader[4],
                command->epsp31uconfig2.CmdHeader[5],
                command->epsp31uconfig2.CmdHeader[6],
                command->epsp31uconfig2.CmdHeader[7]);

    ImGui::Text("Payload: cmd=%u", command->epsp31uconfig2.cmd);
    break;
}

case 211: { // EPS P31U SET OUT SINGLE (CC 10)
    static uint16_t msgid   = 6257;
    static uint8_t  fnccode = 10;

    ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
    ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

    ImGui::Separator();
    ImGui::Text("EPS P31U SET OUT SINGLE Payload");

    ImGui::InputScalar("channel u8", ImGuiDataType_U8,  &command->epsp31usetoutsingle.channel);
    ImGui::InputScalar("value u8",   ImGuiDataType_U8,  &command->epsp31usetoutsingle.value);
    ImGui::InputScalar("delay u16",  ImGuiDataType_U16, &command->epsp31usetoutsingle.delay);

    if (ImGui::Button("Generate CMD")) {
        WriteSystemName(msgid);

        uint16_t mid = htons(msgid);
        uint8_t  seq[2] = {0xC0, 0x00};
        uint8_t  len[2] = {0x00, (uint8_t)(sizeof(EPS_P31U_SET_OUT_SINGLE) - 7)};

        memcpy(command->epsp31usetoutsingle.CmdHeader,     &mid, 2);
        memcpy(command->epsp31usetoutsingle.CmdHeader + 2, seq,  2);
        memcpy(command->epsp31usetoutsingle.CmdHeader + 4, len,  2);
        memcpy(command->epsp31usetoutsingle.CmdHeader + 6, &fnccode, 1);

        command->epsp31usetoutsingle.CmdHeader[7] = 0;
        uint16_t total = sizeof(EPS_P31U_SET_OUT_SINGLE);
        const uint8_t *p = (const uint8_t *)&command->epsp31usetoutsingle;
        uint8_t crc = 0xFF;
        while (total--) crc ^= *(p++);
        command->epsp31usetoutsingle.CmdHeader[7] = crc;

        packetsign *pkt =
            (packetsign *)malloc(2 + 2 + 4 + sizeof(EPS_P31U_SET_OUT_SINGLE));
        pkt->Identifier = HVD_TEST;
        pkt->PacketType = MIM_PT_TMTC_TEST;
        pkt->Length     = sizeof(EPS_P31U_SET_OUT_SINGLE);
        memcpy(pkt->Data, &command->epsp31usetoutsingle, sizeof(EPS_P31U_SET_OUT_SINGLE));

        pthread_join(p_thread[4], NULL);
        pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
    }

    ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                command->epsp31usetoutsingle.CmdHeader[0],
                command->epsp31usetoutsingle.CmdHeader[1],
                command->epsp31usetoutsingle.CmdHeader[2],
                command->epsp31usetoutsingle.CmdHeader[3],
                command->epsp31usetoutsingle.CmdHeader[4],
                command->epsp31usetoutsingle.CmdHeader[5],
                command->epsp31usetoutsingle.CmdHeader[6],
                command->epsp31usetoutsingle.CmdHeader[7]);

    ImGui::Text("Payload: channel=%u value=%u delay=%u",
                command->epsp31usetoutsingle.channel,
                command->epsp31usetoutsingle.value,
                command->epsp31usetoutsingle.delay);
    break;
}

case 212: { // SP DEPLOY (CC 2)
    static uint16_t msgid   = 6232;
    static uint8_t  fnccode = 2;

    ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
    ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

    ImGui::Separator();
    ImGui::Text("SP DEPLOY Payload");

    ImGui::InputScalar("SP u8",     ImGuiDataType_U8, &command->spdeploy.SP);
    ImGui::InputScalar("deploy u8", ImGuiDataType_U8, &command->spdeploy.deploy);

    if (ImGui::Button("Generate CMD")) {
        WriteSystemName(msgid);

        uint16_t mid = htons(msgid);
        uint8_t  seq[2] = {0xC0, 0x00};
        uint8_t  len[2] = {0x00, (uint8_t)(sizeof(SP_DEPLOY) - 7)};

        memcpy(command->spdeploy.CmdHeader,     &mid, 2);
        memcpy(command->spdeploy.CmdHeader + 2, seq,  2);
        memcpy(command->spdeploy.CmdHeader + 4, len,  2);
        memcpy(command->spdeploy.CmdHeader + 6, &fnccode, 1);

        command->spdeploy.CmdHeader[7] = 0;
        uint16_t total = sizeof(SP_DEPLOY);
        const uint8_t *p = (const uint8_t *)&command->spdeploy;
        uint8_t crc = 0xFF;
        while (total--) crc ^= *(p++);
        command->spdeploy.CmdHeader[7] = crc;

        packetsign *pkt =
            (packetsign *)malloc(2 + 2 + 4 + sizeof(SP_DEPLOY));
        pkt->Identifier = HVD_TEST;
        pkt->PacketType = MIM_PT_TMTC_TEST;
        pkt->Length     = sizeof(SP_DEPLOY);
        memcpy(pkt->Data, &command->spdeploy, sizeof(SP_DEPLOY));

        pthread_join(p_thread[4], NULL);
        pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
    }

    ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
                command->spdeploy.CmdHeader[0],
                command->spdeploy.CmdHeader[1],
                command->spdeploy.CmdHeader[2],
                command->spdeploy.CmdHeader[3],
                command->spdeploy.CmdHeader[4],
                command->spdeploy.CmdHeader[5],
                command->spdeploy.CmdHeader[6],
                command->spdeploy.CmdHeader[7]);

    ImGui::Text("Payload: SP=%u deploy=%u",
                command->spdeploy.SP,
                command->spdeploy.deploy);
    break;
}

case 213: { // FTP_filenameCmd
        static uint16_t msgid = 0x1815;
        static uint8_t fnccode = 15;
        static char path_buf[64] = "/cf/sdcard/";

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);
        ImGui::InputText("path", path_buf, sizeof(path_buf));
        ImGui::InputScalar("DirListOffset u32",   ImGuiDataType_U32, &command->ftpdirlistpktcmd.DirListOffset);
        ImGui::InputScalar("GetSizeTimeMode u8",   ImGuiDataType_U8, &command->ftpdirlistpktcmd.GetSizeTimeMode);
        ImGui::InputScalar("Spare01[0] u8",   ImGuiDataType_U8, &command->ftpdirlistpktcmd.Spare01_0);
        ImGui::InputScalar("Spare01[1] u8",   ImGuiDataType_U8, &command->ftpdirlistpktcmd.Spare01_1);
        ImGui::InputScalar("Spare01[2] u8",   ImGuiDataType_U8, &command->ftpdirlistpktcmd.Spare01_2);

        if (ImGui::Button("Generate CMD")) {

            uint16_t msgid_be = htons(msgid);
            memset(command->ftpdirlistpktcmd.path, 0, sizeof(command->ftpdirlistpktcmd.path));
            strncpy(command->ftpdirlistpktcmd.path, path_buf, sizeof(command->ftpdirlistpktcmd.path) - 1);
            
            uint8_t sequence[2] = {0xC0, 0x00};
            uint8_t length[2] = {0x00, (uint8_t)(sizeof(FTP_dirlistpktCmd_t) - 7)};
            memcpy(command->ftpdirlistpktcmd.CmdHeader + 0, &msgid_be, sizeof(uint16_t));
            memcpy(command->ftpdirlistpktcmd.CmdHeader + 2, sequence, sizeof(uint16_t));
            memcpy(command->ftpdirlistpktcmd.CmdHeader + 4, length, sizeof(uint16_t));
            memcpy(command->ftpdirlistpktcmd.CmdHeader + 6, &fnccode, sizeof(uint8_t));
            command->ftpdirlistpktcmd.CmdHeader[7] = 0x00;
            pthread_join(p_thread[4], NULL);
            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + sizeof(FTP_dirlistpktCmd_t));
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;
            TestPacket->Length = sizeof(FTP_dirlistpktCmd_t);
            uint16_t len = sizeof(FTP_dirlistpktCmd_t);
            const uint8_t* byteptr = reinterpret_cast<const uint8_t*>(&command->ftpdirlistpktcmd);
            uint8_t checksum = 0xFF;
            while (len--) checksum ^= *(byteptr++);
            memcpy(command->ftpdirlistpktcmd.CmdHeader + 7, &checksum, sizeof(uint8_t));
            memcpy(TestPacket->Data, &command->ftpdirlistpktcmd, sizeof(FTP_dirlistpktCmd_t));
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
        }

        ImGui::Text("Header: %02X %02X %02X %02X %02X %02X %02X %02X",
            command->ftpdirlistpktcmd.CmdHeader[0],
            command->ftpdirlistpktcmd.CmdHeader[1],
            command->ftpdirlistpktcmd.CmdHeader[2],
            command->ftpdirlistpktcmd.CmdHeader[3],
            command->ftpdirlistpktcmd.CmdHeader[4],
            command->ftpdirlistpktcmd.CmdHeader[5],
            command->ftpdirlistpktcmd.CmdHeader[6],
            command->ftpdirlistpktcmd.CmdHeader[7]);

        // ImGui::Text("Params: path = %s", path_buf);
        ImGui::Text("Params:  Path            : %s", command->ftpdirlistpktcmd.path);

        // 2. 주요 설정값 확인
        ImGui::Text("  DirListOffset   : %u", command->ftpdirlistpktcmd.DirListOffset);
        ImGui::Text("  GetSizeTimeMode : %u", command->ftpdirlistpktcmd.GetSizeTimeMode);

        // 3. Spare(Padding) 값 확인 - 16진수로 보는 것이 편함
        ImGui::Text("  Spare Bytes     : [%02X] [%02X] [%02X]", 
            command->ftpdirlistpktcmd.Spare01_0,
            command->ftpdirlistpktcmd.Spare01_1,
            command->ftpdirlistpktcmd.Spare01_2);

        break;
    }
    case 214: { //  COSMIC UEL Pi ON
        static uint16_t msgid   = 6277;
        static uint8_t  fnccode = 8;
        ImGui::Text("MsgID   : 0x%04X", msgid);
        ImGui::Text("FncCode : %u",     fnccode);


        if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);
                pthread_join(p_thread[4], NULL);

                packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + 8);
                TestPacket->Identifier = HVD_TEST;
                TestPacket->PacketType = MIM_PT_TMTC_TEST;
                TestPacket->Length = 8;
                memset(TestPacket->Data, 0, TestPacket->Length);

                uint8_t cmd[8] = {0,};
                msgid = htons(msgid);
                memcpy(cmd, &msgid, sizeof(uint16_t));
                memcpy(cmd + 6, &fnccode, sizeof(uint8_t));
                cmd[2] = 0xc0;
                cmd[3] = 0x00;
                cmd[4] = 0x00;
                cmd[5] = 0x01;

                uint16_t len = 8;
                const uint8_t* byteptr = cmd;
                uint8_t checksum = 0xFF;
                while (len--)
                    checksum ^= *(byteptr++);
                cmd[7] = checksum;

                memcpy(TestPacket->Data, cmd, sizeof(cmd));
                for (int i = 0; i < TestPacket->Length; i++)
                    printf("0x%x ", cmd[i]);
                printf("\n");

                pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
                
                msgid = htons(msgid);
            }
            break;
        }
    case 215: { //  COSMIC UEL Pi OFF
        static uint16_t msgid   = 6277;
        static uint8_t  fnccode = 9;
        ImGui::Text("MsgID   : 0x%04X", msgid);
        ImGui::Text("FncCode : %u",     fnccode);


        if (ImGui::Button("Generate CMD")) {
                WriteSystemName(msgid);
                pthread_join(p_thread[4], NULL);

                packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + 8);
                TestPacket->Identifier = HVD_TEST;
                TestPacket->PacketType = MIM_PT_TMTC_TEST;
                TestPacket->Length = 8;
                memset(TestPacket->Data, 0, TestPacket->Length);

                uint8_t cmd[8] = {0,};
                msgid = htons(msgid);
                memcpy(cmd, &msgid, sizeof(uint16_t));
                memcpy(cmd + 6, &fnccode, sizeof(uint8_t));
                cmd[2] = 0xc0;
                cmd[3] = 0x00;
                cmd[4] = 0x00;
                cmd[5] = 0x01;

                uint16_t len = 8;
                const uint8_t* byteptr = cmd;
                uint8_t checksum = 0xFF;
                while (len--)
                    checksum ^= *(byteptr++);
                cmd[7] = checksum;

                memcpy(TestPacket->Data, cmd, sizeof(cmd));
                for (int i = 0; i < TestPacket->Length; i++)
                    printf("0x%x ", cmd[i]);
                printf("\n");

                pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
                
                msgid = htons(msgid);
            }
            break;
        }

    case 216: { // es noop

        static uint16_t msgid   = 0x1875;
        static uint8_t  fnccode = EPS_P60_GET_DOCK_INFO_CC;

        ImGui::InputScalar("msgid",   ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8,  &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {
                0x00,
                (uint8_t)(sizeof(EPS_P60_GetDockInfoCmd_t) - 7)
            };

            memcpy(command->epsp60getdockinfocmd.CmdHeader,     &mid, 2);
            memcpy(command->epsp60getdockinfocmd.CmdHeader + 2, seq,  2);
            memcpy(command->epsp60getdockinfocmd.CmdHeader + 4, len,  2);
            memcpy(command->epsp60getdockinfocmd.CmdHeader + 6, &fnccode, 1);

            uint16_t total = sizeof(EPS_P60_GetDockInfoCmd_t);
            const uint8_t *p =
                (const uint8_t *)&command->epsp60getdockinfocmd;
            uint8_t crc = 0xFF;
            while (total--) crc ^= *(p++);

            memcpy(command->epsp60getdockinfocmd.CmdHeader + 7, &crc, 1);

            packetsign *pkt =
                (packetsign *)malloc(2 + 2 + 4 +
                                     sizeof(EPS_P60_GetDockInfoCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(EPS_P60_GetDockInfoCmd_t);
            memcpy(pkt->Data, &command->epsp60getdockinfocmd,
                   sizeof(EPS_P60_GetDockInfoCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL,
                           task_uplink_onorbit, (void *)pkt);
        }
        break;
    }


 //8192, 0, 15

    case 217: { // es start app cmd
        static uint16_t msgid = CFE_ES_CMD_MID;
        static uint8_t fnccode = 4;
        static char app_buf[20] = "ADCS";
        static char entry_buf[20] = "ADCS2App_Main";
        static char filename_buf[64] = "/cf/adcs3.so";

        static uint32_t StackSize = 8192;
        static uint8_t  ExceptionAction = 0;
        static uint16_t Priority = 15;

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);


        ImGui::InputText("application", app_buf, sizeof(app_buf));
        ImGui::InputText("app entry point", entry_buf, sizeof(entry_buf));
        ImGui::InputText("app file name", filename_buf, sizeof(filename_buf));

        ImGui::InputScalar("Stack Size u32", ImGuiDataType_U32, &StackSize);
        ImGui::InputScalar("Exception Action u8",   ImGuiDataType_U8, &ExceptionAction);
        ImGui::InputScalar("Priority u16",    ImGuiDataType_U16,  &Priority);





        if (ImGui::Button("Generate CMD")) {
            uint16_t msgid_be = htons(msgid);


            memset(command->cfeesstartappcmd.Application, 0, sizeof(command->cfeesstartappcmd.Application));
            strncpy(command->cfeesstartappcmd.Application, app_buf, sizeof(command->cfeesstartappcmd.Application) - 1);


            memset(command->cfeesstartappcmd.AppEntryPoint, 0, sizeof(command->cfeesstartappcmd.AppEntryPoint));
            strncpy(command->cfeesstartappcmd.AppEntryPoint, entry_buf, sizeof(command->cfeesstartappcmd.AppEntryPoint) - 1);

            memset(command->cfeesstartappcmd.AppFileName, 0, sizeof(command->cfeesstartappcmd.AppFileName));
            strncpy(command->cfeesstartappcmd.AppFileName, filename_buf, sizeof(command->cfeesstartappcmd.AppFileName) - 1);

            uint8_t sequence[2] = {0xC0, 0x00};
            uint8_t  length[2]   = {0x00, (uint8_t)(sizeof(CFE_ES_StartAppCmd_t) - 7)};
            memcpy(command->cfeesstartappcmd.CmdHeader + 0, &msgid_be, sizeof(uint16_t));
            memcpy(command->cfeesstartappcmd.CmdHeader + 2, sequence, sizeof(uint16_t));
            memcpy(command->cfeesstartappcmd.CmdHeader + 4, length, sizeof(uint16_t));
            memcpy(command->cfeesstartappcmd.CmdHeader + 6, &fnccode, sizeof(uint8_t));


            command->cfeesstartappcmd.CmdHeader[7] = 0x00;

            pthread_join(p_thread[4], NULL);
            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + sizeof(CFE_ES_StartAppCmd_t));
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;

            TestPacket->Length = sizeof(CFE_ES_StartAppCmd_t);
            uint16_t len = sizeof(CFE_ES_StartAppCmd_t);


            const uint8_t* byteptr = reinterpret_cast<const uint8_t*>(&command->cfeesstartappcmd);

            uint8_t checksum = 0xFF;
            while (len--) checksum ^= *(byteptr++);
            memcpy(command->cfeesstartappcmd.CmdHeader + 7, &checksum, sizeof(uint8_t));
            memcpy(TestPacket->Data, &command->cfeesstartappcmd, sizeof(CFE_ES_StartAppCmd_t));
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
        }


        break;
    }






    case 218: { // ES stop app
        static uint16_t msgid = CFE_ES_CMD_MID;
        static uint8_t fnccode = CFE_ES_STOP_APP_CC;
        static char app_buf[20] = "ADCS";

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);
        ImGui::InputText("application", app_buf, sizeof(app_buf));


        if (ImGui::Button("Generate CMD")) {
            uint16_t msgid_be = htons(msgid);
            memset(command->esstopapp.Application, 0, sizeof(command->esstopapp.Application));
            strncpy(command->esstopapp.Application, app_buf, sizeof(command->esstopapp.Application) - 1);

            uint8_t sequence[2] = {0xC0, 0x00};
            uint8_t  length[2]   = {0x00, (uint8_t)(sizeof(CFE_ES_StopAppCmd_t) - 7)};
            memcpy(command->esstopapp.CmdHeader + 0, &msgid_be, sizeof(uint16_t));
            memcpy(command->esstopapp.CmdHeader + 2, sequence, sizeof(uint16_t));
            memcpy(command->esstopapp.CmdHeader + 4, length, sizeof(uint16_t));
            memcpy(command->esstopapp.CmdHeader + 6, &fnccode, sizeof(uint8_t));


            command->esstopapp.CmdHeader[7] = 0x00;

            pthread_join(p_thread[4], NULL);
            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + sizeof(CFE_ES_StopAppCmd_t));
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;

            TestPacket->Length = sizeof(CFE_ES_StopAppCmd_t);
            uint16_t len = sizeof(CFE_ES_StopAppCmd_t);


            const uint8_t* byteptr = reinterpret_cast<const uint8_t*>(&command->esstopapp);

            uint8_t checksum = 0xFF;
            while (len--) checksum ^= *(byteptr++);
            memcpy(command->esstopapp.CmdHeader + 7, &checksum, sizeof(uint8_t));
            memcpy(TestPacket->Data, &command->esstopapp, sizeof(CFE_ES_StopAppCmd_t));
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
        }


        break;
    }








    case 219: { // ADDED FOR FTP TEST 1210 - ES reload app
        static uint16_t msgid = CFE_ES_CMD_MID;
        static uint8_t fnccode = CFE_ES_RELOAD_APP_CC;
        static char app_buf[20] = "TO_LAB_APP";
        static char path_buf[64] = "/cf/to_lab23.so";

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);
        ImGui::InputText("application", app_buf, sizeof(app_buf));
        ImGui::InputText("app file name", path_buf, sizeof(path_buf));


        if (ImGui::Button("Generate CMD")) {
            uint16_t msgid_be = htons(msgid);
            memset(command->toloadcmd.app, 0, sizeof(command->toloadcmd.app));
            strncpy(command->toloadcmd.app, app_buf, sizeof(command->toloadcmd.app) - 1);

            memset(command->toloadcmd.filename, 0, sizeof(command->toloadcmd.filename));
            strncpy(command->toloadcmd.filename, path_buf, sizeof(command->toloadcmd.filename) - 1);

            uint8_t sequence[2] = {0xC0, 0x00};
            uint8_t  length[2]   = {0x00, (uint8_t)(sizeof(ES_LoadCmd_t) - 7)};
            memcpy(command->toloadcmd.CmdHeader + 0, &msgid_be, sizeof(uint16_t));
            memcpy(command->toloadcmd.CmdHeader + 2, sequence, sizeof(uint16_t));
            memcpy(command->toloadcmd.CmdHeader + 4, length, sizeof(uint16_t));
            memcpy(command->toloadcmd.CmdHeader + 6, &fnccode, sizeof(uint8_t));


            command->toloadcmd.CmdHeader[7] = 0x00;

            pthread_join(p_thread[4], NULL);
            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + sizeof(ES_LoadCmd_t));
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;

            TestPacket->Length = sizeof(ES_LoadCmd_t);
            uint16_t len = sizeof(ES_LoadCmd_t);


            const uint8_t* byteptr = reinterpret_cast<const uint8_t*>(&command->toloadcmd);

            uint8_t checksum = 0xFF;
            while (len--) checksum ^= *(byteptr++);
            memcpy(command->toloadcmd.CmdHeader + 7, &checksum, sizeof(uint8_t));
            memcpy(TestPacket->Data, &command->toloadcmd, sizeof(ES_LoadCmd_t));
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
        }


        break;
    }

    case 220: { // ES QueryOneCmd
        static uint16_t msgid = CFE_ES_CMD_MID;
        static uint8_t fnccode = CFE_ES_QUERY_ONE_CC;
        static char app_buf[20] = "TO_LAB_APP";

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);
        ImGui::InputText("application", app_buf, sizeof(app_buf));


        if (ImGui::Button("Generate CMD")) {
            uint16_t msgid_be = htons(msgid);
            memset(command->esqueryonecmd.Application, 0, sizeof(command->esqueryonecmd.Application));
            strncpy(command->esqueryonecmd.Application, app_buf, sizeof(command->esqueryonecmd.Application) - 1);


            uint8_t sequence[2] = {0xC0, 0x00};
            uint8_t  length[2]   = {0x00, (uint8_t)(sizeof(CFE_ES_StopAppCmd_t) - 7)};
            memcpy(command->esqueryonecmd.CmdHeader + 0, &msgid_be, sizeof(uint16_t));
            memcpy(command->esqueryonecmd.CmdHeader + 2, sequence, sizeof(uint16_t));
            memcpy(command->esqueryonecmd.CmdHeader + 4, length, sizeof(uint16_t));
            memcpy(command->esqueryonecmd.CmdHeader + 6, &fnccode, sizeof(uint8_t));


            command->esqueryonecmd.CmdHeader[7] = 0x00;

            pthread_join(p_thread[4], NULL);
            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + sizeof(CFE_ES_StopAppCmd_t));
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;

            TestPacket->Length = sizeof(CFE_ES_StopAppCmd_t);
            uint16_t len = sizeof(CFE_ES_StopAppCmd_t);


            const uint8_t* byteptr = reinterpret_cast<const uint8_t*>(&command->esqueryonecmd);

            uint8_t checksum = 0xFF;
            while (len--) checksum ^= *(byteptr++);
            memcpy(command->esqueryonecmd.CmdHeader + 7, &checksum, sizeof(uint8_t));
            memcpy(TestPacket->Data, &command->esqueryonecmd, sizeof(CFE_ES_StopAppCmd_t));
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
        }


        break;
    }

    case 221: { // ES QueryAllCmd
        static uint16_t msgid = CFE_ES_CMD_MID;
        static uint8_t fnccode = CFE_ES_QUERY_ALL_CC;
        static char app_buf[64] = "TO_LAB_APP";

        ImGui::InputScalar("msgid", ImGuiDataType_U16, &msgid);
        ImGui::InputScalar("fnccode", ImGuiDataType_U8, &fnccode);
        ImGui::InputText("application", app_buf, sizeof(app_buf));


        if (ImGui::Button("Generate CMD")) {
            uint16_t msgid_be = htons(msgid);
            memset(command->esqueryallcmd.FileName, 0, sizeof(command->esqueryallcmd.FileName));
            strncpy(command->esqueryallcmd.FileName, app_buf, sizeof(command->esqueryallcmd.FileName) - 1);


            uint8_t sequence[2] = {0xC0, 0x00};
            uint8_t  length[2]   = {0x00, (uint8_t)(sizeof(CFE_ES_StopAppCmd_t) - 7)};
            memcpy(command->esqueryallcmd.CmdHeader + 0, &msgid_be, sizeof(uint16_t));
            memcpy(command->esqueryallcmd.CmdHeader + 2, sequence, sizeof(uint16_t));
            memcpy(command->esqueryallcmd.CmdHeader + 4, length, sizeof(uint16_t));
            memcpy(command->esqueryallcmd.CmdHeader + 6, &fnccode, sizeof(uint8_t));


            command->esqueryallcmd.CmdHeader[7] = 0x00;

            pthread_join(p_thread[4], NULL);
            packetsign* TestPacket = (packetsign*)malloc(2 + 2 + 4 + sizeof(CFE_ES_StopAppCmd_t));
            TestPacket->Identifier = HVD_TEST;
            TestPacket->PacketType = MIM_PT_TMTC_TEST;

            TestPacket->Length = sizeof(CFE_ES_StopAppCmd_t);
            uint16_t len = sizeof(CFE_ES_StopAppCmd_t);


            const uint8_t* byteptr = reinterpret_cast<const uint8_t*>(&command->esqueryallcmd);

            uint8_t checksum = 0xFF;
            while (len--) checksum ^= *(byteptr++);
            memcpy(command->esqueryallcmd.CmdHeader + 7, &checksum, sizeof(uint8_t));
            memcpy(TestPacket->Data, &command->esqueryallcmd, sizeof(CFE_ES_StopAppCmd_t));
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
        }


        break;
    }

    /********************************************5차 추가******************************************************/
    case 230: { /* 230 : LGPM NOOP (CC 0, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_NOOP_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_NoopCmd_t) - 7};

            memcpy(command->lgpmnoopcmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmnoopcmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmnoopcmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmnoopcmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmnoopcmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_NoopCmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmnoopcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmnoopcmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_NoopCmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_NoopCmd_t);
                
            memcpy(pkt->Data, &command->lgpmnoopcmd, sizeof(PAYUEL_LGPM_NoopCmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 231: { /* 231 : LGPM Reset Counter (CC 1, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_RESET_COUNTERS_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_ResetCountersCmd_t) - 7};

            memcpy(command->lgpmresetcountercmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmresetcountercmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmresetcountercmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmresetcountercmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmresetcountercmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_ResetCountersCmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmresetcountercmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmresetcountercmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_ResetCountersCmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_ResetCountersCmd_t);
                
            memcpy(pkt->Data, &command->lgpmresetcountercmd, sizeof(PAYUEL_LGPM_ResetCountersCmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 232: { /* 232 : LGPM MCU_ALIVE_CHECK (CC 2, With Payload) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_MCU_ALIVE_CHECK_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Ground UTC Time", ImGuiDataType_U32, 
            &command->lgpmmcualivecheckcmd.Ground_UTC_Time); 

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_MCU_ALIVE_CHECK_Cmd_t) - 7};

            memcpy(command->lgpmmcualivecheckcmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmmcualivecheckcmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmmcualivecheckcmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmmcualivecheckcmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmmcualivecheckcmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_MCU_ALIVE_CHECK_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmmcualivecheckcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmmcualivecheckcmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_MCU_ALIVE_CHECK_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_MCU_ALIVE_CHECK_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmmcualivecheckcmd, sizeof(PAYUEL_LGPM_MCU_ALIVE_CHECK_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 233: { /* 233 : LGPM 3V3_PWR_ON (CC 3, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_3V3_PWR_ON_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_3V3_PWR_ON_Cmd_t) - 7};

            memcpy(command->lgpm3v3pwroncmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpm3v3pwroncmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpm3v3pwroncmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpm3v3pwroncmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpm3v3pwroncmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_3V3_PWR_ON_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpm3v3pwroncmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpm3v3pwroncmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_3V3_PWR_ON_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_3V3_PWR_ON_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpm3v3pwroncmd, sizeof(PAYUEL_LGPM_3V3_PWR_ON_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 234: { /* 234 : LGPM 3V3_PWR_OFF (CC 4, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_3V3_PWR_OFF_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_3V3_PWR_OFF_Cmd_t) - 7};

            memcpy(command->lgpm3v3pwroffcmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpm3v3pwroffcmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpm3v3pwroffcmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpm3v3pwroffcmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpm3v3pwroffcmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_3V3_PWR_OFF_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpm3v3pwroffcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpm3v3pwroffcmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_3V3_PWR_OFF_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_3V3_PWR_OFF_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpm3v3pwroffcmd, sizeof(PAYUEL_LGPM_3V3_PWR_OFF_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 235: { /* 235 : LGPM MAIN_BOOST_SW_ON (CC 5, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_MAIN_BOOST_SW_ON_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_MAIN_BOOST_SW_ON_Cmd_t) - 7};

            memcpy(command->lgpmmainboostswoncmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmmainboostswoncmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmmainboostswoncmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmmainboostswoncmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmmainboostswoncmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_MAIN_BOOST_SW_ON_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmmainboostswoncmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmmainboostswoncmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_MAIN_BOOST_SW_ON_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_MAIN_BOOST_SW_ON_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmmainboostswoncmd, sizeof(PAYUEL_LGPM_MAIN_BOOST_SW_ON_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 236: { /* 236 : LGPM MAIN_BOOST_SW_OFF (CC 6, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_MAIN_BOOST_SW_OFF_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_MAIN_BOOST_SW_OFF_Cmd_t) - 7};

            memcpy(command->lgpmmainboostswoffcmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmmainboostswoffcmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmmainboostswoffcmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmmainboostswoffcmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmmainboostswoffcmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_MAIN_BOOST_SW_OFF_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmmainboostswoffcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmmainboostswoffcmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_MAIN_BOOST_SW_OFF_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_MAIN_BOOST_SW_OFF_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmmainboostswoffcmd, sizeof(PAYUEL_LGPM_MAIN_BOOST_SW_OFF_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 237: { /* 237 : LGPM SUB_BOOST_SW_ON (CC 7, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_SUB_BOOST_SW_ON_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_SUB_BOOST_SW_ON_Cmd_t) - 7};

            memcpy(command->lgpmsubboostswoncmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmsubboostswoncmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmsubboostswoncmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmsubboostswoncmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmsubboostswoncmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_SUB_BOOST_SW_ON_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmsubboostswoncmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmsubboostswoncmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_SUB_BOOST_SW_ON_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_SUB_BOOST_SW_ON_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmsubboostswoncmd, sizeof(PAYUEL_LGPM_SUB_BOOST_SW_ON_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 238: { /* 238 : LGPM SUB_BOOST_SW_OFF (CC 8, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_SUB_BOOST_SW_OFF_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_SUB_BOOST_SW_OFF_Cmd_t) - 7};

            memcpy(command->lgpmsubboostswoffcmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmsubboostswoffcmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmsubboostswoffcmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmsubboostswoffcmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmsubboostswoffcmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_SUB_BOOST_SW_OFF_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmsubboostswoffcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmsubboostswoffcmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_SUB_BOOST_SW_OFF_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_SUB_BOOST_SW_OFF_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmsubboostswoffcmd, sizeof(PAYUEL_LGPM_SUB_BOOST_SW_OFF_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 239: { /* 239 : LGPM V28_MAIN_ON (CC 9, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_V28_MAIN_ON_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_V28_MAIN_ON_Cmd_t) - 7};

            memcpy(command->lgpmv28mainoncmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmv28mainoncmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmv28mainoncmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmv28mainoncmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmv28mainoncmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_V28_MAIN_ON_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmv28mainoncmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmv28mainoncmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_V28_MAIN_ON_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_V28_MAIN_ON_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmv28mainoncmd, sizeof(PAYUEL_LGPM_V28_MAIN_ON_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 240: { /* 240 : LGPM V28_MAIN_OFF (CC 10, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_V28_MAIN_OFF_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_V28_MAIN_OFF_Cmd_t) - 7};

            memcpy(command->lgpmv28mainoffcmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmv28mainoffcmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmv28mainoffcmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmv28mainoffcmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmv28mainoffcmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_V28_MAIN_OFF_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmv28mainoffcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmv28mainoffcmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_V28_MAIN_OFF_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_V28_MAIN_OFF_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmv28mainoffcmd, sizeof(PAYUEL_LGPM_V28_MAIN_OFF_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 241: { /* 241 : LGPM V28_SUB_ON (CC 11, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_V28_SUB_ON_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_V28_SUB_ON_Cmd_t) - 7};

            memcpy(command->lgpmv28suboncmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmv28suboncmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmv28suboncmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmv28suboncmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmv28suboncmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_V28_SUB_ON_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmv28suboncmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmv28suboncmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_V28_SUB_ON_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_V28_SUB_ON_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmv28suboncmd, sizeof(PAYUEL_LGPM_V28_SUB_ON_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 242: { /* 242 : LGPM V28_SUB_OFF (CC 12, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_V28_SUB_OFF_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_V28_SUB_OFF_Cmd_t) - 7};

            memcpy(command->lgpmv28suboffcmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmv28suboffcmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmv28suboffcmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmv28suboffcmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmv28suboffcmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_V28_SUB_OFF_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmv28suboffcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmv28suboffcmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_V28_SUB_OFF_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_V28_SUB_OFF_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmv28suboffcmd, sizeof(PAYUEL_LGPM_V28_SUB_OFF_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 243: { /* 243 : LGPM V12_MAIN_ON (CC 13, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_V12_MAIN_ON_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_V12_MAIN_ON_Cmd_t) - 7};

            memcpy(command->lgpmv12mainoncmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmv12mainoncmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmv12mainoncmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmv12mainoncmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmv12mainoncmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_V12_MAIN_ON_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmv12mainoncmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmv12mainoncmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_V12_MAIN_ON_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_V12_MAIN_ON_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmv12mainoncmd, sizeof(PAYUEL_LGPM_V12_MAIN_ON_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 244: { /* 244 : LGPM V12_MAIN_OFF (CC 14, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_V12_MAIN_OFF_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_V12_MAIN_OFF_Cmd_t) - 7};

            memcpy(command->lgpmv12mainoffcmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmv12mainoffcmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmv12mainoffcmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmv12mainoffcmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmv12mainoffcmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_V12_MAIN_OFF_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmv12mainoffcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmv12mainoffcmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_V12_MAIN_OFF_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_V12_MAIN_OFF_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmv12mainoffcmd, sizeof(PAYUEL_LGPM_V12_MAIN_OFF_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 245: { /* 245 : LGPM PWR_SENSE_INFO (CC 15, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_PWR_SENSE_INFO_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_PWR_SENSE_INFO_Cmd_t) - 7};

            memcpy(command->lgpmpwrsenseinfocmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmpwrsenseinfocmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmpwrsenseinfocmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmpwrsenseinfocmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmpwrsenseinfocmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_PWR_SENSE_INFO_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmpwrsenseinfocmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmpwrsenseinfocmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_PWR_SENSE_INFO_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_PWR_SENSE_INFO_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmpwrsenseinfocmd, sizeof(PAYUEL_LGPM_PWR_SENSE_INFO_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 246: { /* 246 : LGPM PWR_SEQ_ON (CC 16, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_PWR_SEQ_ON_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_PWR_SEQ_ON_Cmd_t) - 7};

            memcpy(command->lgpmpwrseqoncmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmpwrseqoncmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmpwrseqoncmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmpwrseqoncmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmpwrseqoncmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_PWR_SEQ_ON_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmpwrseqoncmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmpwrseqoncmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_PWR_SEQ_ON_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_PWR_SEQ_ON_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmpwrseqoncmd, sizeof(PAYUEL_LGPM_PWR_SEQ_ON_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 247: { /* 247 : LGPM PWR_SEQ_OFF (CC 17, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_PWR_SEQ_OFF_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_PWR_SEQ_OFF_Cmd_t) - 7};

            memcpy(command->lgpmpwrseqoffcmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmpwrseqoffcmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmpwrseqoffcmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmpwrseqoffcmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmpwrseqoffcmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_PWR_SEQ_OFF_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmpwrseqoffcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmpwrseqoffcmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_PWR_SEQ_OFF_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_PWR_SEQ_OFF_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmpwrseqoffcmd, sizeof(PAYUEL_LGPM_PWR_SEQ_OFF_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 248: { /* 248 : LGPM RWA_CONTROL_idx1 (CC 18, With Payload) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;    
        static uint8_t fnccode = PAYUEL_LGPM_RWA_CONTROL_idx1_CC; 

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Target Speed", ImGuiDataType_S16,
             &command->lgpmrwacontrolidx1cmd.payload.TargetSpeed_RPM);
        ImGui::InputScalar("Target Acc", ImGuiDataType_S16,
             &command->lgpmrwacontrolidx1cmd.payload.TargetAcc_RPM);
        ImGui::InputScalar("Duration", ImGuiDataType_S16,
             &command->lgpmrwacontrolidx1cmd.payload.Operating_Time);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_RWA_CONTROL_idx1_Cmd_t) - 7};

            memcpy(command->lgpmrwacontrolidx1cmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmrwacontrolidx1cmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmrwacontrolidx1cmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmrwacontrolidx1cmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmrwacontrolidx1cmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_RWA_CONTROL_idx1_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmrwacontrolidx1cmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmrwacontrolidx1cmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_RWA_CONTROL_idx1_Cmd_t)); 
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUEL_LGPM_RWA_CONTROL_idx1_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmrwacontrolidx1cmd, sizeof(PAYUEL_LGPM_RWA_CONTROL_idx1_Cmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 249: { /* 249 : LGPM RWA_CONTROL_idx2 (CC 19, With Payload) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;    
        static uint8_t fnccode = PAYUEL_LGPM_RWA_CONTROL_idx2_CC; 

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        // Payload Input 
        ImGui::InputScalar("Motor Power", ImGuiDataType_U8, &command->lgpmrwacontrolidx2cmd.payload.Motor_power);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_RWA_CONTROL_idx2_Cmd_t) - 7};

            memcpy(command->lgpmrwacontrolidx2cmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmrwacontrolidx2cmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmrwacontrolidx2cmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmrwacontrolidx2cmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmrwacontrolidx2cmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_RWA_CONTROL_idx2_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmrwacontrolidx2cmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmrwacontrolidx2cmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_RWA_CONTROL_idx2_Cmd_t)); 
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUEL_LGPM_RWA_CONTROL_idx2_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmrwacontrolidx2cmd, sizeof(PAYUEL_LGPM_RWA_CONTROL_idx2_Cmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 250: { /* 250 : LGPM RWA_CONTROL_idx3 (CC 20, With Payload) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;    
        static uint8_t fnccode = PAYUEL_LGPM_RWA_CONTROL_idx3_CC; 

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        // Payload Input 
        ImGui::InputScalar("Motor Driver EN", ImGuiDataType_U8, &command->lgpmrwacontrolidx3cmd.payload.Motor_driver_EN);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_RWA_CONTROL_idx3_Cmd_t) - 7};

            memcpy(command->lgpmrwacontrolidx3cmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmrwacontrolidx3cmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmrwacontrolidx3cmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmrwacontrolidx3cmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmrwacontrolidx3cmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_RWA_CONTROL_idx3_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmrwacontrolidx3cmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmrwacontrolidx3cmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_RWA_CONTROL_idx3_Cmd_t)); 
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length     = sizeof(PAYUEL_LGPM_RWA_CONTROL_idx3_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmrwacontrolidx3cmd, sizeof(PAYUEL_LGPM_RWA_CONTROL_idx3_Cmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 251: { /* 251 : LGPM RWA_PWR_ON (CC 21, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_RWA_PWR_ON_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_RWA_PWR_ON_Cmd_t) - 7};

            memcpy(command->lgpmrwapwroncmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmrwapwroncmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmrwapwroncmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmrwapwroncmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmrwapwroncmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_RWA_PWR_ON_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmrwapwroncmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmrwapwroncmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_RWA_PWR_ON_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_RWA_PWR_ON_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmrwapwroncmd, sizeof(PAYUEL_LGPM_RWA_PWR_ON_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }
    
    case 252: { /* 252 : LGPM RWA_PWR_OFF (CC 22, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_RWA_PWR_OFF_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_RWA_PWR_OFF_Cmd_t) - 7};

            memcpy(command->lgpmrwapwroffcmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmrwapwroffcmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmrwapwroffcmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmrwapwroffcmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmrwapwroffcmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_RWA_PWR_OFF_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmrwapwroffcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmrwapwroffcmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_RWA_PWR_OFF_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_RWA_PWR_OFF_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmrwapwroffcmd, sizeof(PAYUEL_LGPM_RWA_PWR_OFF_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }

    case 253: { /* 253 : LGPM RWA_SENSE_INFO (CC 23, No-Arg) */
        static uint16_t msgid = PAYUEL_LGPM_CMD_ID;
        static uint8_t fnccode = PAYUEL_LGPM_RWA_SENSE_INFO_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL ,"0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")){
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = {0xC0, 0x00};
            uint8_t  len[2] = {0x00, sizeof(PAYUEL_LGPM_RWA_SENSE_INFO_Cmd_t) - 7};

            memcpy(command->lgpmrwasenseinfocmd.CmdHeader    , &mid    , 2); 
            memcpy(command->lgpmrwasenseinfocmd.CmdHeader + 2, seq     , 2); 
            memcpy(command->lgpmrwasenseinfocmd.CmdHeader + 4, len     , 2); 
            memcpy(command->lgpmrwasenseinfocmd.CmdHeader + 6, &fnccode, 1); 

            command->lgpmrwasenseinfocmd.CmdHeader[7] = 0; 
            uint16_t total = sizeof(PAYUEL_LGPM_RWA_SENSE_INFO_Cmd_t); 
            const uint8_t *p = (const uint8_t *)&command->lgpmrwasenseinfocmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->lgpmrwasenseinfocmd.CmdHeader[7] = crc; 

            packetsign *pkt = (packetsign *)malloc(2 + 2 + 4 + sizeof(PAYUEL_LGPM_RWA_SENSE_INFO_Cmd_t)); 
            pkt->Identifier = HVD_TEST;            
            pkt->PacketType = MIM_PT_TMTC_TEST;    
            pkt->Length     = sizeof(PAYUEL_LGPM_RWA_SENSE_INFO_Cmd_t);
                
            memcpy(pkt->Data, &command->lgpmrwasenseinfocmd, sizeof(PAYUEL_LGPM_RWA_SENSE_INFO_Cmd_t));

            pthread_join(p_thread[4], NULL); 
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)pkt);
        }
        break;
    }
    
    case 280 :
    {
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_NOOP_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_NoopCmd_t) - 7 };

            memcpy(command->romanoopcmd.CmdHeader, &mid, 2);
            memcpy(command->romanoopcmd.CmdHeader + 2, seq, 2);
            memcpy(command->romanoopcmd.CmdHeader + 4, len, 2);
            memcpy(command->romanoopcmd.CmdHeader + 6, &fnccode, 1);

            command->romanoopcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_NoopCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romanoopcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romanoopcmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_NoopCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_NoopCmd_t);

            memcpy(pkt->Data, &command->romanoopcmd, sizeof(PAYUEL_ROMA_NoopCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 281 : 
    {
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_RESET_COUNTERS_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_ResetCountersCmd_t) - 7 };

            memcpy(command->romaresetcountercmd.CmdHeader, &mid, 2);
            memcpy(command->romaresetcountercmd.CmdHeader + 2, seq, 2);
            memcpy(command->romaresetcountercmd.CmdHeader + 4, len, 2);
            memcpy(command->romaresetcountercmd.CmdHeader + 6, &fnccode, 1);

            command->romaresetcountercmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_ResetCountersCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romaresetcountercmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romaresetcountercmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_ResetCountersCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_ResetCountersCmd_t);

            memcpy(pkt->Data, &command->romaresetcountercmd, sizeof(PAYUEL_ROMA_ResetCountersCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
       }
       break;
    }

    case 282 :
    {
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_COMM_TEST_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_CommTestCmd_t) - 7 };

            memcpy(command->romacommtestcmd.CmdHeader, &mid, 2);
            memcpy(command->romacommtestcmd.CmdHeader + 2, seq, 2);
            memcpy(command->romacommtestcmd.CmdHeader + 4, len, 2);
            memcpy(command->romacommtestcmd.CmdHeader + 6, &fnccode, 1);

            command->romacommtestcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_CommTestCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romacommtestcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romacommtestcmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_CommTestCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_CommTestCmd_t);

            memcpy(pkt->Data, &command->romacommtestcmd, sizeof(PAYUEL_ROMA_CommTestCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 283 :
    {
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_GET_SPECIFIC_LINE_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romagetspcificlinecmd.Payload.type);
        ImGui::InputScalar("Line No", ImGuiDataType_U16, &command->romagetspcificlinecmd.Payload.line_no);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_GetSpecificLineCmd_t) - 7 };

            memcpy(command->romagetspcificlinecmd.CmdHeader, &mid, 2);
            memcpy(command->romagetspcificlinecmd.CmdHeader + 2, seq, 2);
            memcpy(command->romagetspcificlinecmd.CmdHeader + 4, len, 2);
            memcpy(command->romagetspcificlinecmd.CmdHeader + 6, &fnccode, 1);

            command->romagetspcificlinecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_GetSpecificLineCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romagetspcificlinecmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romagetspcificlinecmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_GetSpecificLineCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_GetSpecificLineCmd_t);

            memcpy(pkt->Data, &command->romagetspcificlinecmd, sizeof(PAYUEL_ROMA_GetSpecificLineCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 284 :
    {
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_GET_MULTIPLE_LINES_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romagetmultiplelinescmd.Payload.type);
        ImGui::InputScalar("Line Start", ImGuiDataType_U16, &command->romagetmultiplelinescmd.Payload.line_start);
        ImGui::InputScalar("Line Stop", ImGuiDataType_U16, &command->romagetmultiplelinescmd.Payload.line_stop);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_GetMultipleLinesCmd_t) - 7 };

            memcpy(command->romagetmultiplelinescmd.CmdHeader, &mid, 2);
            memcpy(command->romagetmultiplelinescmd.CmdHeader + 2, seq, 2);
            memcpy(command->romagetmultiplelinescmd.CmdHeader + 4, len, 2);
            memcpy(command->romagetmultiplelinescmd.CmdHeader + 6, &fnccode, 1);

            command->romagetmultiplelinescmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_GetMultipleLinesCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romagetmultiplelinescmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romagetmultiplelinescmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_GetMultipleLinesCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_GetMultipleLinesCmd_t);

            memcpy(pkt->Data, &command->romagetmultiplelinescmd, sizeof(PAYUEL_ROMA_GetMultipleLinesCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 285: { /* ROMA GET_LATEST_LINE (CC 8) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_GET_LATEST_LINE_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romagetlatestlinecmd.Payload.type);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_GetLatestLineCmd_t) - 7 };

            memcpy(command->romagetlatestlinecmd.CmdHeader, &mid, 2);
            memcpy(command->romagetlatestlinecmd.CmdHeader + 2, seq, 2);
            memcpy(command->romagetlatestlinecmd.CmdHeader + 4, len, 2);
            memcpy(command->romagetlatestlinecmd.CmdHeader + 6, &fnccode, 1);

            command->romagetlatestlinecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_GetLatestLineCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romagetlatestlinecmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romagetlatestlinecmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_GetLatestLineCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_GetLatestLineCmd_t);

            memcpy(pkt->Data, &command->romagetlatestlinecmd, sizeof(PAYUEL_ROMA_GetLatestLineCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 286: { /* ROMA GET_LATEST_N_LINES (CC 9) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_GET_LATEST_N_LINES_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romagetlatestNlinescmd.Payload.type);
        ImGui::InputScalar("N Lines", ImGuiDataType_U16, &command->romagetlatestNlinescmd.Payload.n_lines);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_GetLatestNLinesCmd_t) - 7 };

            memcpy(command->romagetlatestNlinescmd.CmdHeader, &mid, 2);
            memcpy(command->romagetlatestNlinescmd.CmdHeader + 2, seq, 2);
            memcpy(command->romagetlatestNlinescmd.CmdHeader + 4, len, 2);
            memcpy(command->romagetlatestNlinescmd.CmdHeader + 6, &fnccode, 1);

            command->romagetlatestNlinescmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_GetLatestNLinesCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romagetlatestNlinescmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romagetlatestNlinescmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_GetLatestNLinesCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_GetLatestNLinesCmd_t);

            memcpy(pkt->Data, &command->romagetlatestNlinescmd, sizeof(PAYUEL_ROMA_GetLatestNLinesCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 287: { /* ROMA CLEAR_ALL_LINES (CC 10) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_CLEAR_ALL_LINES_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romaclearalllinescmd.Payload.type);
        ImGui::InputScalar("Code", ImGuiDataType_U16, &command->romaclearalllinescmd.Payload.code);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_ClearAllLinesCmd_t) - 7 };

            memcpy(command->romaclearalllinescmd.CmdHeader, &mid, 2);
            memcpy(command->romaclearalllinescmd.CmdHeader + 2, seq, 2);
            memcpy(command->romaclearalllinescmd.CmdHeader + 4, len, 2);
            memcpy(command->romaclearalllinescmd.CmdHeader + 6, &fnccode, 1);

            command->romaclearalllinescmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_ClearAllLinesCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romaclearalllinescmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romaclearalllinescmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_ClearAllLinesCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_ClearAllLinesCmd_t);

            memcpy(pkt->Data, &command->romaclearalllinescmd, sizeof(PAYUEL_ROMA_ClearAllLinesCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 293: { /* ROMA SET_ROUTE_DEFAULT (CC 16) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_SET_ROUTE_DEFAULT_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romasetroutedefaultcmd.Payload.type);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_SetRouteDefaultCmd_t) - 7 };

            memcpy(command->romasetroutedefaultcmd.CmdHeader, &mid, 2);
            memcpy(command->romasetroutedefaultcmd.CmdHeader + 2, seq, 2);
            memcpy(command->romasetroutedefaultcmd.CmdHeader + 4, len, 2);
            memcpy(command->romasetroutedefaultcmd.CmdHeader + 6, &fnccode, 1);

            command->romasetroutedefaultcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_SetRouteDefaultCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romasetroutedefaultcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romasetroutedefaultcmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_SetRouteDefaultCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_SetRouteDefaultCmd_t);

            memcpy(pkt->Data, &command->romasetroutedefaultcmd, sizeof(PAYUEL_ROMA_SetRouteDefaultCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 294: { /* ROMA RESET_ROUTE (CC 17) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_RESET_ROUTE_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romaresetroutecmd.Payload.type);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_ResetRouteCmd_t) - 7 };

            memcpy(command->romaresetroutecmd.CmdHeader, &mid, 2);
            memcpy(command->romaresetroutecmd.CmdHeader + 2, seq, 2);
            memcpy(command->romaresetroutecmd.CmdHeader + 4, len, 2);
            memcpy(command->romaresetroutecmd.CmdHeader + 6, &fnccode, 1);

            command->romaresetroutecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_ResetRouteCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romaresetroutecmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romaresetroutecmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_ResetRouteCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_ResetRouteCmd_t);

            memcpy(pkt->Data, &command->romaresetroutecmd, sizeof(PAYUEL_ROMA_ResetRouteCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 295: { /* ROMA LOAD_ROUTE (CC 18) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_LOAD_ROUTE_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romaloadroutecmd.Payload.type);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_LoadRouteCmd_t) - 7 };

            memcpy(command->romaloadroutecmd.CmdHeader, &mid, 2);
            memcpy(command->romaloadroutecmd.CmdHeader + 2, seq, 2);
            memcpy(command->romaloadroutecmd.CmdHeader + 4, len, 2);
            memcpy(command->romaloadroutecmd.CmdHeader + 6, &fnccode, 1);

            command->romaloadroutecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_LoadRouteCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romaloadroutecmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romaloadroutecmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_LoadRouteCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_LoadRouteCmd_t);

            memcpy(pkt->Data, &command->romaloadroutecmd, sizeof(PAYUEL_ROMA_LoadRouteCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 296: { /* ROMA SAVE_ROUTE (CC 19) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_SAVE_ROUTE_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romasaveroutecmd.Payload.type);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_SaveRouteCmd_t) - 7 };

            memcpy(command->romasaveroutecmd.CmdHeader, &mid, 2);
            memcpy(command->romasaveroutecmd.CmdHeader + 2, seq, 2);
            memcpy(command->romasaveroutecmd.CmdHeader + 4, len, 2);
            memcpy(command->romasaveroutecmd.CmdHeader + 6, &fnccode, 1);

            command->romasaveroutecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_SaveRouteCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romasaveroutecmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romasaveroutecmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_SaveRouteCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_SaveRouteCmd_t);

            memcpy(pkt->Data, &command->romasaveroutecmd, sizeof(PAYUEL_ROMA_SaveRouteCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 297: { /* ROMA SEND_ROUTE (CC 20) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_SEND_ROUTE_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romasendroutecmd.Payload.type);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_SendRouteCmd_t) - 7 };

            memcpy(command->romasendroutecmd.CmdHeader, &mid, 2);
            memcpy(command->romasendroutecmd.CmdHeader + 2, seq, 2);
            memcpy(command->romasendroutecmd.CmdHeader + 4, len, 2);
            memcpy(command->romasendroutecmd.CmdHeader + 6, &fnccode, 1);

            command->romasendroutecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_SendRouteCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romasendroutecmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romasendroutecmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_SendRouteCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_SendRouteCmd_t);

            memcpy(pkt->Data, &command->romasendroutecmd, sizeof(PAYUEL_ROMA_SendRouteCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 298: { /* ROMA SET_ROUTE (CC 21) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_SET_ROUTE_CC;
        static char route_buf[127] = "Route_Buf";

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romasetroutecmd.Payload.type);
        ImGui::InputText("Route Buf", route_buf, sizeof(route_buf));

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_SetRouteCmd_t) - 7 };

            memset(command->romasetroutecmd.Payload.route, 0, sizeof(command->romasetroutecmd.Payload.route));
            strncpy(command->romasetroutecmd.Payload.route, route_buf, sizeof(command->romasetroutecmd.Payload.route) - 1);

            memcpy(command->romasetroutecmd.CmdHeader, &mid, 2);
            memcpy(command->romasetroutecmd.CmdHeader + 2, seq, 2);
            memcpy(command->romasetroutecmd.CmdHeader + 4, len, 2);
            memcpy(command->romasetroutecmd.CmdHeader + 6, &fnccode, 1);

            command->romasetroutecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_SetRouteCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romasetroutecmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romasetroutecmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_SetRouteCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_SetRouteCmd_t);

            memcpy(pkt->Data, &command->romasetroutecmd, sizeof(PAYUEL_ROMA_SetRouteCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 299: { /* ROMA PAR_GET (CC 22) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_PAR_GET_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romapargetcmd.Payload.type);
        ImGui::InputScalar("Table", ImGuiDataType_U8, &command->romapargetcmd.Payload.table);
        ImGui::InputScalar("Param", ImGuiDataType_U8, &command->romapargetcmd.Payload.param);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_ParGetCmd_t) - 7 };

            memcpy(command->romapargetcmd.CmdHeader, &mid, 2);
            memcpy(command->romapargetcmd.CmdHeader + 2, seq, 2);
            memcpy(command->romapargetcmd.CmdHeader + 4, len, 2);
            memcpy(command->romapargetcmd.CmdHeader + 6, &fnccode, 1);

            command->romapargetcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_ParGetCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romapargetcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romapargetcmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_ParGetCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_ParGetCmd_t);

            memcpy(pkt->Data, &command->romapargetcmd, sizeof(PAYUEL_ROMA_ParGetCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 300: { /* ROMA PAR_SET (CC 23) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_PAR_SET_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romaparsetcmd.Payload.type);
        ImGui::InputScalar("Table", ImGuiDataType_U8, &command->romaparsetcmd.Payload.table);
        ImGui::InputScalar("Param", ImGuiDataType_U8, &command->romaparsetcmd.Payload.param);
        ImGui::InputScalar("Value", ImGuiDataType_S32, &command->romaparsetcmd.Payload.value);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_ParSetCmd_t) - 7 };

            memcpy(command->romaparsetcmd.CmdHeader, &mid, 2);
            memcpy(command->romaparsetcmd.CmdHeader + 2, seq, 2);
            memcpy(command->romaparsetcmd.CmdHeader + 4, len, 2);
            memcpy(command->romaparsetcmd.CmdHeader + 6, &fnccode, 1);

            command->romaparsetcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_ParSetCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romaparsetcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romaparsetcmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_ParSetCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_ParSetCmd_t);

            memcpy(pkt->Data, &command->romaparsetcmd, sizeof(PAYUEL_ROMA_ParSetCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 301: { /* ROMA PAR_DEFAULTS (CC 24) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_PAR_DEFAULTS_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romapardefaultscmd.Payload.type);
        ImGui::InputScalar("Table", ImGuiDataType_U8, &command->romapardefaultscmd.Payload.table);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_ParDefaultsCmd_t) - 7 };

            memcpy(command->romapardefaultscmd.CmdHeader, &mid, 2);
            memcpy(command->romapardefaultscmd.CmdHeader + 2, seq, 2);
            memcpy(command->romapardefaultscmd.CmdHeader + 4, len, 2);
            memcpy(command->romapardefaultscmd.CmdHeader + 6, &fnccode, 1);

            command->romapardefaultscmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_ParDefaultsCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romapardefaultscmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romapardefaultscmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_ParDefaultsCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_ParDefaultsCmd_t);

            memcpy(pkt->Data, &command->romapardefaultscmd, sizeof(PAYUEL_ROMA_ParDefaultsCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 302: { /* ROMA PAR_SAVE (CC 25) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_PAR_SAVE_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romaparsavecmd.Payload.type);
        ImGui::InputScalar("Table", ImGuiDataType_U8, &command->romaparsavecmd.Payload.table);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_ParSaveCmd_t) - 7 };

            memcpy(command->romaparsavecmd.CmdHeader, &mid, 2);
            memcpy(command->romaparsavecmd.CmdHeader + 2, seq, 2);
            memcpy(command->romaparsavecmd.CmdHeader + 4, len, 2);
            memcpy(command->romaparsavecmd.CmdHeader + 6, &fnccode, 1);

            command->romaparsavecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_ParSaveCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romaparsavecmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romaparsavecmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_ParSaveCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_ParSaveCmd_t);

            memcpy(pkt->Data, &command->romaparsavecmd, sizeof(PAYUEL_ROMA_ParSaveCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 303: { /* ROMA PAR_RESTORE (CC 26) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_PAR_RESTORE_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romaparrestorecmd.Payload.type);
        ImGui::InputScalar("Table", ImGuiDataType_U8, &command->romaparrestorecmd.Payload.table);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_ParRestoreCmd_t) - 7 };

            memcpy(command->romaparrestorecmd.CmdHeader, &mid, 2);
            memcpy(command->romaparrestorecmd.CmdHeader + 2, seq, 2);
            memcpy(command->romaparrestorecmd.CmdHeader + 4, len, 2);
            memcpy(command->romaparrestorecmd.CmdHeader + 6, &fnccode, 1);

            command->romaparrestorecmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_ParRestoreCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romaparrestorecmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romaparrestorecmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_ParRestoreCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_ParRestoreCmd_t);

            memcpy(pkt->Data, &command->romaparrestorecmd, sizeof(PAYUEL_ROMA_ParRestoreCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 304: { /* ROMA PAR_LOAD (CC 27) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_PAR_LOAD_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romaparloadcmd.Payload.type);
        ImGui::InputScalar("Table", ImGuiDataType_U8, &command->romaparloadcmd.Payload.table);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_ParLoadCmd_t) - 7 };

            memcpy(command->romaparloadcmd.CmdHeader, &mid, 2);
            memcpy(command->romaparloadcmd.CmdHeader + 2, seq, 2);
            memcpy(command->romaparloadcmd.CmdHeader + 4, len, 2);
            memcpy(command->romaparloadcmd.CmdHeader + 6, &fnccode, 1);

            command->romaparloadcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_ParLoadCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romaparloadcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romaparloadcmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_ParLoadCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_ParLoadCmd_t);

            memcpy(pkt->Data, &command->romaparloadcmd, sizeof(PAYUEL_ROMA_ParLoadCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 305: { /* ROMA PAR_SET_OOB (CC 28) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_PAR_SET_OOB_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputScalar("Type", ImGuiDataType_U8, &command->romaparsetOOBcmd.Payload.type);
        ImGui::InputScalar("Table", ImGuiDataType_U8, &command->romaparsetOOBcmd.Payload.table);
        ImGui::InputScalar("Enable", ImGuiDataType_U8, &command->romaparsetOOBcmd.Payload.enable);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_ParSetOOBCmd_t) - 7 };

            memcpy(command->romaparsetOOBcmd.CmdHeader, &mid, 2);
            memcpy(command->romaparsetOOBcmd.CmdHeader + 2, seq, 2);
            memcpy(command->romaparsetOOBcmd.CmdHeader + 4, len, 2);
            memcpy(command->romaparsetOOBcmd.CmdHeader + 6, &fnccode, 1);

            command->romaparsetOOBcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_ParSetOOBCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romaparsetOOBcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romaparsetOOBcmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_ParSetOOBCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_ParSetOOBCmd_t);

            memcpy(pkt->Data, &command->romaparsetOOBcmd, sizeof(PAYUEL_ROMA_ParSetOOBCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 306: { /* ROMA SEND_COMMAND (CC 29) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_SEND_COMMAND_CC;
        static char cmd_buf[128] = "Cmd_Buf";

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputText("Command", cmd_buf, sizeof(cmd_buf));

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_SendCommandCmd_t) - 7 };

            memset(command->romasendcommandcmd.Payload.cmd, 0, sizeof(command->romasendcommandcmd.Payload.cmd));
            strncpy(command->romasendcommandcmd.Payload.cmd, cmd_buf, sizeof(command->romasendcommandcmd.Payload.cmd) - 1);

            memcpy(command->romasendcommandcmd.CmdHeader, &mid, 2);
            memcpy(command->romasendcommandcmd.CmdHeader + 2, seq, 2);
            memcpy(command->romasendcommandcmd.CmdHeader + 4, len, 2);
            memcpy(command->romasendcommandcmd.CmdHeader + 6, &fnccode, 1);

            command->romasendcommandcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_SendCommandCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romasendcommandcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romasendcommandcmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_SendCommandCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_SendCommandCmd_t);

            memcpy(pkt->Data, &command->romasendcommandcmd, sizeof(PAYUEL_ROMA_SendCommandCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 307: { /* ROMA SEND_MSG (CC 30) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_SEND_MSG_CC;
        static char msg_buf[128] = "Msg_Buf";

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);
        ImGui::InputText("Message", msg_buf, sizeof(msg_buf));

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_SendMsgCmd_t) - 7 };

            memset(command->romasendmsgcmd.Payload.msg, 0, sizeof(command->romasendmsgcmd.Payload.msg));
            strncpy(command->romasendmsgcmd.Payload.msg, msg_buf, sizeof(command->romasendmsgcmd.Payload.msg) - 1);

            memcpy(command->romasendmsgcmd.CmdHeader, &mid, 2);
            memcpy(command->romasendmsgcmd.CmdHeader + 2, seq, 2);
            memcpy(command->romasendmsgcmd.CmdHeader + 4, len, 2);
            memcpy(command->romasendmsgcmd.CmdHeader + 6, &fnccode, 1);

            command->romasendmsgcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_SendMsgCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romasendmsgcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romasendmsgcmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_SendMsgCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_SendMsgCmd_t);

            memcpy(pkt->Data, &command->romasendmsgcmd, sizeof(PAYUEL_ROMA_SendMsgCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 308: { /* ROMA SYNC_RX (CC 31) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_SYNC_RX_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_SyncRxCmd_t) - 7 };

            memcpy(command->romasyncrxcmd.CmdHeader, &mid, 2);
            memcpy(command->romasyncrxcmd.CmdHeader + 2, seq, 2);
            memcpy(command->romasyncrxcmd.CmdHeader + 4, len, 2);
            memcpy(command->romasyncrxcmd.CmdHeader + 6, &fnccode, 1);

            command->romasyncrxcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_SyncRxCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romasyncrxcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romasyncrxcmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_SyncRxCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_SyncRxCmd_t);

            memcpy(pkt->Data, &command->romasyncrxcmd, sizeof(PAYUEL_ROMA_SyncRxCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 309: { /* ROMA SYNC_TX (CC 32) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_SYNC_TX_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_SyncTxCmd_t) - 7 };

            memcpy(command->romasynctxcmd.CmdHeader, &mid, 2);
            memcpy(command->romasynctxcmd.CmdHeader + 2, seq, 2);
            memcpy(command->romasynctxcmd.CmdHeader + 4, len, 2);
            memcpy(command->romasynctxcmd.CmdHeader + 6, &fnccode, 1);

            command->romasynctxcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_SyncTxCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romasynctxcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romasynctxcmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_SyncTxCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_SyncTxCmd_t);

            memcpy(pkt->Data, &command->romasynctxcmd, sizeof(PAYUEL_ROMA_SyncTxCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
    }

    case 310: { /* ROMA PAY_INIT (CC 33) */
        static uint16_t msgid = PAYUEL_ROMA_CMD_MID;
        static uint8_t fnccode = PAYUEL_ROMA_PAY_INIT_CC;

        ImGui::InputScalar("Msg ID", ImGuiDataType_U16, &msgid, NULL, NULL, "0x%04X");
        ImGui::InputScalar("Command Code(CC)", ImGuiDataType_U8, &fnccode);

        if (ImGui::Button("Generate CMD")) {
            WriteSystemName(msgid);
            uint16_t mid = htons(msgid);
            uint8_t  seq[2] = { 0xC0, 0x00 };
            uint8_t  len[2] = { 0x00, sizeof(PAYUEL_ROMA_PayInitCmd_t) - 7 };

            memcpy(command->romapayinitcmd.CmdHeader, &mid, 2);
            memcpy(command->romapayinitcmd.CmdHeader + 2, seq, 2);
            memcpy(command->romapayinitcmd.CmdHeader + 4, len, 2);
            memcpy(command->romapayinitcmd.CmdHeader + 6, &fnccode, 1);

            command->romapayinitcmd.CmdHeader[7] = 0;
            uint16_t total = sizeof(PAYUEL_ROMA_PayInitCmd_t);
            const uint8_t* p = (const uint8_t*)&command->romapayinitcmd;
            uint8_t crc = 0xFF;

            while (total--) crc ^= *(p++);
            command->romapayinitcmd.CmdHeader[7] = crc;

            packetsign* pkt = (packetsign*)malloc(2 + 2 + 4 + sizeof(PAYUEL_ROMA_PayInitCmd_t));
            pkt->Identifier = HVD_TEST;
            pkt->PacketType = MIM_PT_TMTC_TEST;
            pkt->Length = sizeof(PAYUEL_ROMA_PayInitCmd_t);

            memcpy(pkt->Data, &command->romapayinitcmd, sizeof(PAYUEL_ROMA_PayInitCmd_t));

            pthread_join(p_thread[4], NULL);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)pkt);
        }
        break;
        }
    }
}




void ImGui_ControlWindow(float fontscale)
{
    ImGui::Begin("Control GUI", &p_open, mim_winflags);
    ImGui::SetWindowFontScale(fontscale);

    ImGuiStyle& style = ImGui::GetStyle();
    float buttonHeight = ImGui::GetFontSize() * 1.5f;

    /************************5차 변경************************************/
    float rows = 3.0f;
    /********************************************************************/

    float pingRegionHeight =
        buttonHeight * rows
        + style.FramePadding.y * 2.5f
        + style.ItemSpacing.y * (rows + 2);

    ImGui::BeginChild("PingRegion",
                      ImVec2(ImGui::GetContentRegionAvail().x, pingRegionHeight),
                      true,
                      mim_winflags | ImGuiWindowFlags_NoScrollbar);

    float regionW = ImGui::GetContentRegionAvail().x;
    float eachW   = (regionW - style.ItemSpacing.x) * 0.5f; 

    /***************************5차 변경*************************************/
    if (ImGui::Button("Switch Satellite", ImVec2(eachW, buttonHeight)))
    {
        show_satellite_select = true;
    }
    /***************************************************************************/

    if (ImGui::Button("Ping(AX100)", ImVec2(eachW, buttonHeight)))
    {
        pthread_join(p_thread[4], NULL);
        pthread_create(&p_thread[4], NULL, csp_ping_console, NULL);
    }

    ImGui::SameLine();
    if (ImGui::Button("Ping(OBC)", ImVec2(eachW, buttonHeight)))
    {
        pthread_join(p_thread[4], NULL);
        FSWTle* MIM_TLEinfo = (FSWTle*)malloc(sizeof(FSWTle) + MIM_HAND_DATAFDSTART);
        int NowPingInfo = GetNowTracking();

        if (NowPingInfo < -1)
        {
            console.AddLog("[ERROR]##You Tried Ping without Tracking. Please Check Again.");
        }
        else if (NowPingInfo == -1)
        {
            FSWTleConverter(State.Fatellites->tle, MIM_TLEinfo);
            packetsign* TestPacket = PingInit(MIM_TLEinfo);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
        }
        else
        {
            FSWTleConverter(State.Satellites[NowPingInfo]->tle, MIM_TLEinfo);
            packetsign* TestPacket = PingInit(MIM_TLEinfo);
            pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void*)TestPacket);
        }
    }

    if (ImGui::Button("Ping each node", ImVec2(eachW, buttonHeight)))
    {
        show_pingWindow = true;
    }


    ImGui::SameLine();
    if (ImGui::Button("Custom RParam Get", ImVec2(eachW, buttonHeight)))
    {
        show_rparamWindow = true;
    }

    if (ImGui::Button("Send Telecommand", ImVec2(eachW, buttonHeight)))
    {
        show_tmtcWindow = true;
        csp_rtable_print();
    }

    ImGui::SameLine();
    if (ImGui::Button("Initial Ops", ImVec2(eachW, buttonHeight)))
    {
        show_initialControlWindow = true;
    }

    ImGui::EndChild();

    ImGui::SetWindowFontScale(1.0f);



    if (ImGui::Button("Baud Rate Calibration",
                      ImVec2(ImGui::GetContentRegionAvail().x * 0.33f,
                             ImGui::GetFontSize() * 1.5f)))
    {
        pthread_join(p_thread[5], NULL);
        pthread_create(&p_thread[5], NULL, csp_baud_calibration, NULL);
    }

    if (ImGui::Button("Frequency Calibration",
                      ImVec2(ImGui::GetContentRegionAvail().x * 0.33f,
                             ImGui::GetFontSize() * 1.5f)))
    {
        pthread_join(p_thread[5], NULL);
        pthread_create(&p_thread[5], NULL, csp_freq_calibration, NULL);
    }

    ImGui::SameLine();
    if (!State.AMPON)
    {
        if (State.AmpTime > 0)
        {
            strftime(AmpTimeBuf, sizeof(AmpTimeBuf), "Disabled  %M:%S", State.AmpTM);
            if (ImGui::Button(AmpTimeBuf,
                              ImVec2(ImGui::GetContentRegionAvail().x,
                                     ImGui::GetFontSize() * 1.5f)))
            {
                pid_t pid_off = fork();
                if (pid_off == -1)
                    console.AddLog("[ERROR]##Failed to fork process");
                else if (pid_off == 0)
                    execl("../amp/ampoff", "ampoff", NULL);
                else
                    console.AddLog("[OK]##Forked Safe Amplifier Process. (OFF)");
                State.AMPON = false;
            }
        }
        else
        {
            sprintf(AmpTimeBuf, "AMP ON");
            if (ImGui::Button(AmpTimeBuf,
                              ImVec2(ImGui::GetContentRegionAvail().x,
                                     ImGui::GetFontSize() * 1.5f)))
            {
                pthread_join(p_thread[6], NULL);
                pid_t pid_on = fork();
                if (pid_on == -1)
                    console.AddLog("[ERROR]##Failed to fork process");
                else if (pid_on == 0)
                    execl("../amp/ampcontrol", "ampcontrol", NULL);
                else
                {
                    console.AddLog("[OK]##Forked Safe Amplifier Process.(ON, Timer)");
                    pthread_create(&p_thread[6], NULL, AmpTimer, NULL);
                }
            }
        }
    }
    else
    {
        if (State.AmpTime > 0)
            strftime(AmpTimeBuf, sizeof(AmpTimeBuf), "AMP OFF %M:%S", State.AmpTM);
        else
            sprintf(AmpTimeBuf, "AMP OFF");

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.650f, 0.266f, 0.322f, 1.0f));
        if (ImGui::Button(AmpTimeBuf,
                          ImVec2(ImGui::GetContentRegionAvail().x,
                                 ImGui::GetFontSize() * 1.5f)))
        {
            pid_t pid_off = fork();
            if (pid_off == -1)
                console.AddLog("[ERROR]##Failed to fork process");
            else if (pid_off == 0)
                execl("../amp/ampoff", "ampoff", NULL);
            else
                console.AddLog("[OK]##Forked Safe Amplifier Process. (OFF)");
            State.AMPON = false;
        }
        ImGui::PopStyleColor();
    }

    if (ImGui::Button("CMD Manager",
                      ImVec2(ImGui::GetContentRegionAvail().x * 0.33f,
                             ImGui::GetFontSize() * 1.5f)))
    {
        State.Display_CMD = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear",
                      ImVec2(ImGui::GetContentRegionAvail().x,
                             ImGui::GetFontSize() * 1.5f)))
    {
        NowCMD = 0;
        for (int i = 0; i < 256; i++)
        {
            if (SatCMD[i] != NULL)
            {
                delete SatCMD[i];
                SatCMD[i] = NULL;
            }
        }
    }

    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();




    ImGuiIO& io = ImGui::GetIO();
    ImVec2 popup_size(600.0f, 380.0f);
    ImVec2 popup_pos((io.DisplaySize.x - popup_size.x) * 0.5f,
                     (io.DisplaySize.y - popup_size.y) * 0.5f);

    if (show_tmtcWindow)
    {
        ImGui::SetNextWindowSize(popup_size, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(popup_pos, ImGuiCond_FirstUseEver);
        ImGui::Begin("TMTC Command Test##msgid_fnccode_TypeWindow", &show_tmtcWindow);
        DrawCmdGeneratorBody(false);
        ImGui::End();
    }

    if (show_initialControlWindow)
    {
        ImGui::SetNextWindowSize(popup_size, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(popup_pos, ImGuiCond_FirstUseEver);
        ImGui::Begin("Initial Operations##INITIAL_CMD_WINDOW", &show_initialControlWindow);
        DrawCmdGeneratorBody(true);
        ImGui::End();
    }

    if (show_pingWindow)
    {
        ImGui::SetNextWindowSize(popup_size, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(popup_pos, ImGuiCond_FirstUseEver);
        ImGui::Begin("Ping each node##PING_WINDOW", &show_pingWindow);
        PingWindow();
        ImGui::End();
    }

    if (show_rparamWindow)
    {
        ImGui::SetNextWindowSize(popup_size, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(popup_pos, ImGuiCond_FirstUseEver);
        ImGui::Begin("Get Custom Rparam##RPARAM_WINDOW", &show_rparamWindow);
        RparamWindow();
        ImGui::End();
    }

    /************************************5차 추가**************************************/
    if (show_satellite_select)
    {
        ImGui::SetNextWindowSize(popup_size, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(popup_pos, ImGuiCond_FirstUseEver);
        ImGui::Begin("Switch HMAC Key##SATELLITE_SELECT", &show_satellite_select);
        HMACWindow();
        ImGui::End();
    }
    /**********************************************************************************/
}



void ImGui_CommandWindow(float fontscale)
{  
    ImGui::Begin("Command GUI", &p_open, mim_winflags);
    ImGui::SetWindowFontScale(fontscale);
    ImGui::BeginChild("##FTPManager", ImVec2(ImGui::GetContentRegionAvail().x * 0.4, ImGui::GetContentRegionAvail().y), true, mim_winflags);
    ImGui::Text("File List");
    ImGui::Text("Telemetry : ");
    ImGui::SameLine();
    ImGui::RadioButton("HK ##ftpwindow", &temptarget, MIM_DLTYPE_HK);
    ImGui::SameLine();
    ImGui::RadioButton("AOD##ftpwindow", &temptarget, MIM_DLTYPE_AOD);
    ImGui::SameLine();
    ImGui::RadioButton("LOG##ftpwindow", &temptarget, MIM_DLTYPE_LOG);
    ImGui::SameLine();
    ImGui::RadioButton("SNSR##ftpwindow", &temptarget, MIM_DLTYPE_SNSR);
    ImGui::Text("            ");
    ImGui::SameLine();
    ImGui::RadioButton("GPS##ftpwindow", &temptarget, MIM_DLTYPE_GPS);
    ImGui::SameLine();
    ImGui::RadioButton("CTRLO##ftpwindow", &temptarget, MIM_DLTYPE_CTRLO);

    ImGui::Text("Status    : ");
    ImGui::SameLine();
    ImGui::RadioButton("NOW##ftpwindow", &tempstatus, MIM_DLSTAT_NEW);
    ImGui::SameLine();
    ImGui::RadioButton("OLD##ftpwindow", &tempstatus, MIM_DLSTAT_OLD);
    ImGui::SameLine();
    ImGui::RadioButton("CUR##ftpwindow", &tempstatus, MIM_DLSTAT_ING);
    ImGui::BeginTable("##filelisttable", 3, false, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 3.0f));
    
    ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
    ImGui::TableSetupColumn("Number##tle",                ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide, 0.0f);
    ImGui::TableSetupColumn("Step##tle",                 ImGuiTableColumnFlags_NoHide, 0.0f);
    ImGui::TableSetupColumn("Offset##tle",            ImGuiTableColumnFlags_NoHide, 0.0f);
    ImGui::TableHeadersRow();
    if(tempfilenum < 1)
        tempfilenum = 1;
    if(tempstep < 0)
        tempstep = 0;
    if(tempoffset < 0)
        tempoffset = 0;
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::InputInt("##ftpwindow_filenum", &tempfilenum);
    ImGui::TableSetColumnIndex(1);
    ImGui::InputInt("##ftpwindow_step", &tempstep);
    ImGui::TableSetColumnIndex(2);
    ImGui::InputInt("##ftpwindow_offset", &tempoffset);
    ImGui::EndTable();
    

    if (ImGui::Button("Save##ftpwindow", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetContentRegionAvail().y)))
    {
        request->target = (uint16_t) temptarget;
        request->filestatus = (uint16_t) tempstatus;
        request->filenum = (uint32_t) tempfilenum;
        request->offset = (uint32_t) tempoffset;
        request->step = (uint32_t) tempstep;
    }
    ImGui::SameLine();
    if (ImGui::Button("Request##ftpwindow", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y)))
    {
        packetsign * request_ = (packetsign *)malloc(sizeof(dlreqdata) + 8);
        request_->Identifier = MIM_ID;
        request_->PacketType = MIM_PT_DLREQ;
        request_->Length = sizeof(dlreqdata);
        memcpy(request_->Data, request, sizeof(dlreqdata));
        pthread_create(&p_thread[4], NULL, task_uplink_onorbit, (void *)request_); 
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("##FTPSending", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), true, mim_winflags);
    ImGui::Text("FTP  ");
    ImGui::SameLine();
    ImGui::RadioButton("V1      ##FTPVersionSelect1", &State.ftp_version, 1);
    ImGui::SameLine();
    ImGui::RadioButton("V2      ##FTPVersionSelect2", &State.ftp_version, 2);
    ImGui::SameLine();
    ImGui::Text("           ");
    ImGui::SameLine();
    ImGui::Text("Chunk Size ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputScalar("##ftpchunksize", ImGuiDataType_U32, &State.chunk_sz, NULL, NULL, "%u", NULL);
    if(State.chunk_sz < 1)
        State.chunk_sz = 1;
    ImGui::Text("Task ");
    ImGui::SameLine();
    ImGui::RadioButton("Upload  ##FTPTaskSelect00", &State.ftp_task, FTP_UPLOAD_REQUEST);
    ImGui::SameLine();
    ImGui::RadioButton("Download##FTPTaskSelect02", &State.ftp_task, FTP_DOWNLOAD_REQUEST);
    ImGui::SameLine();
    ImGui::RadioButton("Listup  ##FTPTaskSelect07", &State.ftp_task, FTP_LIST_REQUEST);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    
    if(!State.AMPON)
    {
        if(State.AmpTime > 0)
        {
            strftime(AmpTimeBuf, sizeof(AmpTimeBuf), "Disabled  %M:%S", State.AmpTM);
            if (ImGui::Button(AmpTimeBuf, ImVec2(ImGui::GetContentRegionAvail().x , ImGui::GetFontSize() * 1.5)))
            {
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
            }
        }
            
        else
        {
            sprintf(AmpTimeBuf, "AMP ON");
            if (ImGui::Button(AmpTimeBuf, ImVec2(ImGui::GetContentRegionAvail().x , ImGui::GetFontSize() * 1.5)))
            {
                pthread_join(p_thread[6], NULL);
                pid_t pid_on = fork();

                if (pid_on == -1) {
                    console.AddLog("[ERROR]##Failed to fork process");
                }
                else if (pid_on == 0) {
                    // Child process
                    execl("../amp/ampcontrol", "ampcontrol", NULL);
                    
                }
                else
                {
                    console.AddLog("[OK]##Forked Safe Amplifier Process.(ON, Timer)");
                    pthread_create(&p_thread[6], NULL, AmpTimer, NULL);
                }
            }
        }
            
        
    }
    else
    {
        if(State.AmpTime > 0)
            strftime(AmpTimeBuf, sizeof(AmpTimeBuf), "AMP OFF %M:%S", State.AmpTM);
        else
            sprintf(AmpTimeBuf, "AMP OFF");
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.650f, 0.266f, 0.322f, 1.0f));
        if (ImGui::Button(AmpTimeBuf, ImVec2(ImGui::GetContentRegionAvail().x , ImGui::GetFontSize() * 1.5)))
        {
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
        }
        ImGui::PopStyleColor();
    }
    // ImGui::Text("     ");
    // ImGui::SameLine();
    // ImGui::RadioButton("Move    ##FTPTaskSelect10", &State.ftp_task, FTP_MOVE_REQUEST);
    // ImGui::SameLine();
    // ImGui::RadioButton("Remove  ##FTPTaskSelect12", &State.ftp_task, FTP_REMOVE_REQUEST);
    // ImGui::SameLine();
    // ImGui::RadioButton("Copy    ##FTPTaskSelect23", &State.ftp_task, FTP_COPY_REQUEST);
    // ImGui::SameLine();
    // ImGui::RadioButton("MKDIR   ##FTPTaskSelect25", &State.ftp_task, FTP_MKDIR_REQUEST);
    // ImGui::SameLine();
    // ImGui::RadioButton("RMDIR   ##FTPTaskSelect27", &State.ftp_task, FTP_RMDIR_REQUEST);
    switch(State.ftp_task)
    {
    case FTP_UPLOAD_REQUEST :{
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.8);
        if (ImGui::BeginCombo("##FTPCombobox", State.ftplistup[NowFTP].name))
        {
            for (int FTPindex = 0; FTPindex < IM_ARRAYSIZE(State.ftplistup); FTPindex++)
            {
                if (State.ftplistup[FTPindex].name[0] == 0)
                    continue;
                bool SelectedFTP = (NowFTP == FTPindex);
                char Selectedlabel[30];
                sprintf(Selectedlabel, "%s##%d", State.ftplistup[FTPindex].name, FTPindex);
                if (ImGui::Selectable(Selectedlabel, SelectedFTP))
                    NowFTP = FTPindex;
                        
                if (SelectedFTP)
                    ImGui::SetItemDefaultFocus();
                usleep(10);
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Upload", ImVec2(ImGui::GetContentRegionAvail().x , ImGui::GetFontSize() * 1.5)))
        {
            if(State.ftp_mode)
                console.AddLog("[ERROR]##Still Uplink Thread is Running. Please Retry Later.");
            else
            {
                pthread_join(p_thread[8], NULL);
                if(State.ftp_version == 1)
                    pthread_create(&p_thread[8], NULL, ftp_uplink_force, &State.ftplistup[NowFTP]);
                else
                    pthread_create(&p_thread[8], NULL, ftp_uplink_onorbit, &State.ftplistup[NowFTP]);
            }
        }
        ImGui::Text("Local  ");
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##FTP_local_filepath : ", State.ftplistup[NowFTP].local_path, sizeof(State.ftplistup[NowFTP].local_path));
        ImGui::PopItemWidth();
        ImGui::Text("Remote ");
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##FTP_remote_filepath : ", State.ftplistup[NowFTP].remote_path, sizeof(State.ftplistup[NowFTP].remote_path));
        ImGui::PopItemWidth();
        ImGui::Text(State.FTPWindowBuffer);
        break;
    }
    case FTP_DOWNLOAD_REQUEST :{
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.8);
        if (ImGui::BeginCombo("##FTPCombobox", State.ftplistup[NowFTP].name))
        {
            for (int FTPindex = 0; FTPindex < IM_ARRAYSIZE(State.ftplistup); FTPindex++)
            {
                if (State.ftplistup[FTPindex].name[0] == 0)
                    continue;
                bool SelectedFTP = (NowFTP == FTPindex);
                char Selectedlabel[30];
                sprintf(Selectedlabel, "%s##%d", State.ftplistup[FTPindex].name, FTPindex);
                if (ImGui::Selectable(Selectedlabel, SelectedFTP))
                    NowFTP = FTPindex;
                        
                if (SelectedFTP)
                    ImGui::SetItemDefaultFocus();
                usleep(10);
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Download", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(State.ftp_mode)
                console.AddLog("[ERROR]##Still Uplink Thread is Running. Please Retry Later.");
            else
            {
                pthread_join(p_thread[8], NULL);
                if(State.ftp_version == 1)
                    pthread_create(&p_thread[8], NULL, ftp_downlink_force, &State.ftplistup[NowFTP]);
                else
                    pthread_create(&p_thread[8], NULL, ftp_downlink_onorbit, &State.ftplistup[NowFTP]);
            }
        }
        ImGui::Text("Local  ");
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##FTP_local_filepath : ", State.ftplistup[NowFTP].local_path, sizeof(State.ftplistup[NowFTP].local_path));
        ImGui::PopItemWidth();
        ImGui::Text("Remote ");
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##FTP_remote_filepath : ", State.ftplistup[NowFTP].remote_path, sizeof(State.ftplistup[NowFTP].remote_path));
        ImGui::PopItemWidth();
        ImGui::Text(State.FTPWindowBuffer);

        break;
    }
    case FTP_LIST_REQUEST :{
        if(State.ftp_version != 2)
            State.ftp_task = FTP_UPLOAD_REQUEST;
        ImGui::Text("Path   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##FTP_listup_path : ", State.gslistup->fpathbuf, sizeof(State.gslistup->fpathbuf));
        ImGui::PopItemWidth();
        if (ImGui::Button("Listup", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(State.ftp_mode)
                console.AddLog("[ERROR]##Still Uplink Thread is Running. Please Retry Later.");
            else
            {
                pthread_join(p_thread[8], NULL);
                pthread_create(&p_thread[8], NULL, ftp_list_onorbit, NULL);
            }
        }
        ImGui::Text(State.FTPWindowBuffer);
        ImGui::Text(State.gslistup->fdispbuf);
        break;
    }
    case FTP_MOVE_REQUEST :{
        if(State.ftp_version != 2)
            State.ftp_task = FTP_UPLOAD_REQUEST;
        ImGui::Text("From   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##FTP_movefrom_path : ", State.gsmove->from, sizeof(State.gsmove->from));
        ImGui::PopItemWidth();
        ImGui::Text("To     ");
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##FTP_moveto_path : ", State.gsmove->to, sizeof(State.gsmove->to));
        ImGui::PopItemWidth();
        if (ImGui::Button("Move", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(State.ftp_mode)
                console.AddLog("[ERROR]##Still Uplink Thread is Running. Please Retry Later.");
            else
            {
                pthread_join(p_thread[8], NULL);
                pthread_create(&p_thread[8], NULL, ftp_move_onorbit, NULL);
            }
        }
        ImGui::Text(State.FTPWindowBuffer);
        break;
    }
    case FTP_REMOVE_REQUEST :{
        if(State.ftp_version != 2)
            State.ftp_task = FTP_UPLOAD_REQUEST;
        ImGui::Text("Path   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##FTP_REMOVE_path : ", State.gsremove->path, sizeof(State.gsremove->path));
        ImGui::PopItemWidth();
        if (ImGui::Button("Remove", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(State.ftp_mode)
                console.AddLog("[ERROR]##Still Uplink Thread is Running. Please Retry Later.");
            else
            {
                pthread_join(p_thread[8], NULL);
                pthread_create(&p_thread[8], NULL, ftp_remove_onorbit, NULL);
            }
        }
        ImGui::Text(State.FTPWindowBuffer);
        break;
    }
    case FTP_COPY_REQUEST :{
        if(State.ftp_version != 2)
            State.ftp_task = FTP_UPLOAD_REQUEST;
        ImGui::Text("From   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##FTP_copyfrom_path : ", State.gscopy->from, sizeof(State.gscopy->from));
        ImGui::PopItemWidth();
        ImGui::Text("To     ");
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##FTP_copyto_path : ", State.gscopy->to, sizeof(State.gscopy->to));
        ImGui::PopItemWidth();
        if (ImGui::Button("Copy", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(State.ftp_mode)
                console.AddLog("[ERROR]##Still Uplink Thread is Running. Please Retry Later.");
            else
            {
                pthread_join(p_thread[8], NULL);
                pthread_create(&p_thread[8], NULL, ftp_copy_onorbit, NULL);
            }
        }
        ImGui::Text(State.FTPWindowBuffer);
        break;
    }
    case FTP_MKDIR_REQUEST :{
        if(State.ftp_version != 2)
            State.ftp_task = FTP_UPLOAD_REQUEST;
        ImGui::Text("Path   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##FTP_mkdir_path : ", State.gsmkdir->path, sizeof(State.gsmkdir->path));
        ImGui::PopItemWidth();
        if (ImGui::Button("Make Directory", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(State.ftp_mode)
                console.AddLog("[ERROR]##Still Uplink Thread is Running. Please Retry Later.");
            else
            {
                pthread_join(p_thread[8], NULL);
                pthread_create(&p_thread[8], NULL, ftp_mkdir_onorbit, NULL);
            }
        }
        ImGui::Text(State.FTPWindowBuffer);
        break;
    }
    case FTP_RMDIR_REQUEST :{
        if(State.ftp_version != 2)
            State.ftp_task = FTP_UPLOAD_REQUEST;
        ImGui::Text("Path   ");
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##FTP_rmdir_path : ", State.gsrmdir->path, sizeof(State.gsrmdir->path));
        ImGui::PopItemWidth();
        if (ImGui::Button("Remove Directory", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            if(State.ftp_mode)
                console.AddLog("[ERROR]##Still Uplink Thread is Running. Please Retry Later.");
            else
            {
                pthread_join(p_thread[8], NULL);
                pthread_create(&p_thread[8], NULL, ftp_rmdir_onorbit, NULL);
            }
        }
        ImGui::Text(State.FTPWindowBuffer);
        break;
    }
    default : {
        State.ftp_version = 2;
        State.ftp_task = FTP_UPLOAD_REQUEST;
        break;
    }
    }

    ImGui::EndChild();
    ImGui::SetWindowFontScale(1.0);
    ImGui::End();
}

bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int width, int height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;

    return true;
}

void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


int Console::Stricmp(const char* str1, const char* str2)
{
    int d;
    while (d = toupper(*str2) - toupper(*str1) == 0 && *str1)
    {
        str1++;
        str2++;
    }
    return d;
}

int Console::Strnicmp(const char* str1, const char* str2, int n)
{
    int d = 0;
    while(n > 0 && (d = toupper(*str2) - toupper(*str1)==0 && *str1) )
    {
        str1++;
        str2++;
        n--;
    }
    return d;
}
char* Console::Strdup(const char *str)
{
    size_t len = strlen(str) + 1;
    void* buf = malloc(len);
    IM_ASSERT(buf);
    return (char*)memcpy(buf, (const void*)str, len);
}
void Console::Strtrim(char* str)
{
    char* str_end = str + strlen(str);
    while (str_end > str && str_end[-1] == ' ')
        str_end--;
    *str_end = 0;
}

void Console::ClearLog()
{
    for (int i = 0; i < _items.Size; i++)
        free(_items[i]);
    _items.clear();
}

void Console::AddLog(const char* fmt, ...) IM_FMTARGS(2)
{
    char str[1024];
    char buf[1024];
    char tmp[1024];
    char * pos;
    char * bug;
    _now = time(NULL);
    _now_time = *localtime(&_now);

    strftime(tmp, sizeof(tmp), "[%H:%M:%S]  ", &_now_time);
    
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
    buf[IM_ARRAYSIZE(buf) - 1] = 0;
    va_end(args);

    if ((pos = strstr(buf, "##")) != 0)
    {
        strcpy(str, tmp);
        strcat(str, pos + 2);
        strcpy(pos + 2, str);
        //Write to IMGUI Console.
        if ((bug = strstr(buf, "[DEBUG]##")) != 0)
        {
            if(State.Debugmode)
            {
                _debug_fs.write(str, strlen(str));
                _debug_fs.write("\n", 1);
            }
                
        }
        else if ((bug = strstr(buf, "[FTP]##")) != 0)
        {
            _history_fs.write(str, strlen(str));
            _history_fs.write("\n", 1);
        }
        else
        {
            _items.push_back(Strdup(buf));
            _history_fs.write(str, strlen(str));
            _history_fs.write("\n", 1);
        }
    }
    else
    {
        strcpy(str, tmp);
        strcat(str, buf);
        //Write to IMGUI Console.
        _items.push_back(Strdup(str));
        _history_fs.write(str, strlen(str));
        _history_fs.write("\n", 1);
    }

    this->_history_fs.flush();
    this->_debug_fs.flush();
   if (log_ptr) {
        fprintf(log_ptr, "%s\n", str);
        fflush(log_ptr);
    }
    
}

void Console::DelStartingCharLog(const char* fmt)
{
    char buf[1024];
    strcpy(buf, fmt);
    for (int i = 0; i < _items.Size; i++)
    {
        char* item = _items[i];
        if(strstr(item, buf))
        {
            _items.erase(&_items[i]);
        }
    }
}

void Console::DelPrefixLog(const char* fmt)
{
    char buf[1024];
    strcpy(buf, fmt);
    for (int i = 0; i < _items.Size; i++)
    {
        char * item = _items[i];
        if (strstr(item, buf))
        {
            _items.insert(&_items[i], item + 1);
            _items.erase(&_items[i + 1]);
        }
    }
}

void Console::Draw(const char* title, bool* p_open, float fontscale)
{



    ImGui::SetNextWindowPos(ImVec2(this -> _x_pos, this -> _y_pos), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(this -> _width, this -> _height), ImGuiCond_Always);

    if (!ImGui::Begin(title, p_open, this -> _window_flags))
    {
        ImGui::End();
        return;
    }
    ImGui::SetWindowFontScale(fontscale);

    bool copy_to_clipboard = false;

    const float footer_height_to_reserve = 0.0;





    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
    //ImGui::BeginChild("scrolling");
    if(ImGui::BeginPopupContextWindow())
    {
        if (ImGui::Selectable("Clear"))
            ClearLog();
        copy_to_clipboard = ImGui::Selectable("Copy");
        ImGui::EndPopup();
    }
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,1));
    if (copy_to_clipboard)
        ImGui::LogToClipboard();
    for(int i = 0; i < _items.Size; i++)
    {
        char*item = _items[i];

        bool pop_color = false;
        if(strstr(item, "[ERROR]##"))
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.38f, 0.36f, 1.0f));
            pop_color = true;
            item += 9;
        }
        else if (strstr(item, "[OK]##"))
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.05f, 0.05f, 0.07f, 1.0f));
            pop_color = true;
            item += 6;
        }
        else if (strncmp(item, "# ", 2) == 0)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f));
            pop_color = true;
        }
        else if (strstr(item, "$"))
        {
            item += 1;
        }
        ImGui::SetWindowFontScale(1.1);
        ImGui::TextUnformatted(item);
        if (pop_color)
        {
            ImGui::PopStyleColor();
        }
        ImGui::SetWindowFontScale(1.0);
    }

    if (copy_to_clipboard)
        ImGui::LogFinish();

    if (_push_to_bottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::SetWindowFontScale(1.0);

    ImGui::End();

}

void Console::ChangeWindowSize(float x_pos, float y_pos, float width, float height)
{
    this -> _x_pos = x_pos;
    this -> _y_pos = y_pos;
    this -> _width = width;
    this -> _height = height;
}
// void ImGuiCustomStyle(ImGuiStyle* style)
// {
//     style->WindowPadding     = ImVec2(12, 12);
//     style->WindowRounding    = 6.0f;
//     style->FramePadding      = ImVec2(6, 4);
//     style->FrameRounding     = 4.0f;
//     style->ItemSpacing       = ImVec2(10, 6);
//     style->ItemInnerSpacing  = ImVec2(6, 4);
//     style->IndentSpacing     = 20.0f;
//     style->ScrollbarSize     = 14.0f;
//     style->ScrollbarRounding = 6.0f;
//     style->GrabMinSize       = 8.0f;
//     style->GrabRounding      = 4.0f;

//     ImVec4* c = style->Colors;

//     // === Base pastel palette (low saturation, soft colors) ===
//     ImVec4 bg_main     = ImVec4(0.84f, 0.84f, 0.95f, 1.00f); // soft lavender-gray
//     ImVec4 window_bg   = ImVec4(0.90f, 0.90f, 0.96f, 0.92f);
//     ImVec4 frame_bg    = ImVec4(0.82f, 0.82f, 0.92f, 0.95f);
//     ImVec4 frame_hover = ImVec4(0.76f, 0.76f, 0.88f, 0.98f);
//     ImVec4 frame_active= ImVec4(0.70f, 0.70f, 0.86f, 1.00f);

//     ImVec4 accent_mint = ImVec4(0.70f, 0.88f, 0.88f, 1.00f);
//     ImVec4 accent_blue = ImVec4(0.60f, 0.78f, 0.96f, 1.00f);
//     ImVec4 accent_lav  = ImVec4(0.75f, 0.68f, 0.90f, 1.00f);

//     ImVec4 title_soft  = ImVec4(0.78f, 0.76f, 0.92f, 1.00f);
//     ImVec4 title_active= ImVec4(0.70f, 0.85f, 0.95f, 1.00f);

//     ImVec4 text_main   = ImVec4(0.15f, 0.13f, 0.25f, 1.00f);  // dark lavender-gray
//     ImVec4 text_muted  = ImVec4(0.45f, 0.43f, 0.55f, 1.00f);

//     ImVec4 border      = ImVec4(0.62f, 0.60f, 0.75f, 1.00f);

//     // === Text ===
//     c[ImGuiCol_Text]         = text_main;
//     c[ImGuiCol_TextDisabled] = text_muted;

//     // === Windows ===
//     c[ImGuiCol_WindowBg]     = window_bg;
//     c[ImGuiCol_ChildBg]      = ImVec4(0,0,0,0);
//     c[ImGuiCol_PopupBg]      = window_bg;

//     c[ImGuiCol_Border]       = border;
//     c[ImGuiCol_BorderShadow] = ImVec4(0,0,0,0);

//     // === Frame ===
//     c[ImGuiCol_FrameBg]        = frame_bg;
//     c[ImGuiCol_FrameBgHovered] = frame_hover;
//     c[ImGuiCol_FrameBgActive]  = frame_active;

//     // === Title bars ===
//     c[ImGuiCol_TitleBg]          = title_soft;
//     c[ImGuiCol_TitleBgActive]    = title_active;
//     c[ImGuiCol_TitleBgCollapsed] = ImVec4(title_soft.x, title_soft.y, title_soft.z, 0.6f);

//     // === Tabs ===
//     c[ImGuiCol_Tab]              = title_soft;
//     c[ImGuiCol_TabHovered]       = accent_blue;
//     c[ImGuiCol_TabActive]        = accent_lav;
//     c[ImGuiCol_TabUnfocused]     = title_soft;
//     c[ImGuiCol_TabUnfocusedActive] = accent_blue;

//     // === Buttons ===
//     c[ImGuiCol_Button]           = accent_lav;
//     c[ImGuiCol_ButtonHovered]    = accent_blue;
//     c[ImGuiCol_ButtonActive]     = accent_mint;

//     // === Scroll ===
//     c[ImGuiCol_ScrollbarBg]      = ImVec4(0,0,0,0);
//     c[ImGuiCol_ScrollbarGrab]    = accent_lav;
//     c[ImGuiCol_ScrollbarGrabHovered] = accent_blue;
//     c[ImGuiCol_ScrollbarGrabActive]  = accent_mint;

//     // === Header ===
//     c[ImGuiCol_Header]           = accent_lav;
//     c[ImGuiCol_HeaderHovered]    = accent_blue;
//     c[ImGuiCol_HeaderActive]     = accent_mint;

//     style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.70f, 0.70f, 0.92f, 0.55f);

//     // === Optional: mild transparency on windows (soft look) ===
//     style->Colors[ImGuiCol_WindowBg].w = 0.90f;
// }

void ImGuiCustomStyle(ImGuiStyle* style)
{
    style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

    //Window Background
    ImVec4 Colorset_1 = ImVec4(0.141f, 0.364f, 0.376f, 1.00f);
    //Disabled
    ImVec4 Colorset_2 = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    //Active Title
    ImVec4 Colorset_3 = ImVec4(0.235f, 0.603f, 0.623f, 1.00f);
    //General Title, Button
    ImVec4 Colorset_4 = ImVec4(0.180f, 0.462f, 0.478f, 1.00f);
    //ScrollBar
    ImVec4 Colorset_5 = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    //Active Button
    ImVec4 Colorset_6 = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
 
	style->Colors[ImGuiCol_Text] = ImVec4(0.972f, 0.733f, 0.701f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = Colorset_2;
	style->Colors[ImGuiCol_WindowBg] = Colorset_1;
	style->Colors[ImGuiCol_PopupBg] = Colorset_1;
	style->Colors[ImGuiCol_Border] = ImVec4(0.972f, 0.733f, 0.701f, 1.00f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = Colorset_4;
	style->Colors[ImGuiCol_FrameBgHovered] = Colorset_2;
	style->Colors[ImGuiCol_FrameBgActive] = Colorset_6;
	style->Colors[ImGuiCol_TitleBg] = Colorset_4;
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style->Colors[ImGuiCol_TitleBgActive] = Colorset_3;
	style->Colors[ImGuiCol_MenuBarBg] = Colorset_4;
	style->Colors[ImGuiCol_ScrollbarBg] = Colorset_1;
	style->Colors[ImGuiCol_ScrollbarGrab] = Colorset_5 ;
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = Colorset_6;
	style->Colors[ImGuiCol_ScrollbarGrabActive] = Colorset_5;
	style->Colors[ImGuiCol_CheckMark] = Colorset_5 ;
	style->Colors[ImGuiCol_SliderGrab] = Colorset_5 ;
	style->Colors[ImGuiCol_SliderGrabActive] = Colorset_1;
	style->Colors[ImGuiCol_Button] = Colorset_4;
	style->Colors[ImGuiCol_ButtonHovered] = Colorset_2;
	style->Colors[ImGuiCol_ButtonActive] = Colorset_6;
	style->Colors[ImGuiCol_Header] = Colorset_4;
	style->Colors[ImGuiCol_HeaderHovered] = Colorset_6;
	style->Colors[ImGuiCol_HeaderActive] = Colorset_1;
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = Colorset_6;
	style->Colors[ImGuiCol_ResizeGripActive] = Colorset_1;
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = Colorset_3;
    style->Colors[ImGuiCol_Tab] = Colorset_4;
    style->Colors[ImGuiCol_TabActive] = Colorset_3;
}


bool popup_setup(Setup * setup)
{
    ImGui::OpenPopup("MIMAN Setup Page");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(720, 540), ImGuiCond_Appearing);

    if ( ImGui::BeginPopupModal("MIMAN Setup Page", NULL, ImGuiWindowFlags_AlwaysAutoResize) )
    {
        ImGui::SetWindowFontScale(0.8f);
        ImGui::Text("Transceiver");
        ImGui::BeginChild("##setup_transceiver", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 13.5), true, mim_winflags);
        ImGui::Text("Device Name");
        ImGui::InputText("##GS100_name", &setup->Transciever_devname[0], sizeof(setup->Transciever_devname));
        ImGui::Text("Default Frequency(Hz)");
        ImGui::InputDouble("##GS100_freq", &setup->default_freq);
        ImGui::Text("Baudrate");
        ImGui::InputScalar("##GS100_baud", ImGuiDataType_U32, &setup->Transceiver_baud, NULL, NULL, "%u");
        ImGui::Text("Delay(ms)");
        ImGui::InputScalar("##GS100_delay", ImGuiDataType_U32, &setup->default_timeout, NULL, NULL, "%u");
        ImGui::EndChild();

        ImGui::Text("Switch Box");
        ImGui::BeginChild("##setup_switch", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 7.5), true, mim_winflags);
        ImGui::Text("Device Name");
        ImGui::InputText("##KTA223_name", &setup->Switch_devname[0], sizeof(setup->Switch_devname));
        ImGui::Text("Baudrate");
        ImGui::InputScalar("##KTA223_baud", ImGuiDataType_U32, &setup->Switch_baud, NULL, NULL, "%u");
        ImGui::EndChild();

        ImGui::Text("Rotator Controller");
        ImGui::BeginChild("##setup_rotator", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 7.5), true, mim_winflags);
        ImGui::Text("Device Name");
        ImGui::InputText("##GS232B_name", &setup->Rotator_devname[0], sizeof(setup->Rotator_devname));
        ImGui::Text("Baudrate");
        ImGui::InputScalar("##GS232B_baud", ImGuiDataType_U32, &setup->Rotator_baud, NULL, NULL, "%u");
        ImGui::EndChild();

        ImGui::Text("Route Table");
        ImGui::BeginChild("##setup_node", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 8.0), true, mim_winflags);
        ImGui::Text("GS100 node");
        ImGui::SameLine();
        ImGui::InputScalar("##gs100_node", ImGuiDataType_U8, &setup->gs100_node, NULL, NULL, "%u");
        ImGui::Text("GS PC node");
        ImGui::SameLine();
        ImGui::InputScalar("##gspc_node", ImGuiDataType_U8, &setup->kiss_node, NULL, NULL, "%u");
        ImGui::Text("AX100 node");
        ImGui::SameLine();
        ImGui::InputScalar("##ax100_node", ImGuiDataType_U8, &setup->ax100_node, NULL, NULL, "%u");
        ImGui::Text("OBC   node");
        ImGui::SameLine();
        ImGui::InputScalar("##obc_node", ImGuiDataType_U8, &setup->obc_node, NULL, NULL, "%u");
        ImGui::EndChild();

        ImGui::Text("Use S-Band GS");
        ImGui::SameLine();
        ImGui::Checkbox("##s_band_use_state_check", &State.SbandUse);
        if(State.SbandUse)
        {
            ImGui::Text("S-Band");
            ImGui::BeginChild("##setup_s_band", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 6.5), true, mim_winflags);
            ImGui::Text("Username");
            ImGui::SameLine();
            ImGui::InputText("##S-BandUsername", &setup->S_Username[0], sizeof(setup->S_Username));
            ImGui::Text("IP Address");
            ImGui::SameLine();
            ImGui::InputText("##S-BandIP", &setup->S_Address[0], sizeof(setup->S_Address));
            ImGui::Text("Password");
            ImGui::SameLine();
            ImGui::InputText("##S-BandPW", &setup->S_passwd[0], sizeof(setup->S_passwd));
            ImGui::EndChild();
        }
        ImGui::SetWindowFontScale(1.0f);
        

        if (ImGui::Button("Start", ImVec2(ImGui::GetContentRegionAvail().x * 0.33, ImGui::GetFontSize() * 1.5)))
        {
            State.GUI = true;
            State.InitializeRequired = true;
            set_serial_spec_micsec(setup->Transceiver_baud, setup->Switch_baud, 1000000, setup->queue_delay, setup->gs100_node, setup->default_timeout, setup->guard_delay);


            ImGui::EndPopup();
            ImGui::CloseCurrentPopup();
            return false;
        }
        ImGui::SameLine();
        if(ImGui::Button("Reset Watchdog", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
        {
            ResetWatchdog();
        }
        ImGui::SameLine();
        if (ImGui::Button("Quit", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            State.GUI = false;
            ImGui::EndPopup();
            ImGui::CloseCurrentPopup();
            return false;
        }
        
        ImGui::EndPopup();
        return true;
    }
}

//Functions
bool popup_param_table0()
{
    ImGui::OpenPopup("GS100 Parameter Table 0");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(720, 720), ImGuiCond_Appearing);
    if ( ImGui::BeginPopupModal("GS100 Parameter Table 0", NULL, ImGuiWindowFlags_AlwaysAutoResize) )
    {
        ImGui::BeginChild("##Ptable0", ImVec2(ImGui::GetContentRegionAvail().x, 650), true, mim_winflags);
        ImGui::PushItemWidth(300);
        ImGui::Text("RSSI Offset        : ");
        ImGui::SameLine();
        ImGui::InputScalar("##rssi_offset", ImGuiDataType_S8, &param0.rssi_offset, NULL, NULL, "%d");
        ImGui::Text("Max Temperature    : ");
        ImGui::SameLine();
        ImGui::InputScalar("°C##max_temp", ImGuiDataType_S16, &param0.max_temp, NULL, NULL, "%d");
        ImGui::Text("bgndrssl_ema       : ");
        ImGui::SameLine();
        ImGui::InputScalar("##bgndrssl_ema", ImGuiDataType_Float, &param0.bgndrssl_ema);
        ImGui::Text("CSP Node           : ");
        ImGui::SameLine();
        ImGui::InputScalar("##csp_node", ImGuiDataType_U8, &param0.csp_node, NULL, NULL, "%u");
        ImGui::Text("I2C                : ");
        ImGui::SameLine();
        ImGui::Checkbox("##i2c_enable", &param0.i2c_en);
        ImGui::Text("CAN                : ");
        ImGui::SameLine();
        ImGui::Checkbox("##can_enable", &param0.can_en);
        ImGui::Text("EXTPTT             : ");
        ImGui::SameLine();
        ImGui::Checkbox("##extptt_enable", &param0.extptt_en);
        ImGui::SameLine();
        ImGui::InputScalar("##max_temp", ImGuiDataType_S8, &param0.max_temp, NULL, NULL, "%d");
        ImGui::Text("I2C Address        : ");
        ImGui::SameLine();
        ImGui::InputScalar("##i2c_addr", ImGuiDataType_U8, &param0.i2c_addr, NULL, NULL, "%u");
        ImGui::Text("I2C Frequency      : ");
        ImGui::SameLine();
        ImGui::InputScalar("kHz##i2c_khz", ImGuiDataType_U16, &param0.i2c_khz, NULL, NULL, "%u");
        ImGui::Text("CAN Frequency      : ");
        ImGui::SameLine();
        ImGui::InputScalar("kHz##can_khz", ImGuiDataType_U16, &param0.can_khz, NULL, NULL, "%u");
        ImGui::Text("Reboot in          : ");
        ImGui::SameLine();
        ImGui::InputScalar("##reboot_in", ImGuiDataType_U16, &param0.reboot_in, NULL, NULL, "%u");
        ImGui::Text("TX Inhibit         : ");
        ImGui::SameLine();
        ImGui::InputScalar("##tx_inhibit", ImGuiDataType_U32, &param0.tx_inhibit, NULL, NULL, "%u");
        ImGui::Text("Store Log          : ");
        ImGui::SameLine();
        ImGui::InputScalar("##log_store", ImGuiDataType_U8, &param0.log_store, NULL, NULL, "%u");
        ImGui::Text("TX Power           : ");
        ImGui::SameLine();
        ImGui::InputScalar("##tx_power", ImGuiDataType_U8, &param0.tx_pwr, NULL, NULL, "%d");
        ImGui::Text("Maximum TX Time    : ");
        ImGui::SameLine();
        ImGui::InputScalar("second##max_tx_time", ImGuiDataType_U16, &param0.max_tx_time, NULL, NULL, "%u");
        ImGui::Text("Maximum IDLE Time  : ");
        ImGui::SameLine();
        ImGui::InputScalar("second##max_idle_time", ImGuiDataType_U16, &param0.max_idle_time, NULL, NULL, "%u");
        ImGui::Text("CSP Routing Table  : ");
        ImGui::SameLine();
        ImGui::InputText("##csp_rtable", param0.csp_rtable, sizeof(param0.csp_rtable));
        ImGui::PopItemWidth();
        ImGui::EndChild();

        if (ImGui::Button("Update", ImVec2(ImGui::GetContentRegionAvail().x * 0.25, 40)))
            pthread_create(&p_thread[0], NULL, &Load_Paramtable0, NULL);
        ImGui::SameLine();
        if (ImGui::Button("Save", ImVec2(ImGui::GetContentRegionAvail().x * 0.33, 40)))
            pthread_create(&p_thread[0], NULL, &Save_Paramtable0, NULL);
        ImGui::SameLine();
        if (ImGui::Button("Save to FRAM", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, 40)))
        {
            int8_t id = 0;
            pthread_create(&p_thread[0], NULL, &Save2FRAM, &id);
            ImGui::EndPopup();
            ImGui::CloseCurrentPopup();
            return false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(ImGui::GetContentRegionAvail().x, 40)))
        {
            ImGui::EndPopup();
            ImGui::CloseCurrentPopup();
            return false;
        }
        
        ImGui::EndPopup();
        return true;
    }
}

bool popup_param_table1()
{
    ImGui::OpenPopup("GS100 Parameter Table 1");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(720, 540), ImGuiCond_Appearing);
    if ( ImGui::BeginPopupModal("GS100 Parameter Table 1", NULL, ImGuiWindowFlags_AlwaysAutoResize) )
    {
        ImGui::BeginChild("##Ptable1", ImVec2(ImGui::GetContentRegionAvail().x, 650), true, mim_winflags);
        ImGui::PushItemWidth(300);
        ImGui::Text("Rx Frequency       : ");
        ImGui::SameLine();
        ImGui::InputScalar("##freq", ImGuiDataType_U32, &param1.freq, NULL, NULL, "%u");
        ImGui::Text("BaudRate           : ");
        ImGui::SameLine();
        ImGui::InputScalar("##baud", ImGuiDataType_U32, &param1.baud, NULL, NULL, "%u");
        ImGui::Text("modindex           : ");
        ImGui::SameLine();
        ImGui::InputScalar("##modeindex", ImGuiDataType_Float, &param1.modindex, NULL, NULL, "%f");
        ImGui::Text("Guard              : ");
        ImGui::SameLine();
        ImGui::InputScalar("##guard", ImGuiDataType_U16, &param1.guard, NULL, NULL, "%u");
        ImGui::Text("pllrang            : ");
        ImGui::SameLine();
        ImGui::InputScalar("##pllrang", ImGuiDataType_U8, &param1.pllrang, NULL, NULL, "%u");
        ImGui::Text("mode               : ");
        ImGui::SameLine();
        ImGui::InputScalar("##mode", ImGuiDataType_U8, &param1.mode, NULL, NULL, "%u");
        ImGui::Text("csp_hmac           : ");
        ImGui::SameLine();
        ImGui::Checkbox("##csp_hmac", &param1.csp_hmac);
        ImGui::Text("csp_rs             : ");
        ImGui::SameLine();
        ImGui::Checkbox("##csp_rs", &param1.csp_rs);
        ImGui::Text("csp_crc            : ");
        ImGui::SameLine();
        ImGui::Checkbox("##csp_crc", &param1.csp_crc);
        ImGui::Text("csp_rand           : ");
        ImGui::SameLine();
        ImGui::Checkbox("##csp_rand", &param1.csp_rand);
        ImGui::Text("AX25 Call          : ");
        ImGui::SameLine();
        ImGui::InputText("##ax25_call", param1.ax25_call, sizeof(param1.ax25_call));
        ImGui::Text("Bandwidth          : ");
        ImGui::SameLine();
        ImGui::InputScalar("##bw", ImGuiDataType_U32, &param1.bw, NULL, NULL, "%u");
        ImGui::Text("AFC Range          : ");
        ImGui::SameLine();
        ImGui::InputScalar("##afcrange", ImGuiDataType_S32, &param1.afcrange, NULL, NULL, "%d");
        ImGui::PopItemWidth();
        ImGui::EndChild();
        

        if (ImGui::Button("Update", ImVec2(ImGui::GetContentRegionAvail().x * 0.25, 40)))
            pthread_create(&p_thread[0], NULL, &Load_Paramtable1, NULL);
        ImGui::SameLine();
        if (ImGui::Button("Save", ImVec2(ImGui::GetContentRegionAvail().x * 0.33, 40)))
            pthread_create(&p_thread[0], NULL, &Save_Paramtable1, NULL);
        ImGui::SameLine();
        if (ImGui::Button("Save to FRAM", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, 40)))
        {
            int8_t id = 1;
            pthread_create(&p_thread[0], NULL, &Save2FRAM, &id);
            ImGui::EndPopup();
            ImGui::CloseCurrentPopup();
            return false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(ImGui::GetContentRegionAvail().x, 40)))
        {
            ImGui::EndPopup();
            ImGui::CloseCurrentPopup();
            return false;
        }
        ImGui::EndPopup();
        return true;
    }
}

bool popup_param_table5()
{
    ImGui::OpenPopup("GS100 Parameter Table 5");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(720, 540), ImGuiCond_Appearing);
    if ( ImGui::BeginPopupModal("GS100 Parameter Table 5", NULL, ImGuiWindowFlags_AlwaysAutoResize) )
    {
        ImGui::BeginChild("##Ptable5", ImVec2(ImGui::GetContentRegionAvail().x, 650), true, mim_winflags);
        ImGui::PushItemWidth(300);
        ImGui::Text("Tx Frequency       : ");
        ImGui::SameLine();
        ImGui::InputScalar("##freq", ImGuiDataType_U32, &param5.freq, NULL, NULL, "%u");
        ImGui::Text("BaudRate           : ");
        ImGui::SameLine();
        ImGui::InputScalar("##baud", ImGuiDataType_U32, &param5.baud, NULL, NULL, "%u");
        ImGui::Text("modindex           : ");
        ImGui::SameLine();
        ImGui::InputScalar("##modeindex", ImGuiDataType_Float, &param5.modindex, NULL, NULL, "%f");
        ImGui::Text("Guard              : ");
        ImGui::SameLine();
        ImGui::InputScalar("##guard", ImGuiDataType_U16, &param5.guard, NULL, NULL, "%u");
        ImGui::Text("pllrang            : ");
        ImGui::SameLine();
        ImGui::InputScalar("##pllrang", ImGuiDataType_U8, &param5.pllrang, NULL, NULL, "%u");
        ImGui::Text("mode               : ");
        ImGui::SameLine();
        ImGui::InputScalar("##mode", ImGuiDataType_U8, &param5.mode, NULL, NULL, "%u");
        ImGui::Text("csp_hmac           : ");
        ImGui::SameLine();
        ImGui::Checkbox("##csp_hmac", &param5.csp_hmac);
        ImGui::Text("csp_rs             : ");
        ImGui::SameLine();
        ImGui::Checkbox("##csp_rs", &param5.csp_rs);
        ImGui::Text("csp_crc            : ");
        ImGui::SameLine();
        ImGui::Checkbox("##csp_crc", &param5.csp_crc);
        ImGui::Text("csp_rand           : ");
        ImGui::SameLine();
        ImGui::Checkbox("##csp_rand", &param5.csp_rand);
        ImGui::Text("AX25 Call          : ");
        ImGui::SameLine();
        ImGui::InputText("##ax25_call", param5.ax25_call, sizeof(param5.ax25_call));
        ImGui::Text("preamblen          : ");
        ImGui::SameLine();
        ImGui::InputScalar("##preamblen", ImGuiDataType_U8, &param5.preamblen, NULL, NULL, "%u");
        ImGui::Text("preambflags        : ");
        ImGui::SameLine();
        ImGui::InputScalar("##preambflags", ImGuiDataType_U8, &param5.preambflags, NULL, NULL, "%u");
        ImGui::Text("intfrmlen          : ");
        ImGui::SameLine();
        ImGui::InputScalar("##intfrmlen", ImGuiDataType_U8, &param5.intfrmlen, NULL, NULL, "%u");
        ImGui::Text("intfrmflags        : ");
        ImGui::SameLine();
        ImGui::InputScalar("##intfrmflags", ImGuiDataType_U8, &param5.intfrmflags, NULL, NULL, "%u");
        ImGui::Text("RSSI busy          : ");
        ImGui::SameLine();
        ImGui::InputScalar("##rssibusy", ImGuiDataType_S16, &param5.rssibusy, NULL, NULL, "%d");
        ImGui::Text("kup delay          : ");
        ImGui::SameLine();
        ImGui::InputScalar("##kup_delay", ImGuiDataType_U8, &param5.kup_delay, NULL, NULL, "%u");
        ImGui::Text("PA Level           : ");
        ImGui::SameLine();
        ImGui::InputScalar("##pa_level", ImGuiDataType_S16, &param5.pa_level, NULL, NULL, "%d");
        ImGui::PopItemWidth();
        ImGui::EndChild();

        if (ImGui::Button("Update", ImVec2(ImGui::GetContentRegionAvail().x * 0.25, 40)))
            pthread_create(&p_thread[0], NULL, &Load_Paramtable5, NULL);
        ImGui::SameLine();
        if (ImGui::Button("Save", ImVec2(ImGui::GetContentRegionAvail().x * 0.33, 40)))
            pthread_create(&p_thread[0], NULL, &Save_Paramtable5, NULL);
        ImGui::SameLine();
        if (ImGui::Button("Save to FRAM", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, 40)))
        {
            int8_t id = 5;
            pthread_create(&p_thread[0], NULL, &Save2FRAM, &id);
            ImGui::EndPopup();
            ImGui::CloseCurrentPopup();
            return false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(ImGui::GetContentRegionAvail().x, 40)))
        {
            ImGui::EndPopup();
            ImGui::CloseCurrentPopup();
            return false;
        }
        ImGui::EndPopup();
        return true;
    }
}

bool popup_load()
{
    ImGui::OpenPopup("Load");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 200), ImGuiCond_Appearing);
    if ( ImGui::BeginPopupModal("Load", NULL, ImGuiWindowFlags_AlwaysAutoResize) )
    {
        ImGui::Text("Progress : %d%c", State.loadindex * 100 / State.tleallindex, '%');
        ImGui::EndPopup();
    }
    return true;
}

//For Temporary multiple tle operation
char localtleurl[256] = "/home/miman/Downloads/NORAD_TLE.txt";

bool popup_tle()
{
    ImGui::OpenPopup("TLE Manager");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(1080, 720), ImGuiCond_Appearing);
    if ( ImGui::BeginPopupModal("TLE Manager", NULL, ImGuiWindowFlags_AlwaysAutoResize) )
    {
        if (ImGui::BeginCombo("##TLECombobox", State.tleinfolistup[NowTLE]->label))
        {
            for (int TLEindex = 0; TLEindex < IM_ARRAYSIZE(State.ftplistup); TLEindex++)
            {
                if (!State.tleinfolistup[TLEindex])
                    break;
                bool SelectedTLE = (NowTLE == TLEindex);
                char Selectedlabel[64];
                strcpy(Selectedlabel, State.tleinfolistup[TLEindex]->label);
                if (ImGui::Selectable(Selectedlabel, SelectedTLE))
                {
                    NowTLE = TLEindex;
                }
                if (SelectedTLE)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        ImGui::InputText("##", SearchBuf, sizeof(SearchBuf));
        ImGui::SameLine();
        ImGui::Text("SGP4");
        ImGui::SameLine();
        ImGui::Checkbox("##sgp4checkbox", &sgp4check);

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.6);
        ImGui::InputText("##localtleimport", localtleurl, sizeof(localtleurl), 0, NULL, NULL);
        ImGui::SameLine();
        if(ImGui::Button("Load from local", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            
            State.NowTracking = false;
            memset(State.tleinfolistup[NowTLE]->local, 0, sizeof(State.tleinfolistup[NowTLE]->local));
            memcpy(State.tleinfolistup[NowTLE]->local, localtleurl, sizeof(localtleurl));
            
            console.AddLog("[OK]##TLE \"%s\" Download Completed.", State.tleinfolistup[NowTLE]->label);

            pthread_create(&p_thread[15], NULL, &SatelliteInitialize,  &sgp4check);
            // SatelliteInitialize(NULL);
                
            
        }
        
        if (ImGui::BeginTable("##TLESatelliteListup", 4, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Sortable, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.9)))
        {
            ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
            ImGui::TableSetupColumn("Name##tle",                ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide, 0.0f);
            ImGui::TableSetupColumn("Use##tle",                 ImGuiTableColumnFlags_NoHide, 0.0f);
            ImGui::TableSetupColumn("Next AOS##tle",            ImGuiTableColumnFlags_NoHide, 0.0f);
            ImGui::TableSetupColumn("Max Elevation##tle",       ImGuiTableColumnFlags_NoHide, 0.0f);
            ImGui::TableHeadersRow();
            tlepopupindex = 0;

            // if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
            // {
            //     if (sorts_specs->SpecsDirty)
            //     {
            //         // If the sorting specs have changed, update the sorting state
            //         if (items.Size > 1)
            //             qsort(&items[0], (size_t)items.Size, sizeof(items[0]), MyItem::CompareWithSortSpecs);
            //         MyItem::s_current_sort_specs = NULL;
            //         sorts_specs->SpecsDirty = false;
            //         sort_column = sorts_specs->Specs[0].ColumnIndex;
            //         sort_ascending = sorts_specs->Specs[0].SortDirection == ImGuiSortDirection_Ascending;
            //     }
            // }

            while(1)
            {
                    
                if(strlen(State.Satellites[tlepopupindex]->Name())== NULL)
                    break;
                else
                {
                    if(State.Satellites[tlepopupindex]->_max_elevation[0] < 0 && sgp4check)
                    {
                        tlepopupindex += 1;
                        continue;
                    }
                    if(strlen(SearchBuf) != 0 && strstr(State.Satellites[tlepopupindex]->Name(), SearchBuf) == NULL)
                    {
                        tlepopupindex += 1;
                        continue;
                    }
                        
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", State.Satellites[tlepopupindex]->Name());
                    ImGui::TableSetColumnIndex(1);
                    if(State.Satellites[tlepopupindex]->use == false)
                    {
                        sprintf(SelectButtonTextBuf, "Select##TLE%d", tlepopupindex);
                        if(ImGui::Button(SelectButtonTextBuf, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
                        {
                            State.Satellites[tlepopupindex]->use = true;
                            if(State.Satellites[tlepopupindex]->cal == false)
                                State.Satellites[tlepopupindex]->Refresh(State.Satellites[tlepopupindex]->tle, State.Satellites[tlepopupindex]->obs, true, true);
                            SatelliteModelInitialize(tlepopupindex);
                        }
                    }
                    else
                    {
                        sprintf(DeleteButtonTextBuf, "Delete##TLE%d", tlepopupindex);
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.650f, 0.266f, 0.322f, 1.0f));
                        if(ImGui::Button(DeleteButtonTextBuf, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
                        {
                            State.Satellites[tlepopupindex]->use = false;
                            SatelliteModelDelete(tlepopupindex);
                        }
                        ImGui::PopStyleColor();
                    }
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d/%d %d:%d:%d",   State.Satellites[tlepopupindex]->_nextaos[0].AddHours(9).Month(), 
                                                    State.Satellites[tlepopupindex]->_nextaos[0].AddHours(9).Day(),
                                                    State.Satellites[tlepopupindex]->_nextaos[0].AddHours(9).Hour(),
                                                    State.Satellites[tlepopupindex]->_nextaos[0].AddHours(9).Minute(),
                                                    State.Satellites[tlepopupindex]->_nextaos[0].AddHours(9).Second());
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%.3lf°", State.Satellites[tlepopupindex]->_max_elevation[0] * RAD_TO_DEG);
                }
                tlepopupindex += 1;
            }

            
            ImGui::EndTable();
        }
        if(ImGui::Button("Load TLE", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetContentRegionAvail().y)))
        {
            
            if(DownloadTLE(State.tleinfolistup[NowTLE]->remote, State.tleinfolistup[NowTLE]->local))
            {
                State.NowTracking = false;
                
                console.AddLog("[OK]##TLE \"%s\" Download Completed.", State.tleinfolistup[NowTLE]->label);

                pthread_create(&p_thread[15], NULL, &SatelliteInitialize,  &sgp4check);
                // SatelliteInitialize(NULL);
                
            }
            else
            {
                console.AddLog("[ERROR]##TLE Download failed. Please check again.");
            }
            
        }
        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y)))
        {
            ImGui::EndPopup();
            ImGui::CloseCurrentPopup();
            return false;
        }
        ImGui::EndPopup();
        return true;
    }
}




void Initialize_CMDLabels()
{   // Common type cmd
    snprintf(Templabels[0], 64, "no arg");
    snprintf(Templabels[1], 64, "u8");
    snprintf(Templabels[2], 64, "u16");
    snprintf(Templabels[3], 64, "u32");
    snprintf(Templabels[4], 64, "u64");
    snprintf(Templabels[5], 64, "s8");
    snprintf(Templabels[6], 64, "s16");
    snprintf(Templabels[7], 64, "s32");
    snprintf(Templabels[8], 64, "s64");
    snprintf(Templabels[9], 64, "float");

    snprintf(Templabels[10], 64, "EPS NoOp");
    snprintf(Templabels[11], 64, "EPS Reset Counters");
    snprintf(Templabels[12], 64, "EPS Get Counters");
    snprintf(Templabels[13], 64, "EPS Get App Data");
    snprintf(Templabels[14], 64, "EPS Report App Data");
    snprintf(Templabels[15], 64, "EPS_P60_Dock Set Channel Single");
    snprintf(Templabels[16], 64, "EPS_P60_Dock Get Channel Single");
    snprintf(Templabels[17], 64, "EPS_P60_Dock Set Channels");
    snprintf(Templabels[18], 64, "EPS_P60_Dock Get Channels");

    snprintf(Templabels[19], 64, "EPS_P60_PDU Set Channel Single");
    snprintf(Templabels[20], 64, "EPS_P60_PDU Get Channel Single");
    snprintf(Templabels[21], 64, "EPS_P60_PDU Set Channels");
    snprintf(Templabels[22], 64, "EPS_P60_PDU Get Channels");

    snprintf(Templabels[23], 64, "EPS_P60_ACU Set MPPT Mode");
    snprintf(Templabels[24], 64, "EPS_P60_ACU Get MPPT Mode");

    snprintf(Templabels[25], 64, "EPS_P60_Dock Get Table HK");
    snprintf(Templabels[26], 64, "EPS_P60_Dock Get Table Conf");
    snprintf(Templabels[27], 64, "EPS_P60_Dock Get Table Cal");

    snprintf(Templabels[28], 64, "EPS_P60_PDU Get Table HK");
    snprintf(Templabels[29], 64, "EPS_P60_PDU Get Table Conf");
    snprintf(Templabels[30], 64, "EPS_P60_PDU Get Table Cal");

    snprintf(Templabels[31], 64, "EPS_P60_ACU Get Table HK");
    snprintf(Templabels[32], 64, "EPS_P60_ACU Get Table Conf");
    snprintf(Templabels[33], 64, "EPS_P60_ACU Get Table Cal");

    snprintf(Templabels[34], 64, "EPS_P60_Dock Reset GND WDT");
    snprintf(Templabels[35], 64, "EPS_P60_PDU Reset GND WDT");
    snprintf(Templabels[36], 64, "EPS_P60_ACU Reset GND WDT");

    snprintf(Templabels[37], 64, "EPS_P60 Get Param");
    snprintf(Templabels[38], 64, "EPS_P60 Get Param Array");
    snprintf(Templabels[39], 64, "EPS_P60 Set Param");

    snprintf(Templabels[40], 64, "EPS_P60 Get Table");
    snprintf(Templabels[41], 64, "EPS_P60 Load Table");
    snprintf(Templabels[42], 64, "EPS_P60 Save Table");
    snprintf(Templabels[43], 64, "EPS_P60 Table Get Static");
    snprintf(Templabels[44], 64, "EPS_P60 Table Dump Static");

    snprintf(Templabels[45], 64, "EPS_P60 Get Dock Info");
    snprintf(Templabels[46], 64, "EPS_P60 Reset");


    // ADCS 

    snprintf(Templabels[47],  64, "ADCS NoOp");
    snprintf(Templabels[48],  64, "ADCS Reset Counters");
    snprintf(Templabels[49],  64, "ADCS Reset App Cmd Counters");
    snprintf(Templabels[50],  64, "ADCS Reset Device Cmd Counters");
    snprintf(Templabels[51],  64, "ADCS Set Communication Mode (CAN)");

    snprintf(Templabels[52],  64, "ADCS GPIO Enable High");
    snprintf(Templabels[53],  64, "ADCS GPIO Enable Low");
    snprintf(Templabels[54],  64, "ADCS GPIO Boot High");
    snprintf(Templabels[55],  64, "ADCS GPIO Boot Low");
    snprintf(Templabels[56],  64, "ADCS Exit Bootloader");

    snprintf(Templabels[57],  64, "ADCS Reset");
    snprintf(Templabels[58],  64, "ADCS Set Current Unix Time");
    snprintf(Templabels[59],  64, "ADCS Set Error Log Setting");
    snprintf(Templabels[60],  64, "ADCS Persist Config");

    snprintf(Templabels[61],  64, "ADCS Set Control & Estimation Mode");
    snprintf(Templabels[62],  64, "ADCS Disable Mag/RWL Maintenance");
    snprintf(Templabels[63],  64, "ADCS Set Reference IRC Vector");
    snprintf(Templabels[64],  64, "ADCS Set Reference LLH Target");
    snprintf(Templabels[65],  64, "ADCS Set Orbit Mode");
    snprintf(Templabels[66],  64, "ADCS Set Mag Deploy");
    snprintf(Templabels[67],  64, "ADCS Set Reference RPY");
    snprintf(Templabels[68],  64, "ADCS Set Open-Loop MTQ Command");
    snprintf(Templabels[69],  64, "ADCS Set Power State");
    snprintf(Templabels[70],  64, "ADCS Set Run Mode");
    snprintf(Templabels[71],  64, "ADCS Set Control Mode");
    snprintf(Templabels[72],  64, "ADCS Set Wheel Config");
    snprintf(Templabels[73],  64, "ADCS Set Satellite Config");
    snprintf(Templabels[74],  64, "ADCS Set Controller Config");
    snprintf(Templabels[75],  64, "ADCS Set MAG0 MMT Calib Config");
    snprintf(Templabels[76],  64, "ADCS Set Default Mode Config");
    snprintf(Templabels[77],  64, "ADCS Set Mounting Config");
    snprintf(Templabels[78],  64, "ADCS Set MAG1 MMT Calib Config");
    snprintf(Templabels[79],  64, "ADCS Set Estimator Config");
    snprintf(Templabels[80],  64, "ADCS Set Sat Orbit Param Config");
    snprintf(Templabels[81],  64, "ADCS Set Node Selection Config");
    snprintf(Templabels[82],  64, "ADCS Set MTQ Config");
    snprintf(Templabels[83],  64, "ADCS Set Estimation Mode");
    snprintf(Templabels[84],  64, "ADCS Set Operational State");
    snprintf(Templabels[85],  64, "ADCS Set Mag Sensing Element Config");
    snprintf(Templabels[86],  64, "ADCS Set Unsolicited TLM Msg Setup");
    snprintf(Templabels[87],  64, "ADCS Set Unsolicited Event Msg Setup");
    // Telemetry GET commands
    snprintf(Templabels[88],  64, "ADCS CC 101");



    // Sequences
    snprintf(Templabels[126], 64, "ADCS Sequence: Detumbling");
    snprintf(Templabels[127], 64, "ADCS Sequence: Sun Pointing");
    snprintf(Templabels[128], 64, "ADCS Sequence: Velocity Pointing");
    snprintf(Templabels[129], 64, "ADCS Sequence: KSC Earth Pointing");
    snprintf(Templabels[130], 64, "ADCS Sequence: LGC Earth Pointing");
    snprintf(Templabels[131], 64, "ADCS Sequence: RPY Pointing");

    // Extra
    snprintf(Templabels[132], 64, "ADCS Error Log Clear");
    snprintf(Templabels[133], 64, "ADCS Get Current Unix Time (Internal)");

    snprintf(Templabels[134], 64, "ADCS Send HK");
    snprintf(Templabels[135], 64, "ADCS Send Beacon");


    // SC
    snprintf(Templabels[136], 64, "SC NoOp");
    snprintf(Templabels[137], 64, "SC Reset Counters");
    snprintf(Templabels[138], 64, "SC Start ATS");
    snprintf(Templabels[139], 64, "SC Stop ATS");
    snprintf(Templabels[140], 64, "SC Start RTS");
    snprintf(Templabels[141], 64, "SC Stop RTS");
    snprintf(Templabels[142], 64, "SC Disable RTS");
    snprintf(Templabels[143], 64, "SC Enable RTS");
    snprintf(Templabels[144], 64, "SC Switch ATS");
    snprintf(Templabels[145], 64, "SC Jump ATS Time");
    snprintf(Templabels[146], 64, "SC Set Continue-ATS-On-Failure");
    snprintf(Templabels[147], 64, "SC Append ATS");
    snprintf(Templabels[148], 64, "SC Manage Table");
    snprintf(Templabels[149], 64, "SC Start RTS Group");
    snprintf(Templabels[150], 64, "SC Stop RTS Group");
    snprintf(Templabels[151], 64, "SC Disable RTS Group");
    snprintf(Templabels[152], 64, "SC Enable RTS Group");

    // EO
    snprintf(Templabels[153], 64, "TO LAB NoOp");
    snprintf(Templabels[154], 64, "TO LAB Reset Status");
    snprintf(Templabels[155], 64, "TO LAB Add Packet(Enable Beacon RN)");
    snprintf(Templabels[156], 64, "TO LAB Send Data Types");
    snprintf(Templabels[157], 64, "TO LAB Remove Packet(Stop Beacon)");
    snprintf(Templabels[158], 64, "TO LAB Remove All Packet");    
    snprintf(Templabels[159], 64, "TO LAB Output Enable");

    snprintf(Templabels[160], 64, "UANT Get Status");
    snprintf(Templabels[161], 64, "UANT Burn Channel Command");
    snprintf(Templabels[162], 64, "UANT Set Settings Command");
    snprintf(Templabels[163], 64, "UANT Auto Deploy Command");

    snprintf(Templabels[164], 64, "Deploy Solar Panel 1");
    snprintf(Templabels[165], 64, "Deploy Solar Panel 2");
    snprintf(Templabels[166], 64, "Deploy Solar Panel 3");
    snprintf(Templabels[167], 64, "Deploy Solar Panel 4");   

    // KISSCAM
    snprintf(Templabels[168], 64, "PAYUZUC NoOp");
    snprintf(Templabels[169], 64, "PAYUZUC Reset Counters");
    snprintf(Templabels[170], 64, "PAYUZUC Ping");
    snprintf(Templabels[171], 64, "PAYUZUC Set Mode");
    snprintf(Templabels[172], 64, "PAYUZUC Memory Status");
    snprintf(Templabels[173], 64, "PAYUZUC Set Exposure");
    snprintf(Templabels[174], 64, "PAYUZUC Capture");
    snprintf(Templabels[175], 64, "PAYUZUC Download");
    snprintf(Templabels[176], 64, "PAYUZUC Read Register");
    snprintf(Templabels[177], 64, "PAYUZUC Write Register");
    snprintf(Templabels[178], 64, "PAYUZUC Download All");
    snprintf(Templabels[179], 64, "PAYUZUC Mosaic");
    snprintf(Templabels[180], 64, "PAYUZUC Download All Child");



 
    snprintf(Templabels[201], 64, "TO Delete");  
    snprintf(Templabels[202], 64, "TO Rename");  



    snprintf(Templabels[203], 64, "COSMIC FTP Send File Command");
    snprintf(Templabels[204], 64, "FM Get File Info");
    snprintf(Templabels[205], 64, "Beacon Mukbang (Useless but may try)");
    snprintf(Templabels[206], 64, "UTRX GNDWDT Clear");


    snprintf(Templabels[207], 64, "To COSMIC... From Yonsei...");
    snprintf(Templabels[208], 64, "COSMIC S/UHF Dual Emission For DoSsam");
    

    
    snprintf(Templabels[209], 64, "COSMIC EPS P31U SET CONFIG2");
    
    snprintf(Templabels[210], 64, "COSMIC EPS P31U CONFIG2");
    
    snprintf(Templabels[211], 64, "COSMIC EPS P31U SET OUT SINGLE");
    
    snprintf(Templabels[212], 64, "COSMIC SP DEPLOY");

    snprintf(Templabels[213], 64, "FM Get Dir List Pkt");

    snprintf(Templabels[214], 64, "COSMIC UEL Pi ON");
    snprintf(Templabels[215], 64, "COSMIC UEL Pi OFF");

    snprintf(Templabels[216], 64, "ES NoOp");
    snprintf(Templabels[217], 64, "ES Start App");
    snprintf(Templabels[218], 64, "ES Stop App");
    snprintf(Templabels[219], 64, "ES Reload App");
    snprintf(Templabels[220], 64, "ES Query One"); 
    snprintf(Templabels[221], 64, "ES Query All");

    /******************************5차 추가*************************************************/
    snprintf(Templabels[230], 64, "PAYUEL_LGPM NOOP");
    snprintf(Templabels[231], 64, "PAYUEL_LGPM RESET_COUNTERS");
    snprintf(Templabels[232], 64, "PAYUEL_LGPM MCU_ALIVE_CHECK");
    snprintf(Templabels[233], 64, "PAYUEL_LGPM 3V3_PWR_ON");
    snprintf(Templabels[234], 64, "PAYUEL_LGPM 3V3_PWR_OFF");
    snprintf(Templabels[235], 64, "PAYUEL_LGPM MAIN_BOOST_SW_ON");
    snprintf(Templabels[236], 64, "PAYUEL_LGPM MAIN_BOOST_SW_OFF");
    snprintf(Templabels[237], 64, "PAYUEL_LGPM SUB_BOOST_SW_ON");
    snprintf(Templabels[238], 64, "PAYUEL_LGPM SUB_BOOST_SW_OFF");
    snprintf(Templabels[239], 64, "PAYUEL_LGPM V28_MAIN_ON");
    snprintf(Templabels[240], 64, "PAYUEL_LGPM V28_MAIN_OFF");
    snprintf(Templabels[241], 64, "PAYUEL_LGPM V28_SUB_ON");
    snprintf(Templabels[242], 64, "PAYUEL_LGPM V28_SUB_OFF");
    snprintf(Templabels[243], 64, "PAYUEL_LGPM V12_MAIN_ON");
    snprintf(Templabels[244], 64, "PAYUEL_LGPM V12_MAIN_OFF");
    snprintf(Templabels[245], 64, "PAYUEL_LGPM PWR_SENSE_INFO");
    snprintf(Templabels[246], 64, "PAYUEL_LGPM PWR_SEQ_ON");
    snprintf(Templabels[247], 64, "PAYUEL_LGPM PWR_SEQ_OFF");
    snprintf(Templabels[248], 64, "PAYUEL_LGPM RWA_CONTROL_idx1");
    snprintf(Templabels[249], 64, "PAYUEL_LGPM RWA_CONTROL_idx2");
    snprintf(Templabels[250], 64, "PAYUEL_LGPM RWA_CONTROL_idx3");
    snprintf(Templabels[251], 64, "PAYUEL_LGPM RWA_PWR_ON");
    snprintf(Templabels[252], 64, "PAYUEL_LGPM RWA_PWR_OFF");
    snprintf(Templabels[253], 64, "PAYUEL_LGPM RWA_SENSE_INFO");
    
    snprintf(Templabels[280], 64, "PAYUEL_ROMA NOOP");
    snprintf(Templabels[281], 64, "PAYUEL_ROMA RESET_COUNTERS");
    snprintf(Templabels[282], 64, "PAYUEL_ROMA COMM_TEST");
    snprintf(Templabels[283], 64, "PAYUEL_ROMA GET_SPECIFIC_LINE");
    snprintf(Templabels[284], 64, "PAYUEL_ROMA GET_MULTIPLE_LINES");
    snprintf(Templabels[285], 64, "PAYUEL_ROMA GET_LATEST_LINE");
    snprintf(Templabels[286], 64, "PAYUEL_ROMA GET_LATEST_N_LINES");
    snprintf(Templabels[287], 64, "PAYUEL_ROMA CLEAR_ALL_LINES");
    snprintf(Templabels[293], 64, "PAYUEL_ROMA SET_ROUTE_DEFAULT");
    snprintf(Templabels[294], 64, "PAYUEL_ROMA RESET_ROUTE");
    snprintf(Templabels[295], 64, "PAYUEL_ROMA LOAD_ROUTE");
    snprintf(Templabels[296], 64, "PAYUEL_ROMA SAVE_ROUTE");
    snprintf(Templabels[297], 64, "PAYUEL_ROMA SEND_ROUTE");
    snprintf(Templabels[298], 64, "PAYUEL_ROMA SET_ROUTE");
    snprintf(Templabels[299], 64, "PAYUEL_ROMA PAR_GET");
    snprintf(Templabels[300], 64, "PAYUEL_ROMA PAR_SET");
    snprintf(Templabels[301], 64, "PAYUEL_ROMA PAR_DEFAULTS");
    snprintf(Templabels[302], 64, "PAYUEL_ROMA PAR_SAVE");
    snprintf(Templabels[303], 64, "PAYUEL_ROMA PAR_RESTORE");
    snprintf(Templabels[304], 64, "PAYUEL_ROMA PAR_LOAD");
    snprintf(Templabels[305], 64, "PAYUEL_ROMA PAR_SET_OOB");
    snprintf(Templabels[306], 64, "PAYUEL_ROMA SEND_COMMAND");
    snprintf(Templabels[307], 64, "PAYUEL_ROMA SEND_MSG");
    snprintf(Templabels[308], 64, "PAYUEL_ROMA SYNC_RX");
    snprintf(Templabels[309], 64, "PAYUEL_ROMA SYNC_TX");
    snprintf(Templabels[310], 64, "PAYUEL_ROMA PAY_INIT");

    /*****************************************************************************************/
}

int CMDDataGenerator(uint32_t msgid, uint16_t fnccode, void *Requested, size_t RequestedSize) 
{
    ImGui::Checkbox("Checksum", &ChecksumState);
    ImGui::SameLine();
	ImGui::Checkbox("Scheduler", &SchedulerState);
	if(SchedulerState)
	{
        ChecksumState = true;
		ImGui::Text("Execution Time     : ");
		ImGui::SameLine();
		ImGui::InputScalar("##ExecutionTime", ImGuiDataType_U32, &ExecutionTimeBuf, NULL, NULL, "%u");
		ImGui::Text("Execution Window   : ");
		ImGui::SameLine();
		ImGui::InputScalar("##ExecutionWindow", ImGuiDataType_U32, &ExecutionWindowBuf, NULL, NULL, "%u");
		ImGui::Text("Entry ID           : ");
		ImGui::SameLine();
		ImGui::InputScalar("##EntryID", ImGuiDataType_U16, &EntryIDBuf, NULL, NULL, "%u");
		ImGui::Text("GroupID            : ");
		ImGui::SameLine();
		ImGui::InputScalar("##GroupID", ImGuiDataType_U16, &GroupIDBuf, NULL, NULL, "%u");
	}
	if(ImGui::Button("Generate", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetFontSize() * 1.5)))
	{
		SatCMD[NowCMD] = new CmdGenerator_GS();
        console.AddLog("[DEBUG]##Generate : %u, %u, %d\n", msgid, fnccode, RequestedSize);
        if(RequestedSize == sizeof(CFE_MSG_CommandHeader))
            SatCMD[NowCMD]->GenerateCmdHeader(msgid, fnccode, RequestedSize, NULL);
        else
		    SatCMD[NowCMD]->GenerateCmdHeader(msgid, fnccode, RequestedSize, Requested + 8);
		if(SchedulerState)
        {
            SatCMD[NowCMD]->Scheduled = true;
            SatCMD[NowCMD]->Scheduling(ExecutionTimeBuf, ExecutionWindowBuf, EntryIDBuf, GroupIDBuf);
        }
        else if(!ChecksumState)
        {
            SatCMD[NowCMD]->Checksum = false;
        }
			
		NowCMD ++;

		console.AddLog("Generated Command Message.");
        memset(Requested, 0, RequestedSize);
		uint32_t ExecutionTimeBuf = 0;
		uint32_t ExecutionWindowBuf = 0;
		uint16_t EntryIDBuf = 0;
		uint16_t GroupIDBuf = 0;
        ChecksumState = true;
        SchedulerState = false;
	}
	ImGui::SameLine();
	if(ImGui::Button("Close", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
	{
		memset(Requested, 0, RequestedSize);
		return 0;
	}
    return 1;
	
}


bool popup_fds()
{
    ImGui::OpenPopup("FDS Manager");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(640, 300), ImGuiCond_Appearing);
    if ( ImGui::BeginPopupModal("FDS Manager", NULL, ImGuiWindowFlags_AlwaysAutoResize) )
    {
        ImGui::Text("FDS Path");
        ImGui::InputText("##FDSPath", FDSFilePath, sizeof(FDSFilePath), 0, NULL);
        if (ImGui::Button("Load FDS", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetContentRegionAvail().y)))
        {
            ReadTLELines(FDSFilePath, true);
            if(strlen(State.Fatellites->Name()) != 0)
            {
                State.Fatellites->use = true;
            }
            else
                console.AddLog("[ERROR]##Cannot Load FDS File.");
            // State.Fatellites = new SatelliteObject(Tle(), Yonsei, false);
            SatelliteModelInitialize(-1);
            ImGui::EndPopup();
            ImGui::CloseCurrentPopup();
            return false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y)))
        {
            ImGui::EndPopup();
            ImGui::CloseCurrentPopup();
            return false;
        }
        ImGui::EndPopup();
        return true;
    }
}



bool popup_satinfo(int index)
{
    if(index < -1)
        return false;
    else if(index == -1)
        sprintf(SatWindowLabelBuf, "Details : %s", State.Fatellites->Name());
    else
        sprintf(SatWindowLabelBuf, "Details : %s", State.Satellites[index]->Name());

    ImGui::OpenPopup(SatWindowLabelBuf);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(1080, 640), ImGuiCond_Appearing);
    if ( ImGui::BeginPopupModal(SatWindowLabelBuf, NULL, ImGuiWindowFlags_AlwaysAutoResize) )
    {
        if (ImGui::BeginTable("##State.SatellitesInformations", 3, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable, ImVec2(ImGui::GetContentRegionAvail().x, 480)));
        {
            ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);
            ImGui::TableSetupColumn("AOS",           ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_NoHide, 0.0f);
            ImGui::TableSetupColumn("LOS",       ImGuiTableColumnFlags_NoHide, 0.0f);
            ImGui::TableSetupColumn("Max Elevation",  ImGuiTableColumnFlags_NoHide, 0.0f);
            ImGui::TableHeadersRow();
            
            // ImGui::TableNextColumn();
            if(index == -1)
            {
                for(int i = 0; i < 64; i++)
                {
                    if(State.Fatellites->_max_elevation[i] <= 0.0f || State.Satellites[i]->_max_elevation[i] > 90.0f)
                        break;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d/%d %d:%d:%d",   State.Fatellites->_nextaos[i].AddHours(9).Month(), 
                                                    State.Fatellites->_nextaos[i].AddHours(9).Day(),
                                                    State.Fatellites->_nextaos[i].AddHours(9).Hour(),
                                                    State.Fatellites->_nextaos[i].AddHours(9).Minute(),
                                                    State.Fatellites->_nextaos[i].AddHours(9).Second());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d/%d %d:%d:%d",   State.Fatellites->_nextlos[i].AddHours(9).Month(), 
                                                    State.Fatellites->_nextlos[i].AddHours(9).Day(),
                                                    State.Fatellites->_nextlos[i].AddHours(9).Hour(),
                                                    State.Fatellites->_nextlos[i].AddHours(9).Minute(),
                                                    State.Fatellites->_nextlos[i].AddHours(9).Second());
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.2f", State.Fatellites->_max_elevation[i] * RAD_TO_DEG);
                }
            }
            else
            {
                for(int i = 0; i < 64; i++)
                {
                    if(State.Satellites[index]->_max_elevation[i] <= 0.0f || State.Satellites[index]->_max_elevation[i] > 90.0f)
                        break;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d/%d %d:%d:%d",   State.Satellites[index]->_nextaos[i].AddHours(9).Month(), 
                                                    State.Satellites[index]->_nextaos[i].AddHours(9).Day(),
                                                    State.Satellites[index]->_nextaos[i].AddHours(9).Hour(),
                                                    State.Satellites[index]->_nextaos[i].AddHours(9).Minute(),
                                                    State.Satellites[index]->_nextaos[i].AddHours(9).Second());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d/%d %d:%d:%d",   State.Satellites[index]->_nextlos[i].AddHours(9).Month(), 
                                                    State.Satellites[index]->_nextlos[i].AddHours(9).Day(),
                                                    State.Satellites[index]->_nextlos[i].AddHours(9).Hour(),
                                                    State.Satellites[index]->_nextlos[i].AddHours(9).Minute(),
                                                    State.Satellites[index]->_nextlos[i].AddHours(9).Second());
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.2f", State.Satellites[index]->_max_elevation[i] * RAD_TO_DEG);
                }
            }
            ImGui::EndTable();
            
        }
        if (ImGui::Button("Close", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFontSize() * 1.5)))
        {
            ImGui::EndPopup();
            ImGui::CloseCurrentPopup();
            return false;
        }
        ImGui::EndPopup();
        return true;
    }
}

bool popup_s_band()
{
    ImGui::OpenPopup("S-Band Manager");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(720, 540), ImGuiCond_Appearing);
    if ( ImGui::BeginPopupModal("S-Band Manager", NULL, ImGuiWindowFlags_AlwaysAutoResize) )
    {
        ImGui::Text("S-Band GS PC Remote Control");
        
        ImGui::InputText("##s-bandcmd", inputbuf, sizeof(inputbuf), NULL, NULL);
        ImGui::SameLine();
        if(ImGui::Button("Send##s-band"))
        {
            sgs->cmd(inputbuf);
        }
        ImGui::BeginChild("##s-band", ImVec2(ImGui::GetContentRegionAvail().x, 650), true, mim_winflags);
        ImGui::Text(sgs->buffer);
        ImGui::EndChild();
        if (ImGui::Button("Close", ImVec2(ImGui::GetContentRegionAvail().x, 40)))
        {
            ImGui::EndPopup();
            ImGui::CloseCurrentPopup();
            return false;
        }
        ImGui::EndPopup();
        return true;
    }
}
