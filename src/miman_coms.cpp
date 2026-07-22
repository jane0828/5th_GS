
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
EPS_Report_State_t g_eps_report_state;
EPS_RParam_Reassembly_State_t g_eps_rparam_reassembly;
int8_t g_roma_parget_requested_index = -1;
//int signallen = 8;
BEE1000_Beacon_t* bee1000_beacon = (BEE1000_Beacon_t *)malloc(BEE1000_LEN_BEACON);
BEE1012_Beacon_t* bee1012_beacon = (BEE1012_Beacon_t *)malloc(BEE1012_LEN_BEACON);
UELYSYS_Beacon_t* uelysys_beacon = (UELYSYS_Beacon_t *)malloc(UELYSYS_LEN_BEACON);
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

static const unsigned int REPORT_CONN_DRAIN_TIMEOUT_MS = 100;
static constexpr size_t REPORT_WIRE_SIZE = 540;

static uint8_t g_report_wire[REPORT_WIRE_SIZE];
static size_t  g_report_off = 0;
static bool    g_report_collecting = false;
static int     g_report_dport = -1;

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

static bool DecodeEPSQueryHeader(EPS_Report_Metadata_t &meta,
                                 const uint8_t *payload,
                                 uint16_t payload_len,
                                 uint8_t command_code,
                                 int32_t return_code)
{
    if (!payload || payload_len < EPS_QUERY_REPORT_HEADER_SIZE) return false;

    memset(&meta, 0, sizeof(meta));
    meta.valid            = true;
    meta.command_code     = command_code;
    meta.return_code      = return_code;
    meta.return_data_size = payload_len;
    meta.source           = payload[0];
    meta.data_id          = payload[1];
    meta.arg0             = payload[2];
    meta.arg1             = payload[3];
    memcpy(&meta.sequence,   payload + 4, sizeof(meta.sequence));
    memcpy(&meta.offset,     payload + 6, sizeof(meta.offset));
    memcpy(&meta.total_size, payload + 8, sizeof(meta.total_size));
    memcpy(&meta.chunk_size, payload + 10, sizeof(meta.chunk_size));

    const uint32_t available = payload_len - EPS_QUERY_REPORT_HEADER_SIZE;
    if (meta.chunk_size > available ||
        meta.chunk_size > EPS_QUERY_REPORT_DATA_MAX_LEN ||
        meta.offset > meta.total_size ||
        (uint32_t)meta.offset + meta.chunk_size > meta.total_size) {
        return false;
    }

    return true;
}

static bool DecodeEPSQueryPayloadToView(ReportView_t &v, const uint8_t *payload, uint16_t payload_len)
{
    EPS_Report_Metadata_t meta;
    if (!DecodeEPSQueryHeader(meta, payload, payload_len, v.reflected_cc, v.ret_code)) {
        v.kind = REPORT_KIND_SC_GENERIC;
        return false;
    }

    v.eps_meta = meta;
    const uint8_t *data = payload + EPS_QUERY_REPORT_HEADER_SIZE;
    const uint16_t data_len = meta.chunk_size;

    if (v.reflected_cc == EPS_P80_POWER_IF_GET_CC &&
        meta.data_id == EPS_QUERY_REPORT_P80_POWER_IF_STATUS) {
        if (data_len != sizeof(EPS_PowerIfStatus_Report_t)) {
            v.kind = REPORT_KIND_SC_GENERIC;
            return false;
        }
        v.kind = REPORT_KIND_EPS_POWER_IF_STATUS;
        memcpy(&v.u.eps_power_if_status, data, sizeof(EPS_PowerIfStatus_Report_t));
        return true;
    }

    if (v.reflected_cc == EPS_P80_POWER_IF_LIST_CC &&
        meta.data_id == EPS_QUERY_REPORT_P80_POWER_IF_LIST) {
        if (data_len != sizeof(EPS_PowerIfList_Report_t)) {
            v.kind = REPORT_KIND_SC_GENERIC;
            return false;
        }
        v.kind = REPORT_KIND_EPS_POWER_IF_LIST;
        memcpy(&v.u.eps_power_if_list, data, sizeof(EPS_PowerIfList_Report_t));
        return true;
    }

    if ((v.reflected_cc == EPS_GET_HK_CC || v.reflected_cc == EPS_GET_HK_ALL_CC) &&
        meta.data_id == EPS_QUERY_REPORT_P80_PMU_HK &&
        meta.source == 1) {
        if (data_len != sizeof(EPS_P80_PMU_HK_Report_t)) {
            v.kind = REPORT_KIND_SC_GENERIC;
            return false;
        }
        v.kind = REPORT_KIND_EPS_P80_PMU_HK;
        memcpy(&v.u.eps_p80_pmu_hk, data, sizeof(EPS_P80_PMU_HK_Report_t));
        return true;
    }

    if ((v.reflected_cc == EPS_GET_HK_CC || v.reflected_cc == EPS_GET_HK_ALL_CC) &&
        meta.data_id == EPS_QUERY_REPORT_P80_PDU_HK &&
        meta.source == 4) {
        if (data_len != sizeof(EPS_P80_PDU_HK_Report_t)) {
            v.kind = REPORT_KIND_SC_GENERIC;
            return false;
        }
        v.kind = REPORT_KIND_EPS_P80_PDU_HK;
        memcpy(&v.u.eps_p80_pdu_hk, data, sizeof(EPS_P80_PDU_HK_Report_t));
        return true;
    }

    if ((v.reflected_cc == EPS_GET_HK_CC || v.reflected_cc == EPS_GET_HK_ALL_CC) &&
        meta.data_id == EPS_QUERY_REPORT_P80_ACU_HK &&
        (meta.source == 2 || meta.source == 3)) {
        if (data_len != sizeof(EPS_P80_ACU_HK_Report_t)) {
            v.kind = REPORT_KIND_SC_GENERIC;
            return false;
        }
        v.kind = (meta.source == 2) ? REPORT_KIND_EPS_P80_ACU1_HK : REPORT_KIND_EPS_P80_ACU2_HK;
        memcpy(&v.u.eps_p80_acu_hk, data, sizeof(EPS_P80_ACU_HK_Report_t));
        return true;
    }

    if ((v.reflected_cc == EPS_GET_HK_CC || v.reflected_cc == EPS_GET_HK_ALL_CC) &&
        meta.data_id == EPS_QUERY_REPORT_BP8_HK &&
        meta.source == 7) {
        if (data_len != sizeof(EPS_BP8_HK_Report_t)) {
            v.kind = REPORT_KIND_SC_GENERIC;
            return false;
        }
        v.kind = REPORT_KIND_EPS_BP8_HK;
        memcpy(&v.u.eps_bp8_hk, data, sizeof(EPS_BP8_HK_Report_t));
        return true;
    }

    v.kind = REPORT_KIND_EPS_QUERY_RAW;
    v.u.uelysys.size = (data_len > sizeof(v.u.uelysys.bytes))
                           ? (uint16_t)sizeof(v.u.uelysys.bytes)
                           : data_len;
    memcpy(v.u.uelysys.bytes, data, v.u.uelysys.size);
    return true;
}

static void UpdateEPSReportStateFromView(const ReportView_t &v)
{
    if (!v.eps_meta.valid) return;

    switch (v.kind) {
        case REPORT_KIND_EPS_POWER_IF_STATUS:
            g_eps_report_state.power_if_status_valid = true;
            g_eps_report_state.power_if_status_meta = v.eps_meta;
            g_eps_report_state.power_if_status = v.u.eps_power_if_status;
            break;

        case REPORT_KIND_EPS_POWER_IF_LIST:
            g_eps_report_state.power_if_list_valid = true;
            g_eps_report_state.power_if_list_meta = v.eps_meta;
            g_eps_report_state.power_if_list = v.u.eps_power_if_list;
            break;

        case REPORT_KIND_EPS_P80_PMU_HK:
            g_eps_report_state.pmu_hk_valid = true;
            g_eps_report_state.pmu_hk_meta = v.eps_meta;
            g_eps_report_state.pmu_hk = v.u.eps_p80_pmu_hk;
            break;

        case REPORT_KIND_EPS_P80_PDU_HK:
            g_eps_report_state.pdu_hk_valid = true;
            g_eps_report_state.pdu_hk_meta = v.eps_meta;
            g_eps_report_state.pdu_hk = v.u.eps_p80_pdu_hk;
            break;

        case REPORT_KIND_EPS_P80_ACU1_HK:
            g_eps_report_state.acu1_hk_valid = true;
            g_eps_report_state.acu1_hk_meta = v.eps_meta;
            g_eps_report_state.acu1_hk = v.u.eps_p80_acu_hk;
            break;

        case REPORT_KIND_EPS_P80_ACU2_HK:
            g_eps_report_state.acu2_hk_valid = true;
            g_eps_report_state.acu2_hk_meta = v.eps_meta;
            g_eps_report_state.acu2_hk = v.u.eps_p80_acu_hk;
            break;

        case REPORT_KIND_EPS_BP8_HK:
            g_eps_report_state.bp8_hk_valid = true;
            g_eps_report_state.bp8_hk_meta = v.eps_meta;
            g_eps_report_state.bp8_hk = v.u.eps_bp8_hk;
            break;

        case REPORT_KIND_EPS_QUERY_RAW:
            if (v.reflected_cc == EPS_RPARAM_GET_FULL_TABLE_CC &&
                (v.eps_meta.data_id == EPS_QUERY_REPORT_RPARAM_TABLE_ROWS ||
                 v.eps_meta.data_id == EPS_QUERY_REPORT_RPARAM_TABLE_MEMORY)) {
                auto &s = g_eps_rparam_reassembly;
                const auto &m = v.eps_meta;
                const bool new_transaction =
                    !s.active || s.complete || s.error ||
                    s.source != m.source || s.data_id != m.data_id ||
                    s.arg0 != m.arg0 || s.arg1 != m.arg1 ||
                    s.total_size != m.total_size ||
                    (m.sequence == 0 && s.expected_sequence != 0);
                if (new_transaction) {
                    memset(&s, 0, sizeof(s));
                    s.active = true;
                    s.valid = true;
                    s.source = m.source;
                    s.data_id = m.data_id;
                    s.arg0 = m.arg0;
                    s.arg1 = m.arg1;
                    s.total_size = m.total_size;
                }
                if (m.total_size > EPS_RPARAM_REASSEMBLY_CAPACITY ||
                    (uint32_t)m.offset + m.chunk_size > s.total_size) {
                    s.valid = false;
                    s.error = true;
                    break;
                }
                if (m.sequence > s.expected_sequence) s.missing_sequence_seen = true;
                if (m.sequence < s.expected_sequence) s.duplicate_seen = true;
                if (m.sequence >= s.expected_sequence) s.expected_sequence = (uint16_t)(m.sequence + 1);
                s.last_sequence = m.sequence;
                s.last_offset = m.offset;
                s.last_chunk_size = m.chunk_size;
                for (uint16_t i = 0; i < m.chunk_size; ++i) {
                    const uint16_t pos = (uint16_t)(m.offset + i);
                    const uint8_t new_byte = v.u.uelysys.bytes[i];
                    if (s.coverage[pos]) {
                        s.duplicate_seen = true;
                        if (s.buffer[pos] != new_byte) {
                            s.error = true;
                            continue;
                        }
                    } else {
                        s.coverage[pos] = 1;
                        s.buffer[pos] = new_byte;
                        ++s.received_size;
                    }
                }
                s.complete = (!s.error && s.received_size == s.total_size);
                s.valid = !s.error;
            }
            break;

        default:
            break;
    }
}


