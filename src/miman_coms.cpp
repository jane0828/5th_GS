/* Customs */
#include "miman_config.h"
#include "miman_csp.h"
#include "miman_coms.h"
#include "miman_imgui.h"
#include "miman_config.h"
#include "miman_orbital.h"
#include "miman_radial.h"
#include "miman_ftp.h"
#include <netinet/in.h>

#include <mutex>

#define _CRT_SECURE_NO_WARNINGS

extern FILE *log_ptr;
extern bool raw_data;
std::mutex g_report_view_mtx;
ReportView_t g_report_view;
//int signallen = 8;
Beacon* beacon = (Beacon *)malloc(MIM_LEN_BEACON);
MissionBeacon* missionbeacon = (MissionBeacon *)malloc(BEE_LEN_MISSIONBEACON);
Report* report = (Report *)malloc(BEE_LEN_REPORT);
Event* event = (Event *)malloc(BEE_LEN_EVENT);
GETFILEINFO* getfileinfo = (GETFILEINFO *)malloc(BEE_LEN_GETFILEINFO);
int NowFTP = 0;

char HKbuf[202];
int HKbufCursor = 0;
char AODbuf[131];
int AODbufCufsor = 0;
CmdGenerator_GS * SatCMD[256];
extern StateCheckUnit State;
extern pthread_t p_thread[16];
extern pthread_mutex_t conn_lock;

// HK *NowHK;
// AOD *NowAOD;

extern Console console;
extern Setup * setup;

pthread_t LinkTrhead;

int BeaconCounter;
int MissionBeaconCounter;
int ReportCounter;
int PingCounter;
uint32_t remote_total_rx_bytes = 0;
uint16_t remote_boot_count = 0;


static constexpr size_t REPORT_WIRE_SIZE = 540;

static uint8_t g_report_wire[REPORT_WIRE_SIZE];
static size_t  g_report_off = 0;
static bool    g_report_collecting = false;

static time_t  g_report_last_chunk_time = 0;
static bool    g_report_have_last_time = false;


static uint16_t be16(const uint8_t *p) {
    return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}
static uint32_t be32u(const uint8_t *p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}
static int32_t be32s(const uint8_t *p) {
    return (int32_t)be32u(p);
}