static bool DecodePayloadToView(ReportView_t &v, const uint8_t *payload, uint16_t payload_len) {
    if (!payload) return false;
    v.raw_size = (payload_len > sizeof(v.raw_bytes)) ? (uint16_t)sizeof(v.raw_bytes) : payload_len;
    memcpy(v.raw_bytes, payload, v.raw_size);

    if (v.kind == REPORT_KIND_EPS_QUERY_REPORT) {
        return DecodeEPSQueryPayloadToView(v, payload, payload_len);
    }

    if (v.kind == REPORT_KIND_EPS_BCN_REPORT) {
        if (payload_len != sizeof(v.u.eps_bcn)) return false;
        memcpy(&v.u.eps_bcn, payload, sizeof(v.u.eps_bcn));
        return true;
    }

    if (v.kind == REPORT_KIND_UELYSYS_UTRX) {
        if (v.reflected_cc == UELYSYS_UTRX_NOOP_CC) {
            if (payload_len != sizeof(v.u.utrx_noop)) return false;
            memcpy(&v.u.utrx_noop, payload, sizeof(v.u.utrx_noop));
        } else if (v.reflected_cc == UELYSYS_UTRX_RXCONF_GET_BAUD_CC) {
            if (payload_len != sizeof(v.u.utrx_rx_baud)) return false;
            memcpy(&v.u.utrx_rx_baud, payload, sizeof(v.u.utrx_rx_baud));
        } else if (payload_len != 0) {
            return false;
        }
        return true;
    }

    if (v.kind == REPORT_KIND_UELYSYS_PAYUEL_OBC ||
        v.kind == REPORT_KIND_UELYSYS_PAYUEL_CAM) {
        const bool is_meta =
            (v.kind == REPORT_KIND_UELYSYS_PAYUEL_OBC &&
             v.reflected_cc == UELYSYS_PAYUEL_OBC_DOWNLOAD_META_CC) ||
            (v.kind == REPORT_KIND_UELYSYS_PAYUEL_CAM &&
             v.reflected_cc == UELYSYS_PAYUEL_CAM_DOWNLOAD_META_CC);
        const bool is_status =
            (v.kind == REPORT_KIND_UELYSYS_PAYUEL_OBC &&
             v.reflected_cc == UELYSYS_PAYUEL_OBC_CAM_SHOT_CC) ||
            (v.kind == REPORT_KIND_UELYSYS_PAYUEL_CAM &&
             (v.reflected_cc == UELYSYS_PAYUEL_CAM_SHOT_CC ||
              v.reflected_cc == UELYSYS_PAYUEL_CAM_PROCESS_BINNING_CC));
        const bool is_chunk =
            (v.kind == REPORT_KIND_UELYSYS_PAYUEL_OBC &&
             v.reflected_cc == UELYSYS_PAYUEL_OBC_CHUNK_DOWNLOAD_CC) ||
            (v.kind == REPORT_KIND_UELYSYS_PAYUEL_CAM &&
             v.reflected_cc == UELYSYS_PAYUEL_CAM_CHUNK_DOWNLOAD_CC);
        const bool is_download =
            (v.kind == REPORT_KIND_UELYSYS_PAYUEL_OBC &&
             v.reflected_cc == UELYSYS_PAYUEL_OBC_DOWNLOAD_IMAGE_CC) ||
            (v.kind == REPORT_KIND_UELYSYS_PAYUEL_CAM &&
             v.reflected_cc == UELYSYS_PAYUEL_CAM_DOWNLOAD_IMAGE_CC);
        if (is_meta && payload_len == sizeof(v.u.image_meta))
            memcpy(&v.u.image_meta, payload, sizeof(v.u.image_meta));
        else if (is_status && payload_len == sizeof(v.u.payload_status))
            memcpy(&v.u.payload_status, payload, sizeof(v.u.payload_status));
        else if (is_download && payload_len == sizeof(v.u.image_meta))
            memcpy(&v.u.image_meta, payload, sizeof(v.u.image_meta));
        else if (is_download && payload_len == sizeof(v.u.download_request))
            memcpy(&v.u.download_request, payload, sizeof(v.u.download_request));
        else if ((is_meta || is_status || is_chunk) &&
                 payload_len == sizeof(v.u.payload_error))
            memcpy(&v.u.payload_error, payload, sizeof(v.u.payload_error));
        else if (payload_len != 0)
            return false;
        return true;
    }

    if (v.kind == REPORT_KIND_UELYSYS_LGBAT) {
        if (v.reflected_cc == UELYSYS_LGBAT_REQUEST_DATA_CC) {
            if (payload_len != sizeof(v.u.lgbat_block)) return false;
            memcpy(&v.u.lgbat_block, payload, sizeof(v.u.lgbat_block));
        } else if (v.reflected_cc == UELYSYS_LGBAT_REQUEST_ALL_DATA_CC) {
            if (payload_len != sizeof(v.u.lgbat_all)) return false;
            memcpy(&v.u.lgbat_all, payload, sizeof(v.u.lgbat_all));
        } else if (payload_len != 0) {
            return false;
        }
        return true;
    }

    if (v.kind == REPORT_KIND_UELYSYS_AOS) {
        if (v.reflected_cc == UELYSYS_PAYUEL_AOS_NOOP_CC) {
            if (payload_len != sizeof(v.u.aos_noop)) return false;
            memcpy(&v.u.aos_noop, payload, sizeof(v.u.aos_noop));
        } else if (v.reflected_cc == UELYSYS_PAYUEL_AOS_READ_REGISTER_CC) {
            if (payload_len != sizeof(v.u.aos_read_register)) return false;
            memcpy(&v.u.aos_read_register, payload, sizeof(v.u.aos_read_register));
        } else if (v.reflected_cc == UELYSYS_PAYUEL_AOS_READ_ALL_CHANNELS_CC) {
            if (payload_len != sizeof(v.u.aos_read_all)) return false;
            memcpy(&v.u.aos_read_all, payload, sizeof(v.u.aos_read_all));
        } else if (payload_len != 0) {
            return false;
        }
        return true;
    }

    if (v.kind == REPORT_KIND_UELYSYS_STX &&
        v.reflected_cc == UELYSYS_STX_FILESYS_CC_DIR) {
        if (payload_len < sizeof(v.u.stx_dir)) return false;
        memcpy(&v.u.stx_dir, payload, sizeof(v.u.stx_dir));
        return true;
    }

    if (v.kind == REPORT_KIND_UELYSYS_TTC ||
        v.kind == REPORT_KIND_UELYSYS_MEOW ||
        v.kind == REPORT_KIND_UELYSYS_STX ||
        v.kind == REPORT_KIND_UELYSYS_UANT ||
        v.kind == REPORT_KIND_UELYSYS_GPIO ||
        v.kind == REPORT_KIND_UELYSYS_PAYUEL_OBC ||
        v.kind == REPORT_KIND_UELYSYS_PAYUEL_CAM) {
        const uint16_t copy_len =
            (payload_len > sizeof(v.u.uelysys.bytes)) ? (uint16_t)sizeof(v.u.uelysys.bytes) : payload_len;
        v.u.uelysys.size = copy_len;
        memset(v.u.uelysys.bytes, 0, sizeof(v.u.uelysys.bytes));
        memcpy(v.u.uelysys.bytes, payload, copy_len);
        return true;
    }

    if (v.kind == REPORT_KIND_GPIO_DEP1_EN_ON ||
        v.kind == REPORT_KIND_GPIO_DEP1_EN_OFF ||
        v.kind == REPORT_KIND_GPIO_DEP2_EN_ON ||
        v.kind == REPORT_KIND_GPIO_DEP2_EN_OFF ||
        v.kind == REPORT_KIND_GPIO_SP_IN_READ_5S) {
        return true;
    }

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
            memcpy(&v.u.adcs_referencellhtarget, payload, sizeof(ADCS_ReferenceLLHTargetTlm_Payload_t));
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
        if (payload_len != sizeof(payuel_roma_Noop_tlm_payload_t))
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
        if (payload_len != sizeof(payuel_roma_ResetCounters_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_resetcounters, payload, sizeof(payuel_roma_ResetCounters_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_COMMTEST)
    {
        if (payload_len != sizeof(payuel_roma_CommTest_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_commtest, payload, sizeof(payuel_roma_CommTest_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_GETSPECIFICLINE)
    {
        if (payload_len != sizeof(payuel_roma_GetSpecificLine_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_getspecificline, payload, sizeof(payuel_roma_GetSpecificLine_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_GETMULTIPLELINES)
    {
        if (payload_len != sizeof(payuel_roma_GetMultipleLines_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_getmultiplelines, payload, sizeof(payuel_roma_GetMultipleLines_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_GETLATESTLINE)
    {
        if (payload_len != sizeof(payuel_roma_GetLatestLine_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_getlatestline, payload, sizeof(payuel_roma_GetLatestLine_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_GETLATESTNLINES)
    {
        if (payload_len != sizeof(payuel_roma_GetLatest_N_Lines_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_getlatestNlines, payload, sizeof(payuel_roma_GetLatest_N_Lines_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_SETROUTEDEFAULT)
    {
        if (payload_len != sizeof(payuel_roma_SetRouteDefault_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_setroutedefault, payload, sizeof(payuel_roma_SetRouteDefault_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_RESETROUTE)
    {
        if (payload_len != sizeof(payuel_roma_ResetRoute_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_resetroute, payload, sizeof(payuel_roma_ResetRoute_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_LOADROUTE)
    {
        if (payload_len != sizeof(payuel_roma_LoadRoute_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_loadroute, payload, sizeof(payuel_roma_LoadRoute_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_SAVEROUTE)
    {
        if (payload_len != sizeof(payuel_roma_SaveRoute_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_saveroute, payload, sizeof(payuel_roma_SaveRoute_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_SENDROUTE)
    {
        if (payload_len != sizeof(payuel_roma_SendRoute_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_sendroute, payload, sizeof(payuel_roma_SendRoute_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_PARGET)
    {
        if (payload_len != sizeof(payuel_roma_ParGet_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_parget, payload, sizeof(payuel_roma_ParGet_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_PARSET)
    {
        if (payload_len != sizeof(payuel_roma_ParSet_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_parset, payload, sizeof(payuel_roma_ParSet_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_PARDEFAULTS)
    {
        if (payload_len != sizeof(payuel_roma_ParDefaults_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_pardefaults, payload, sizeof(payuel_roma_ParDefaults_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_PARSAVE)
    {
        if (payload_len != sizeof(payuel_roma_ParSave_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_parsave, payload, sizeof(payuel_roma_ParSave_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_PARRESTORE)
    {
        if (payload_len != sizeof(payuel_roma_ParRestore_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_parrestore, payload, sizeof(payuel_roma_ParRestore_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_PARLOAD)
    {
        if (payload_len != sizeof(payuel_roma_ParLoad_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_parload, payload, sizeof(payuel_roma_ParLoad_tlm_payload_t));
            return true;
        }
    }
    
    if (v.kind == REPORT_KIND_PAYUEL_ROMA_PARSETOOB)
    {
        if (payload_len != sizeof(payuel_roma_ParSetOOB_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_parsetOOB, payload, sizeof(payuel_roma_ParSetOOB_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_SENDCOMMAND)
    {
        if (payload_len != sizeof(payuel_roma_SendCommand_tlm_payload_t))
        {
            v.kind = REPORT_KIND_SC_GENERIC;
        }
        else{
            memcpy(&v.u.roma_sendcommand, payload, sizeof(payuel_roma_SendCommand_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_SETROUTE)
    {
        if (payload_len != sizeof(payuel_roma_SetRoute_tlm_payload_t))
            v.kind = REPORT_KIND_SC_GENERIC;
        else {
            memcpy(&v.u.roma_setroute, payload, sizeof(payuel_roma_SetRoute_tlm_payload_t));
            return true;
        }
    }

    if (v.kind == REPORT_KIND_PAYUEL_ROMA_SENDMSG ||
        v.kind == REPORT_KIND_PAYUEL_ROMA_PAYINIT)
        return payload_len == 0;

    if (v.kind == REPORT_KIND_PAYUEL_LGPM_NOOP)
    {
        if (payload_len != sizeof(PAYUEL_LGPM_Noop_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_ResetCounters_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_MCU_ALIVE_CHECK_Tlm_Payload))
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
        if (payload_len != sizeof(PAYUEL_LGPM_3V3PwrOn_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_3V3PwrOff_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_MainBoostSwOn_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_MainBoostSwOff_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_SubBoostSwOn_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_SubBoostSwOff_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_V28MainOn_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_V28MainOff_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_V28SubOn_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_V28SubOff_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_V12MainOn_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_V12MainOff_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_PwrSenseInfo_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_PwrSeqOn_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_PwrSeqOff_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_RwaControlIdx1_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_RwaControlIdx2_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_RwaControlIdx3_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_RwaPwrOn_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_RwaPwrOff_tlm_payload_t))
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
        if (payload_len != sizeof(PAYUEL_LGPM_RwaSenseInfo_tlm_payload_t))
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
    // if(bee1000_beacon->UXTotRXByte != remote_total_rx_bytes)
    //     remote_total_rx_bytes = bee1000_beacon->UXTotRXByte;
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
    if(!csp_bind(sock, 31)) {
        console.AddLog("[OK]##BEE-1000 BCN Port 31 bind success.");
    }
    if(!csp_bind(sock, 25)) {
        console.AddLog("[OK]##BEE-1000 RPT Port 25 bind success.");
    }
    if(!csp_bind(sock, 27)) {
        console.AddLog("[OK]##BEE-1000 Event Port 27 bind success.");
    }
    if(!csp_bind(sock, 23)) {
        console.AddLog("[OK]##BEE-100 FM Port 23 bind success.");
    }
        
    if(!csp_bind(sock, 12)) {
        console.AddLog("[OK]##UEL-Y-Sys. BCN Port 12 bind success.");
    }
    if(!csp_bind(sock, 10)) {
        console.AddLog("[OK]##UEL-Y-Sys. RPT Port 10 bind success.");
    }
    if(!csp_bind(sock, 11)) {
        console.AddLog("[OK]##UEL-Y-Sys. Event Port 11 bind success.");
    }
    if(!csp_bind(sock, 14)) {
        console.AddLog("[OK]##UEL-Y-Sys. FM Port 14 bind success.");
    }

    if(!csp_bind(sock, 17)) {
        console.AddLog("[OK]##BEE-1012 BCN Port 17 bind success.");
    }
    if(!csp_bind(sock, 15)) {
        console.AddLog("[OK]##BEE-1012 RPT Port 15 bind success.");
    }
    if(!csp_bind(sock, 16)) {
        console.AddLog("[OK]##BEE-1012 Event Port 16 bind success.");
    }
    if(!csp_bind(sock, 18)) {
        console.AddLog("[OK]##BEE-1012 FM Port 18 bind success.");
    }
    if(!csp_bind(sock, 29)) {
        console.AddLog("[OK]##BEE-1012 AIOBC Port 29 bind success.");
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
    tmp.roma_parget_index = (tmp.kind == REPORT_KIND_PAYUEL_ROMA_PARGET)
                                ? g_roma_parget_requested_index : -1;

    tmp.payload_valid = DecodePayloadToView(tmp, rpt.RetVal, tmp.ret_val_size);

    tmp.valid = true;

	    {
	        std::lock_guard<std::mutex> lk(g_report_view_mtx);
	        g_report_view = tmp;
	        UpdateEPSReportStateFromView(tmp);
	        if (tmp.reflected_msg_id == EPS_CMD_ID &&
	            tmp.reflected_cc == EPS_RPARAM_GET_FULL_TABLE_CC &&
	            !tmp.payload_valid) {
	            g_eps_rparam_reassembly.active = true;
	            g_eps_rparam_reassembly.valid = false;
	            g_eps_rparam_reassembly.complete = false;
	            g_eps_rparam_reassembly.error = true;
	        }
	    }
	}


int BEE1000BeaconSaver(BEE1000_Beacon_t* bec)
{
    if (!bec) return -1;

    BeaconCounter++;

    char filename[128];
    time_t tmtime = time(0);
    struct tm* local = localtime(&tmtime);

    sprintf(filename,
            "../data/beacon/BEE1000Beacon--%04d-%02d-%02d-%02d-%02d-%02d--.txt",
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

static void WriteRawBytes(FILE* fp, const char* label, const uint8_t* data, size_t len)
{
    fprintf(fp, "%-24s: ", label);
    for (size_t i = 0; i < len; i++) fprintf(fp, "%02X ", data[i]);
    fprintf(fp, "\n");
}

static void WriteCallSign(FILE* fp, const char* label, const char* call_sign)
{
    char call_sign_buf[8] = {0};
    memcpy(call_sign_buf, call_sign, 7);
    fprintf(fp, "%-24s: %s\n", label, call_sign_buf);
}

static void WriteBinaryDump(FILE* fp, const void* data, size_t len)
{
    const uint8_t* bytes = (const uint8_t*)data;
    fprintf(fp, "\n[BINARY DATA]\n");
    for (size_t i = 0; i < len; i++) {
        fprintf(fp, "%02X ", bytes[i]);
        if ((i + 1) % 16 == 0) fprintf(fp, "\n");
    }
    if (len % 16 != 0) fprintf(fp, "\n");
}

int BEE1012BeaconSaver(BEE1012_Beacon_t* bec)
{
    if (!bec) return -1;

    BeaconCounter++;

    char filename[128];
    time_t tmtime = time(0);
    struct tm* local = localtime(&tmtime);

    sprintf(filename,
            "../data/BEE1000/beacon/beacon_parsed/BEE1012Beacon--%04d-%02d-%02d-%02d-%02d-%02d--.txt",
            local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
            local->tm_hour, local->tm_min, local->tm_sec);

    FILE* fp = fopen(filename, "w");
    if (!fp) return -2;

    fprintf(fp, "================= BEE1012 BEACON SAVE =================\n");
    fprintf(fp, "\n[HEADER]\n");
    WriteCallSign(fp, "Call Sign", bec->header.call_sign);
    WriteRawBytes(fp, "Message ID", bec->header.msg_id, sizeof(bec->header.msg_id));
    WriteRawBytes(fp, "Sequence", bec->header.sequence, sizeof(bec->header.sequence));
    WriteRawBytes(fp, "Length", bec->header.length, sizeof(bec->header.length));
    WriteRawBytes(fp, "Time Code", bec->header.time_code, sizeof(bec->header.time_code));

    fprintf(fp, "\n[RPT]\n");
    fprintf(fp, "boot_count              : %" PRIu16 "\n", bec->rpt.boot_count);
    fprintf(fp, "sequence                : %" PRIu32 "\n", bec->rpt.sequence);
    fprintf(fp, "reset_cause             : %" PRIu8 "\n", bec->rpt.reset_cause);

    fprintf(fp, "\n[COMS]\n");
    fprintf(fp, "AX100 active_conf       : %" PRIu8 "\n", bec->coms.ax100.active_conf);
    fprintf(fp, "AX100 boot_count        : %" PRIu16 "\n", bec->coms.ax100.boot_count);
    fprintf(fp, "AX100 boot_cause        : %" PRIu32 "\n", bec->coms.ax100.boot_cause);
    fprintf(fp, "AX100 temp_board        : %" PRId16 "\n", bec->coms.ax100.temp_board);
    fprintf(fp, "LTRX temp               : %" PRId16 "\n", bec->coms.ltrx.temp);
    fprintf(fp, "LTRX connection_quality : %" PRId8 "\n", bec->coms.ltrx.connection_quality);
    fprintf(fp, "LTRX battery_capacity   : %" PRIu16 "\n", bec->coms.ltrx.battery_capacity);

    fprintf(fp, "\n[EPS - PMU]\n");
    fprintf(fp, "bootcause               : %" PRIu32 "\n", bec->eps.pmu.bootcause);
    fprintf(fp, "resetcause              : %" PRIu16 "\n", bec->eps.pmu.resetcause);
    fprintf(fp, "bootcount               : %" PRIu16 "\n", bec->eps.pmu.bootcount);
    fprintf(fp, "out_en                  : ");
    for (int i = 0; i < 6; i++) fprintf(fp, "%" PRIu8 " ", bec->eps.pmu.out_en[i]);
    fprintf(fp, "\ntemp                    : %" PRId16 " %" PRId16 "\n", bec->eps.pmu.temp[0], bec->eps.pmu.temp[1]);
    fprintf(fp, "batt_mode               : %" PRIu8 "\n", bec->eps.pmu.batt_mode);
    fprintf(fp, "batt_i                  : %" PRId16 "\n", bec->eps.pmu.batt_i);
    fprintf(fp, "batt_v                  : %" PRIu16 "\n", bec->eps.pmu.batt_v);
    fprintf(fp, "sm_en                   : %" PRIu8 "\n", bec->eps.pmu.sm_en);
    fprintf(fp, "gnd_wdt_cnt             : %" PRIu16 "\n", bec->eps.pmu.gnd_wdt_cnt);
    fprintf(fp, "bus_wdt_cnt             : %" PRIu16 "\n", bec->eps.pmu.bus_wdt_cnt);
    fprintf(fp, "gnd_wdt_left            : %" PRIu32 "\n", bec->eps.pmu.gnd_wdt_left);
    fprintf(fp, "bus_wdt_left            : %" PRIu32 "\n", bec->eps.pmu.bus_wdt_left);

    fprintf(fp, "\n[EPS - PDU]\nout_i                   : ");
    for (int i = 0; i < 13; i++) fprintf(fp, "%" PRId16 " ", bec->eps.pdu.out_i[i]);
    fprintf(fp, "\nout_en                  : ");
    for (int i = 0; i < 13; i++) fprintf(fp, "%" PRIu8 " ", bec->eps.pdu.out_en[i]);
    fprintf(fp, "\n");

    fprintf(fp, "\n[EPS - ACU]\n");
    for (int unit = 0; unit < 2; unit++) {
        fprintf(fp, "ACU %d input_i           : ", unit);
        for (int i = 0; i < 6; i++) fprintf(fp, "%" PRId16 " ", bec->eps.acu.acu[unit].input_i[i]);
        fprintf(fp, "\nACU %d input_v           : ", unit);
        for (int i = 0; i < 6; i++) fprintf(fp, "%" PRIu16 " ", bec->eps.acu.acu[unit].input_v[i]);
        fprintf(fp, "\nACU %d mppt_mode         : %" PRIu8 "\n", unit, bec->eps.acu.acu[unit].mppt_mode);
    }

    fprintf(fp, "\n[EPS - BP8]\n");
    fprintf(fp, "bootcount               : %" PRIu16 "\n", bec->eps.bp8.bootcount);
    fprintf(fp, "bootcause               : %" PRIu16 "\n", bec->eps.bp8.bootcause);
    fprintf(fp, "resetcause              : %" PRIu16 "\n", bec->eps.bp8.resetcause);
    fprintf(fp, "soc                     : %f\n", bec->eps.bp8.soc);
    fprintf(fp, "bat_avr_temp            : %f\n", bec->eps.bp8.bat_avr_temp);
    fprintf(fp, "vbat                    : %" PRIu16 "\n", bec->eps.bp8.vbat);
    fprintf(fp, "i                       : %f\n", bec->eps.bp8.i);
    fprintf(fp, "heater_i                : %" PRIu16 "\n", bec->eps.bp8.heater_i);
    fprintf(fp, "\n[EPS - GPIO]\ngpio_status             : 0x%04X\n", bec->eps.gpio.gpio_status);
    fprintf(fp, "sp_deploy_status        : %" PRIu8 "\n", bec->eps.gpio.sp_deploy_status);

    fprintf(fp, "\n[ADCS]\n");
    fprintf(fp, "power_state             : %" PRIu8 "\n", bec->adcs.power_state);
    fprintf(fp, "control_mode            : %" PRIu8 "\n", bec->adcs.control_mode);
    fprintf(fp, "gyro0_rate_x            : %f\n", bec->adcs.gyro0_calibrated_rate_x);
    fprintf(fp, "gyro0_rate_y            : %f\n", bec->adcs.gyro0_calibrated_rate_y);
    fprintf(fp, "gyro0_rate_z            : %f\n", bec->adcs.gyro0_calibrated_rate_z);
    fprintf(fp, "css                     : ");
    for (int i = 0; i < 6; i++) fprintf(fp, "%" PRIu8 " ", bec->adcs.css[i]);
    fprintf(fp, "\n");

    WriteBinaryDump(fp, bec, sizeof(*bec));
    fclose(fp);
    return 0;
}

int UELYSYSBeaconSaver(UELYSYS_Beacon_t* bec)
{
    if (!bec) return -1;

    BeaconCounter++;

    char filename[128];
    time_t tmtime = time(0);
    struct tm* local = localtime(&tmtime);

    sprintf(filename,
            "../data/UELYSYS/beacon/beacon_parsed/UELYSYSBeacon--%04d-%02d-%02d-%02d-%02d-%02d--.txt",
            local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
            local->tm_hour, local->tm_min, local->tm_sec);

    FILE* fp = fopen(filename, "w");
    if (!fp) return -2;

    fprintf(fp, "================= UELYSYS BEACON SAVE =================\n");
    fprintf(fp, "\n[HEADER]\n");
    WriteCallSign(fp, "Call Sign", bec->header.call_sign);
    WriteRawBytes(fp, "Message ID", bec->header.msg_id, sizeof(bec->header.msg_id));
    WriteRawBytes(fp, "Sequence", bec->header.sequence, sizeof(bec->header.sequence));
    WriteRawBytes(fp, "Length", bec->header.length, sizeof(bec->header.length));
    WriteRawBytes(fp, "Time Code", bec->header.time_code, sizeof(bec->header.time_code));

    fprintf(fp, "\n[FSW]\n");
    fprintf(fp, "boot_count              : %" PRIu16 "\n", bec->fsw.boot_count);
    fprintf(fp, "sequence                : %" PRIu32 "\n", bec->fsw.sequence);
    fprintf(fp, "reset_cause             : %" PRIu8 "\n", bec->fsw.reset_cause);

    fprintf(fp, "\n[COMS]\n");
    fprintf(fp, "i2c1 0x05 ch0 status    : %" PRIu8 "\n", bec->coms.i2c1_0x05_channel_0_status);
    fprintf(fp, "i2c1 0x05 ch1 status    : %" PRIu8 "\n", bec->coms.i2c1_0x05_channel_1_status);
    fprintf(fp, "i2c1 0x06 ch0 status    : %" PRIu8 "\n", bec->coms.i2c1_0x06_channel_0_status);
    fprintf(fp, "i2c1 0x06 ch1 status    : %" PRIu8 "\n", bec->coms.i2c1_0x06_channel_1_status);
    fprintf(fp, "AX100 active_conf       : %" PRIu8 "\n", bec->coms.ax100_active_conf);
    fprintf(fp, "AX100 boot_count        : %" PRIu16 "\n", bec->coms.ax100_boot_count);
    fprintf(fp, "AX100 boot_cause        : %" PRIu32 "\n", bec->coms.ax100_boot_cause);
    fprintf(fp, "AX100 temp_board        : %" PRId16 "\n", bec->coms.ax100_temp_board);

    fprintf(fp, "\n[EPS - PMU]\n");
    fprintf(fp, "bootcause               : %" PRIu32 "\n", bec->eps.pmu.bootcause);
    fprintf(fp, "resetcause              : %" PRIu16 "\n", bec->eps.pmu.resetcause);
    fprintf(fp, "bootcount               : %" PRIu16 "\n", bec->eps.pmu.bootcount);
    fprintf(fp, "out_en                  : ");
    for (int i = 0; i < 6; i++) fprintf(fp, "%" PRIu8 " ", bec->eps.pmu.out_en[i]);
    fprintf(fp, "\ntemp                    : %" PRId16 " %" PRId16 "\n", bec->eps.pmu.temp[0], bec->eps.pmu.temp[1]);
    fprintf(fp, "batt_mode               : %" PRIu8 "\n", bec->eps.pmu.batt_mode);
    fprintf(fp, "batt_i                  : %" PRId16 "\n", bec->eps.pmu.batt_i);
    fprintf(fp, "batt_v                  : %" PRIu16 "\n", bec->eps.pmu.batt_v);
    fprintf(fp, "sm_en                   : %" PRIu8 "\n", bec->eps.pmu.sm_en);
    fprintf(fp, "gnd_wdt_cnt             : %" PRIu16 "\n", bec->eps.pmu.gnd_wdt_cnt);
    fprintf(fp, "bus_wdt_cnt             : %" PRIu16 "\n", bec->eps.pmu.bus_wdt_cnt);
    fprintf(fp, "gnd_wdt_left            : %" PRIu32 "\n", bec->eps.pmu.gnd_wdt_left);
    fprintf(fp, "bus_wdt_left            : %" PRIu32 "\n", bec->eps.pmu.bus_wdt_left);

    fprintf(fp, "\n[EPS - PDU]\nout_i                   : ");
    for (int i = 0; i < 13; i++) fprintf(fp, "%" PRId16 " ", bec->eps.pdu.out_i[i]);
    fprintf(fp, "\nout_en                  : ");
    for (int i = 0; i < 13; i++) fprintf(fp, "%" PRIu8 " ", bec->eps.pdu.out_en[i]);
    fprintf(fp, "\n");

    fprintf(fp, "\n[EPS - ACU]\n");
    for (int unit = 0; unit < 2; unit++) {
        fprintf(fp, "ACU %d input_i           : ", unit);
        for (int i = 0; i < 6; i++) fprintf(fp, "%" PRId16 " ", bec->eps.acu.acu[unit].input_i[i]);
        fprintf(fp, "\nACU %d input_v           : ", unit);
        for (int i = 0; i < 6; i++) fprintf(fp, "%" PRIu16 " ", bec->eps.acu.acu[unit].input_v[i]);
        fprintf(fp, "\nACU %d mppt_mode         : %" PRIu8 "\n", unit, bec->eps.acu.acu[unit].mppt_mode);
    }

    fprintf(fp, "\n[EPS - BP8]\n");
    fprintf(fp, "bootcount               : %" PRIu16 "\n", bec->eps.bp8.bootcount);
    fprintf(fp, "bootcause               : %" PRIu16 "\n", bec->eps.bp8.bootcause);
    fprintf(fp, "resetcause              : %" PRIu16 "\n", bec->eps.bp8.resetcause);
    fprintf(fp, "soc                     : %f\n", bec->eps.bp8.soc);
    fprintf(fp, "bat_avr_temp            : %f\n", bec->eps.bp8.bat_avr_temp);
    fprintf(fp, "vbat                    : %" PRIu16 "\n", bec->eps.bp8.vbat);
    fprintf(fp, "i                       : %f\n", bec->eps.bp8.i);
    fprintf(fp, "heater_i                : %" PRIu16 "\n", bec->eps.bp8.heater_i);
    fprintf(fp, "\n[EPS - DSP]\n");
    fprintf(fp, "dsp_i2c1_0x07_status    : %" PRIu8 "\n", bec->eps.dsp.dsp_i2c1_0x07_status);
    fprintf(fp, "dsp_i2c1_0x08_status    : %" PRIu8 "\n", bec->eps.dsp.dsp_i2c1_0x08_status);
    fprintf(fp, "dsp_i2c1_0x09_status    : %" PRIu8 "\n", bec->eps.dsp.dsp_i2c1_0x09_status);
    fprintf(fp, "dsp_i2c1_0x10_status    : %" PRIu8 "\n", bec->eps.dsp.dsp_i2c1_0x10_status);
    fprintf(fp, "dsp_GPIO_status    : %" PRIu8 "\n", bec->eps.dsp.dsp_GPIO_status);

    fprintf(fp, "\n[ADCS]\n");
    fprintf(fp, "power_state             : %" PRIu8 "\n", bec->adcs.power_state);
    fprintf(fp, "control_mode            : %" PRIu8 "\n", bec->adcs.control_mode);
    fprintf(fp, "gyro0_rate_x            : %f\n", bec->adcs.gyro0_calibrated_rate_x);
    fprintf(fp, "gyro0_rate_y            : %f\n", bec->adcs.gyro0_calibrated_rate_y);
    fprintf(fp, "gyro0_rate_z            : %f\n", bec->adcs.gyro0_calibrated_rate_z);
    fprintf(fp, "css                     : ");
    for (int i = 0; i < 6; i++) fprintf(fp, "%" PRIu8 " ", bec->adcs.css[i]);
    fprintf(fp, "\n");

    WriteBinaryDump(fp, bec, sizeof(*bec));
    fclose(fp);
    return 0;
}



static void DumpBeaconPacketToFile(FILE* fp, const csp_packet_t* packet)
{
    for (int i = 0; i < packet->length; i++) {
        if (!(i % 10) && i != 0) {
            printf("\n");
            if (fp) fprintf(fp, "\n");
        }
        printf("0x%x ", packet->data[i]);
        if (fp) fprintf(fp, "%02hhx\t", packet->data[i]);
    }
}

static void HandleBEE1012BeaconPacket(const csp_packet_t* packet, int dport)
{
    char bcnpktfilename[128];
    time_t tmtime = time(0);
    struct tm *local = localtime(&tmtime);

    sprintf(bcnpktfilename,
            "../data/BEE1012/beacon/beacon_raw/BEE1012BeaconPkt--%04d-%02d-%02d-%02d-%02d-%02d--",
            local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
            local->tm_hour, local->tm_min, local->tm_sec);

    console.AddLog("Received BEE1012 Beacon from port : %d.", dport);

    FILE *bcn_fp = fopen(bcnpktfilename, "wb");
    printf("\nBEE1012 Beacon Length: %u", packet->length);
    DumpBeaconPacketToFile(bcn_fp, packet);

    if (packet->length == BEE1012_LEN_BEACON) {
        memset(bee1012_beacon, 0, sizeof(*bee1012_beacon));
        memcpy(bee1012_beacon, packet->data, BEE1012_LEN_BEACON);
        BEE1012BeaconSaver(bee1012_beacon);
    } else {
        console.AddLog("Received BEE1012 Beacon length mismatch. expected=%zu actual=%u",
                       (size_t)BEE1012_LEN_BEACON, packet->length);
    }

    if (bcn_fp) fclose(bcn_fp);
}

static void HandleUELYSYSBeaconPacket(const csp_packet_t* packet, int dport)
{
    char bcnpktfilename[128];
    time_t tmtime = time(0);
    struct tm *local = localtime(&tmtime);

    sprintf(bcnpktfilename,
            "../data/UELYSYS/beacon/beacon_raw/UELYSYSBeaconPkt--%04d-%02d-%02d-%02d-%02d-%02d--",
            local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
            local->tm_hour, local->tm_min, local->tm_sec);

    console.AddLog("Received UELYSYS Beacon from port : %d.", dport);

    FILE *bcn_fp = fopen(bcnpktfilename, "wb");
    printf("\nUELYSYS Beacon Length: %u", packet->length);
    DumpBeaconPacketToFile(bcn_fp, packet);

    if (packet->length == UELYSYS_LEN_BEACON) {
        memset(uelysys_beacon, 0, sizeof(*uelysys_beacon));
        memcpy(uelysys_beacon, packet->data, UELYSYS_LEN_BEACON);
        UELYSYSBeaconSaver(uelysys_beacon);
    } else {
        console.AddLog("Received UELYSYS Beacon length mismatch. expected=%zu actual=%u",
                       (size_t)UELYSYS_LEN_BEACON, packet->length);
    }

    if (bcn_fp) fclose(bcn_fp);
}

int MissionBeaconSaver(MissionBeacon* misnbec)
{
    if (!misnbec) return -1;

    MissionBeaconCounter++;

    char filename[128];
    time_t tmtime = time(0);
    struct tm* local = localtime(&tmtime);

    sprintf(filename,
            "../data/BEE1000/missionbeacon/missionbeacon_parsed/MissionBeacon--%04d-%02d-%02d-%02d-%02d-%02d--.txt",
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

static void DumpFixedName8(FILE *fp, const char *label, const char name[8])
{
    char buf[9] = {0};
    memcpy(buf, name, 8);
    fprintf(fp, "%s: %s\n", label, buf);
}

static void DumpIndexedBytes(FILE *fp, const char *name, const uint8_t *data, uint16_t size);

static void DumpEPSQueryMeta(FILE *fp, const EPS_Report_Metadata_t &meta)
{
    fprintf(fp, "source     : %" PRIu8 "\n", meta.source);
    fprintf(fp, "data_id    : %" PRIu8 "\n", meta.data_id);
    fprintf(fp, "arg0       : %" PRIu8 "\n", meta.arg0);
    fprintf(fp, "arg1       : %" PRIu8 "\n", meta.arg1);
    fprintf(fp, "sequence   : %" PRIu16 "\n", meta.sequence);
    fprintf(fp, "offset     : %" PRIu16 "\n", meta.offset);
    fprintf(fp, "total_size : %" PRIu16 "\n", meta.total_size);
    fprintf(fp, "chunk_size : %" PRIu16 "\n", meta.chunk_size);
}

static bool DumpEPSReportParsed(FILE *fp, const Report *rpt)
{
    EPS_Report_Metadata_t meta;
    if (!DecodeEPSQueryHeader(meta, rpt->RetVal, rpt->RetValSize, rpt->ReflectedCC, rpt->RetCode)) {
        fprintf(fp, "\n[EPS] Query payload header decode failed\n");
        return false;
    }

    const uint8_t *data = rpt->RetVal + EPS_QUERY_REPORT_HEADER_SIZE;

    fprintf(fp, "\n[EPS Query Report]\n");
    DumpEPSQueryMeta(fp, meta);

    if (rpt->ReflectedCC == EPS_P80_POWER_IF_GET_CC &&
        meta.data_id == EPS_QUERY_REPORT_P80_POWER_IF_STATUS &&
        meta.chunk_size == sizeof(EPS_PowerIfStatus_Report_t)) {
        EPS_PowerIfStatus_Report_t pl;
        memcpy(&pl, data, sizeof(pl));
        fprintf(fp, "\n[EPS Power IF Status]\n");
        fprintf(fp, "ch_idx     : %" PRIu8 "\n", pl.ch_idx);
        fprintf(fp, "mode       : %" PRIu8 "\n", pl.mode);
        fprintf(fp, "on_cnt     : %" PRIu16 "\n", pl.on_cnt);
        fprintf(fp, "off_cnt    : %" PRIu16 "\n", pl.off_cnt);
        fprintf(fp, "cur_lu_lim : %" PRIu16 "\n", pl.cur_lu_lim);
        fprintf(fp, "cur_lim    : %" PRIu16 "\n", pl.cur_lim);
        fprintf(fp, "voltage    : %" PRIu16 "\n", pl.voltage);
        fprintf(fp, "current    : %" PRIi16 "\n", pl.current);
        fprintf(fp, "latchup    : %" PRIu16 "\n", pl.latchup);
        DumpFixedName8(fp, "name       ", pl.name);
        return true;
    }

    if (rpt->ReflectedCC == EPS_P80_POWER_IF_LIST_CC &&
        meta.data_id == EPS_QUERY_REPORT_P80_POWER_IF_LIST &&
        meta.chunk_size == sizeof(EPS_PowerIfList_Report_t)) {
        EPS_PowerIfList_Report_t pl;
        memcpy(&pl, data, sizeof(pl));
        uint8_t count = (pl.count > 24) ? 24 : pl.count;
        fprintf(fp, "\n[EPS Power IF List]\n");
        fprintf(fp, "cmd    : %" PRIu8 "\n", pl.cmd);
        fprintf(fp, "status : %" PRIu8 "\n", pl.status);
        fprintf(fp, "count  : %" PRIu8 "\n", pl.count);
        for (uint8_t i = 0; i < count; i++) {
            char name[9] = {0};
            memcpy(name, pl.list[i].name, 8);
            fprintf(fp, "list[%u]: ch_idx=%" PRIu8 " mode=%" PRIu8 " name=%s\n",
                    (unsigned)i, pl.list[i].ch_idx, pl.list[i].mode, name);
        }
        return true;
    }

    if ((rpt->ReflectedCC == EPS_GET_HK_CC || rpt->ReflectedCC == EPS_GET_HK_ALL_CC) &&
        meta.data_id == EPS_QUERY_REPORT_P80_PMU_HK &&
        meta.source == 1 &&
        meta.chunk_size == sizeof(EPS_P80_PMU_HK_Report_t)) {
        EPS_P80_PMU_HK_Report_t pl;
        memcpy(&pl, data, sizeof(pl));
        fprintf(fp, "\n[EPS P80 PMU HK]\n");
        fprintf(fp, "uptime       : %" PRIu32 "\n", pl.uptime);
        fprintf(fp, "bootcause    : %" PRIu32 "\n", pl.bootcause);
        fprintf(fp, "resetcause   : %" PRIu16 "\n", pl.resetcause);
        fprintf(fp, "bootcount    : %" PRIu16 "\n", pl.bootcount);
        fprintf(fp, "batt_v       : %" PRIu16 "\n", pl.batt_v);
        fprintf(fp, "batt_i       : %" PRIi16 "\n", pl.batt_i);
        fprintf(fp, "batt_mode    : %" PRIu8 "\n", pl.batt_mode);
        fprintf(fp, "vbat_v       : %" PRIu16 "\n", pl.vbat_v);
        fprintf(fp, "vcc_v        : %" PRIu16 "\n", pl.vcc_v);
        DumpArr_i16(fp, "temp[0..1]", pl.temp, 2);
        DumpArr_u8(fp, "out_en[0..5]", pl.out_en, 6);
        DumpArr_i16(fp, "out_i[0..5]", pl.out_i, 6);
        DumpArr_u8(fp, "sm_en[0..7]", pl.sm_en, 8);
        fprintf(fp, "gnd_wdt_cnt  : %" PRIu16 "\n", pl.gnd_wdt_cnt);
        fprintf(fp, "bus_wdt_cnt  : %" PRIu16 "\n", pl.bus_wdt_cnt);
        fprintf(fp, "gnd_wdt_left : %" PRIu32 "\n", pl.gnd_wdt_left);
        fprintf(fp, "bus_wdt_left : %" PRIu32 "\n", pl.bus_wdt_left);
        return true;
    }

    if ((rpt->ReflectedCC == EPS_GET_HK_CC || rpt->ReflectedCC == EPS_GET_HK_ALL_CC) &&
        meta.data_id == EPS_QUERY_REPORT_P80_PDU_HK &&
        meta.source == 4 &&
        meta.chunk_size == sizeof(EPS_P80_PDU_HK_Report_t)) {
        EPS_P80_PDU_HK_Report_t pl;
        memcpy(&pl, data, sizeof(pl));
        fprintf(fp, "\n[EPS P80 PDU HK]\n");
        fprintf(fp, "uptime       : %" PRIu32 "\n", pl.uptime);
        fprintf(fp, "bootcause    : %" PRIu32 "\n", pl.bootcause);
        fprintf(fp, "bootcount    : %" PRIu32 "\n", pl.bootcount);
        fprintf(fp, "resetcause   : %" PRIu16 "\n", pl.resetcause);
        fprintf(fp, "vcc_v        : %" PRIu16 "\n", pl.vcc_v);
        fprintf(fp, "vcc_i        : %" PRIu16 "\n", pl.vcc_i);
        fprintf(fp, "vbat_v       : %" PRIu16 "\n", pl.vbat_v);
        fprintf(fp, "temp         : %" PRIi16 "\n", pl.temp);
        fprintf(fp, "batt_mode    : %" PRIu8 "\n", pl.batt_mode);
        DumpArr_u8(fp, "out_en[0..23]", pl.out_en, 24);
        DumpArr_i16(fp, "out_i[0..23]", pl.out_i, 24);
        fprintf(fp, "gnd_wdt_cnt  : %" PRIu32 "\n", pl.gnd_wdt_cnt);
        fprintf(fp, "bus_wdt_cnt  : %" PRIu32 "\n", pl.bus_wdt_cnt);
        fprintf(fp, "gnd_wdt_left : %" PRIu32 "\n", pl.gnd_wdt_left);
        fprintf(fp, "bus_wdt_left : %" PRIu32 "\n", pl.bus_wdt_left);
        return true;
    }

    if ((rpt->ReflectedCC == EPS_GET_HK_CC || rpt->ReflectedCC == EPS_GET_HK_ALL_CC) &&
        meta.data_id == EPS_QUERY_REPORT_P80_ACU_HK &&
        (meta.source == 2 || meta.source == 3) &&
        meta.chunk_size == sizeof(EPS_P80_ACU_HK_Report_t)) {
        EPS_P80_ACU_HK_Report_t pl;
        memcpy(&pl, data, sizeof(pl));
        fprintf(fp, "\n[EPS P80 ACU%u HK]\n", (meta.source == 2) ? 1U : 2U);
        fprintf(fp, "uptime       : %" PRIu32 "\n", pl.uptime);
        fprintf(fp, "bootcause    : %" PRIu32 "\n", pl.bootcause);
        fprintf(fp, "bootcount    : %" PRIu32 "\n", pl.bootcount);
        fprintf(fp, "resetcause   : %" PRIu16 "\n", pl.resetcause);
        DumpArr_i16(fp, "input_i[0..5]", pl.input_i, 6);
        DumpArr_u16(fp, "input_v[0..5]", pl.input_v, 6);
        fprintf(fp, "vcc_v        : %" PRIu16 "\n", pl.vcc_v);
        fprintf(fp, "vbat_v       : %" PRIu16 "\n", pl.vbat_v);
        DumpArr_i16(fp, "temp[0..2]", pl.temp, 3);
        fprintf(fp, "mppt_mode    : %" PRIu8 "\n", pl.mppt_mode);
        fprintf(fp, "gnd_wdt_cnt  : %" PRIu32 "\n", pl.gnd_wdt_cnt);
        fprintf(fp, "gnd_wdt_left : %" PRIu32 "\n", pl.gnd_wdt_left);
        return true;
    }

    if ((rpt->ReflectedCC == EPS_GET_HK_CC || rpt->ReflectedCC == EPS_GET_HK_ALL_CC) &&
        meta.data_id == EPS_QUERY_REPORT_BP8_HK &&
        meta.source == 7 &&
        meta.chunk_size == sizeof(EPS_BP8_HK_Report_t)) {
        EPS_BP8_HK_Report_t pl;
        memcpy(&pl, data, sizeof(pl));
        fprintf(fp, "\n[EPS BP8 HK]\n");
        fprintf(fp, "Uptime        : %" PRIu32 "\n", pl.Uptime);
        fprintf(fp, "BootCount     : %" PRIu16 "\n", pl.BootCount);
        fprintf(fp, "BootCause     : %" PRIu16 "\n", pl.BootCause);
        fprintf(fp, "ResetCause    : %" PRIu16 "\n", pl.ResetCause);
        fprintf(fp, "Vbat          : %" PRIu16 "\n", pl.Vbat);
        fprintf(fp, "Soc           : %f\n", pl.Soc);
        fprintf(fp, "Current       : %f\n", pl.Current);
        fprintf(fp, "InCurrent     : %" PRIu16 "\n", pl.InCurrent);
        fprintf(fp, "OutCurrent    : %" PRIu16 "\n", pl.OutCurrent);
        fprintf(fp, "HeaterCurrent : %" PRIu16 "\n", pl.HeaterCurrent);
        fprintf(fp, "IntTemp       : %" PRIi16 "\n", pl.IntTemp);
        fprintf(fp, "BatAvrTemp    : %f\n", pl.BatAvrTemp);
        DumpArr_i16(fp, "BatTemp[0..3]", pl.BatTemp, 4);
        fprintf(fp, "OVoltCount    : %" PRIu16 "\n", pl.OVoltCount);
        fprintf(fp, "BatFault      : %" PRIu8 "\n", pl.BatFault);
        return true;
    }

    fprintf(fp, "\n[EPS Query Data]\n");
    if (meta.data_id == EPS_QUERY_REPORT_CSP_PING_MS && meta.chunk_size >= sizeof(int32_t)) {
        int32_t elapsed_ms = 0;
        memcpy(&elapsed_ms, data, sizeof(elapsed_ms));
        fprintf(fp, "elapsed_ms : %" PRIi32 "\n", elapsed_ms);
    }
    fprintf(fp, "query_data (%" PRIu16 " bytes):\n", meta.chunk_size);
    for (uint16_t i = 0; i < meta.chunk_size; ++i)
        fprintf(fp, "  [%03" PRIu16 "] = 0x%02X (%" PRIu8 ")\n", i, data[i], data[i]);
    if (rpt->ReflectedCC == EPS_RPARAM_GET_FULL_TABLE_CC) {
        EPS_RParam_Reassembly_State_t s{};
        {
            std::lock_guard<std::mutex> lk(g_report_view_mtx);
            s = g_eps_rparam_reassembly;
        }
        fprintf(fp, "\n[EPS CC42 Reassembly]\nactive=%u\nvalid=%u\ncomplete=%u\nerror=%u\n"
                    "duplicate_seen=%u\nmissing_sequence_seen=%u\nexpected_sequence=%" PRIu16
                    "\nlast_sequence=%" PRIu16 "\ntotal_size=%" PRIu16 "\nreceived_size=%" PRIu16
                    "\nlast_offset=%" PRIu16 "\nlast_chunk_size=%" PRIu16 "\n",
                s.active, s.valid, s.complete, s.error, s.duplicate_seen, s.missing_sequence_seen,
                s.expected_sequence, s.last_sequence, s.total_size, s.received_size,
                s.last_offset, s.last_chunk_size);
        if (s.complete) DumpIndexedBytes(fp, "reassembled_data", s.buffer, s.total_size);
    }
    return true;
}

template <typename T>
static bool CopyReportPayload(const Report *rpt, T &out)
{
    if (rpt->RetValSize < sizeof(T)) return false;
    memcpy(&out, rpt->RetVal, sizeof(T));
    return true;
}

static void DumpIndexedBytes(FILE *fp, const char *name, const uint8_t *data, uint16_t size)
{
    fprintf(fp, "%s (%" PRIu16 " bytes):\n", name, size);
    for (uint16_t i = 0; i < size; ++i)
        fprintf(fp, "  [%03" PRIu16 "] = 0x%02X (%" PRIu8 ")\n", i, data[i], data[i]);
}

static uint16_t ReadU16BE(const uint8_t b[2])
{
    return (uint16_t)(((uint16_t)b[0] << 8) | b[1]);
}

static uint32_t ReadU32BE(const uint8_t b[4])
{
    return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) | ((uint32_t)b[2] << 8) | b[3];
}

static void DumpLgbatBlock(FILE *fp, const UELYSYS_LGBAT_Block_Report_t &b)
{
    fprintf(fp, "ID=0x%02X\n", b.raw[0]);
    switch (b.raw[0]) {
        case 0x01: fprintf(fp, "Pack_Voltage=%" PRIu16 "\nPack_Current=%" PRId16 "\nAverage_Time_To_Empty=%" PRIu16 "\nAverage_Time_To_Full=%" PRIu16 "\nCheckSum=0x%02X\n", b.Data01.Pack_Voltage, b.Data01.Pack_Current, b.Data01.Average_Time_To_Empty, b.Data01.Average_Time_To_Full, b.Data01.CheckSum); break;
        case 0x02: fprintf(fp, "Power_Supply_Status=%" PRIu8 "\nSOH=%" PRIu8 "\nSOC=%" PRIu16 "\nRC_Remaining_Capacity=%" PRIu16 "\nAE_Available_Energy=%" PRIu16 "\nCheckSum=0x%02X\n", b.Data02.Power_Supply_Status, b.Data02.SOH, b.Data02.SOC, b.Data02.RC_Remaining_Capacity, b.Data02.AE_Available_Energy, b.Data02.CheckSum); break;
        case 0x03: fprintf(fp, "System_Max_Voltage=%" PRIu16 "\nPower_Supply_Health=%" PRIu8 "\nFETTestRequiredVoltage=%" PRIu16 "\n", b.Data03.System_Max_Voltage, b.Data03.Power_Supply_Health, b.Data03.FETTestRequiredVoltage); DumpArr_u8(fp, "Battery_Information_Reserved", b.Data03.Battery_Information_Reserved, 3); fprintf(fp, "CheckSum=0x%02X\n", b.Data03.CheckSum); break;
        case 0x04: fprintf(fp, "Percentage=%" PRIu16 "\nDesign_Capacity=%" PRIu16 "\nCapacity=%" PRIu16 "\nCharge=%" PRIu16 "\nCheckSum=0x%02X\n", b.Data04.Percentage, b.Data04.Design_Capacity, b.Data04.Capacity, b.Data04.Charge, b.Data04.CheckSum); break;
        case 0x05: fprintf(fp, "Cell_Voltage_Max=%" PRIu16 "\nCell_Voltage_Min=%" PRIu16 "\nCell_Temperature_Max=%" PRId16 "\nCell_Temperature_Min=%" PRId16 "\nCheckSum=0x%02X\n", b.Data05.Cell_Voltage_Max, b.Data05.Cell_Voltage_Min, b.Data05.Cell_Temperature_Max, b.Data05.Cell_Temperature_Min, b.Data05.CheckSum); break;
        case 0x06: fprintf(fp, "Cell_Voltage_01=%" PRIu16 "\nCell_Voltage_02=%" PRIu16 "\nReserved_01=%" PRIu16 "\nReserved_02=%" PRIu16 "\nCheckSum=0x%02X\n", b.Data06.Cell_Voltage_01, b.Data06.Cell_Voltage_02, b.Data06.Reserved_01, b.Data06.Reserved_02, b.Data06.CheckSum); break;
        case 0x07: fprintf(fp, "Cell_Temperature_02=%" PRId16 "\nCell_Temperature_01=%" PRId16 "\nBalancing_R_Temperature=%" PRId16 "\nPreCharge_R_Temperature=%" PRId16 "\nCheckSum=0x%02X\n", b.Data07.Cell_Temperature_02, b.Data07.Cell_Temperature_01, b.Data07.Balancing_R_Temperature, b.Data07.PreCharge_R_Temperature, b.Data07.CheckSum); break;
        case 0x08: fprintf(fp, "FET_Down_Temperature=%" PRId16 "\nFET_Up_Temperature=%" PRId16 "\nCtrlCBStatus=%" PRIu16 "\nTemperature_002_Reserved=%" PRIu8 "\nSoftVersion=%" PRIu8 "\nCheckSum=0x%02X\n", b.Data08.FET_Down_Temperature, b.Data08.FET_Up_Temperature, b.Data08.CtrlCBStatus, b.Data08.Temperature_002_Reserved, b.Data08.SoftVersion, b.Data08.CheckSum); break;
        case 0x09: fprintf(fp, "BMS_Wakeup=%" PRIu8 "\nWakeupHoldStatus=%" PRIu8 "\nVoltCurrDiag=%" PRIu8 "\nTempFailLevel=%" PRIu8 "\nFETStatus=%" PRIu8 "\n", b.Data09.BMS_Wakeup, b.Data09.WakeupHoldStatus, b.Data09.VoltCurrDiag, b.Data09.TempFailLevel, b.Data09.FETStatus); DumpArr_u8(fp, "Reserved", b.Data09.Reserved, 3); fprintf(fp, "CheckSum=0x%02X\n", b.Data09.CheckSum); break;
        case 0x0A: fprintf(fp, "FailStatus2=0x%02X\nFailStatus3=0x%02X\n", b.Data0A.FailStatus2, b.Data0A.FailStatus3); DumpArr_u8(fp, "Reserved", b.Data0A.Reserved, 6); fprintf(fp, "CheckSum=0x%02X\n", b.Data0A.CheckSum); break;
        case 0x0B: fprintf(fp, "BOOST_Out_Voltage=%" PRIu16 "\nMCU_B_Plus_Volt=%" PRIu16 "\nMCU_P_Plus_Volt=%" PRIu16 "\nMCU_BMIC_REG_Out_Volt=%" PRIu16 "\nCheckSum=0x%02X\n", b.Data0B.BOOST_Out_Voltage, b.Data0B.MCU_B_Plus_Volt, b.Data0B.MCU_P_Plus_Volt, b.Data0B.MCU_BMIC_REG_Out_Volt, b.Data0B.CheckSum); break;
        case 0x0C: fprintf(fp, "BMS_Current=%" PRId16 "\nPCB_Temperature=%" PRId16 "\nBOOST_Temperature=%" PRId16 "\nMCU_SBC_AMUX_Voltage=%" PRIu16 "\nCheckSum=0x%02X\n", b.Data0C.BMS_Current, b.Data0C.PCB_Temperature, b.Data0C.BOOST_Temperature, b.Data0C.MCU_SBC_AMUX_Voltage, b.Data0C.CheckSum); break;
    }
}

static void DumpUelysysReportParsed(FILE *fp, const Report *rpt)
{
    const uint16_t n = (rpt->RetValSize > sizeof(rpt->RetVal))
                           ? (uint16_t)sizeof(rpt->RetVal)
                           : rpt->RetValSize;
    const uint8_t *p = rpt->RetVal;
    ReportView_t decoded{};
    decoded.reflected_msg_id = rpt->ReflectedMID;
    decoded.reflected_cc = rpt->ReflectedCC;
    decoded.kind = DetermineReportKind(rpt->ReflectedMID, rpt->ReflectedCC);
    const bool payload_valid = DecodePayloadToView(decoded, p, n);
    fprintf(fp, "payload_valid=%u\n", payload_valid ? 1U : 0U);

    if (rpt->ReflectedMID == UELYSYS_TTC_CMD_ID || rpt->ReflectedMID == 0x8018)
    {
        fprintf(fp, "\n[UELYSYS TTC]\n");
        switch (rpt->ReflectedCC)
        {
            case UELYSYS_TTC_NOOP_CC:
            case UELYSYS_TTC_RESET_COUNTERS_CC:
            case UELYSYS_TTC_GET_TIMELINE_HK_CC:
            case UELYSYS_TTC_RESET_TIMELINE_HK_CC:
            case UELYSYS_TTC_PAUSE_TIMELINE_PROCESSING_CC:
            case UELYSYS_TTC_RESUME_TIMELINE_PROCESSING_CC: {
                UELYSYS_TTC_Counters_Report_t x{};
                if (CopyReportPayload(rpt, x))
                    fprintf(fp, "CommandCounter=%" PRIu16 "\nErrorCounter=%" PRIu16 "\n",
                            x.counters[0], x.counters[1]);
                break;
            }
            case UELYSYS_TTC_START_BEACON_BURST_CC: {
                UELYSYS_TTC_BeaconBurst_Report_t x{};
                if (CopyReportPayload(rpt, x))
                    fprintf(fp, "GroupId=%" PRIu16 "\nFirstEntryId=%" PRIu16
                                "\nScheduledEntries=%" PRIu16 "\nFailCount=%" PRIu16 "\n",
                            x.group_id, x.first_entry_id, x.scheduled_entries, x.fail_count);
                break;
            }
            case UELYSYS_TTC_GET_PENDING_ENTRY_COUNT_CC:
            case UELYSYS_TTC_DELETE_ALL_ENTRIES_CC:
            case UELYSYS_TTC_PLUMB_PURGE_TIMELINE_CC: {
                uint16_t x = 0;
                if (n >= sizeof(x)) memcpy(&x, p, sizeof(x));
                fprintf(fp, "Count=%" PRIu16 "\n", x);
                break;
            }
            case UELYSYS_TTC_GET_NEXT_ENTRY_ID_CC: {
                UELYSYS_TTC_NextEntry_Report_t x{};
                if (CopyReportPayload(rpt, x))
                    fprintf(fp, "EntryId=%" PRIu16 "\nGroupId=%" PRIu16 "\nHasEntry=%" PRIu8 "\n",
                            x.entry_id, x.group_id, x.has_entry);
                break;
            }
            case UELYSYS_TTC_GET_NEXT_EXEC_TIME_CC: {
                UELYSYS_TTC_NextExecution_Report_t x{};
                if (CopyReportPayload(rpt, x))
                    fprintf(fp, "NextExecutionTime=%" PRIu32 "\nHasEntry=%" PRIu8 "\n",
                            x.next_execution_time, x.has_entry);
                break;
            }
            case UELYSYS_TTC_INSERT_ABS_CMD_ENTRY_CC: {
                UELYSYS_TTC_InsertAbs_Report_t x{};
                if (CopyReportPayload(rpt, x))
                    fprintf(fp, "ExecutionTimeAbsolute=%" PRIu32 "\nStalenessThreshold=%" PRIu16
                                "\nEntryId=%" PRIu16 "\nGroupId=%" PRIu16
                                "\nExecutionType=%" PRIu8 "\nCommandSize=%" PRIu16 "\n",
                            x.execution_time_absolute, x.staleness_threshold, x.entry_id,
                            x.group_id, x.execution_type, x.command_size);
                break;
            }
            case UELYSYS_TTC_INSERT_REL_CMD_ENTRY_CC: {
                UELYSYS_TTC_InsertRel_Report_t x{};
                if (CopyReportPayload(rpt, x))
                    fprintf(fp, "ExecutionTimeRelative=%" PRIu16 "\nStalenessThreshold=%" PRIu16
                                "\nEntryId=%" PRIu16 "\nGroupId=%" PRIu16
                                "\nExecutionType=%" PRIu8 "\nCommandSize=%" PRIu16 "\n",
                            x.execution_time_relative, x.staleness_threshold, x.entry_id,
                            x.group_id, x.execution_type, x.command_size);
                break;
            }
            case UELYSYS_TTC_DELETE_CMD_ENTRY_CC:
            case UELYSYS_TTC_PLUMB_INIT_ENTRY_CC:
            case UELYSYS_TTC_PLUMB_DELETE_RESERVED_ENTRY_CC: {
                UELYSYS_TTC_EntryGroup_Report_t x{};
                if (CopyReportPayload(rpt, x))
                    fprintf(fp, "EntryId=%" PRIu16 "\nGroupId=%" PRIu16 "\n", x.entry_id, x.group_id);
                break;
            }
            case UELYSYS_TTC_DELETE_CMD_GROUP_CC: {
                UELYSYS_TTC_DeleteGroup_Report_t x{};
                if (CopyReportPayload(rpt, x))
                    fprintf(fp, "GroupId=%" PRIu16 "\nDeleteCount=%" PRIu16 "\n", x.group_id, x.delete_count);
                break;
            }
            case UELYSYS_TTC_EXECUTE_CMD_ENTRY_CC: {
                UELYSYS_TTC_ExecuteEntry_Report_t x{};
                if (CopyReportPayload(rpt, x))
                    fprintf(fp, "GroupId=%" PRIu16 "\nEntryId=%" PRIu16 "\nPersistent=%" PRIu8 "\n",
                            x.group_id, x.entry_id, x.persistent);
                break;
            }
            case UELYSYS_TTC_EXECUTE_CMD_GROUP_CC: {
                UELYSYS_TTC_ExecuteGroup_Report_t x{};
                if (CopyReportPayload(rpt, x))
                    fprintf(fp, "GroupId=%" PRIu16 "\nPersistent=%" PRIu8 "\n", x.group_id, x.persistent);
                break;
            }
            case UELYSYS_TTC_PLUMB_WRITE_ENTRY_CC: {
                UELYSYS_TTC_PlumbWrite_Report_t x{};
                if (CopyReportPayload(rpt, x)) {
                    fprintf(fp, "EntryId=%" PRIu16 "\nGroupId=%" PRIu16 "\nOffset=%" PRIu16
                                "\nChunkSize=%" PRIu16 "\n",
                            x.entry_id, x.group_id, x.offset, x.chunk_size);
                    DumpIndexedBytes(fp, "Data", x.data, (x.chunk_size > 220) ? 220 : x.chunk_size);
                }
                break;
            }
            case UELYSYS_TTC_PLUMB_FINALIZE_ENTRY_CC: {
                UELYSYS_TTC_PlumbFinalize_Report_t x{};
                if (CopyReportPayload(rpt, x))
                    fprintf(fp, "EntryId=%" PRIu16 "\nGroupId=%" PRIu16 "\nTimeTag=%" PRIu32
                                "\nStalenessThreshold=%" PRIu16 "\nCommandSize=%" PRIu16
                                "\nTimeTagType=%" PRIu8 "\nExecutionType=%" PRIu8 "\n",
                            x.entry_id, x.group_id, x.time_tag, x.staleness_threshold,
                            x.command_size, x.time_tag_type, x.execution_type);
                break;
            }
            default:
                fprintf(fp, "No typed return payload for CC=%" PRIu8 "\n", rpt->ReflectedCC);
                break;
        }
        return;
    }

    if (rpt->ReflectedMID == BEE1012_MEOW_CMD_ID || rpt->ReflectedMID == 0x8218 ||
        rpt->ReflectedMID == UELYSYS_MEOW_CMD_ID || rpt->ReflectedMID == 0x8618)
    {
        const bool bee1012_meow = (rpt->ReflectedMID == BEE1012_MEOW_CMD_ID || rpt->ReflectedMID == 0x8218);
        fprintf(fp, "\n[%s MEOW]\n", bee1012_meow ? "BEE1012" : "UELYSYS");
        if (rpt->ReflectedCC == UELYSYS_MEOW_FILE_STAT_CC) {
            UELYSYS_MEOW_FileStat_Report_t x{};
            if (CopyReportPayload(rpt, x)) {
                fprintf(fp, "size=%" PRIu64 "\nmtime=%" PRId64 "\nctime=%" PRId64
                            "\nmode=0x%08" PRIX32 "\nis_dir=%" PRId32 "\npreview_len=%" PRIu64 "\n",
                        x.size, x.mtime, x.ctime, x.mode, x.is_dir, x.preview_len);
                DumpIndexedBytes(fp, "preview", x.preview,
                                 (x.preview_len > sizeof(x.preview)) ? sizeof(x.preview) : (uint16_t)x.preview_len);
            }
        } else if (rpt->ReflectedCC == UELYSYS_MEOW_DISK_STAT_CC) {
            UELYSYS_MEOW_DiskStat_Report_t x{};
            if (CopyReportPayload(rpt, x))
                fprintf(fp, "total_bytes=%" PRIu64 "\navail_bytes=%" PRIu64 "\n", x.total_bytes, x.avail_bytes);
        } else if (rpt->ReflectedCC == UELYSYS_MEOW_SYS_INFO_CC) {
            UELYSYS_MEOW_SysInfo_Report_t x{};
            if (CopyReportPayload(rpt, x))
                fprintf(fp, "uptime=%" PRIu32 "\nload1=%" PRIu32 "\nload5=%" PRIu32
                            "\nload15=%" PRIu32 "\ntotal_ram=%" PRIu64 "\nfree_ram=%" PRIu64
                            "\nnum_procs=%" PRIu16 "\n",
                        x.uptime, x.load1, x.load5, x.load15, x.total_ram, x.free_ram, x.num_procs);
        } else if (rpt->ReflectedCC == UELYSYS_MEOW_SYS_TIME_GET_CC) {
            UELYSYS_MEOW_SysTime_Report_t x{};
            if (CopyReportPayload(rpt, x))
                fprintf(fp, "sec=%" PRId64 "\nnsec=%" PRIu32 "\n", x.sec, x.nsec);
        } else if (rpt->ReflectedCC == UELYSYS_MEOW_CSP_IFSTATS_CC) {
            UELYSYS_MEOW_CspIfStats_Report_t x{};
            if (CopyReportPayload(rpt, x))
                fprintf(fp, "rx=%" PRIu32 "\nrx_error=%" PRIu32 "\ntx=%" PRIu32
                            "\ntx_error=%" PRIu32 "\ndrop=%" PRIu32 "\n",
                        x.rx, x.rx_error, x.tx, x.tx_error, x.drop);
        } else if (rpt->ReflectedCC == UELYSYS_MEOW_FILE_CHECKSUM_CC && n >= 4) {
            uint32_t crc = 0; memcpy(&crc, p, sizeof(crc));
            fprintf(fp, "crc32=0x%08" PRIX32 "\n", crc);
        } else if (rpt->ReflectedCC == UELYSYS_MEOW_FILE_READ_CC ||
                   rpt->ReflectedCC == UELYSYS_MEOW_FILE_TAIL_CC) {
            DumpIndexedBytes(fp, "file_data", p, n);
        } else if (n == sizeof(UELYSYS_MEOW_ShellResult_Report_t)) {
            UELYSYS_MEOW_ShellResult_Report_t x{}; memcpy(&x, p, sizeof(x));
            fprintf(fp, "value0=%" PRId32 "\nvalue1=%" PRId32 "\n", x.exit_code, x.pid);
        } else if (n == sizeof(int32_t)) {
            int32_t value = 0; memcpy(&value, p, sizeof(value));
            fprintf(fp, "value=%" PRId32 "\n", value);
        } else {
            DumpIndexedBytes(fp, "return_data", p, n);
        }
        return;
    }

    if (rpt->ReflectedMID == UELYSYS_STX_CMD_ID || rpt->ReflectedMID == 0x5418)
    {
        fprintf(fp, "\n[UELYSYS STX]\n");
        if (rpt->ReflectedCC == UELYSYS_STX_GET_ALL_PRAMETERS_CC) {
            UELYSYS_STX_AllParameters_Report_t x{};
            if (CopyReportPayload(rpt, x))
                fprintf(fp, "symbol_rate=%" PRIu8 "\ntransmit_power=%" PRIu8 "\nmodcod=%" PRIu8
                            "\nroll_off=%" PRIu8 "\npilot_signal=%" PRIu8 "\nfec_frame_size=%" PRIu8
                            "\npretransmission_delay=%" PRIu16 "\ncenter_frequency=%f\n",
                        x.symbol_rate, x.transmit_power, x.modcod, x.rolloff, x.pilot_signal,
                        x.fec_frame_size, x.pretx_delay, x.center_frequency);
        } else if (rpt->ReflectedCC == UELYSYS_STX_FILESYS_CC_DIR && n >= 4) {
            const UELYSYS_STX_DirHeader_Report_t &hdr = decoded.u.stx_dir;
            fprintf(fp, "command_status=%" PRIu8 "\nflag_more=%" PRIu8 "\nfile_count=%" PRIu16 "\n",
                    hdr.command_status, hdr.flag_more, hdr.file_count);
            const uint8_t *cur = p + sizeof(hdr), *end = p + n;
            uint16_t parsed = 0;
            while (parsed < hdr.file_count && parsed < 20 && cur < end) {
                const uint8_t *nul = (const uint8_t *)memchr(cur, 0, (size_t)(end - cur));
                if (!nul || nul + 1 + sizeof(uint32_t) > end) break;
                char filename[32] = {0};
                size_t name_len = (size_t)(nul - cur);
                if (name_len > sizeof(filename) - 1) name_len = sizeof(filename) - 1;
                memcpy(filename, cur, name_len);
                uint32_t file_size = 0; memcpy(&file_size, nul + 1, sizeof(file_size));
                fprintf(fp, "file[%" PRIu16 "].name=%s\nfile[%" PRIu16 "].size=%" PRIu32 "\n",
                        parsed, filename, parsed, file_size);
                cur = nul + 1 + sizeof(file_size);
                ++parsed;
            }
            fprintf(fp, "parsed_file_count=%" PRIu16 "\n", parsed);
            if (cur < end) DumpIndexedBytes(fp, "unparsed_list_bytes", cur, (uint16_t)(end - cur));
        } else if (rpt->ReflectedCC == UELYSYS_STX_FILESYS_CC_CREATEFILE) {
            UELYSYS_STX_CreateFile_Report_t x{};
            if (CopyReportPayload(rpt, x))
                fprintf(fp, "command_status=%" PRIu8 "\nfile_handle=%" PRIu32 "\n",
                        x.command_status, x.file_handle);
        } else if (rpt->ReflectedCC == UELYSYS_STX_FILESYS_CC_WRITEFILE) {
            UELYSYS_STX_WriteFile_Report_t x{};
            if (CopyReportPayload(rpt, x))
                fprintf(fp, "result=%" PRId32 "\npacket_number=%" PRIu32 "\ncommand_status=%" PRIu8 "\n",
                        x.result, x.packet_number, x.command_status);
        } else if (n == 2) {
            uint16_t value = 0; memcpy(&value, p, sizeof(value));
            fprintf(fp, "value=%" PRIu16 "\n", value);
        } else if (n == 1) {
            fprintf(fp, "value=%" PRIu8 "\n", p[0]);
        } else {
            DumpIndexedBytes(fp, "return_data", p, n);
        }
        return;
    }

    if (rpt->ReflectedMID == UELYSYS_UANT_CMD_ID || rpt->ReflectedMID == 0x4018)
    {
        fprintf(fp, "\n[UELYSYS UANT]\n");
        if (rpt->ReflectedCC == UELYSYS_UANT_GET_STATUS_CC && n >= 8) {
            static const char *names[8] = {"channel_0_state", "channel_0_status", "channel_0_burn_time_left",
                                           "channel_0_burn_tries", "channel_1_state", "channel_1_status",
                                           "channel_1_burn_time_left", "channel_1_burn_tries"};
            for (int i = 0; i < 8; ++i) fprintf(fp, "%s=%" PRIu8 "\n", names[i], p[i]);
        } else if (rpt->ReflectedCC == UELYSYS_UANT_GET_BOARD_STATUS_CC) {
            UELYSYS_UANT_BoardStatus_Report_t x{};
            if (CopyReportPayload(rpt, x))
                fprintf(fp, "seconds_since_boot=%" PRIu32 "\nreboot_count=%" PRIu8 "\n",
                        x.seconds_since_boot, x.reboot_count);
        } else if (rpt->ReflectedCC == UELYSYS_UANT_AUTODEPLOY_CC && n >= 1) {
            fprintf(fp, "autodeploy_result=%" PRId8 "\n", (int8_t)p[0]);
        } else {
            DumpIndexedBytes(fp, "return_data", p, n);
        }
        return;
    }

    if (rpt->ReflectedMID == UELYSYS_UTRX_CMD_ID || rpt->ReflectedMID == 0x5018) {
        fprintf(fp, "\n[UELYSYS UTRX]\n");
        if (rpt->ReflectedCC == UELYSYS_UTRX_NOOP_CC && n == sizeof(UELYSYS_UTRX_Noop_Report_t)) {
            fprintf(fp, "cmd_counter=%" PRIu8 "\napp_err_counter=%" PRIu8 "\ndevice_err_counter=%" PRIu8 "\n",
                    decoded.u.utrx_noop.cmd_counter, decoded.u.utrx_noop.app_err_counter,
                    decoded.u.utrx_noop.device_err_counter);
        } else if (rpt->ReflectedCC == UELYSYS_UTRX_RXCONF_GET_BAUD_CC && n == sizeof(UELYSYS_UTRX_RxBaud_Report_t)) {
            fprintf(fp, "rx_baud=%" PRIu32 "\n", decoded.u.utrx_rx_baud.baud);
        } else if (n == 0) {
            fprintf(fp, "ack_payload=empty\n");
        } else {
            fprintf(fp, "payload_size_mismatch=1\n");
            DumpIndexedBytes(fp, "unexpected_return_data", p, n);
        }
        return;
    }

    if (rpt->ReflectedMID == UELYSYS_PAYUEL_OBC_CMD_ID || rpt->ReflectedMID == 0xC018 ||
        rpt->ReflectedMID == UELYSYS_PAYUEL_CAM_CMD_ID || rpt->ReflectedMID == 0xC318) {
        const bool obc = rpt->ReflectedMID == UELYSYS_PAYUEL_OBC_CMD_ID || rpt->ReflectedMID == 0xC018;
        fprintf(fp, "\n[UELYSYS PAYUEL %s]\n", obc ? "OBC" : "CAM");
        const bool meta = (obc && rpt->ReflectedCC == UELYSYS_PAYUEL_OBC_DOWNLOAD_META_CC) ||
                          (!obc && rpt->ReflectedCC == UELYSYS_PAYUEL_CAM_DOWNLOAD_META_CC);
        const bool download = (obc && rpt->ReflectedCC == UELYSYS_PAYUEL_OBC_DOWNLOAD_IMAGE_CC) ||
                              (!obc && rpt->ReflectedCC == UELYSYS_PAYUEL_CAM_DOWNLOAD_IMAGE_CC);
        const bool chunk = (obc && rpt->ReflectedCC == UELYSYS_PAYUEL_OBC_CHUNK_DOWNLOAD_CC) ||
                           (!obc && rpt->ReflectedCC == UELYSYS_PAYUEL_CAM_CHUNK_DOWNLOAD_CC);
        const bool status = (obc && rpt->ReflectedCC == UELYSYS_PAYUEL_OBC_CAM_SHOT_CC) ||
                            (!obc && (rpt->ReflectedCC == UELYSYS_PAYUEL_CAM_SHOT_CC ||
                                     rpt->ReflectedCC == UELYSYS_PAYUEL_CAM_PROCESS_BINNING_CC));
        if ((meta || download) && n == sizeof(UELYSYS_ImageMeta_Report_t)) {
            const UELYSYS_ImageMeta_Report_t &x = decoded.u.image_meta;
            fprintf(fp, "%s=0x%02X\nimage_index=%" PRIu8 "\nchunk_count=%" PRIu16
                        "\nlast_chunk_size=%" PRIu8 "\nfile_crc32=0x%08" PRIX32 "\n",
                    obc ? "camera_id" : "image_slot", x.object_id, x.image_index,
                    ReadU16BE(x.chunk_count_be), x.last_chunk_size, ReadU32BE(x.file_crc32_be));
        } else if (download && n == sizeof(UELYSYS_DownloadRequest_Report_t)) {
            const UELYSYS_DownloadRequest_Report_t &x = decoded.u.download_request;
            fprintf(fp, "%s=0x%02X\nimage_index=%" PRIu8 "\ndownload_state=queue_or_start_failure\n",
                    obc ? "camera_id" : "image_slot", x.object_id, x.image_index);
        } else if (status && n == sizeof(UELYSYS_PayloadStatus_Report_t)) {
            fprintf(fp, "hardware_status=0x%02X\n", decoded.u.payload_status.status);
        } else if ((meta || status || chunk) && n == sizeof(UELYSYS_PayloadError_Report_t)) {
            const UELYSYS_PayloadError_Report_t &x = decoded.u.payload_error;
            fprintf(fp, "error_command_id=0x%02X\nerror_status=0x%02X\nerror_crc16=0x%04X\n",
                    x.command_id, x.status, ReadU16BE(x.crc16_be));
            DumpIndexedBytes(fp, "payload_error",
                             reinterpret_cast<const uint8_t *>(&x), sizeof(x));
        } else if (n == 0) {
            fprintf(fp, "return_payload=empty\n");
        } else {
            fprintf(fp, "payload_size_mismatch=1\n");
            DumpIndexedBytes(fp, "unexpected_return_data", p, n);
        }
        return;
    }

    if (rpt->ReflectedMID == UELYSYS_PAYUEL_AOS_CMD_ID || rpt->ReflectedMID == 0x4718) {
        fprintf(fp, "\n[UELYSYS PAYUEL AOS]\n");
        if (rpt->ReflectedCC == UELYSYS_PAYUEL_AOS_NOOP_CC &&
            n == sizeof(UELYSYS_AOS_Noop_Report_t)) {
            const UELYSYS_AOS_Noop_Report_t &x = decoded.u.aos_noop;
            fprintf(fp, "cmd_counter=%" PRIu8 "\nerr_counter=%" PRIu8 "\n", x.cmd_counter, x.err_counter);
        } else if (rpt->ReflectedCC == UELYSYS_PAYUEL_AOS_READ_REGISTER_CC &&
            n == sizeof(UELYSYS_AOS_ReadRegister_Report_t)) {
            const UELYSYS_AOS_ReadRegister_Report_t &x = decoded.u.aos_read_register;
            fprintf(fp, "msb=0x%02X\nlsb=0x%02X\nraw_be=%" PRIi16 "\n",
                    x.msb, x.lsb, (int16_t)(((uint16_t)x.msb << 8) | x.lsb));
        } else if (rpt->ReflectedCC == UELYSYS_PAYUEL_AOS_READ_ALL_CHANNELS_CC &&
                   n == sizeof(UELYSYS_AOS_ReadAll_Report_t)) {
            const UELYSYS_AOS_ReadAll_Report_t &x = decoded.u.aos_read_all;
            for (int i = 0; i < 4; ++i)
                fprintf(fp, "resistance[%d]=%f\n", i, x.resistance[i]);
        } else if ((rpt->ReflectedCC == UELYSYS_PAYUEL_AOS_RESET_CC ||
                    rpt->ReflectedCC == UELYSYS_PAYUEL_AOS_WRITE_REGISTER_CC) && n == 0) {
            fprintf(fp, "ack_payload=empty\n");
        } else {
            fprintf(fp, "payload_size_mismatch=1\n");
            DumpIndexedBytes(fp, "unexpected_return_data", p, n);
        }
        return;
    }

    if (rpt->ReflectedMID == UELYSYS_LGBAT_CMD_ID || rpt->ReflectedMID == 0xC618) {
        fprintf(fp, "\n[UELYSYS PAYUEL LGBAT]\n");
        if (rpt->ReflectedCC == UELYSYS_LGBAT_REQUEST_DATA_CC && n == 10) {
            DumpLgbatBlock(fp, decoded.u.lgbat_block);
        } else if (rpt->ReflectedCC == UELYSYS_LGBAT_REQUEST_ALL_DATA_CC && n == 120) {
            const uint8_t *all_blocks =
                reinterpret_cast<const uint8_t *>(&decoded.u.lgbat_all);
            for (int i = 0; i < 12; ++i) {
                fprintf(fp, "\n[BMS Block 0x%02X]\n", i + 1);
                UELYSYS_LGBAT_Block_Report_t block{};
                memcpy(block.raw, all_blocks + i * sizeof(block.raw), sizeof(block.raw));
                DumpLgbatBlock(fp, block);
            }
        } else {
            fprintf(fp, "payload_size_mismatch=1\n");
            DumpIndexedBytes(fp, "unexpected_return_data", p, n);
        }
        return;
    }

    fprintf(fp, "\n[UELYSYS APP REPORT]\n");
    DumpIndexedBytes(fp, "return_data", p, n);
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

    if (DetermineReportKind(rpt->ReflectedMID, rpt->ReflectedCC) >= REPORT_KIND_UELYSYS_TTC &&
        DetermineReportKind(rpt->ReflectedMID, rpt->ReflectedCC) <= REPORT_KIND_UELYSYS_AOS)
    {
        DumpUelysysReportParsed(fp, rpt);
    }
    else if (rpt->ReflectedMID == PAYUEL_ROMA_CMD_MID || rpt->ReflectedMID == 0x3018)
    {
        ReportView_t v{};
        v.reflected_msg_id = rpt->ReflectedMID;
        v.reflected_cc = rpt->ReflectedCC;
        v.ret_code = rpt->RetCode;
        v.kind = DetermineReportKind(v.reflected_msg_id, v.reflected_cc);
        v.roma_parget_index = (v.kind == REPORT_KIND_PAYUEL_ROMA_PARGET)
                                  ? g_roma_parget_requested_index : -1;
        const bool ok = DecodePayloadToView(v, p, payload_len);
        fprintf(fp, "\n[PAYUEL ROMA]\npayload_valid=%u\n", ok);
        switch (v.kind) {
            case REPORT_KIND_PAYUEL_ROMA_NOOP:
                fprintf(fp, "CmdCounter=%" PRIu8 "\nErrCounter=%" PRIu8 "\n", v.u.roma_noop.CmdCounter, v.u.roma_noop.ErrCounter); break;
            case REPORT_KIND_PAYUEL_ROMA_RESETCOUNTERS:
                fprintf(fp, "CmdCounter=%" PRIu8 "\nErrCounter=%" PRIu8 "\n", v.u.roma_resetcounters.CmdCounter, v.u.roma_resetcounters.ErrCounter); break;
            case REPORT_KIND_PAYUEL_ROMA_COMMTEST:
                DumpIndexedBytes(fp, "RxBuf", v.u.roma_commtest.RxBuf, sizeof(v.u.roma_commtest.RxBuf)); break;
            case REPORT_KIND_PAYUEL_ROMA_GETSPECIFICLINE:
                fprintf(fp, "line_no=%" PRIu16 "\ninfo=%" PRIu8 "\nlen=%" PRIu8 "\ntime_s=%" PRIu32 "\ntime_ms=%" PRIu16 "\nincr=%" PRIu16 "\nline=%.*s\n",
                        v.u.roma_getspecificline.line_no, v.u.roma_getspecificline.info, v.u.roma_getspecificline.len,
                        v.u.roma_getspecificline.time_s, v.u.roma_getspecificline.time_ms, v.u.roma_getspecificline.incr,
                        (int)sizeof(v.u.roma_getspecificline.line), v.u.roma_getspecificline.line); break;
            case REPORT_KIND_PAYUEL_ROMA_GETMULTIPLELINES:
                fprintf(fp, "offset=%" PRIu16 "\n", v.u.roma_getmultiplelines.offset);
                DumpIndexedBytes(fp, "buffer", v.u.roma_getmultiplelines.buffer, sizeof(v.u.roma_getmultiplelines.buffer)); break;
            case REPORT_KIND_PAYUEL_ROMA_GETLATESTLINE:
                fprintf(fp, "line_no=%" PRIu16 "\ninfo=%" PRIu8 "\nlen=%" PRIu8 "\ntime_s=%" PRIu32 "\ntime_ms=%" PRIu16 "\nincr=%" PRIu16 "\nline=%.*s\n",
                        v.u.roma_getlatestline.line_no, v.u.roma_getlatestline.info, v.u.roma_getlatestline.len,
                        v.u.roma_getlatestline.time_s, v.u.roma_getlatestline.time_ms, v.u.roma_getlatestline.incr,
                        (int)sizeof(v.u.roma_getlatestline.line), v.u.roma_getlatestline.line); break;
            case REPORT_KIND_PAYUEL_ROMA_GETLATESTNLINES:
                fprintf(fp, "offset=%" PRIu16 "\n", v.u.roma_getlatestNlines.offset);
                DumpIndexedBytes(fp, "buffer", v.u.roma_getlatestNlines.buffer, sizeof(v.u.roma_getlatestNlines.buffer)); break;
            case REPORT_KIND_PAYUEL_ROMA_SAVEROUTE:
                fprintf(fp, "route=%.*s\n", (int)sizeof(v.u.roma_saveroute.route), v.u.roma_saveroute.route); break;
            case REPORT_KIND_PAYUEL_ROMA_PARGET:
                fprintf(fp, "value=%" PRId32 "\nrequested_index=%" PRId8 "\n", v.u.roma_parget.value, v.roma_parget_index); break;
            case REPORT_KIND_PAYUEL_ROMA_SETROUTEDEFAULT: fprintf(fp, "result=%" PRIu8 "\n", v.u.roma_setroutedefault.result); break;
            case REPORT_KIND_PAYUEL_ROMA_RESETROUTE: fprintf(fp, "result=%" PRIu8 "\n", v.u.roma_resetroute.result); break;
            case REPORT_KIND_PAYUEL_ROMA_LOADROUTE: fprintf(fp, "result=%" PRIu8 "\n", v.u.roma_loadroute.result); break;
            case REPORT_KIND_PAYUEL_ROMA_SENDROUTE: fprintf(fp, "result=%" PRIu8 "\n", v.u.roma_sendroute.result); break;
            case REPORT_KIND_PAYUEL_ROMA_SETROUTE: fprintf(fp, "result=%" PRIu8 "\n", v.u.roma_setroute.result); break;
            case REPORT_KIND_PAYUEL_ROMA_PARSET: fprintf(fp, "result=%" PRIu8 "\n", v.u.roma_parset.result); break;
            case REPORT_KIND_PAYUEL_ROMA_PARDEFAULTS: fprintf(fp, "result=%" PRIu8 "\n", v.u.roma_pardefaults.result); break;
            case REPORT_KIND_PAYUEL_ROMA_PARSAVE: fprintf(fp, "result=%" PRIu8 "\n", v.u.roma_parsave.result); break;
            case REPORT_KIND_PAYUEL_ROMA_PARRESTORE: fprintf(fp, "result=%" PRIu8 "\n", v.u.roma_parrestore.result); break;
            case REPORT_KIND_PAYUEL_ROMA_PARLOAD: fprintf(fp, "result=%" PRIu8 "\n", v.u.roma_parload.result); break;
            case REPORT_KIND_PAYUEL_ROMA_PARSETOOB: fprintf(fp, "result=%" PRIu8 "\n", v.u.roma_parsetOOB.result); break;
            case REPORT_KIND_PAYUEL_ROMA_SENDCOMMAND: fprintf(fp, "result=%" PRIu8 "\n", v.u.roma_sendcommand.result); break;
            case REPORT_KIND_PAYUEL_ROMA_SENDMSG:
            case REPORT_KIND_PAYUEL_ROMA_PAYINIT:
                fprintf(fp, "ack_payload=empty\n"); break;
            default: break;
        }
    }
    else if ((rpt->ReflectedMID == EPS_CMD_ID) ||(rpt->ReflectedMID == 0x7518))
	    {
	        switch (rpt->ReflectedCC)
	        {
	            case EPS_P80_POWER_IF_GET_CC:
	            case EPS_P80_POWER_IF_LIST_CC:
	            case EPS_GET_HK_CC:
	            case EPS_GET_HK_ALL_CC:
	            case EPS_RPARAM_GET_FULL_TABLE_CC:
	            case EPS_CSP_PING_CC:
	                DumpEPSReportParsed(fp, rpt);
	                break;

	            case EPS_REPORT_BCN_CC: {
	                EPS_Bcn_Report_t bcn{};
	                if (rpt->RetValSize < sizeof(bcn)) break;
	                memcpy(&bcn, rpt->RetVal, sizeof(bcn));
	                fprintf(fp, "\n[EPS Beacon Report]\n");
	                fprintf(fp, "PMU.bootcause=%" PRIu32 "\nPMU.resetcause=%" PRIu16 "\nPMU.bootcount=%" PRIu16 "\n",
	                        bcn.PMU.bootcause, bcn.PMU.resetcause, bcn.PMU.bootcount);
	                DumpArr_u8(fp, "PMU.out_en", bcn.PMU.out_en, 6);
	                DumpArr_i16(fp, "PMU.temp", bcn.PMU.temp, 2);
	                fprintf(fp, "PMU.batt_mode=%" PRIu8 "\nPMU.batt_i=%" PRIi16 "\nPMU.batt_v=%" PRIu16
	                            "\nPMU.sm_en=%" PRIu8 "\nPMU.gnd_wdt_cnt=%" PRIu16 "\nPMU.bus_wdt_cnt=%" PRIu16
	                            "\nPMU.gnd_wdt_left=%" PRIu32 "\nPMU.bus_wdt_left=%" PRIu32 "\n",
	                        bcn.PMU.batt_mode, bcn.PMU.batt_i, bcn.PMU.batt_v, bcn.PMU.sm_en,
	                        bcn.PMU.gnd_wdt_cnt, bcn.PMU.bus_wdt_cnt,
	                        bcn.PMU.gnd_wdt_left, bcn.PMU.bus_wdt_left);
	                DumpArr_i16(fp, "PDU.out_i", bcn.PDU.out_i, 12);
	                DumpArr_u8(fp, "PDU.out_en", bcn.PDU.out_en, 12);
	                for (int acu = 0; acu < 2; ++acu) {
	                    char label[32];
	                    snprintf(label, sizeof(label), "ACU[%d].input_i", acu);
	                    DumpArr_i16(fp, label, bcn.ACU[acu].input_i, 6);
	                    snprintf(label, sizeof(label), "ACU[%d].input_v", acu);
	                    DumpArr_u16(fp, label, bcn.ACU[acu].input_v, 6);
	                    fprintf(fp, "ACU[%d].mppt_mode=%" PRIu8 "\n", acu, bcn.ACU[acu].mppt_mode);
	                }
	                fprintf(fp, "BP8.bootcount=%" PRIu16 "\nBP8.bootcause=%" PRIu16 "\nBP8.resetcause=%" PRIu16
	                            "\nBP8.soc=%f\nBP8.bat_avr_temp=%f\nBP8.vbat=%" PRIu16
	                            "\nBP8.current=%f\nBP8.heater_i=%" PRIu16 "\n",
	                        bcn.BP8.bootcount, bcn.BP8.bootcause, bcn.BP8.resetcause,
	                        bcn.BP8.soc, bcn.BP8.bat_avr_temp, bcn.BP8.vbat,
	                        bcn.BP8.i, bcn.BP8.heater_i);
	                break;
	            }

	            default:
	                fprintf(fp, "\n[EPS P60] Unknown CC: 0x%02X\n", (unsigned)rpt->ReflectedCC);
                break;
        }
    }
    else if ((rpt->ReflectedMID == GPIO_CMD_ID) || (rpt->ReflectedMID == 0x9218))
    {
        const char *cmd_name = "GPIO Unknown";
        switch (rpt->ReflectedCC)
        {
            case GPIO_DEP1_EN_ON_CC:
                cmd_name = "GPIO DEP1 EN ON";
                break;
            case GPIO_DEP1_EN_OFF_CC:
                cmd_name = "GPIO DEP1 EN OFF";
                break;
            case GPIO_DEP2_EN_ON_CC:
                cmd_name = "GPIO DEP2 EN ON";
                break;
            case GPIO_DEP2_EN_OFF_CC:
                cmd_name = "GPIO DEP2 EN OFF";
                break;
            case GPIO_SP_IN_READ_5S_CC:
                cmd_name = "GPIO SP IN READ 5S";
                break;
            default:
                break;
        }

        fprintf(fp, "\n[GPIO Report]\n");
        fprintf(fp, "Command      : %s\n", cmd_name);
        fprintf(fp, "ReturnData   : none\n");
    }
    else if ((rpt->ReflectedMID == UELYSYS_STX_CMD_ID) || (rpt->ReflectedMID == 0x5418))
    {
        fprintf(fp, "\n[UELYSYS STX Report]\n");
        switch (rpt->ReflectedCC)
        {
            case UELYSYS_STX_SET_MODULE_ID_CC:
                fprintf(fp, "Command      : STX SET MODULE ID\n");
                if (payload_len >= sizeof(uint16_t))
                {
                    uint16_t module_id = 0;
                    memcpy(&module_id, p, sizeof(module_id));
                    fprintf(fp, "ModuleId     : 0x%04X (%u)\n", module_id, module_id);
                }
                else
                {
                    fprintf(fp, "Payload      : length mismatch, expected >= 2\n");
                }
                break;

            case UELYSYS_STX_FILESYS_CC_CREATEFILE:
                fprintf(fp, "Command      : STX FILESYS CREATEFILE\n");
                if (payload_len >= 5)
                {
                    uint8_t command_status = p[0];
                    uint32_t file_handle = 0;
                    memcpy(&file_handle, p + 1, sizeof(file_handle));
                    fprintf(fp, "commad_status: %u\n", command_status);
                    fprintf(fp, "file_handle  : %" PRIu32 "\n", file_handle);
                }
                else
                {
                    fprintf(fp, "Payload      : length mismatch, expected >= 5\n");
                }
                break;

            case UELYSYS_STX_FILESYS_CC_WRITEFILE:
                fprintf(fp, "Command      : STX FILESYS WRITEFILE\n");
                if (payload_len >= 9)
                {
                    int32_t ret = 0;
                    uint32_t packet_number = 0;
                    memcpy(&ret, p, sizeof(ret));
                    memcpy(&packet_number, p + 4, sizeof(packet_number));
                    fprintf(fp, "ret          : %" PRId32 "\n", ret);
                    fprintf(fp, "packet_number: %" PRIu32 "\n", packet_number);
                    fprintf(fp, "cstatus      : %u\n", p[8]);
                }
                else
                {
                    fprintf(fp, "Payload      : length mismatch, expected >= 9\n");
                }
                break;

            case UELYSYS_STX_FILESYS_CC_SENDFILE:
                fprintf(fp, "Command      : STX FILESYS SENDFILE\n");
                if (payload_len >= 1)
                    fprintf(fp, "rxdata       : %u\n", p[0]);
                else
                    fprintf(fp, "Payload      : empty (error/no result data)\n");
                break;

            default:
                fprintf(fp, "Command      : UELYSYS STX CC 0x%02X\n", (unsigned)rpt->ReflectedCC);
                fprintf(fp, "Parser       : not specialized\n");
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




int BEE1000ReportSaver(Report* rpt)
{
    if (!rpt) return -1;

    ReportCounter++;

    char filename[128];
    time_t tmtime = time(0);
    struct tm* local = localtime(&tmtime);

    sprintf(filename,
            "../data/BEE1000/report/report_parsed/Report--%04d-%02d-%02d-%02d-%02d-%02d--.txt",
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

int BEE1012ReportSaver(Report* rpt)
{
    if (!rpt) return -1;

    ReportCounter++;

    char filename[128];
    time_t tmtime = time(0);
    struct tm* local = localtime(&tmtime);

    sprintf(filename,
            "../data/BEE1012/report/report_parsed/Report--%04d-%02d-%02d-%02d-%02d-%02d--.txt",
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

int UELYSYSReportSaver(Report* rpt)
{
    if (!rpt) return -1;

    ReportCounter++;

    char filename[128];
    time_t tmtime = time(0);
    struct tm* local = localtime(&tmtime);

    sprintf(filename,
            "../data/UELYSYS/report/report_parsed/Report--%04d-%02d-%02d-%02d-%02d-%02d--.txt",
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
        const int dport = csp_conn_dport(conn);
        unsigned int read_timeout = setup->default_timeout;

        if (dport == 25 || dport == 15 || dport == 10) {
            read_timeout = REPORT_CONN_DRAIN_TIMEOUT_MS;
        }

        while ((packet = csp_read(conn, read_timeout)) != NULL) {

            const int dport = csp_conn_dport(conn);
            if ((dport == 25 || dport == 15 || dport == 10) &&
                g_report_collecting && g_report_dport != dport)
            {
                console.AddLog("[WARN]##Report port changed during assembly (%d -> %d). Dropping partial report.",
                               g_report_dport, dport);
                g_report_collecting = false;
                g_report_off = 0;
                g_report_dport = -1;
                g_report_have_last_time = false;
                g_report_last_chunk_time = 0;
                memset(g_report_wire, 0, sizeof(g_report_wire));
            }

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



                case 29: {
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

                        console.AddLog("Received AIOBC TM from port : %d.\n", dport);

                        FILE *evnt_fp = fopen(eventfilename, "wb");
                        printf("\TM Length: %u", packet->length);

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
                    

                    printf("AIOBC Packet Length: %u\n", packet->length);
                    printf("===== AIOBC TM PACKET DUMP =====\n");
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
                            g_report_dport = -1;
                            memset(g_report_wire, 0, sizeof(g_report_wire));

                            g_report_have_last_time = false;
                            g_report_last_chunk_time = 0;
                        }
                    }

                    if (!g_report_collecting) {
                        g_report_collecting = true;
                        g_report_dport = dport;
                        g_report_off = 0;
                        memset(g_report_wire, 0, sizeof(g_report_wire));
                    }

                    g_report_last_chunk_time = now;
                    g_report_have_last_time = true;

                    if (g_report_off > REPORT_WIRE_SIZE) {
                        g_report_collecting = false;
                        g_report_off = 0;
                        g_report_dport = -1;
                        memset(g_report_wire, 0, sizeof(g_report_wire));
                        g_report_have_last_time = false;
                        g_report_last_chunk_time = 0;
                    }

                    size_t remain = REPORT_WIRE_SIZE - g_report_off;

                    if (chunk_len > remain) {
                        printf("case25: overflow (off=%zu, chunk=%zu). drop current and restart with this chunk.\n",
                            g_report_off, chunk_len);

                        g_report_collecting = true;
                        g_report_dport = dport;
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
                                BEE1000ReportSaver(&rpt);
                            } else {
                                printf("case25: ParseReportWire540 failed\n");
                            }

                            g_report_collecting = false;
                            g_report_off = 0;
                            g_report_dport = -1;
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

                case 15: {
                    FILE *rpt_fp = NULL;

                    char rptpktfilename[128];
                    time_t tmtime = time(0);
                    struct tm *local = localtime(&tmtime);

                    sprintf(rptpktfilename,
                            "../data/BEE1012/report/report_raw/rpt_raw--%04d-%02d-%02d-%02d-%02d-%02d--",
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

                    printf("case15: chunk_len=%zu\n", chunk_len);

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
                            printf("case15: timeout dt=%.1f sec (>5). drop current report assembler.\n", dt);

                            g_report_collecting = false;
                            g_report_off = 0;
                            g_report_dport = -1;
                            memset(g_report_wire, 0, sizeof(g_report_wire));

                            g_report_have_last_time = false;
                            g_report_last_chunk_time = 0;
                        }
                    }

                    if (!g_report_collecting) {
                        g_report_collecting = true;
                        g_report_dport = dport;
                        g_report_off = 0;
                        memset(g_report_wire, 0, sizeof(g_report_wire));
                    }

                    g_report_last_chunk_time = now;
                    g_report_have_last_time = true;

                    if (g_report_off > REPORT_WIRE_SIZE) {
                        g_report_collecting = false;
                        g_report_off = 0;
                        g_report_dport = -1;
                        memset(g_report_wire, 0, sizeof(g_report_wire));
                        g_report_have_last_time = false;
                        g_report_last_chunk_time = 0;
                    }

                    size_t remain = REPORT_WIRE_SIZE - g_report_off;

                    if (chunk_len > remain) {
                        printf("case15: overflow (off=%zu, chunk=%zu). drop current and restart with this chunk.\n",
                            g_report_off, chunk_len);

                        g_report_collecting = true;
                        g_report_dport = dport;
                        g_report_off = 0;
                        memset(g_report_wire, 0, sizeof(g_report_wire));
                        remain = REPORT_WIRE_SIZE;
                    }

                    if (chunk_len <= remain) {
                        memcpy(g_report_wire + g_report_off, packet->data, chunk_len);
                        g_report_off += chunk_len;

                        printf("case15: assembled %zu/%zu\n", g_report_off, (size_t)REPORT_WIRE_SIZE);

                        if (g_report_off == REPORT_WIRE_SIZE) {
                            Report rpt;
                            bool ok = ParseReportWire540(g_report_wire, REPORT_WIRE_SIZE, rpt);

                            if (ok) {
                                printf("\ncase15: REPORT COMPLETE \nMsgId=0x%04x \nRefMID=0x%04x \nCC=0x%02x \nRetSize=%u\n",
                                    rpt.CCSDS_MsgId, rpt.ReflectedMID, rpt.ReflectedCC, (unsigned)rpt.RetValSize);
                                UpdateReportViewFromReport(rpt);
                                BEE1012ReportSaver(&rpt);
                            } else {
                                printf("case15: ParseReportWire540 failed\n");
                            }

                            g_report_collecting = false;
                            g_report_off = 0;
                            g_report_dport = -1;
                            memset(g_report_wire, 0, sizeof(g_report_wire));
                            g_report_have_last_time = false;
                            g_report_last_chunk_time = 0;
                        }
                    } else {
                        printf("case15: chunk too big to fit even after restart (%zu)\n", chunk_len);
                    }

                    if (rpt_fp) fclose(rpt_fp);
                    break;
                }

                case 10: {
                    FILE *rpt_fp = NULL;

                    char rptpktfilename[128];
                    time_t tmtime = time(0);
                    struct tm *local = localtime(&tmtime);

                    sprintf(rptpktfilename,
                            "../data/UELYSYS/report/report_raw/report_raw_%04d-%02d-%02d-%02d-%02d-%02d--",
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

                    printf("case10: chunk_len=%zu\n", chunk_len);

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
                            printf("case10: timeout dt=%.1f sec (>5). drop current report assembler.\n", dt);

                            g_report_collecting = false;
                            g_report_off = 0;
                            g_report_dport = -1;
                            memset(g_report_wire, 0, sizeof(g_report_wire));

                            g_report_have_last_time = false;
                            g_report_last_chunk_time = 0;
                        }
                    }

                    if (!g_report_collecting) {
                        g_report_collecting = true;
                        g_report_dport = dport;
                        g_report_off = 0;
                        memset(g_report_wire, 0, sizeof(g_report_wire));
                    }

                    g_report_last_chunk_time = now;
                    g_report_have_last_time = true;

                    if (g_report_off > REPORT_WIRE_SIZE) {
                        g_report_collecting = false;
                        g_report_off = 0;
                        g_report_dport = -1;
                        memset(g_report_wire, 0, sizeof(g_report_wire));
                        g_report_have_last_time = false;
                        g_report_last_chunk_time = 0;
                    }

                    size_t remain = REPORT_WIRE_SIZE - g_report_off;

                    if (chunk_len > remain) {
                        printf("case10: overflow (off=%zu, chunk=%zu). drop current and restart with this chunk.\n",
                            g_report_off, chunk_len);

                        g_report_collecting = true;
                        g_report_dport = dport;
                        g_report_off = 0;
                        memset(g_report_wire, 0, sizeof(g_report_wire));
                        remain = REPORT_WIRE_SIZE;
                    }

                    if (chunk_len <= remain) {
                        memcpy(g_report_wire + g_report_off, packet->data, chunk_len);
                        g_report_off += chunk_len;

                        printf("case10: assembled %zu/%zu\n", g_report_off, (size_t)REPORT_WIRE_SIZE);

                        if (g_report_off == REPORT_WIRE_SIZE) {
                            Report rpt;
                            bool ok = ParseReportWire540(g_report_wire, REPORT_WIRE_SIZE, rpt);

                            if (ok) {
                                printf("\ncase10: REPORT COMPLETE \nMsgId=0x%04x \nRefMID=0x%04x \nCC=0x%02x \nRetSize=%u\n",
                                    rpt.CCSDS_MsgId, rpt.ReflectedMID, rpt.ReflectedCC, (unsigned)rpt.RetValSize);
                                UpdateReportViewFromReport(rpt);
                                UELYSYSReportSaver(&rpt);
                            } else {
                                printf("case10: ParseReportWire540 failed\n");
                            }

                            g_report_collecting = false;
                            g_report_off = 0;
                            g_report_dport = -1;
                            memset(g_report_wire, 0, sizeof(g_report_wire));
                            g_report_have_last_time = false;
                            g_report_last_chunk_time = 0;
                        }
                    } else {
                        printf("case10: chunk too big to fit even after restart (%zu)\n", chunk_len);
                    }

                    if (rpt_fp) fclose(rpt_fp);
                    break;
                }                

                case 31: {
                    if (packet->length == BEE1000_LEN_BEACON) {
                        char bcnpktfilename[128];
                        time_t tmtime = time(0);
                        struct tm *local = localtime(&tmtime);

                        sprintf(bcnpktfilename,
                                "../data/BEE1000/beacon/beacon_raw/bcnpkt--%04d-%02d-%02d-%02d-%02d-%02d--",
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

                        memset(bee1000_beacon, 0, sizeof(*bee1000_beacon));
                        memcpy(bee1000_beacon, packet->data, BEE1000_LEN_BEACON);
                        BEE1000BeaconSaver(bee1000_beacon);

                        if (bcn_fp) fclose(bcn_fp);
                    }
                    else if (packet->length == BEE_LEN_MISSIONBEACON) {
                        char misnbcnpktfilename[128];
                        time_t tmtime = time(0);
                        struct tm *local = localtime(&tmtime);

                        sprintf(misnbcnpktfilename,
                                "../data/BEE1000/missionbeacon/missionbeacon_raw/misnbcnpkt--%04d-%02d-%02d-%02d-%02d-%02d--",
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
                                "../data/unknown/unknown--%04d-%02d-%02d-%02d-%02d-%02d--",
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
                case 17: {
                    HandleBEE1012BeaconPacket(packet, dport);
                    break;
                }

                case 12: {
                    HandleUELYSYSBeaconPacket(packet, dport);
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


packetsign * PingInit(FSWTle * FSWTleinfo)
{
    packetsign * sign = (packetsign *)malloc(sizeof(FSWTle) + MIM_HAND_DATAFDSTART);
    sign->Identifier = MIM_ID;
    sign->PacketType = MIM_PT_PING;
    sign->Length = sizeof(FSWTle);
    memcpy(sign->Data, (void *)FSWTleinfo, sizeof(FSWTle));
    return sign;
}

csp_packet_t * PacketEncoder(packetsign * sign, bool freeer)
{
    if(sign == NULL)
    {
        console.AddLog("[DEBUG]##NULL POINTER GIVEN TO PACKETENCODER");
        return NULL;
    }

    uint32_t len = sign->Length;
    csp_packet_t *packet = (csp_packet_t *)csp_buffer_get(len);
    if(packet == NULL)
    {
        console.AddLog("[ERROR]##PacketEncoder cannot allocate CSP packet. Length : %u", len);
        if(freeer)
            sign = NULL;
        return NULL;
    }

    packet->length = len;
    memcpy(packet->data, sign->Data, len);

    if(State.Debugmode)
    {
        printf("packet len: %u\n", packet->length);
        for (int i = 0; i < packet->length; i++)
            printf("0x%x ", packet->data[i]);
        printf("\n");
    }

    if(freeer)
        sign = NULL;
    return packet;
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
                if ((txconn = csp_connect(CSP_PRIO_HIGH, setup->obc_node, 13, MIM_DEFAULT_TIMEOUT, 0)) == NULL) {
                /*!!!!!!!!!!!Revise setup->obc_node!!!!!!!!!!*/ //-> Change to 28.
                /*!!!!!!!!!!!!Need to revise Port!!!!!!!!!!!*/
                    continue;
                }
                else
                    break;
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
        // Added for AIOBC
        case B12_UL_AIOBC : { // == 10. Add for TMTC Test
            csp_packet_t * confirm_ = (csp_packet_t *)csp_buffer_get(MIM_LEN_PACKET);
            while(State.uplink_mode)
            {
                if ((txconn = csp_connect(CSP_PRIO_HIGH, 14, 10, MIM_DEFAULT_TIMEOUT, 0)) == NULL) {
                /*!!!!!!!!!!!Revise setup->obc_node!!!!!!!!!!*/ //-> Change to 28.
                /*!!!!!!!!!!!!Need to revise Port!!!!!!!!!!!*/
                    continue;
                }
                else
                    break;
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
    if(sign!=NULL)
    {
        free(sign);
        sign = NULL;
    }

    State.uplink_mode = false;
    if(dlstate)
    {
        State.downlink_mode = dlstate;
    }
    pthread_mutex_unlock(&conn_lock);
}

/* Local scheduled command uplink removed; TTC timeline TCs remain normal TC cases. */
packetsign * PacketDecoder(csp_packet_t * packet)
{
    uint32_t packetsignlen = packet->length +4;
    packetsign * sign = (packetsign *)malloc(packetsignlen);
    memcpy(sign, (packet->data), 4);
    sign->Length = packet->length-4;
    memcpy(sign->Data, packet->data + 4, sign->Length);
    return sign;
}

vector<vector<float>> AIOBC_datachunk_parser(string path)
{
    vector<vector<float>> data;
    ifstream file(path);

    if (!file.is_open()){
        console.AddLog("Fail to open TC File");
        return data;
    }

    string line;
    while(getline(file, line)){
        string cell;
        stringstream ss(line);
        int num_field = 34;

        vector<float> row;

        for (int i = 0 ; i < num_field ; i++)
        {
            if(!getline(ss, cell, ',')){
                break;
            }

            row.push_back(stof(cell));
        }

        data.push_back(row);
    }

    file.close();
    return data;
}

packetsign * CmdGenerator_GS::GenerateCMDPacket(void)
{
    packetsign * ResultPacket = (packetsign * )malloc(this->GetSize() + MIM_HAND_DATAFDSTART);
    ResultPacket->Identifier = MIM_ID;
    if(!this->Checksum)
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