static bool DecodePayloadToView(ReportView_t &v, const uint8_t *payload, uint16_t payload_len) {
    if (!payload) return false;

    if (v.kind == REPORT_KIND_UANT_GET_STATUS_TLM) {
        if (payload_len < sizeof(gs_gssb_ant6_release_status_t)) {
            v.kind = REPORT_KIND_SC_GENERIC;
        } else {
            memcpy(&v.u.uant_getstatus, payload, sizeof(gs_gssb_ant6_release_status_t));
            return true;
        }
    }


    if (v.kind == REPORT_KIND_ADCS_LOG_MASK) {
        if (payload_len < sizeof(ADCS_TlmLogInclMaskTlm_Payload_t)) {
            v.kind = REPORT_KIND_SC_GENERIC;
        } else {
            memcpy(&v.u.adcs_logmask, payload, sizeof(ADCS_TlmLogInclMaskTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_UNSOLICIT_TLM_SETUP_TLM) {
        if (payload_len < sizeof(ADCS_UnsolicitTlmMsgSetupTlm_Payload_t)) {
            v.kind = REPORT_KIND_SC_GENERIC;
        } else {
            memcpy(&v.u.adcs_unsolicited_tlm_tlm, payload, sizeof(ADCS_UnsolicitTlmMsgSetupTlm_Payload_t));
            return true;
        }
    }


    if (v.kind == REPORT_KIND_EPS_P60_DOCK_GET_TABLE_HK) {
        if (payload_len < sizeof(EPS_P60_DOCK_GET_TABLE_HK)) {
            v.kind = REPORT_KIND_SC_GENERIC;
        } else {
            memcpy(&v.u.eps_p60dockgettablehk, payload, sizeof(EPS_P60_DOCK_GET_TABLE_HK));
            return true;
        }
    }


        if (v.kind == REPORT_KIND_EPS_P60_PDU_GET_TABLE_HK) {
        if (payload_len < sizeof(EPS_P60_PDU_GET_TABLE_HK)) {
            v.kind = REPORT_KIND_SC_GENERIC;
        } else {
            memcpy(&v.u.eps_p60pdugettablehk, payload, sizeof(EPS_P60_PDU_GET_TABLE_HK));
            return true;
        }
    }

        if (v.kind == REPORT_KIND_EPS_P60_ACU_GET_TABLE_HK) {
        if (payload_len < sizeof(EPS_P60_ACU_GET_TABLE_HK)) {
            v.kind = REPORT_KIND_SC_GENERIC;
        } else {
            memcpy(&v.u.eps_p60acugettablehk, payload, sizeof(EPS_P60_ACU_GET_TABLE_HK));
            return true;
        }
    }

    /****************************************************************************************************************************** */
    if (v.kind == REPORT_KIND_ADCS_GET_ERROR_LOG_SETTING)
    {
        if (payload_len < sizeof(ADCS_ErrorLogSettingTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_errorlogsetting, payload, sizeof(ADCS_ErrorLogSettingTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_CURRENT_UNIX_TIME)
    {
        if (payload_len < sizeof(ADCS_CurrentUnixTimeTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_currentunixtime, payload, sizeof(ADCS_CurrentUnixTimeTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC)
    {
        if (payload_len < sizeof(ADCS_PersistConfigDiagnosticTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_persistconfigdiagnostic, payload, sizeof(ADCS_PersistConfigDiagnosticTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_COMMUNICATION_STATUS)
    {
        if (payload_len < sizeof(ADCS_CommunicationStatusTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_communicationstatus, payload, sizeof(ADCS_CommunicationStatusTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_CONTROL_ESTIMATION_MODE)
    {
        if (payload_len < sizeof(ADCS_ControlEstimationModeTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_controlestimationmode, payload, sizeof(ADCS_ControlEstimationModeTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_REFERENCE_IRC_VECTOR)
    {
        if (payload_len < sizeof(ADCS_ReferenceIRCVectorTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_referenceircvector, payload, sizeof(ADCS_ReferenceIRCVectorTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_REFERENCE_LLH_TARGET)
    {
        if (payload_len < sizeof(ADCS_ReferenceLLHTargetTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_referenceircvector, payload, sizeof(ADCS_ReferenceLLHTargetTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_ORBIT_MODE)
    {
        if (payload_len < sizeof(ADCS_OrbitModeTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_orbitmode, payload, sizeof(ADCS_OrbitModeTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_HEALTH_TLM_MMT)
    {
        if (payload_len < sizeof(ADCS_HealthTlmMMTTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_healthtlmmmt, payload, sizeof(ADCS_HealthTlmMMTTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_RAW_CUBESENSE_SUN)
    {
        if (payload_len < sizeof(ADCS_RawCubeSenseSunTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_rawcubesensesun, payload, sizeof(ADCS_RawCubeSenseSunTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_REFERENCE_RPY_VALUES)
    {
        if (payload_len < sizeof(ADCS_ReferenceRPYvaluesTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_referencerpyvalues, payload, sizeof(ADCS_ReferenceRPYvaluesTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_OPENLOOPCMD_MTQ)
    {
        if (payload_len < sizeof(ADCS_OpenLoopCmdMTQTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_openloopcmdmtq, payload, sizeof(ADCS_OpenLoopCmdMTQTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_POWER_STATE)
    {
        if (payload_len < sizeof(ADCS_PowerStateTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_powerstate, payload, sizeof(ADCS_PowerStateTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_RUN_MODE)
    {
        if (payload_len < sizeof(ADCS_RunModeTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_runmode, payload, sizeof(ADCS_RunModeTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_CONTROL_MODE)
    {
        if (payload_len < sizeof(ADCS_ControlModeTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_controlmode, payload, sizeof(ADCS_ControlModeTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_MAG0_MMT_CALIB_CONFIG)
    {
        if (payload_len < sizeof(ADCS_Mag0MMTCalibConfigTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_mag0mmtcalibconfig, payload, sizeof(ADCS_Mag0MMTCalibConfigTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_ESTIMATION_MODE)
    {
        if (payload_len < sizeof(ADCS_EstimationModeTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_estimationmode, payload, sizeof(ADCS_EstimationModeTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_OPERATIONAL_STATE)
    {
        if (payload_len < sizeof(ADCS_OperationalStateTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_operationalstate, payload, sizeof(ADCS_OperationalStateTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_RAW_CSS_SENSOR)
    {
        if (payload_len < sizeof(ADCS_RawCSSSensorTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_rawcsssensor, payload, sizeof(ADCS_RawCSSSensorTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_RAW_GYR_SENSOR)
    {
        if (payload_len < sizeof(ADCS_RawGYRSensorTlm_Paylaod_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_rawgyrsensor, payload, sizeof(ADCS_RawGYRSensorTlm_Paylaod_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_CALIBRATED_GYR_SENSOR)
    {
        if (payload_len < sizeof(ADCS_CalibratedGYRSensorTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_calibratedgyrsensor, payload, sizeof(ADCS_CalibratedGYRSensorTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_MAG_SENSING_ELM_CONFIG)
    {
        if (payload_len < sizeof(ADCS_MagSensingElmConfigTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_magsensingelmconfig, payload, sizeof(ADCS_MagSensingElmConfigTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_TLM_LOG_INCLMASK)
    {
        if (payload_len < sizeof(ADCS_TlmLogInclMaskTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_tlmloginclmask, payload, sizeof(ADCS_TlmLogInclMaskTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_UNSOLICIT_TLM_MSG_SETUP)
    {
        if (payload_len < sizeof(ADCS_UnsolicitTlmMsgSetupTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_unsolicittlmmsgsetup, payload, sizeof(ADCS_UnsolicitTlmMsgSetupTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP)
    {
        if (payload_len < sizeof(ADCS_UnsolicitEventMsgSetupTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_unsoliciteventmsgsetup, payload, sizeof(ADCS_UnsolicitEventMsgSetupTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_EVENT_LOG_STATUS_RESPONSE)
    {
        if (payload_len < sizeof(ADCS_EventLogStatusResponseTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_eventlogstatusresponse, payload, sizeof(ADCS_EventLogStatusResponseTlm_Payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_ADCS_GET_PORTMAP)
    {
        if (payload_len < sizeof(ADCS_PortMapTlm_Payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.adcs_portmap, payload, sizeof(ADCS_PortMapTlm_Payload_t));
            return true;
        }
    }
/**************************************************************************************************************************************************** */
    
/******************************************************5차 추가**************************************************************/

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_NOOP)
    {
        if (payload_len < sizeof(payuel_roma_Noop_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_Noop_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_RESETCOUNTERS)
    {
        if (payload_len < sizeof(payuel_roma_ResetCounters_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_ResetCounters_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_COMMTEST)
    {
        if (payload_len < sizeof(payuel_roma_CommTest_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_CommTest_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_GETSPECIFICLINE)
    {
        if (payload_len < sizeof(payuel_roma_GetSpecificLine_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_GetSpecificLine_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_GETMULTIPLELINES)
    {
        if (payload_len < sizeof(payuel_roma_GetMultipleLines_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_GetMultipleLines_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_GETLATESTLINE)
    {
        if (payload_len < sizeof(payuel_roma_GetLatestLine_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_GetLatestLine_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_GETLATESTNLINES)
    {
        if (payload_len < sizeof(payuel_roma_GetLatest_N_Lines_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_GetLatest_N_Lines_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_SETROUTEDEFAULT)
    {
        if (payload_len < sizeof(payuel_roma_SetRouteDefault_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_SetRouteDefault_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_RESETROUTE)
    {
        if (payload_len < sizeof(payuel_roma_ResetRoute_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_ResetRoute_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_LOADROUTE)
    {
        if (payload_len < sizeof(payuel_roma_LoadRoute_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_LoadRoute_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_SAVEROUTE)
    {
        if (payload_len < sizeof(payuel_roma_SaveRoute_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_SaveRoute_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_SENDROUTE)
    {
        if (payload_len < sizeof(payuel_roma_SendRoute_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_SendRoute_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_PARGET)
    {
        if (payload_len < sizeof(payuel_roma_ParGet_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_ParGet_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_PARSET)
    {
        if (payload_len < sizeof(payuel_roma_ParSet_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_ParSet_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_PARDEFAULTS)
    {
        if (payload_len < sizeof(payuel_roma_ParDefaults_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_ParDefaults_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_PARSAVE)
    {
        if (payload_len < sizeof(payuel_roma_ParSave_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_ParSave_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_PARRESTORE)
    {
        if (payload_len < sizeof(payuel_roma_ParRestore_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_ParRestore_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_PARLOAD)
    {
        if (payload_len < sizeof(payuel_roma_ParLoad_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_ParLoad_tlm_payload_t));
            return true;
        }
    }
    
    if (v.kind == REPORT_KIND_PAYUEL_ROMA_PARSETOOB)
    {
        if (payload_len < sizeof(payuel_roma_ParSetOOB_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_ParSetOOB_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_SENDCOMMAND)
    {
        if (payload_len < sizeof(payuel_roma_SendCommand_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_noop, payload, sizeof(payuel_roma_SendCommand_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_NOOP)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_Noop_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_noop, payload, sizeof(PAYUEL_LGPM_Noop_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_RESETCOUNTERS)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_ResetCounters_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_resetcounters, payload, sizeof(PAYUEL_LGPM_ResetCounters_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_MCU_ALIVE)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_MCU_ALIVE_CHECK_Tlm_Payload))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_mcualivecheck, payload, sizeof(PAYUEL_LGPM_MCU_ALIVE_CHECK_Tlm_Payload));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_3V3_PWR_ON)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_3V3PwrOn_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_3v3pwron, payload, sizeof(PAYUEL_LGPM_3V3PwrOn_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_3V3_PWR_OFF)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_3V3PwrOff_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_3v3pwroff, payload, sizeof(PAYUEL_LGPM_3V3PwrOff_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_ON)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_MainBoostSwOn_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_mainboostswon, payload, sizeof(PAYUEL_LGPM_MainBoostSwOn_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_OFF)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_MainBoostSwOff_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_mainboostswoff, payload, sizeof(PAYUEL_LGPM_MainBoostSwOff_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_ON)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_SubBoostSwOn_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_subboostswon, payload, sizeof(PAYUEL_LGPM_SubBoostSwOn_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_OFF)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_SubBoostSwOff_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_subboostswoff, payload, sizeof(PAYUEL_LGPM_SubBoostSwOff_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_V28_MAIN_ON)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_V28MainOn_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_v28mainon, payload, sizeof(PAYUEL_LGPM_V28MainOn_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_V28_MAIN_OFF)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_V28MainOff_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_v28mainoff, payload, sizeof(PAYUEL_LGPM_V28MainOff_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_V28_SUB_ON)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_V28SubOn_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_v28subon, payload, sizeof(PAYUEL_LGPM_V28SubOn_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_V28_SUB_OFF)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_V28SubOff_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_v28suboff, payload, sizeof(PAYUEL_LGPM_V28SubOff_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_V12_MAIN_ON)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_V12MainOn_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_v12mainon, payload, sizeof(PAYUEL_LGPM_V12MainOn_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_V12_MAIN_OFF)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_V12MainOff_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_v12mainoff, payload, sizeof(PAYUEL_LGPM_V12MainOff_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_PWR_SENSE_INFO)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_PwrSenseInfo_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_pwrsenseinfo, payload, sizeof(PAYUEL_LGPM_PwrSenseInfo_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_ON)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_PwrSeqOn_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_pwrseqon, payload, sizeof(PAYUEL_LGPM_PwrSeqOn_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_OFF)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_PwrSeqOff_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_pwrseqoff, payload, sizeof(PAYUEL_LGPM_PwrSeqOff_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX1)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_RwaControlIdx1_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_rwacontrol_idx1, payload, sizeof(PAYUEL_LGPM_RwaControlIdx1_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX2)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_RwaControlIdx2_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_rwacontrol_idx2, payload, sizeof(PAYUEL_LGPM_RwaControlIdx2_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX3)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_RwaControlIdx3_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_rwacontrol_idx3, payload, sizeof(PAYUEL_LGPM_RwaControlIdx3_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_RWA_PWR_ON)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_RwaPwrOn_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_rwapwron, payload, sizeof(PAYUEL_LGPM_RwaPwrOn_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_RWA_PWR_OFF)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_RwaPwrOff_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_rwapwroff, payload, sizeof(PAYUEL_LGPM_RwaPwrOff_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_RWA_SENSE_INFO)
    {
        if (payload_len < sizeof(PAYUEL_LGPM_RwaSenseInfo_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.lgpm_rwasenseinfo, payload, sizeof(PAYUEL_LGPM_RwaSenseInfo_tlm_payload_t));
            return true;
        }
    }
    /********************************************************************************************************************************* */

    v.kind = REPORT_KIND_SC_GENERIC;

    uint16_t n = payload_len;
    if (n > sizeof(v.u.generic.bytes)) n = sizeof(v.u.generic.bytes);
    memcpy(v.u.generic.bytes, payload, n);
    if (n < sizeof(v.u.generic.bytes)) memset(v.u.generic.bytes + n, 0, sizeof(v.u.generic.bytes) - n);

    return true;
}


static bool ParseReportWire540(const uint8_t *buf, size_t len, Report &out) {
    if (!buf) return false;
    if (len < REPORT_WIRE_SIZE) return false;

    memset(&out, 0, sizeof(out));

    // ===== CCSDS-like header (16B assumed in your format) =====
    out.CCSDS_MsgId = be16(buf + 0);
    out.CCSDS_Seq   = be16(buf + 2);
    out.CCSDS_Len   = be16(buf + 4);
    memcpy(out.CCSDS_TimeCode, buf + 6, 6);
    out.CCSDS_Padding = be32u(buf + 12);

    // ===== Report body (starts at offset 16) =====
// ===== Report body (starts at offset 16) =====
    memcpy(&out.ReflectedMID, buf + 16, sizeof(out.ReflectedMID));
    memcpy(&out.ReflectedCC,  buf + 18, sizeof(out.ReflectedCC));
    memcpy(&out.RetType,      buf + 19, sizeof(out.RetType));
    memcpy(&out.RetCode,      buf + 20, sizeof(out.RetCode));
    memcpy(&out.RetValSize,   buf + 24, sizeof(out.RetValSize));

    // Return value bytes start at offset 26
    size_t copy_len = 512;
    if (out.RetValSize < copy_len) copy_len = out.RetValSize;
    memcpy(out.RetVal, buf + 26, copy_len);

    if (out.RetValSize > 512)
        out.RetValSize = 512;

    return true;
}


void * TRxController(void *)
{
    csp_socket_t * DLsocket = DL_sock_initialize();
    while(State.TRx_mode)
    {
        if(State.downlink_mode)
        {
            printf("Create downlink onorbit...\n");
            pthread_create(&LinkTrhead, NULL, task_downlink_onorbit, DLsocket);
            pthread_join(LinkTrhead, NULL);
            State.uplink_mode = true;
        }
        else
            continue;
    }
}

void buf_allclear()
{
    memset(HKbuf, 0, sizeof(HKbuf));
    memset(AODbuf, 0, sizeof(HKbuf));
    HKbufCursor = 0;
    //AODBufCursor = 0;
}

void * SignalTest(void*)
{
    State.Signaltest = true;
    while(State.Signaltest)
    {
        if(csp_ping(19, 100, 1, 0))
            continue;
    }
}

void now_rx_bytes_update()
{
    // if(beacon->UXTotRXByte != remote_total_rx_bytes)
    //     remote_total_rx_bytes = beacon->UXTotRXByte;
}

void set_rx_bytes(uint32_t nowbytes)
{
    remote_total_rx_bytes = nowbytes;
}

uint32_t get_rx_bytes()
{
    return remote_total_rx_bytes;
}

uint16_t get_boot_count()
{
    return remote_boot_count;
}

uint32_t * get_rx_bytes_address()
{
    return &remote_total_rx_bytes;
}

uint16_t * get_boot_count_address()
{
    return &remote_boot_count;
}

CmdGenerator_GS::CmdGenerator_GS(void) {
    uint8_t* pool = new uint8_t[MIM_DEFAULT_DATALEN];
    if (!pool) {
        console.AddLog("Generator init error: alloc failed");
        return;
    }
    memset(pool, 0, MIM_DEFAULT_DATALEN);
    this->CmdHeader = (CFE_MSG_CommandHeader*) pool;
}

CmdGenerator_GS::~CmdGenerator_GS(void) {
    uint8_t* pool = (uint8_t*) this->CmdHeader;
    delete[] CmdHeader;
}

int CmdGenerator_GS::GenerateCmdHeader(uint32_t MsgId, uint16_t FncCode, uint32_t Size, void* Data) {
    memset(this->CmdHeader->Msg.Byte, 0, Size);

    if (this->SetHasSecondaryHeader(true) != 0) {
        return -1;
    }
    if (this->SetSegmentationFlag(CFE_MSG_SEGFLG_UNSEG) != 0) {
        return -1;
    }
    if (this->SetMsgId(MsgId) != 0) {
        return -1;
    }
    if (this->SetSize(Size) != 0) {
        return -1;
    }
    if (this->SetFncCode(FncCode) != 0) {
        return -1;
    }
    if (this->GenerateChecksum() != 0) {
        return -1;
    }
    if (Data && (Size - sizeof(CFE_MSG_CommandHeader)) > 0)
        memcpy(this->CmdHeader->Msg.Byte + sizeof(CFE_MSG_CommandHeader), Data, Size - sizeof(CFE_MSG_CommandHeader));
    return 0;
}

void CmdGenerator_GS::CopyCmdHeaderToBuffer(uint8_t* Buffer) {
    memcpy(Buffer, this->CmdHeader->Msg.Byte, this->GetSize());
}

int CmdGenerator_GS::SetFncCode(uint16_t FncCode) {
    if (!this->HasSecondaryHeader()) {
        return -1;
    }
    this->CmdHeader->Sec.FunctionCode = FncCode;
    return 0;
}


void CmdGenerator_GS::SetHeaderWord(uint8_t* Word16, uint16_t Value, uint16_t Mask) {
    Word16[0] = (Word16[0] & ~(Mask >> 8)) | ((Value & Mask) >> 8);
    Word16[1] = ((Word16[1] & ~Mask) | (Value & Mask)) & 0xFF;
}


void CmdGenerator_GS::GetHeaderWord(const uint8_t* ByteField, uint16_t& Value, uint16_t Mask) {
    Value = (ByteField[0] << 8 | ByteField[1]) & Mask;
}

void CmdGenerator_GS::SetHeaderByte(uint8_t* Byte, uint8_t Value, uint8_t Mask) {
    Byte[0] = (Byte[0] & ~Mask) | (Value & Mask);
}

int CmdGenerator_GS::SetHasSecondaryHeader(bool HasSec) {
    SetHeaderByte(&this->CmdHeader->Msg.CCSDS.Pri.StreamId[0], HasSec, 0x08);
    return 0;
}

bool CmdGenerator_GS::HasSecondaryHeader(void) const {
    return (this->CmdHeader->Msg.CCSDS.Pri.StreamId[0] & (CFE_MSG_SHDR_MASK >> 8)) != 0;
}


int CmdGenerator_GS::SetSize(uint16_t Size) {
    if (Size < CFE_MSG_SIZE_OFFSET || Size > (0xFFFF + CFE_MSG_SIZE_OFFSET)) {
        return -1;
    }
    Size -= CFE_MSG_SIZE_OFFSET;
    SetHeaderWord(this->CmdHeader->Msg.CCSDS.Pri.Length, Size, 0xFFFF);
    return 0;
}

uint16_t CmdGenerator_GS::GetSize(void) {
    uint16_t RetVal;
    GetHeaderWord(this->CmdHeader->Msg.CCSDS.Pri.Length, RetVal, 0xFFFF);
    return RetVal + CFE_MSG_SIZE_OFFSET;
}

uint16_t CmdGenerator_GS::GetFncCode(void) const
{
    return this->CmdHeader->Sec.FunctionCode;
}

int CmdGenerator_GS::SetMsgId(uint16_t MsgId) {
    SetHeaderWord(this->CmdHeader->Msg.CCSDS.Pri.StreamId, MsgId, 0xFFFF);
    return 0;
}

int CmdGenerator_GS::SetSegmentationFlag(uint16_t SegFlag) {
    SetHeaderWord(this->CmdHeader->Msg.CCSDS.Pri.Sequence, SegFlag, CFE_MSG_SEGFLG_MASK);
    return 0;
}


int CmdGenerator_GS::GenerateChecksum(void) {
    CFE_MSG_CommandHeader* Cmd = this->CmdHeader;
    Cmd->Sec.Checksum = 0;
    Cmd->Sec.Checksum = this->ComputeCheckSum();
    return 0;
}

uint32_t CmdGenerator_GS::ComputeCheckSum(void) {
    uint16_t Len = this->GetSize();
    const uint8_t* BytePtr = this->CmdHeader->Msg.Byte;
    uint32_t chksum  = 0xFF;

    while (Len--) {
        chksum ^= *(BytePtr++);
    }
    return chksum;
}


csp_socket_t * DL_sock_initialize()
{
	csp_socket_t * sock = csp_socket(0);
	if(!csp_bind(sock, 13)) {
        console.AddLog("[OK]##TM Port 13 bind success.");
    }
    if(!csp_bind(sock, 31)) {
        console.AddLog("[OK]##BCN Port 31 bind success.");
    }
    if(!csp_bind(sock, 25)) {
        console.AddLog("[OK]##RPT Port 25 bind success.");
    }
    if(!csp_bind(sock, 27)) {
        console.AddLog("[OK]##Event Port 27 bind success.");
    }
        if(!csp_bind(sock, 23)) {
        console.AddLog("[OK]##COSMIC Beacon Port 23 bind success.");
    }
        
        if(!csp_bind(sock, 24)) {
        console.AddLog("[OK]##COSMIC Report Port 24 bind success.");
    }
        
        
    // while(true) {
    //     if (csp_bind(sock, 23) == 0) { // Add for HVD_TMTC_TEST
    //     console.AddLog("[OK]Bind Success.");
    //     break;
    //     };
    // }
	csp_listen(sock, 10);
    //Fail preventation would be needed!
    return sock;
}

// void * PacketDebugMsg(csp_packet_t * packet)
// {
//     char DebugMsg[1024];
//     (char *)packet->data
//     for(int i = 0; i < packet->length; i++)
//     {
//         sprintf(DebugMsg + i, "%")
//     }
// }
static void UpdateReportViewFromReport(const Report &rpt) {
    ReportView_t tmp;
    memset(&tmp, 0, sizeof(tmp));

    tmp.valid = false;

    tmp.CCMessage_ID = rpt.CCSDS_MsgId;
    tmp.CCCount      = rpt.CCSDS_Seq;
    tmp.CCLength     = rpt.CCSDS_Len;
    memcpy(tmp.CCTime_code, rpt.CCSDS_TimeCode, 6);

    tmp.reflected_msg_id       = rpt.ReflectedMID;
    tmp.reflected_cc           = rpt.ReflectedCC;
    tmp.ret_type     = rpt.RetType;
    tmp.ret_code     = rpt.RetCode;

    tmp.ret_val_size = rpt.RetValSize;
    if (tmp.ret_val_size > 512) tmp.ret_val_size = 512;

    tmp.kind = DetermineReportKind(tmp.reflected_msg_id, tmp.reflected_cc);

    DecodePayloadToView(tmp, rpt.RetVal, tmp.ret_val_size);

    tmp.valid = true;

    {
        std::lock_guard<std::mutex> lk(g_report_view_mtx);
        g_report_view = tmp;
    }
}


int BeaconSaver(Beacon* bec)
{
    if (!bec) return -1;

    BeaconCounter++;

    char filename[128];
    time_t tmtime = time(0);
    struct tm* local = localtime(&tmtime);

    sprintf(filename,
            "../data/beacon/Beacon--%04d-%02d-%02d-%02d-%02d-%02d--.txt",
            local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
            local->tm_hour, local->tm_min, local->tm_sec);

    FILE* fp = fopen(filename, "w");
    if (!fp) return -2;

    fprintf(fp, "================= BEACON SAVE =================\n");

    // ---------------------------------------------------
    //  CCSDS HEADER
    // ---------------------------------------------------
    fprintf(fp, "\n[CCSDS HEADER]\n");
    fprintf(fp, "MID        : %02X %02X\n", bec->CCSDS_MID[0], bec->CCSDS_MID[1]);
    fprintf(fp, "SEQ        : %02X %02X\n", bec->CCSDS_Seq[0], bec->CCSDS_Seq[1]);
    fprintf(fp, "LEN        : %02X %02X\n", bec->CCSDS_Len[0], bec->CCSDS_Len[1]);
    fprintf(fp, "TimeCode   : %02X %02X %02X %02X %02X %02X\n",
            bec->CCSDS_TimeCode[0], bec->CCSDS_TimeCode[1], bec->CCSDS_TimeCode[2],
            bec->CCSDS_TimeCode[3], bec->CCSDS_TimeCode[4], bec->CCSDS_TimeCode[5]);

    // // ---------------------------------------------------
    // //  RPT (FSW) - Should be deprecated
    // // ---------------------------------------------------
    // fprintf(fp, "\n[FSW - RPT]\n");
    // fprintf(fp, "BootCount     : %" PRIu16 "\n", bec->RPT_BootCount);
    // fprintf(fp, "SC Time Sec   : %" PRIu32 "\n", bec->RPT_ScTimeSec);
    // fprintf(fp, "SC Time Sub   : %" PRIu32 "\n", bec->RPT_ScTimeSubsec);
    // fprintf(fp, "Sequence      : %" PRIu32 "\n", bec->RPT_Sequence);
    // fprintf(fp, "Reset Cause   : %" PRIu8  "\n", bec->RPT_ResetCause);
    /********************************************************************/
    /*               BEE RPT Revision (Kweon Hyeokjin)                  */
    /********************************************************************/
    fprintf(fp, "\n[FSW - RPT]\n");
    fprintf(fp, "Cmd Counter   : %" PRIu8 "\n", bec->RPT_CmdCounter);
    fprintf(fp, "Err Counter   : %" PRIu8 "\n", bec->RPT_ErrCounter);
    fprintf(fp, "Report Q Cnt  : %" PRIu8 "\n", bec->RPT_ReportCnt);
    fprintf(fp, "Critical Q Cnt: %" PRIu8 "\n", bec->RPT_CriticalCnt);
    fprintf(fp, "Boot Count    : %" PRIu16 "\n", bec->RPT_BootCount);
    fprintf(fp, "SC Time Sec   : %" PRIu32 "\n", bec->RPT_ScTimeSec);
    fprintf(fp, "SC Time Sub   : %" PRIu32 "\n", bec->RPT_ScTimeSubsec);
    fprintf(fp, "Sequence(LSB) : %" PRIu8  "\n", bec->RPT_Sequence_LSB);
    /*--------------End of BEE RPT Revision (Kweon Hyeokjin)--------------*/

    // ---------------------------------------------------
    // STX (S-band)
    // ---------------------------------------------------
    fprintf(fp, "\n[COMS - STX]\n");
    fprintf(fp, "Symbol Rate        : %" PRIu8 "\n", bec->STX_symbol_rate);
    fprintf(fp, "Tx Power           : %" PRIu8 "\n", bec->STX_transmit_power);
    fprintf(fp, "MODCOD             : %" PRIu8 "\n", bec->STX_modcod);
    fprintf(fp, "Roll-off           : %" PRIu8 "\n", bec->STX_roll_off);
    fprintf(fp, "Pilot Signal       : %" PRIu8 "\n", bec->STX_pilot_signal);
    fprintf(fp, "FEC Frame Size     : %" PRIu8 "\n", bec->STX_fec_frame_size);
    fprintf(fp, "Pre-Tx Delay       : %" PRIu16 "\n", bec->STX_pretransmission_delay);
    fprintf(fp, "Center Frequency   : %f\n", bec->STX_center_frequency);
    fprintf(fp, "Mod Interface Type : %" PRIu8 "\n", bec->STX_modulator_interface_type);
    fprintf(fp, "LVDS IO Type       : %" PRIu8 "\n", bec->STX_lvds_io_type);
    fprintf(fp, "System State       : %" PRIu8 "\n", bec->STX_SystemState);
    fprintf(fp, "Status Flag        : %" PRIu8 "\n", bec->STX_StatusFlag);
    // fprintf(fp, "CPU Temp           : %f\n", bec->STX_CpuTemp);

    // ---------------------------------------------------
    // UANT (UHF Antenna)
    // ---------------------------------------------------
    fprintf(fp, "\n[COMS - UANT]\n");
    fprintf(fp, "UANT1 (0/1/BK) : %" PRIu8 " %" PRIu8 " (BK:%" PRIu8 ")\n",
            bec->UANT1_Chan0, bec->UANT1_Chan1, bec->UANT1_BackupActive);
    fprintf(fp, "UANT2 (0/1/BK) : %" PRIu8 " %" PRIu8 " (BK:%" PRIu8 ")\n",
            bec->UANT2_Chan0, bec->UANT2_Chan1, bec->UANT2_BackupActive);

    // ---------------------------------------------------
    // UTRX
    // ---------------------------------------------------
    fprintf(fp, "\n[COMS - UTRX]\n");
    fprintf(fp, "UTRX ActiveConf : %" PRIu8 "\n", bec->UTRX_ActiveConf);
    fprintf(fp, "UTRX BootCount  : %" PRIu16 "\n", bec->UTRX_BootCount);
    fprintf(fp, "UTRX BootCause  : %" PRIu32 "\n", bec->UTRX_BootCause);
    fprintf(fp, "UTRX Temp       : %" PRId16 "\n", bec->UTRX_BoardTemp);

    // ---------------------------------------------------
    // P60 Dock
    // ---------------------------------------------------
    fprintf(fp, "\n[PCDU - P60 DOCK]\n");
    fprintf(fp, "Cout[0..8]    : ");
    for (int i = 0; i < 9; i++) fprintf(fp, "%d ", bec->P60D_Cout[i]);
    fprintf(fp, "\n");

    fprintf(fp, "Vout[0..8]    : ");
    for (int i = 0; i < 9; i++) fprintf(fp, "%u ", bec->P60D_Vout[i]);
    fprintf(fp, "\n");

    fprintf(fp, "OutEn         : 0x%04X\n", bec->P60D_OutEn);
    fprintf(fp, "BootCause     : %" PRIu32 "\n", bec->P60D_BootCause);
    fprintf(fp, "BootCount     : %" PRIu32 "\n", bec->P60D_BootCount);
    fprintf(fp, "BattMode      : %" PRIu8  "\n", bec->P60D_BattMode);
    fprintf(fp, "HeaterOn      : %" PRIu8  "\n", bec->P60D_HeaterOn);
    fprintf(fp, "VBAT          : %" PRIu16 "\n", bec->P60D_VbatV);
    fprintf(fp, "VCC Current   : %" PRIi16 "\n", bec->P60D_VccC);
    fprintf(fp, "BattV         : %" PRIu16 "\n", bec->P60D_BattV);
    fprintf(fp, "BattTemp      : %d %d\n", bec->P60D_BattTemp[0], bec->P60D_BattTemp[1]);
    fprintf(fp, "WDT CAN Left  : %" PRIu32 "\n", bec->P60D_WdtCanLeft);
    fprintf(fp, "Batt Chg Curr : %" PRId16 "\n", bec->P60D_BattChrg);
    fprintf(fp, "Batt Dis Curr : %" PRId16 "\n", bec->P60D_BattDischrg);

    // ---------------------------------------------------
    // P60 PDU
    // ---------------------------------------------------
    fprintf(fp, "\n[PCDU - P60 PDU]\n");
    fprintf(fp, "Cout : ");
    for (int i = 0; i < 9; i++) fprintf(fp, "%d ", bec->P60P_Cout[i]);
    fprintf(fp, "\n");

    fprintf(fp, "Vout : ");
    for (int i = 0; i < 9; i++) fprintf(fp, "%u ", bec->P60P_Vout[i]);
    fprintf(fp, "\n");

    fprintf(fp, "Vcc     : %d\n", bec->P60P_Vcc);
    fprintf(fp, "ConvEn  : %" PRIu8 "\n", bec->P60P_ConvEn);
    fprintf(fp, "OutEn   : 0x%04X\n", bec->P60P_OutEn);

    // ---------------------------------------------------
    // P60 ACU
    // ---------------------------------------------------
    fprintf(fp, "\n[PCDU - P60 ACU]\n");
    fprintf(fp, "Cin : ");
    for (int i = 0; i < 6; i++) fprintf(fp, "%d ", bec->P60A_Cin[i]);
    fprintf(fp, "\n");

    fprintf(fp, "Vin : ");
    for (int i = 0; i < 6; i++) fprintf(fp, "%u ", bec->P60A_Vin[i]);
    fprintf(fp, "\n");

    // ---------------------------------------------------
    // ADCS
    // ---------------------------------------------------
    fprintf(fp, "\n[ADCS]\n");
    fprintf(fp, "PowerState       : 0x%02X\n", bec->ADCS_PowerState);
    fprintf(fp, "ControlMode      : %" PRIu8 "\n", bec->ADCS_ControlMode);
    fprintf(fp, "GYR0 Calib X     : %f\n", bec->ADCS_GYR0CalibratedRateXComponent);
    fprintf(fp, "GYR0 Calib Y     : %f\n", bec->ADCS_GYR0CalibratedRateYComponent);
    fprintf(fp, "GYR0 Calib Z     : %f\n", bec->ADCS_GYR0CalibratedRateZComponent);

    // ---------------------------------------------------
    // BINARY DUMP
    // ---------------------------------------------------
    fprintf(fp, "\n[BINARY DATA]\n");
    for (size_t i = 0; i < sizeof(*bec); i++)
        fprintf(fp, "%02X ", ((unsigned char*)bec)[i]);
    fprintf(fp, "\n");

    fclose(fp);
    return 0;
}



int MissionBeaconSaver(MissionBeacon* misnbec)
{
    if (!misnbec) return -1;

    MissionBeaconCounter++;

    char filename[128];
    time_t tmtime = time(0);
    struct tm* local = localtime(&tmtime);

    sprintf(filename,
            "../data/missionbeacon/MissionBeacon--%04d-%02d-%02d-%02d-%02d-%02d--.txt",
            local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
            local->tm_hour, local->tm_min, local->tm_sec);

    FILE* fp = fopen(filename, "w");
    if (!fp) return -2;

    fprintf(fp, "================= MISSION BEACON SAVE =================\n");

    /* ===================================================
    *  CCSDS HEADER
    * =================================================== */
    fprintf(fp, "\n[CCSDS HEADER]\n");
    fprintf(fp, "MID        : %02X %02X\n",
            misnbec->CCSDS_MID[0], misnbec->CCSDS_MID[1]);
    fprintf(fp, "SEQ        : %02X %02X\n",
            misnbec->CCSDS_Seq[0], misnbec->CCSDS_Seq[1]);
    fprintf(fp, "LEN        : %02X %02X\n",
            misnbec->CCSDS_Len[0], misnbec->CCSDS_Len[1]);
    fprintf(fp, "TimeCode   : %02X %02X %02X %02X %02X %02X\n",
            misnbec->CCSDS_TimeCode[0], misnbec->CCSDS_TimeCode[1],
            misnbec->CCSDS_TimeCode[2], misnbec->CCSDS_TimeCode[3],
            misnbec->CCSDS_TimeCode[4], misnbec->CCSDS_TimeCode[5]);

    /* ===================================================
    *  SRL HOUSEKEEPING
    * =================================================== */
    fprintf(fp, "\n[SRL HOUSEKEEPING]\n");
    fprintf(fp, "SRL Command Counter        : %" PRIu8 "\n",
            misnbec->srlpayload.CommandCounter);
    fprintf(fp, "SRL Command Error Counter  : %" PRIu8 "\n",
            misnbec->srlpayload.CommandErrorCounter);

    for (int i = 0; i < 4; i++) {
        fprintf(fp, "IOHandleStatus[%d]         : %" PRIu8 "\n",
                i, misnbec->srlpayload.IOHandleStatus[i]);
        fprintf(fp, "IOHandleTxCount[%d]        : %" PRIu16 "\n",
                i, misnbec->srlpayload.IOHandleTxCount[i]);
    }

    /* ===================================================
    *  RPT PAYLOAD SUMMARY
    * =================================================== */
    fprintf(fp, "\n[RPT PAYLOAD SUMMARY]\n");
    fprintf(fp, "CmdCounter                : %" PRIu8 "\n",
            misnbec->rptpayload.CmdCounter);
    fprintf(fp, "CmdErrCounter             : %" PRIu8 "\n",
            misnbec->rptpayload.CmdErrCounter);

    /* ===================================================
    *  RPT QUEUE INFO
    * =================================================== */
    fprintf(fp, "\n[RPT QUEUE INFO]\n");
    fprintf(fp, "ReportQueueCnt            : %" PRIu8 "\n",
            misnbec->rptpayload.ReportQueueCnt);
    fprintf(fp, "CriticalQueueCnt          : %" PRIu8 "\n",
            misnbec->rptpayload.CriticalQueueCnt);

    /* ===================================================
    *  OPERATION DATA
    * =================================================== */
    fprintf(fp, "\n[OPERATION DATA]\n");
    fprintf(fp, "BootCount                 : %" PRIu16 "\n",
            misnbec->rptpayload.BootCount);
    fprintf(fp, "TimeSec                   : %" PRIu32 "\n",
            misnbec->rptpayload.TimeSec);
    fprintf(fp, "TimeSubsec                : %" PRIu32 "\n",
            misnbec->rptpayload.TimeSubsec);
    fprintf(fp, "Sequence                  : %" PRIu32 "\n",
            misnbec->rptpayload.Sequence);
    fprintf(fp, "ResetCause                : 0x%02X\n",
            misnbec->rptpayload.ResetCause);

    /* ===================================================
    *  MISSION BEACON PAYLOAD
    * =================================================== */
    fprintf(fp, "\n[MISSION BEACON PAYLOAD]\n");
    fprintf(fp, "CommandCounter             : %" PRIu8 "\n",
            misnbec->paybcnpayload.CommandCounter);
    fprintf(fp, "CommandErrorCounter        : %" PRIu8 "\n",
            misnbec->paybcnpayload.CommandErrorCounter);
    fprintf(fp, "System Status              : %" PRIi8 "\n",
            misnbec->paybcnpayload.sys_status);

    fprintf(fp, "NTC Temp 0                 : %" PRIi16 "\n", misnbec->paybcnpayload.temp_ntc_0);
    fprintf(fp, "NTC Temp 1                 : %" PRIi16 "\n", misnbec->paybcnpayload.temp_ntc_1);
    fprintf(fp, "NTC Temp 2                 : %" PRIi16 "\n", misnbec->paybcnpayload.temp_ntc_2);
    fprintf(fp, "NTC Temp 3                 : %" PRIi16 "\n", misnbec->paybcnpayload.temp_ntc_3);
    fprintf(fp, "NTC Temp 4                 : %" PRIi16 "\n", misnbec->paybcnpayload.temp_ntc_4);
    fprintf(fp, "NTC Temp 5                 : %" PRIi16 "\n", misnbec->paybcnpayload.temp_ntc_5);
    fprintf(fp, "NTC Temp 6                 : %" PRIi16 "\n", misnbec->paybcnpayload.temp_ntc_6);
    fprintf(fp, "NTC Temp 7                 : %" PRIi16 "\n", misnbec->paybcnpayload.temp_ntc_7);
    fprintf(fp, "NTC Temp 8                 : %" PRIi16 "\n", misnbec->paybcnpayload.temp_ntc_8);
    fprintf(fp, "NTC Temp 9                 : %" PRIi16 "\n", misnbec->paybcnpayload.temp_ntc_9);
    fprintf(fp, "NTC Temp 10                : %" PRIi16 "\n", misnbec->paybcnpayload.temp_ntc_10);
    fprintf(fp, "NTC Temp 11                : %" PRIi16 "\n", misnbec->paybcnpayload.temp_ntc_11);

    /* ===================================================
    *  MISSION HOUSEKEEPING PAYLOAD
    * =================================================== */
    fprintf(fp, "\n[MISSION HOUSEKEEPING]\n");
    fprintf(fp, "HK CommandCounter          : %" PRIu8 "\n",
            misnbec->payhkpayload.CommandCounter);
    fprintf(fp, "HK CommandErrorCounter     : %" PRIu8 "\n",
            misnbec->payhkpayload.CommandErrorCounter);
    fprintf(fp, "HK System Status           : %" PRIi8 "\n",
            misnbec->payhkpayload.sys_status);

    fprintf(fp, "HK NTC Temp 0              : %" PRIi16 "\n", misnbec->payhkpayload.temp_ntc_0);
    fprintf(fp, "HK NTC Temp 1              : %" PRIi16 "\n", misnbec->payhkpayload.temp_ntc_1);
    fprintf(fp, "HK NTC Temp 2              : %" PRIi16 "\n", misnbec->payhkpayload.temp_ntc_2);
    fprintf(fp, "HK NTC Temp 3              : %" PRIi16 "\n", misnbec->payhkpayload.temp_ntc_3);
    fprintf(fp, "HK NTC Temp 4              : %" PRIi16 "\n", misnbec->payhkpayload.temp_ntc_4);
    fprintf(fp, "HK NTC Temp 5              : %" PRIi16 "\n", misnbec->payhkpayload.temp_ntc_5);
    fprintf(fp, "HK NTC Temp 6              : %" PRIi16 "\n", misnbec->payhkpayload.temp_ntc_6);
    fprintf(fp, "HK NTC Temp 7              : %" PRIi16 "\n", misnbec->payhkpayload.temp_ntc_7);
    fprintf(fp, "HK NTC Temp 8              : %" PRIi16 "\n", misnbec->payhkpayload.temp_ntc_8);
    fprintf(fp, "HK NTC Temp 9              : %" PRIi16 "\n", misnbec->payhkpayload.temp_ntc_9);
    fprintf(fp, "HK NTC Temp 10             : %" PRIi16 "\n", misnbec->payhkpayload.temp_ntc_10);
    fprintf(fp, "HK NTC Temp 11             : %" PRIi16 "\n", misnbec->payhkpayload.temp_ntc_11);

    fprintf(fp, "HK Sensor1 Data 0           : %" PRIu32 "\n",
            misnbec->payhkpayload.sen1_data_0);
    fprintf(fp, "HK Sensor1 Data 1           : %" PRIu32 "\n",
            misnbec->payhkpayload.sen1_data_1);





    // ---------------------------------------------------
    // BINARY DUMP
    // ---------------------------------------------------
    fprintf(fp, "\n[BINARY DATA]\n");
    for (size_t i = 0; i < sizeof(*misnbec); i++)
        fprintf(fp, "%02X ", ((unsigned char*)misnbec)[i]);
    fprintf(fp, "\n");

    fclose(fp);
    return 0;
}

static void DumpHex(FILE *fp, const uint8_t *p, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        fprintf(fp, "%02X ", p[i]);
        if ((i + 1) % 16 == 0) fprintf(fp, "\n");
    }
    if (n % 16 != 0) fprintf(fp, "\n");
}

static void DumpArr_i16(FILE *fp, const char *label, const int16_t *aa, int n)
{
    fprintf(fp, "%s: ", label);
    for (int i = 0; i < n; i++) fprintf(fp, "%" PRIi16 "%s", aa[i], (i == n - 1) ? "" : " ");
    fprintf(fp, "\n");
}

static void DumpArr_u16(FILE *fp, const char *label, const uint16_t *aa, int n)
{
    fprintf(fp, "%s: ", label);
    for (int i = 0; i < n; i++) fprintf(fp, "%" PRIu16 "%s", aa[i], (i == n - 1) ? "" : " ");
    fprintf(fp, "\n");
}

static void DumpArr_u8(FILE *fp, const char *label, const uint8_t *aa, int n)
{
    fprintf(fp, "%s: ", label);
    for (int i = 0; i < n; i++) fprintf(fp, "%" PRIu8 "%s", aa[i], (i == n - 1) ? "" : " ");
    fprintf(fp, "\n");
}

static void DumpArr_u32(FILE *fp, const char *label, const uint32_t *aa, int n)
{
    fprintf(fp, "%s: ", label);
    for (int i = 0; i < n; i++) fprintf(fp, "%" PRIu32 "%s", aa[i], (i == n - 1) ? "" : " ");
    fprintf(fp, "\n");
}

static void DumpReportPayloadParsed_ByMidCc(FILE *fp, const Report *rpt)
{
    uint16_t payload_len = rpt->RetValSize;
    if (payload_len > sizeof(rpt->RetVal)) payload_len = (uint16_t)sizeof(rpt->RetVal);

    fprintf(fp, "\n================= PARSED BY MID/CC =================\n");
    fprintf(fp, "ReflectedMID : 0x%04X\n", (unsigned)rpt->ReflectedMID);
    fprintf(fp, "ReflectedCC  : 0x%02X\n", (unsigned)rpt->ReflectedCC);
    fprintf(fp, "PayloadLen   : %" PRIu16 "\n", payload_len);


    const uint8_t *p = rpt->RetVal;

    if ((rpt->ReflectedMID == EPS_CMD_ID) ||(rpt->ReflectedMID == 0x7518))
    {
        switch (rpt->ReflectedCC)
        {

            default:
                fprintf(fp, "\n[EPS P60] Unknown CC: 0x%02X\n", (unsigned)rpt->ReflectedCC);
                break;
        }
    }
    else
    {
        fprintf(fp, "\n[UNKNOWN MID] No parser for ReflectedMID=0x%04X\n", (unsigned)rpt->ReflectedMID);
    }

    fprintf(fp, "\n[PAYLOAD HEX]\n");
    DumpHex(fp, rpt->RetVal, payload_len);
}




int ReportSaver(Report* rpt)
{
    if (!rpt) return -1;

    ReportCounter++;

    char filename[128];
    time_t tmtime = time(0);
    struct tm* local = localtime(&tmtime);

    sprintf(filename,
            "../data/report_parsed/Report--%04d-%02d-%02d-%02d-%02d-%02d--.txt",
            local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
            local->tm_hour, local->tm_min, local->tm_sec);

    FILE* fp = fopen(filename, "w");
    if (!fp) return -2;

    fprintf(fp, "================= REPORT SAVE =================\n");

    fprintf(fp, "\n[CCSDS HEADER]\n");
    fprintf(fp, "MsgId      : 0x%04X\n", (unsigned int)rpt->CCSDS_MsgId);
    fprintf(fp, "Seq        : 0x%04X\n", (unsigned int)rpt->CCSDS_Seq);
    fprintf(fp, "Len        : 0x%04X\n", (unsigned int)rpt->CCSDS_Len);
    fprintf(fp, "TimeCode   : %02X %02X %02X %02X %02X %02X\n",
            rpt->CCSDS_TimeCode[0], rpt->CCSDS_TimeCode[1],
            rpt->CCSDS_TimeCode[2], rpt->CCSDS_TimeCode[3],
            rpt->CCSDS_TimeCode[4], rpt->CCSDS_TimeCode[5]);
    fprintf(fp, "Padding    : 0x%08X\n", (unsigned int)rpt->CCSDS_Padding);

    fprintf(fp, "\n[REPORT BODY]\n");
    fprintf(fp, "Reflected MID  : 0x%04X\n", (unsigned int)rpt->ReflectedMID);
    fprintf(fp, "Reflected CC   : 0x%02X\n", (unsigned int)rpt->ReflectedCC);
    fprintf(fp, "RetType        : 0x%02X\n", (unsigned int)rpt->RetType);
    fprintf(fp, "RetCode        : %" PRId32 "\n", (int32_t)rpt->RetCode);
    fprintf(fp, "RetValSize     : %" PRIu16 "\n", (uint16_t)rpt->RetValSize);
    DumpReportPayloadParsed_ByMidCc(fp, rpt);
    fprintf(fp, "\n[RETURN VALUE]\n");
    uint16_t dump_size = rpt->RetValSize;
    if (dump_size > sizeof(rpt->RetVal))
        dump_size = sizeof(rpt->RetVal);

    for (uint16_t i = 0; i < dump_size; i++) {
        fprintf(fp, "%02X ", rpt->RetVal[i]);
        if ((i + 1) % 16 == 0)
            fprintf(fp, "\n");
    }
    if (dump_size % 16 != 0)
        fprintf(fp, "\n");

    fprintf(fp, "\n[BINARY DATA]\n");
    for (size_t i = 0; i < sizeof(*rpt); i++)
        fprintf(fp, "%02X ", ((unsigned char*)rpt)[i]);
    fprintf(fp, "\n");

    fclose(fp);
    return 0;
}

int EventSaver(Event *event)
{
    if (!event) return -1;

    char filename[128];
    time_t tmtime = time(0);
    struct tm *local = localtime(&tmtime);

    sprintf(filename,
            "../data/event_parsed/Event--%04d-%02d-%02d-%02d-%02d-%02d--.txt",
            local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
            local->tm_hour, local->tm_min, local->tm_sec);

    FILE *fp = fopen(filename, "w");
    if (!fp) return -2;

    fprintf(fp, "===================== EVENT SAVE =====================\n");

    fprintf(fp, "\n[CCSDS HEADER]\n");
    fprintf(fp, "MsgId      : 0x%04" PRIX16 "\n", (uint16_t)event->CCSDS_MsgId);
    fprintf(fp, "Seq        : 0x%04" PRIX16 "\n", (uint16_t)event->CCSDS_Seq);
    fprintf(fp, "Len        : 0x%04" PRIX16 "\n", (uint16_t)event->CCSDS_Len);
    fprintf(fp, "TimeCode   : %02" PRIX8 " %02" PRIX8 " %02" PRIX8 " %02" PRIX8 " %02" PRIX8 " %02" PRIX8 "\n",
            (uint8_t)event->CCSDS_TimeCode[0], (uint8_t)event->CCSDS_TimeCode[1],
            (uint8_t)event->CCSDS_TimeCode[2], (uint8_t)event->CCSDS_TimeCode[3],
            (uint8_t)event->CCSDS_TimeCode[4], (uint8_t)event->CCSDS_TimeCode[5]);
    fprintf(fp, "Padding    : 0x%08" PRIX32 "\n", (uint32_t)event->CCSDS_Padding);

    fprintf(fp, "\n[EVENT PAYLOAD]\n");
    fprintf(fp, "AppName    : %.*s\n", (int)sizeof(event->AppName), event->AppName);
    fprintf(fp, "EventID    : %" PRIu16 " (0x%04" PRIX16 ")\n", (uint16_t)event->EventID, (uint16_t)event->EventID);
    fprintf(fp, "EventType  : %" PRIu16 " (0x%04" PRIX16 ")\n", (uint16_t)event->EventType, (uint16_t)event->EventType);
    fprintf(fp, "SCID       : %" PRIu32 " (0x%08" PRIX32 ")\n", (uint32_t)event->SpacecraftID, (uint32_t)event->SpacecraftID);
    fprintf(fp, "ProcessorID: %" PRIu32 " (0x%08" PRIX32 ")\n", (uint32_t)event->ProcessorID, (uint32_t)event->ProcessorID);
    fprintf(fp, "Message    : %.*s\n", (int)sizeof(event->Message), event->Message);
    fprintf(fp, "Spare1     : %" PRIu8 " (0x%02" PRIX8 ")\n", (uint8_t)event->Spare1, (uint8_t)event->Spare1);
    fprintf(fp, "Spare2     : %" PRIu8 " (0x%02" PRIX8 ")\n", (uint8_t)event->Spare2, (uint8_t)event->Spare2);

    fprintf(fp, "\n[BINARY DATA]\n");
    for (size_t i = 0; i < sizeof(*event); i++)
        fprintf(fp, "%02X ", ((const unsigned char *)event)[i]);
    fprintf(fp, "\n");

    fclose(fp);
    return 0;
}

void * task_downlink_onorbit(void * socketinfo) 
{
    csp_socket_t * sock = (csp_socket_t *)socketinfo;

    csp_packet_t * packet = NULL;
    packetsign * confirm = (packetsign *)malloc(MIM_LEN_PACKET);
    csp_conn_t * conn = NULL;

    //////////////////////////////////////////////////////////////////////////////
    bool image_packet_received = false;

    std::string filename = "/home/miman/Downloads/FTP_TEST.jpg";
    std::vector<uint8_t> image_packet_data(14956);
    std::vector<int> received_index;
    int index;
    int count;
    std::ofstream fout;

    std::map<uint16_t, std::map<uint16_t, std::map<uint8_t, std::vector<uint8_t>>>> rpt_map;
    /////////////////////////////////////////////////////////////////////////

    float seconds = 0.0f;

    while (State.downlink_mode) {

        conn = csp_accept(sock, setup->default_timeout);
        if (conn == NULL) {
            seconds += 0.5f;
            continue;
        }

        while ((packet = csp_read(conn, setup->default_timeout)) != NULL) {

            const int dport = csp_conn_dport(conn);

            switch (dport) {

                case 23: {
                    if (packet->length == BEE_LEN_GETFILEINFO) {
                        char getfileinfofilename[128];
                        time_t tmtime = time(0);
                        struct tm *local = localtime(&tmtime);

                        sprintf(getfileinfofilename,
                                "../data/response/GETFILEINFO--%04d-%02d-%02d-%02d-%02d-%02d--",
                                local->tm_year + 1900,
                                local->tm_mon + 1,
                                local->tm_mday,
                                local->tm_hour,
                                local->tm_min,
                                local->tm_sec);

                        console.AddLog("Received GETFILEINFO Response from port : %d.\n", dport);

                        FILE *GETFILEINFO_fp = fopen(getfileinfofilename, "wb");
                        printf("Received GETFILEINFO response Length: %u", packet->length);

                        for (int i = 0; i < packet->length; i++) {
                            if (!(i % 10) && i != 0) {
                                printf("\n");
                                if (GETFILEINFO_fp) fprintf(GETFILEINFO_fp, "\n");
                            }
                            printf("0x%x ", packet->data[i]);
                            if (GETFILEINFO_fp) fprintf(GETFILEINFO_fp, "%02hhx\t", packet->data[i]);
                        }

                        memset(getfileinfo, 0, sizeof(*getfileinfo));
                        memcpy(getfileinfo, packet->data, BEE_LEN_GETFILEINFO);

                        if (GETFILEINFO_fp) fclose(GETFILEINFO_fp);
                    }

                    else {

                    char cosbcnfilename[128];
                    time_t tmtime = time(0);
                    struct tm *local = localtime(&tmtime);

                    sprintf(cosbcnfilename,
                            "../data/cosmic/beacon--%04d-%02d-%02d-%02d-%02d-%02d--",
                            local->tm_year + 1900,
                            local->tm_mon + 1,
                            local->tm_mday,
                            local->tm_hour,
                            local->tm_min,
                            local->tm_sec);

                    console.AddLog("!!!!!!!!!Received COSMIC Beacon from port : %d.!!!!!!!!!\n", dport);

                    FILE *cosbcn_fp = fopen(cosbcnfilename, "wb");
                    printf("\nCOSMIC Beacon Length: %u", packet->length);

                    for (int i = 0; i < packet->length; i++) {
                        if (!(i % 10) && i != 0) {
                            printf("\n");
                            if (cosbcn_fp) fprintf(cosbcn_fp, "\n");
                        }
                        printf("0x%x ", packet->data[i]);
                        if (cosbcn_fp) fprintf(cosbcn_fp, "%02hhx\t", packet->data[i]);
                    }

                    if (cosbcn_fp) fclose(cosbcn_fp);

                    printf("Beacon Packet Length: %u\n", packet->length);
                    printf("===== Beacon PACKET DUMP =====\n");
                    for (int i = 0; i < packet->length; i++) {
                        if (!(i % 10) && i != 0) printf("\n");
                        printf("0x%02X ", packet->data[i]);
                    }
                    printf("\n===============================\n");
                }
                    break;
                }

                case 24: {
                    char cosrptfilename[128];
                    time_t tmtime = time(0);
                    struct tm *local = localtime(&tmtime);

                    sprintf(cosrptfilename,
                            "../data/cosmic/report--%04d-%02d-%02d-%02d-%02d-%02d--",
                            local->tm_year + 1900,
                            local->tm_mon + 1,
                            local->tm_mday,
                            local->tm_hour,
                            local->tm_min,
                            local->tm_sec);

                    console.AddLog("!!!!!!!!!Received COSMIC Report from port : %d.!!!!!!!!!\n", dport);

                    FILE *cosrpt_fp = fopen(cosrptfilename, "wb");
                    printf("\nCOSMIC Report Length: %u", packet->length);

                    for (int i = 0; i < packet->length; i++) {
                        if (!(i % 10) && i != 0) {
                            printf("\n");
                            if (cosrpt_fp) fprintf(cosrpt_fp, "\n");
                        }
                        printf("0x%x ", packet->data[i]);
                        if (cosrpt_fp) fprintf(cosrpt_fp, "%02hhx\t", packet->data[i]);
                    }

                    if (cosrpt_fp) fclose(cosrpt_fp);

                    printf("Report Packet Length: %u\n", packet->length);
                    printf("===== Report PACKET DUMP =====\n");
                    for (int i = 0; i < packet->length; i++) {
                        if (!(i % 10) && i != 0) printf("\n");
                        printf("0x%02X ", packet->data[i]);
                    }
                    printf("\n===============================\n");
                    break;
                }

                case 27: {
                    if (packet->length == BEE_LEN_EVENT) {
                        char eventfilename[128];
                        time_t tmtime = time(0);
                        struct tm *local = localtime(&tmtime);

                        sprintf(eventfilename,
                                "../data/event/event--%04d-%02d-%02d-%02d-%02d-%02d--",
                                local->tm_year + 1900,
                                local->tm_mon + 1,
                                local->tm_mday,
                                local->tm_hour,
                                local->tm_min,
                                local->tm_sec);

                        console.AddLog("Received Event from port : %d.\n", dport);

                        FILE *evnt_fp = fopen(eventfilename, "wb");
                        printf("\nEvent Length: %u", packet->length);

                        for (int i = 0; i < packet->length; i++) {
                            if (!(i % 10) && i != 0) {
                                printf("\n");
                                if (evnt_fp) fprintf(evnt_fp, "\n");
                            }
                            printf("0x%x ", packet->data[i]);
                            if (evnt_fp) fprintf(evnt_fp, "%02hhx\t", packet->data[i]);
                        }

                        memset(event, 0, sizeof(*event));
                        memcpy(event, packet->data, BEE_LEN_EVENT);
                        EventSaver(event);

                        if (evnt_fp) fclose(evnt_fp);
                    }

                    printf("Event Packet Length: %u\n", packet->length);
                    printf("===== Event PACKET DUMP =====\n");
                    for (int i = 0; i < packet->length; i++) {
                        if (!(i % 10) && i != 0) printf("\n");
                        printf("0x%02X ", packet->data[i]);
                    }
                    printf("\n===============================\n");
                    break;
                }

                case 25: {
                    FILE *rpt_fp = NULL;

                    char rptpktfilename[128];
                    time_t tmtime = time(0);
                    struct tm *local = localtime(&tmtime);

                    sprintf(rptpktfilename,
                            "../data/report/rpt_raw--%04d-%02d-%02d-%02d-%02d-%02d--",
                            local->tm_year + 1900,
                            local->tm_mon + 1,
                            local->tm_mday,
                            local->tm_hour,
                            local->tm_min,
                            local->tm_sec);

                    console.AddLog("Received Report from port : %d.", dport);
                    rpt_fp = fopen(rptpktfilename, "wb");

                    const time_t now = time(NULL);
                    const size_t chunk_len = packet->length;

                    printf("case25: chunk_len=%zu\n", chunk_len);

                    // ===== RAW DUMP (terminal + file) =====
                    printf("===== REPORT RAW CHUNK DUMP (%zu bytes) =====\n", chunk_len);
                    if (rpt_fp) fprintf(rpt_fp, "===== REPORT RAW CHUNK DUMP (%zu bytes) =====\n", chunk_len);

                    for (size_t i = 0; i < chunk_len; i++) {
                        printf("%02X ", packet->data[i]);
                        if (rpt_fp) fprintf(rpt_fp, "%02hhx ", packet->data[i]);

                        if ((i + 1) % 16 == 0) {
                            printf("\n");
                            if (rpt_fp) fprintf(rpt_fp, "\n");
                        }
                    }

                    if (chunk_len % 16 != 0) {
                        printf("\n");
                        if (rpt_fp) fprintf(rpt_fp, "\n");
                    }

                    printf("===========================================\n");
                    if (rpt_fp) fprintf(rpt_fp, "===========================================\n");
                    // ===== END RAW DUMP =====

                    if (g_report_collecting && g_report_have_last_time) {
                        double dt = difftime(now, g_report_last_chunk_time);
                        if (dt > 5.0) {
                            printf("case25: timeout dt=%.1f sec (>5). drop current report assembler.\n", dt);

                            g_report_collecting = false;
                            g_report_off = 0;
                            memset(g_report_wire, 0, sizeof(g_report_wire));

                            g_report_have_last_time = false;
                            g_report_last_chunk_time = 0;
                        }
                    }

                    if (!g_report_collecting) {
                        g_report_collecting = true;
                        g_report_off = 0;
                        memset(g_report_wire, 0, sizeof(g_report_wire));
                    }

                    g_report_last_chunk_time = now;
                    g_report_have_last_time = true;

                    if (g_report_off > REPORT_WIRE_SIZE) {
                        g_report_collecting = false;
                        g_report_off = 0;
                        memset(g_report_wire, 0, sizeof(g_report_wire));
                        g_report_have_last_time = false;
                        g_report_last_chunk_time = 0;
                    }

                    size_t remain = REPORT_WIRE_SIZE - g_report_off;

                    if (chunk_len > remain) {
                        printf("case25: overflow (off=%zu, chunk=%zu). drop current and restart with this chunk.\n",
                            g_report_off, chunk_len);

                        g_report_collecting = true;
                        g_report_off = 0;
                        memset(g_report_wire, 0, sizeof(g_report_wire));
                        remain = REPORT_WIRE_SIZE;
                    }

                    if (chunk_len <= remain) {
                        memcpy(g_report_wire + g_report_off, packet->data, chunk_len);
                        g_report_off += chunk_len;

                        printf("case25: assembled %zu/%zu\n", g_report_off, (size_t)REPORT_WIRE_SIZE);

                        if (g_report_off == REPORT_WIRE_SIZE) {
                            Report rpt;
                            bool ok = ParseReportWire540(g_report_wire, REPORT_WIRE_SIZE, rpt);

                            if (ok) {
                                printf("\ncase25: REPORT COMPLETE \nMsgId=0x%04x \nRefMID=0x%04x \nCC=0x%02x \nRetSize=%u\n",
                                    rpt.CCSDS_MsgId, rpt.ReflectedMID, rpt.ReflectedCC, (unsigned)rpt.RetValSize);
                                UpdateReportViewFromReport(rpt);
                                ReportSaver(&rpt);
                            } else {
                                printf("case25: ParseReportWire540 failed\n");
                            }

                            g_report_collecting = false;
                            g_report_off = 0;
                            memset(g_report_wire, 0, sizeof(g_report_wire));
                            g_report_have_last_time = false;
                            g_report_last_chunk_time = 0;
                        }
                    } else {
                        printf("case25: chunk too big to fit even after restart (%zu)\n", chunk_len);
                    }

                    if (rpt_fp) fclose(rpt_fp);
                    break;
                }


                case 13: {
                    if (packet->length == MIM_LEN_BEACON) {
                        char bcnpktfilename[128];
                        time_t tmtime = time(0);
                        struct tm *local = localtime(&tmtime);

                        sprintf(bcnpktfilename,
                                "../data/bcnpkt/bcnpktp13--%04d-%02d-%02d-%02d-%02d-%02d--",
                                local->tm_year + 1900,
                                local->tm_mon + 1,
                                local->tm_mday,
                                local->tm_hour,
                                local->tm_min,
                                local->tm_sec);

                        console.AddLog("Received Beacon from port : %d.", dport);

                        FILE *bcn_fp = fopen(bcnpktfilename, "wb");
                        printf("\nBeacon Length: %u", packet->length);

                        for (int i = 0; i < packet->length; i++) {
                            if (!(i % 10) && i != 0) {
                                printf("\n");
                                if (bcn_fp) fprintf(bcn_fp, "\n");
                            }
                            printf("0x%x ", packet->data[i]);
                            if (bcn_fp) fprintf(bcn_fp, "%02hhx\t", packet->data[i]);
                        }

                        memset(beacon, 0, sizeof(*beacon));
                        memcpy(beacon, packet->data, MIM_LEN_BEACON);
                        BeaconSaver(beacon);

                        if (bcn_fp) fclose(bcn_fp);
                    }
                    else if (packet->length == BEE_LEN_MISSIONBEACON) {
                        char misnbcnpktfilename[128];
                        time_t tmtime = time(0);
                        struct tm *local = localtime(&tmtime);

                        sprintf(misnbcnpktfilename,
                                "../data/bcnpkt/misnbcnpkt--%04d-%02d-%02d-%02d-%02d-%02d--",
                                local->tm_year + 1900,
                                local->tm_mon + 1,
                                local->tm_mday,
                                local->tm_hour,
                                local->tm_min,
                                local->tm_sec);

                        console.AddLog("!!!!!!!!!Received Mission Beacon from port : %d.!!!!!!!!!", dport);

                        FILE *misnbcn_fp = fopen(misnbcnpktfilename, "wb");
                        printf("\nMission Beacon Length: %u", packet->length);

                        for (int i = 0; i < packet->length; i++) {
                            if (!(i % 10) && i != 0) {
                                printf("\n");
                                if (misnbcn_fp) fprintf(misnbcn_fp, "\n");
                            }
                            printf("0x%x ", packet->data[i]);
                            if (misnbcn_fp) fprintf(misnbcn_fp, "%02hhx\t", packet->data[i]);
                        }

                        memset(missionbeacon, 0, sizeof(*missionbeacon));
                        memcpy(missionbeacon, packet->data, BEE_LEN_MISSIONBEACON);
                        MissionBeaconSaver(missionbeacon);

                        if (misnbcn_fp) fclose(misnbcn_fp);
                    }
                    else if (packet->length == BEE_LEN_GETFILEINFO) {
                        char getfileinfofilename[128];
                        time_t tmtime = time(0);
                        struct tm *local = localtime(&tmtime);

                        sprintf(getfileinfofilename,
                                "../data/response/GETFILEINFO--%04d-%02d-%02d-%02d-%02d-%02d--",
                                local->tm_year + 1900,
                                local->tm_mon + 1,
                                local->tm_mday,
                                local->tm_hour,
                                local->tm_min,
                                local->tm_sec);

                        console.AddLog("Received GETFILEINFO Response from port : %d.", dport);

                        FILE *GETFILEINFO_fp = fopen(getfileinfofilename, "wb");
                        printf("Received GETFILEINFO response Length: %u", packet->length);

                        for (int i = 0; i < packet->length; i++) {
                            if (!(i % 10) && i != 0) {
                                printf("\n");
                                if (GETFILEINFO_fp) fprintf(GETFILEINFO_fp, "\n");
                            }
                            printf("0x%x ", packet->data[i]);
                            if (GETFILEINFO_fp) fprintf(GETFILEINFO_fp, "%02hhx\t", packet->data[i]);
                        }

                        memset(getfileinfo, 0, sizeof(*getfileinfo));
                        memcpy(getfileinfo, packet->data, BEE_LEN_GETFILEINFO);

                        if (GETFILEINFO_fp) fclose(GETFILEINFO_fp);
                    }
                    else {
                        console.AddLog("Received Something but brocken.");

                        char unknownfilename[128];
                        time_t tmtime = time(0);
                        struct tm *local = localtime(&tmtime);

                        sprintf(unknownfilename,
                                "../data/unknown/port13/unknown_--%04d-%02d-%02d-%02d-%02d-%02d--",
                                local->tm_year + 1900,
                                local->tm_mon + 1,
                                local->tm_mday,
                                local->tm_hour,
                                local->tm_min,
                                local->tm_sec);

                        FILE *unk_fp = fopen(unknownfilename, "wb");

                        printf("Unknown Packet Length: %u\n", packet->length);
                        printf("===== UNKNOWN PACKET DUMP =====\n");
                        for (int i = 0; i < packet->length; i++) {
                            if (!(i % 10) && i != 0) printf("\n");
                            printf("0x%02X ", packet->data[i]);

                            if (unk_fp) {
                                if (!(i % 10) && i != 0) fprintf(unk_fp, "\n");
                                fprintf(unk_fp, "%02hhx\t", packet->data[i]);
                            }
                        }
                        printf("\n===============================\n");

                        if (unk_fp) fclose(unk_fp);
                    }

                    break;
                }

                case 31: {
                    if (packet->length == MIM_LEN_BEACON) {
                        char bcnpktfilename[128];
                        time_t tmtime = time(0);
                        struct tm *local = localtime(&tmtime);

                        sprintf(bcnpktfilename,
                                "../data/bcnpkt/bcnpktp13--%04d-%02d-%02d-%02d-%02d-%02d--",
                                local->tm_year + 1900,
                                local->tm_mon + 1,
                                local->tm_mday,
                                local->tm_hour,
                                local->tm_min,
                                local->tm_sec);

                        console.AddLog("Received Beacon from port : %d.\n", dport);

                        FILE *bcn_fp = fopen(bcnpktfilename, "wb");
                        printf("\nBeacon Length: %u", packet->length);

                        for (int i = 0; i < packet->length; i++) {
                            if (!(i % 10) && i != 0) {
                                printf("\n");
                                if (bcn_fp) fprintf(bcn_fp, "\n");
                            }
                            printf("0x%x ", packet->data[i]);
                            if (bcn_fp) fprintf(bcn_fp, "%02hhx\t", packet->data[i]);
                        }

                        memset(beacon, 0, sizeof(*beacon));
                        memcpy(beacon, packet->data, MIM_LEN_BEACON);
                        BeaconSaver(beacon);

                        if (bcn_fp) fclose(bcn_fp);
                    }
                    else if (packet->length == BEE_LEN_MISSIONBEACON) {
                        char misnbcnpktfilename[128];
                        time_t tmtime = time(0);
                        struct tm *local = localtime(&tmtime);

                        sprintf(misnbcnpktfilename,
                                "../data/bcnpkt/misnbcnpkt--%04d-%02d-%02d-%02d-%02d-%02d--",
                                local->tm_year + 1900,
                                local->tm_mon + 1,
                                local->tm_mday,
                                local->tm_hour,
                                local->tm_min,
                                local->tm_sec);

                        console.AddLog("!!!!!!!!!Received Mission Beacon from port : %d.!!!!!!!!!", dport);

                        FILE *misnbcn_fp = fopen(misnbcnpktfilename, "wb");
                        printf("\nMission Beacon Length: %u\n", packet->length);

                        for (int i = 0; i < packet->length; i++) {
                            if (!(i % 10) && i != 0) {
                                printf("\n");
                                if (misnbcn_fp) fprintf(misnbcn_fp, "\n");
                            }
                            printf("0x%x ", packet->data[i]);
                            if (misnbcn_fp) fprintf(misnbcn_fp, "%02hhx\t", packet->data[i]);
                        }

                        memset(missionbeacon, 0, sizeof(*missionbeacon));
                        memcpy(missionbeacon, packet->data, BEE_LEN_MISSIONBEACON);
                        MissionBeaconSaver(missionbeacon);

                        if (misnbcn_fp) fclose(misnbcn_fp);
                    }
                    else {
                        console.AddLog("Received Beacon but brocken.");

                        char unknownfilename[128];
                        time_t tmtime = time(0);
                        struct tm *local = localtime(&tmtime);

                        sprintf(unknownfilename,
                                "../data/unknown/port31/unknown--%04d-%02d-%02d-%02d-%02d-%02d--",
                                local->tm_year + 1900,
                                local->tm_mon + 1,
                                local->tm_mday,
                                local->tm_hour,
                                local->tm_min,
                                local->tm_sec);

                        FILE *unk_fp = fopen(unknownfilename, "wb");

                        printf("Unknown Packet Length: %u\n", packet->length);
                        printf("===== UNKNOWN PACKET DUMP =====\n");
                        for (int i = 0; i < packet->length; i++) {
                            if (!(i % 10) && i != 0) printf("\n");
                            printf("0x%02X ", packet->data[i]);

                            if (unk_fp) {
                                if (!(i % 10) && i != 0) fprintf(unk_fp, "\n");
                                fprintf(unk_fp, "%02hhx\t", packet->data[i]);
                            }
                        }
                        printf("\n===============================\n");

                        if (unk_fp) fclose(unk_fp);
                    }
                    break;
                }

                case 12: {
                    char bcnpktfilename[128];
                    time_t tmtime = time(0);
                    struct tm *local = localtime(&tmtime);

                    sprintf(bcnpktfilename,
                            "../data/bcnpkt/bcnpktp13--%04d-%02d-%02d-%02d-%02d-%02d--",
                            local->tm_year + 1900,
                            local->tm_mon + 1,
                            local->tm_mday,
                            local->tm_hour,
                            local->tm_min,
                            local->tm_sec);

                    console.AddLog("Received Beacon from port : %d.", dport);

                    FILE *bcn_fp = fopen(bcnpktfilename, "wb");
                    printf("Beacon Length: %u", packet->length);

                    for (int i = 0; i < packet->length; i++) {
                        if (!(i % 10) && i != 0) {
                            printf("\n");
                            if (bcn_fp) fprintf(bcn_fp, "\n");
                        }
                        printf("0x%x ", packet->data[i]);
                        if (bcn_fp) fprintf(bcn_fp, "%02hhx\t", packet->data[i]);
                    }

                    if (packet->length == MIM_LEN_BEACON) {
                        memcpy(beacon, packet->data, MIM_LEN_BEACON);
                    } else {
                        console.AddLog("Received Beacon but brocken.");
                    }

                    if (bcn_fp) fclose(bcn_fp);
                    break;
                }

                default: {
                    if (dport == 1) {
                        csp_service_handler(conn, packet);
                    } else {
                        console.AddLog("Packet Received on unknown port %d", dport);
                        console.AddLog("Packet length is %d", packet->length);

                        char unknownfilename[128];
                        time_t tmtime = time(0);
                        struct tm *local = localtime(&tmtime);

                        sprintf(unknownfilename,
                                "../data/unknown/unknown--%04d-%02d-%02d-%02d-%02d-%02d--",
                                local->tm_year + 1900,
                                local->tm_mon + 1,
                                local->tm_mday,
                                local->tm_hour,
                                local->tm_min,
                                local->tm_sec);

                        uint16_t PacketLength = packet->length;
                        memcpy(confirm, packet->data, PacketLength);

                        FILE *TMTC_fp = fopen(unknownfilename, "wb");
                        if (TMTC_fp) {
                            for (int i = 0; i < PacketLength; i++) {
                                fprintf(TMTC_fp, "Data %d: %u\n", i, packet->data[i]);
                            }
                            fclose(TMTC_fp);
                        }
                    }
                    break;
                }
            }

            csp_buffer_free(packet);
            packet = NULL;
        }

        csp_close(conn);
        conn = NULL;

        RSSI_Monitoring();
    }

    printf("Downlink thread dead.\n");

    if (confirm) {
        free(confirm);
        confirm = NULL;
    }

    return NULL;
}





void * task_uplink_onorbit(void * sign_)
{

    State.uplink_mode = true;
    //State.downlink_mode = false;
    pthread_mutex_lock(&conn_lock);
    //while(!State.RotatorReadReady)
    //    continue;
    bool dlstate = true;


    //This funcion must be on p_thread[4]
    if((dlstate))
    {
        State.downlink_mode = false;
    }
        
    while(!State.uplink_mode)
        continue;
    uint32_t start, time = 0;
    start = csp_get_ms();
    packetsign * sign;
    csp_packet_t * packet;
    csp_conn_t* txconn;
    console.AddLog("[DEBUG]## Start Uplink Task.");

    sign = (packetsign *) sign_;
    // for(int i =0; i < sign->Length; i++)
    //     printf("PacketByte : %d", sign->Data[i]);
    //packet = PacketEncoder(sign);

    

    uint16_t Ptype = sign->PacketType;
    uint32_t Plen = sign->Length;
    uint16_t filetype;
    uint16_t filestatus;

    if(Ptype == MIM_PT_DLREQ)
    {
        filetype = ((dlreqdata *)(sign->Data))->target;
        filestatus = ((dlreqdata *)(sign->Data))->filestatus;
    }
    
    
    if(State.Debugmode)
    {
        for(int i = 0 ; i < sign->Length; i++)
            printf("%x\t", sign->Data[i]);
        printf("\n");
    }
    uint16_t tx_length = (uint16_t)(Plen + MIM_HAND_DATAFDSTART + 4);
    packet = PacketEncoder(sign);
    console.AddLog("[DEBUG]##Send Packet of Ptype : %u. Check Terminal Log.", Ptype);
    

    switch(Ptype){
        //Ptype 1 : Ping
        case MIM_PT_PING : {
            csp_packet_t * confirm_ = (csp_packet_t *)csp_buffer_get(MIM_LEN_PACKET);
            
            while(State.uplink_mode)
            {
                if ((txconn = csp_connect(CSP_PRIO_HIGH, setup->obc_node, TX_PORT, MIM_DEFAULT_TIMEOUT, 0)) == NULL)
                    continue;
                else
                    break;
            }
            while (State.uplink_mode && txconn != NULL)
            {
                if(csp_send(txconn, packet, setup->default_timeout))
                {
                    packet = NULL;
                    break;
                }
                else
                    continue;
            }
            if(txconn == NULL)
            {
                console.AddLog("[ERROR]##Connection Buffer Busy. Skip this command.");
                if(confirm_!=NULL)
                {
                    csp_buffer_free(confirm_);
                    confirm_ = NULL;
                }
                break;
            }
            if((confirm_ = csp_read(txconn, rx_delay_ms(Plen, setup->ax100_node))) != NULL && State.uplink_mode)
            {
                packetsign * confirm = PacketDecoder(confirm_);
                if(State.Debugmode)
                {
                    for(int i = 0; i < confirm->Length; i++)
                        printf("%d\t", confirm->Data[i]);
                    printf("\n");
                }
                

                uint16_t * retcode = (uint16_t *)&confirm->Data[0];
                uint16_t * pingcount = (uint16_t *)&confirm->Data[2];

                if(*retcode == 0)
                {
                    console.AddLog("[OK]##[OBC]Ping Success. retcode : %u, pingcount : %u", *retcode, *pingcount);
                    PingCounter ++;
                }
                else
                    console.AddLog("[ERROR]##[OBC]Ping Received but brocken. retcode : %u, pingcount : %u", *retcode, *pingcount);
                if(confirm != NULL)
                {
                    free(confirm);
                    confirm = NULL;
                }
            }
            else
                console.AddLog("[ERROR]##[OBC]Ping Failed.");
            
            if(confirm_!=NULL)
            {
                csp_buffer_free(confirm_);
                confirm_ = NULL;
            }
            break;
        }
        //Ptype 2 : Uplink Signal
        case MIM_PT_SIGNAL : {
            if(State.Debugmode)
            {
                for(int i = 0; i < packet->length; i++)
                {
                    printf("%x", packet[i]);
                }
                printf("\n");
            }
            
            while(State.uplink_mode)
            {
                if ((txconn = csp_connect(CSP_PRIO_HIGH, setup->obc_node, TX_PORT, MIM_DEFAULT_TIMEOUT, 0)) == NULL)
                    continue;
                else
                    break;
            }
            while (State.uplink_mode && txconn != NULL)
            {
                if(csp_send(txconn, packet, setup->default_timeout))
                {
                    packet = NULL;
                    break;
                }
                else
                    continue;
            }
            if(txconn == NULL)
            {
                console.AddLog("[ERROR]##Connection Buffer Busy. Skip this command.");
                break;
            }
            console.AddLog("Uplink Signal TX Done.");
            //Ping Timeout : 10 sec
            break;
        }
        case MIM_PT_DLREQ : {
            csp_packet_t * confirm_ = (csp_packet_t *)csp_buffer_get(MIM_LEN_PACKET);
            while(State.uplink_mode)
            {
                if ((txconn = csp_connect(CSP_PRIO_HIGH, setup->obc_node, TX_PORT, MIM_DEFAULT_TIMEOUT, 0)) == NULL)
                    continue;
                else
                    break;
            }
            while (State.uplink_mode && txconn != NULL)
            {
                if(csp_send(txconn, packet, setup->default_timeout))
                {
                    packet = NULL;
                    break;
                }
                else
                    continue;
            }
            if(txconn == NULL)
            {
                console.AddLog("[ERROR]##Connection Buffer Busy. Skip this command.");
                if(confirm_!=NULL)
                {
                    csp_buffer_free(confirm_);
                    confirm_ = NULL;
                }
                break;
            }
            console.AddLog("Request FTP filelist.");
            if((confirm_ = csp_read(txconn, rx_delay_ms(Plen, setup->ax100_node))) != NULL && State.uplink_mode)
            {
                filelist * confirm = (filelist *)malloc(confirm_->length);
                packetsign* confirmlist =  PacketDecoder(confirm_);
                memcpy(confirm, confirmlist->Data, confirmlist->Length);
                if(confirm->filenum > 0)
                {
                    for(int i = 0; i < 64; i++)
                    {
                        memset(&State.ftplistup[i], 0, sizeof(ftpinfo));
                    }
                    console.AddLog("[OK]##Received File list.");
                    for(int i = 0; i < 64; i++)
                    {
                        
                        if(i < confirm->filenum)
                        {
                            console.AddLog("[DEBUG]##Make Lists : %d\tFileName: %u", i, confirm->file[i]);
                            memcpy(&State.ftplistup[i], filelisthandler(confirm, filetype, filestatus, i), sizeof(ftpinfo));
                        }
                        else
                            continue;
                    }
                }
                else if(confirm->filenum == 0)
                    console.AddLog("[ERROR]##No more files.");
                else   
                    console.AddLog("[ERROR]##IO Error.");
                if(confirm!=NULL)
                {
                    free(confirm);
                    confirm = NULL;
                    sign = NULL;
                }
                    
            }
            else
            {
                console.AddLog("[ERROR]##Cannot Receive File List.");
            }
            //Ping Timeout : 10 sec
            if(confirm_!=NULL)
            {
                csp_buffer_free(confirm_);
                confirm_ = NULL;
            }
            break;
        }
        case MIM_PT_CMD : { //according to Ptype
            csp_packet_t * confirm_ = (csp_packet_t *)csp_buffer_get(MIM_LEN_PACKET);
            while(State.uplink_mode)
            {
                if ((txconn = csp_connect(CSP_PRIO_HIGH, setup->obc_node, TX_PORT, MIM_DEFAULT_TIMEOUT, 0)) == NULL)
                    continue;
                else
                    break;
            }
            while (State.uplink_mode && txconn != NULL)
            {
                if(csp_send(txconn, packet, setup->default_timeout))
                {
                    packet = NULL;
                    break;
                }
                else
                    continue;
            }
            if(txconn == NULL)
            {
                console.AddLog("[ERROR]##Connection Buffer Busy. Skip this command.");
                if(confirm_!=NULL)
                {
                    csp_buffer_free(confirm_);
                    confirm_ = NULL;
                }
                break;
            }
            if((confirm_ = csp_read(txconn, rx_delay_ms(Plen, setup->ax100_node))) != NULL)
            {
                packetsign* confirmlist =  PacketDecoder(confirm_);
                uint8_t * retcode = (uint8_t *)&confirmlist->Data[0];
                console.AddLog("[OK]Received Command Reply. Retcode %d CMD Count %d", *retcode, State.CMDCount);
                State.CMDCount ++;
                if(confirmlist!=NULL)
                {
                    free(confirmlist);
                    confirmlist = NULL;
                    sign = NULL;
                } 
                if(confirm_!=NULL)
                {
                    csp_buffer_free(confirm_);
                    confirm_ = NULL;
                }
            }
            else
                console.AddLog("[ERROR]##No Command Reply.");
            break;
        }
        case MIM_PT_TMTC_TEST : { // == 10. Add for TMTC Test
            csp_packet_t * confirm_ = (csp_packet_t *)csp_buffer_get(MIM_LEN_PACKET);
            while(State.uplink_mode)
            {
                if(State.Scheduled)
                {
                    if ((txconn = csp_connect(CSP_PRIO_HIGH, 3, 13, MIM_DEFAULT_TIMEOUT, 0)) == NULL) {
                    /*!!!!!!!!!!!Revise setup->obc_node!!!!!!!!!!*/ //-> Change to 28.
                    /*!!!!!!!!!!!!Need to revise Port!!!!!!!!!!!*/
                        continue;
                    }
                    else
                        break;
                }
                else
                {
                    if ((txconn = csp_connect(CSP_PRIO_HIGH, 3, 13, MIM_DEFAULT_TIMEOUT, 0)) == NULL) {
                    /*!!!!!!!!!!!Revise setup->obc_node!!!!!!!!!!*/ //-> Change to 28.
                    /*!!!!!!!!!!!!Need to revise Port!!!!!!!!!!!*/
                        continue;
                    }
                    else
                        break;
                }

            }
            while (State.uplink_mode && txconn != NULL)
            {
                console.AddLog("[OK]CMD Packet Header: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                    packet->data[0],packet->data[1],packet->data[2],packet->data[3],packet->data[4],packet->data[5],packet->data[6],packet->data[7]);
                fprintf(log_ptr, "|| Uplink || Length: %d\n",packet->length);
                    for(int i=0; i<packet->length; i++) {
                        if(!(i%10) && i !=0) {
                            fprintf(log_ptr, "\n");
                        }
                        fprintf(log_ptr, "%02hhx\t",packet->data[i]);
                    } fprintf(log_ptr,"\n\n");
                if(csp_send(txconn, packet, setup->default_timeout)) // Success. then,
                {   
                    packet = NULL; // discard packet and,
                    break; // End process.
                }
                else // Fail. then,
                    continue; //Go to loof, and try again.
            }
            if(txconn == NULL)
            {
                console.AddLog("[ERROR]##Connection Buffer Busy. Skip this command.");
                if(confirm_!=NULL)
                {
                    csp_buffer_free(confirm_);
                    confirm_ = NULL;
                }
                break;
            }
            // if((confirm_ = csp_read(txconn, rx_delay_ms(Plen, setup->ax100_node))) != NULL)
            // {
            //     packetsign* confirmlist =  PacketDecoder(confirm_);
            //     uint8_t * retcode = (uint8_t *)&confirmlist->Data[0];
            //     console.AddLog("[OK]Received Command Reply. Retcode %d CMD Count %d", *retcode, State.CMDCount);
            //     State.CMDCount ++;
            //     if(confirmlist!=NULL)
            //     {
            //         free(confirmlist);
            //         confirmlist = NULL;
            //         sign = NULL;
            //     } 
            //     if(confirm_!=NULL)
            //     {
            //         csp_buffer_free(confirm_);
            //         confirm_ = NULL;
            //     }
            // }
            // else
            //     console.AddLog("[ERROR]##No Command Reply.");
            break;
        }
        case MIM_PT_STCMD : {
            csp_packet_t * confirm_ = (csp_packet_t *)csp_buffer_get(MIM_LEN_PACKET);
            while(State.uplink_mode)
            {
                if ((txconn = csp_connect(CSP_PRIO_HIGH, setup->obc_node, TX_PORT, MIM_DEFAULT_TIMEOUT, 0)) == NULL)
                    continue;
                else
                    break;
            }
            while (State.uplink_mode && txconn != NULL)
            {
                if(csp_send(txconn, packet, setup->default_timeout))
                {
                    packet = NULL;
                    break;
                }
                else
                    continue;
            }
            if(txconn == NULL)
            {
                console.AddLog("[ERROR]##Connection Buffer Busy. Skip this command.");
                if(confirm_!=NULL)
                {
                    csp_buffer_free(confirm_);
                    confirm_ = NULL;
                }
                break;
            }
            if((confirm_ = csp_read(txconn, rx_delay_ms(Plen, setup->ax100_node))) != NULL && State.uplink_mode)
            {
                if(State.uplink_mode)
                {
                    packetsign* confirmlist =  PacketDecoder(confirm_);
                    uint8_t * retcode = (uint8_t *)&confirmlist->Data[0];
                    console.AddLog("[OK]##Get Command Reply. Retcode : %d", *retcode);
                    if(confirmlist!=NULL)
                    {
                        free(confirmlist);
                        confirmlist = NULL;
                        sign = NULL;
                    } 
                    if(confirm_!=NULL)
                    {
                        csp_buffer_free(confirm_);
                        confirm_ = NULL;
                    }
                }
            }
            else
                console.AddLog("[ERROR]##Cannot Receive Command Reply.");
            break;
        }
        case MIM_PT_NCCMD : {
            csp_packet_t * confirm_ = (csp_packet_t *)csp_buffer_get(MIM_LEN_PACKET);
            packetsign* confirmlist =  PacketDecoder(confirm_);
            while(State.uplink_mode)
            {
                if ((txconn = csp_connect(CSP_PRIO_HIGH, setup->obc_node, TX_PORT, MIM_DEFAULT_TIMEOUT, 0)) == NULL)
                    continue;
                else
                    break;
            }
            while (State.uplink_mode && txconn != NULL)
            {
                if(csp_send(txconn, packet, setup->default_timeout))
                {
                    packet = NULL;
                    break;
                }
                else
                    continue;
            }
            if(txconn == NULL)
            {
                console.AddLog("[ERROR]##Connection Buffer Busy. Skip this command.");
                if(confirm_!=NULL)
                {
                    csp_buffer_free(confirm_);
                    confirm_ = NULL;
                }
                break;
            }
            if((confirm_ = csp_read(txconn,rx_delay_ms(Plen, setup->ax100_node))) != NULL)
            {
                if(State.uplink_mode)
                {
                    console.AddLog("[OK]##Get Command Reply. No Checksum.");
                    if(confirmlist!=NULL)
                    {
                        free(confirmlist);
                        confirmlist = NULL;
                        sign = NULL;
                    } 
                    if(confirm_!=NULL)
                    {
                        csp_buffer_free(confirm_);
                        confirm_ = NULL;
                    }
                }
            }
            else
                console.AddLog("[ERROR]##Cannot Receive Command Reply.");
            break;
        }
        default : {
            console.AddLog("[ERROR]##Undefied type of uplink packetsign. Type : %"PRIu16, Ptype);
            break;
        }
    }
    
    if(txconn != NULL)
    {
        csp_close(txconn);
        txconn = NULL;
    }
    
    if(packet!=NULL)
    {
        csp_buffer_free(packet);
        packet = NULL;
    }
    time = csp_get_ms() - start;
    console.AddLog("[DEBUG]##Communication Time : %u", time);

    State.uplink_mode = false;
    if(dlstate)
    {
        State.downlink_mode = dlstate;
    }
    State.downlink_mode = true;
    pthread_mutex_unlock(&conn_lock);
}

int PacketHandler(csp_packet_t *packet, int type, int NowCursor)
{
    while(1) {
        switch(type) {
            case 0: {
                type = packet->data[NowCursor];
                NowCursor ++;
                return PacketHandler(packet, type, NowCursor);
            }
            case 1: {
                // Declare NowHK as a pointer to HK struct
                // HK *NowHK; 
                for (HKbufCursor; HKbufCursor < sizeof(HKbuf); HKbufCursor ++)
                {
                    HKbuf[HKbufCursor] = packet->data[NowCursor];
                    NowCursor ++;
                    if (NowCursor > MIM_LEN_PACKET)
                    {
                        //Read All Packet
                        //Need to go next packet
                        NowCursor = 0;
                        return 0;
                    }
                }
                // NowHK = (HK *)HKbuf;
                buf_allclear();
                return NowCursor;
            }
            case 3: {
                // for (AODbufCursor; AODbufCursor < sizeof(AODbuf); AODbufCursor ++)
                // {
                //     AODbuf[AODbufCursor] = packet->data[NowCursor];
                //     NowCursor ++;
                //     if (NowCursor > MIM_LEN_PACKET)
                //     {
                //         //Read All Packet
                //         //Need to go next packet
                //         NowCursor = 0;
                //         return 0;
                //     }
                // }
                // NowAOD = (AOD *)HKbuf;
                // buf_allclear();
                // return NowCursor;
            }
            default :{
                
            }
        }
    }
}

packetsign * PingInit(FSWTle * FSWTleinfo)
{
    packetsign * sign;
    sign = (packetsign *)malloc(sizeof(FSWTle) + MIM_HAND_DATAFDSTART);
    sign->Identifier = MIM_ID;
    sign->PacketType = MIM_PT_PING;
    sign->Length = sizeof(FSWTle);
    // printf("Start. Length : %u, Second : %u\n", sign->Length, FSWTleinfo->sec);
    memcpy(sign->Data, (void *)FSWTleinfo, sizeof(FSWTle));

    if(FSWTleinfo != NULL)
    {
        // free(FSWTleinfo);
        FSWTleinfo = NULL;
    }
    

    return sign;
}

csp_packet_t * PacketEncoder(packetsign * sign,bool freeer)
{
    if(sign == NULL)
    {
        console.AddLog("[DEBUG]##NULL POINTER GIVEN TO PACKETENCODER");
        return NULL;
    }
    uint32_t len = sign->Length;
    csp_packet_t *packet = (csp_packet_t *)csp_buffer_get(len);    
    packet->length = len;
    printf("packet len: %u\n", packet->length);
    memcpy(packet->data, sign->Data, len);
    // uint32_t packetsignlen = sign->Length +4; // Data size + Identifier(uint16) + PacketType(uint16)
    // csp_packet_t * packet = (csp_packet_t *)csp_buffer_get(packetsignlen);
    // packet->length = packetsignlen;
    // console.AddLog("[DEBUG]##Encoding Pakcets...Lnegth : %u", packetsignlen);
    // //Copy Header
    // memcpy((packet->data), sign, 4);
    // memcpy((packet->data) + 4, sign->Data, packetsignlen-4 ); // copy data
    for (int i =0; i < packet->length; i++) {
        printf("0x%x ", packet->data[i]);
    }printf("\n");
    if(freeer)
        sign = NULL;
    return packet;
}

packetsign * PacketDecoder(csp_packet_t * packet)
{
    uint32_t packetsignlen = packet->length +4;
    packetsign * sign = (packetsign *)malloc(packetsignlen);
    memcpy(sign, (packet->data), 4);
    sign->Length = packet->length-4;
    memcpy(sign->Data, packet->data + 4, sign->Length);
    return sign;
}

int CmdGenerator_GS::Scheduling(uint32_t ExecutionTime, uint32_t ExecutionWindow, uint32_t EntryID, uint16_t GroupID)
{
    uint16_t CMDlen = this->GetSize() + 12;
    Book * container = (Book *)malloc(CMDlen);
    memset(container, 0, CMDlen);
    container->ExTime = ExecutionTime;
    container->ExWindow = ExecutionWindow;
    container->EntryID = EntryID;
    container->GroupID = GroupID;
    this->CopyCmdHeaderToBuffer(container->cmd);
    this->GenerateCmdHeader(TS_CMD_MID, TS_INSERT_SCHEDULE_ENTRY_CC, sizeof(CFE_MSG_CommandHeader) + CMDlen, (void *)container);
    
}

packetsign * CmdGenerator_GS::GenerateCMDPacket(void)
{
    packetsign * ResultPacket = (packetsign * )malloc(this->GetSize() + MIM_HAND_DATAFDSTART);
    ResultPacket->Identifier = MIM_ID;
    if(this->Scheduled)
        ResultPacket->PacketType = MIM_PT_STCMD;
    else if(!this->Checksum)
        ResultPacket->PacketType = MIM_PT_NCCMD;
    else
        ResultPacket->PacketType = MIM_PT_CMD;
    ResultPacket->Length = this->GetSize();
    console.AddLog("[DEBUG]##Packetsign Length : %u", ResultPacket->Length);
    memcpy(ResultPacket->Data, this->CmdHeader, ResultPacket->Length);
    return ResultPacket;
}

void * Direct_Shell(void * data)
{
    pthread_mutex_lock(&conn_lock);
    while(!State.RotatorReadReady)
        continue;
    bool dlstate = true;


    //This funcion must be on p_thread[4]
    if((dlstate))
    {
        State.downlink_mode = false;
    }
        
    while(!State.uplink_mode)
        continue;

    cmd_packet_t * ResultCMD = (cmd_packet_t *)data;
    reply_packet_t * ResultReply = new reply_packet_t;
    csp_conn_t * txconn;
    csp_packet_t * packet = (csp_packet_t *)csp_buffer_get(sizeof(cmd_packet_t));
    csp_packet_t * confirm = (csp_packet_t *)csp_buffer_get(sizeof(reply_packet_t));

    uint32_t start, time = 0;
    start = csp_get_ms();

    memcpy(packet->data, ResultCMD, sizeof(cmd_packet_t));
    packet->length = sizeof(cmd_packet_t);

    if(State.Debugmode)
    {
        printf("Direct Shell Data : ");
        for(int i = 0; i < sizeof(cmd_packet_t); i++)
            printf("%u\t", ((uint8_t *)packet->data)[i]);
        printf("\n");
    }


    while(State.uplink_mode)
    {
        if ((txconn = csp_connect(CSP_PRIO_HIGH, setup->obc_node, 27, MIM_DEFAULT_TIMEOUT, 0)) == NULL)
            continue;
        else
            break;
    }
    while (State.uplink_mode && txconn != NULL)
    {
        if(csp_send(txconn, packet, setup->default_timeout *2))
        {
            packet = NULL;
            break;
        }
        else
            continue;
    }
    if(txconn == NULL)
    {
        console.AddLog("[ERROR]##Connection Buffer Busy. Skip this command.");
        if(confirm!=NULL)
        {
            csp_buffer_free(confirm);
            confirm = NULL;
        }
    }
    if((confirm = csp_read(txconn, 2 *rx_delay_ms(sizeof(MIM_LEN_PACKET), setup->ax100_node))) != NULL && State.uplink_mode)
    {
        if(State.uplink_mode)
        {
            memcpy(ResultReply, confirm->data, confirm->length);
            console.AddLog("[OK]## Received Shell Reply. Type : %"PRIu8", Result : %"PRIu8", Retcode : %d", ResultReply->type, ResultReply->result, ResultReply->retcode);
            if(confirm!=NULL)
            {
                csp_buffer_free(confirm);
                confirm = NULL;
            }
        }
    }
    else
        console.AddLog("[ERROR]##Cannot Receive Shell Reply.");
    
    if(txconn != NULL)
    {
        csp_close(txconn);
        txconn = NULL;
    }
    
    if(packet!=NULL)
    {
        csp_buffer_free(packet);
        packet = NULL;
    }
    time = csp_get_ms() - start;
    console.AddLog("[DEBUG]##Communication Time : %u", time);

    State.uplink_mode = false;
    if(dlstate)
    {
        State.downlink_mode = dlstate;
    }
    pthread_mutex_unlock(&conn_lock);
}