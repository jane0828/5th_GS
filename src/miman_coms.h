#pragma once
#ifndef _MIMAN_COMS_H_
#define _MIMAN_COMS_H_

#include "miman_config.h"
#include "miman_orbital.h"
#include <mutex>

#pragma once
#include <mutex>






typedef struct{
	uint16_t Identifier;
	uint16_t PacketType;
	uint32_t Length;
	uint8_t Data[];

}__attribute__((packed)) packetsign;

typedef struct{
	uint16_t target;
	uint16_t filestatus;
	uint32_t filenum;
	uint32_t offset;
	uint32_t step;
}__attribute__((packed)) dlreqdata;

typedef struct{
	int32_t filenum;
	uint32_t file[];
}__attribute__((packed)) filelist;

// typedef enum {
//     GS_FTP_INFO_FILE  = 0,   /**< File size and checksum */
//     GS_FTP_INFO_CRC = 1,         /**< CRC of remote and local file */
//     GS_FTP_INFO_COMPLETED = 2,   /**< Completed and total chunks */
//     GS_FTP_INFO_PROGRESS = 3,    /**< Current chunk, total_chunks and chunk_size */
// } gs_ftp_info_type_t;

typedef struct {
    FILE       * fp;
    FILE       * fp_map;
    csp_conn_t * conn;
    uint32_t   timeout;
    char       file_name[GS_FTP_PATH_LENGTH];
    uint32_t   file_size;
    uint32_t   chunks;
    int        chunk_size;
    uint32_t   checksum;
    ftp_status_element_t last_status[GS_FTP_STATUS_CHUNKS];
    uint32_t   last_entries;
    gs_ftp_info_callback_t info_callback;
    void       * info_data;
} gs_ftp_state_t;

typedef struct {
    gs_ftp_backend_type_t backend;
    const char * path;
    uint32_t addr;
    uint32_t size;
} gs_ftp_url_t;



// BEE-1000 RPT structure
typedef struct {
    bool valid;
    uint16_t CCMessage_ID;
    uint16_t CCCount;
    uint16_t CCLength;
    uint8_t CCTime_code[6];
    uint16_t msg_id;
    uint8_t cc;
    uint8_t ret_type;
    int32_t ret_code;
    uint16_t ret_val_size;
    std::vector<uint8_t> payload;
} ReportPacket_t;

extern ReportPacket_t g_last_report;


// BEE-1000 Beacon
typedef struct {

	// CCSDS Header
    uint8 CCSDS_MID[2];
    uint8 CCSDS_Seq[2];
    uint8 CCSDS_Len[2];
    uint8 CCSDS_TimeCode[6];

	// FSW - RPT - shoud be deprecated
	// uint16 RPT_BootCount;
    // uint32 RPT_ScTimeSec;
    // uint32 RPT_ScTimeSubsec;
    // uint32 RPT_Sequence;
    // uint8 RPT_ResetCause;

    /* FSW - RPT - Revised (Kweon Hyeokjin) */
    uint8_t RPT_CmdCounter;
    uint8_t RPT_ErrCounter;
    uint8_t RPT_ReportCnt;
    uint8_t RPT_CriticalCnt;
    uint16_t RPT_BootCount;
    uint32_t RPT_ScTimeSec;
    uint32_t RPT_ScTimeSubsec;
    uint8_t RPT_Sequence_LSB;
    /*--------------End of BEE RPT Revision (Kweon Hyeokjin)--------------*/

	// COMS - STX
    // all param
    uint8_t  STX_symbol_rate;
    uint8_t  STX_transmit_power;
    uint8_t  STX_modcod;
    uint8_t  STX_roll_off;
    uint8_t  STX_pilot_signal;
    uint8_t  STX_fec_frame_size;
    uint16_t STX_pretransmission_delay;
    float    STX_center_frequency;
    // modulation interface -> cpu temp 빠짐
    uint8_t STX_modulator_interface_type;  
    uint8_t STX_lvds_io_type;    
	uint8 STX_SystemState;
    uint8 STX_StatusFlag;
    // float STX_CpuTemp;

	// COMS - UANT
    uint8 UANT1_Chan0;
    uint8 UANT1_Chan1;
    uint8 UANT1_BackupActive;
    uint8 UANT2_Chan0;
    uint8 UANT2_Chan1;
    uint8 UANT2_BackupActive;

	// COMS - UTRX
    uint8 UTRX_ActiveConf;
    uint16 UTRX_BootCount;
    uint32 UTRX_BootCause;
    int16 UTRX_BoardTemp;

    // PCDU - P60 Dock
    int16 P60D_Cout[9];
    uint16 P60D_Vout[9];
    uint16 P60D_OutEn;
    uint32 P60D_BootCause;
    uint32 P60D_BootCount;
    uint8 P60D_BattMode;
    uint8 P60D_HeaterOn;
    uint16 P60D_VbatV;
    int16 P60D_VccC;
    uint16 P60D_BattV;
    int16 P60D_BattTemp[2];
    uint32 P60D_WdtCanLeft;
    int16 P60D_BattChrg;
    int16 P60D_BattDischrg;

    // PCDU - P60 PDU
    int16  P60P_Cout[9];
    uint16 P60P_Vout[9];
    int16  P60P_Vcc;
    uint8  P60P_ConvEn;
    uint16 P60P_OutEn;

    // PCDU - P60 ACU
    int16  P60A_Cin[6];
    uint16 P60A_Vin[6];

    // ADCS
       /** Combined Power State
     *  | 7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
     *  +--------------------------------------+
     *  |Rsv|RWL0|RWL1|RWL2|MAG0|GYRO|FSS0|HSS0|
     *  +--------------------------------------+
     */
    uint8 ADCS_PowerState; // ID 183

    uint8 ADCS_ControlMode; // ID 185

    float ADCS_GYR0CalibratedRateXComponent;
    float ADCS_GYR0CalibratedRateYComponent;
    float ADCS_GYR0CalibratedRateZComponent; // ID 207, 12bytes

}__attribute__((packed)) Beacon;

#define a sizeof(Beacon);



typedef struct CFE_SRL_HousekeepingTlm_Payload {
    uint8 CommandCounter;

    uint8 CommandErrorCounter;

    uint8 IOHandleStatus[4];

    uint16 IOHandleTxCount[4];
    
}__attribute__((packed)) CFE_SRL_HousekeepingTlm_Payload_t;

typedef struct RPT_HkTlm_Payload{
    uint8 CmdCounter;
    uint8 CmdErrCounter;

    /**
     * Queue Info
     */
    uint8 ReportQueueCnt;
    uint8 CriticalQueueCnt;

    /**
     * Operation Data
     */
    uint16 BootCount;
    uint32 TimeSec;
    uint32 TimeSubsec;
    uint32 Sequence; /* Backup data numbering */

    /**
     * Reset Cause
     */
    uint8 ResetCause;

}__attribute__((packed)) RPT_BcnTlm_Payload_t;

typedef struct PAY_BcnTlm_Payload {
    uint8 CommandCounter;
    uint8 CommandErrorCounter;

    /**
     * Else ....
     */
    /* compact beacon subset */
    int8  sys_status;         /* payload system status */
    int16 temp_ntc_0;
    int16 temp_ntc_1;
    int16 temp_ntc_2;     
    int16 temp_ntc_3;
    int16 temp_ntc_4;         /* board temp 4 */
    int16 temp_ntc_5;         /* board temp 5 */
    int16 temp_ntc_6;         /* board temp 6 */
    int16 temp_ntc_7;         /* board temp 7 */
    int16 temp_ntc_8;         /* board temp 8 */
    int16 temp_ntc_9;         /* board temp 9 */
    int16 temp_ntc_10;        /* board temp 10 */
    int16 temp_ntc_11;        /* board temp 11 */     


} PAY_BcnTlm_Payload_t;

typedef struct PAY_HkTlm_Payload {
    uint8 CommandCounter;
    uint8 CommandErrorCounter;

    int8  sys_status;         /* payload system status */
    int16 temp_ntc_0;         /* board temp 0 */
    int16 temp_ntc_1;         /* board temp 1 */
    int16 temp_ntc_2;         /* board temp 2 */
    int16 temp_ntc_3;         /* board temp 3 */
    int16 temp_ntc_4;         /* board temp 4 */
    int16 temp_ntc_5;         /* board temp 5 */
    int16 temp_ntc_6;         /* board temp 6 */
    int16 temp_ntc_7;         /* board temp 7 */
    int16 temp_ntc_8;         /* board temp 8 */
    int16 temp_ntc_9;         /* board temp 9 */
    int16 temp_ntc_10;        /* board temp 10 */
    int16 temp_ntc_11;        /* board temp 11 */
    /* Expanded TM (selected currents/sensors) */
    uint32 sen1_data_0;
    uint32 sen1_data_1;  

} PAY_HkTlm_LINPayload_t;

// BEE-1000 Mission Beacon
typedef struct {

	// Telemetry header
    uint8 CCSDS_MID[2];
    uint8 CCSDS_Seq[2];
    uint8 CCSDS_Len[2];
    uint8 CCSDS_TimeCode[6];


    CFE_SRL_HousekeepingTlm_Payload_t srlpayload;

    RPT_BcnTlm_Payload_t    rptpayload;

    // Payload
    PAY_BcnTlm_Payload_t    paybcnpayload;



    PAY_HkTlm_LINPayload_t     payhkpayload;
    

}__attribute__((packed)) MissionBeacon;

#define a sizeof(MissionBeacon);


struct GETFILEINFO {
    // ===== TLM Header =====
    uint8 CCSDS_MID[2];
    uint8 CCSDS_Seq[2];
    uint8 CCSDS_Len[2];
    uint8 CCSDS_TimeCode[6];
    uint8 padding[4];

    // ===== Payload =====
    uint8 FileStatus;
    uint8 CRC_Computed;
    uint8 Spare[2];
    uint32 CRC;
    uint32 FileSize;
    uint32 LastModifiedTime;
    uint32 Mode;
    char Filename[64];

};

#define a sizeof(GETFILEINFO);


struct Report {
    // ===== CCSDS Header =====v  16
    uint16 CCSDS_MsgId;
    uint16 CCSDS_Seq;
    uint16 CCSDS_Len;
    uint8 CCSDS_TimeCode[6];
    uint32 CCSDS_Padding;


    // ===== Report Body =====  10     26 byte
    uint16 ReflectedMID;
    uint8  ReflectedCC;
    uint8  RetType;
    int32  RetCode;
    uint16 RetValSize;
    uint8  RetVal[512];

};

#define a sizeof(Report)

struct Event {

    uint16 CCSDS_MsgId;
    uint16 CCSDS_Seq;
    uint16 CCSDS_Len;
    uint8 CCSDS_TimeCode[6];
    uint32 CCSDS_Padding;

    char AppName[20]; /**< 20임   \cfetlmmnemonic \EVS_APPNAME
                                                \brief Application name */
    uint16 EventID;                        /**< \cfetlmmnemonic \EVS_EVENTID
                                                \brief Numerical event identifier */
    uint16 EventType;    /**< uint16임 \cfetlmmnemonic \EVS_EVENTTYPE  
                                                \brief Numerical event type identifier */
    uint32 SpacecraftID;                   /**< \cfetlmmnemonic \EVS_SCID
                                                \brief Spacecraft identifier */
    uint32 ProcessorID;                    /**< \cfetlmmnemonic \EVS_PROCESSORID
                                                \brief Numerical processor identifier */


    char               Message[122]; /**< 122임 \cfetlmmnemonic \EVS_EVENT
                                                                 \brief Event message string */
    uint8 Spare1;                                                   /**< \cfetlmmnemonic \EVS_SPARE1
                                                                         \brief Structure padding */
    uint8 Spare2;                                                   /**< \cfetlmmnemonic \EVS_SPARE2
                                                                     \brief Structure padding */

};

#define a sizeof(Event)

typedef enum
{
    REPORT_KIND_NONE = 0,

    // ADCS Sunpointing RPT
    REPORT_KIND_ADCS_LOG_MASK,
    REPORT_KIND_ADCS_UNSOLICIT_TLM_SETUP_TLM,

    REPORT_KIND_UANT_GET_STATUS_TLM,

    REPORT_KIND_EPS_P60_DOCK_GET_TABLE_HK,
    REPORT_KIND_EPS_P60_PDU_GET_TABLE_HK,
    REPORT_KIND_EPS_P60_ACU_GET_TABLE_HK,

/****************************************************************************************** */
    REPORT_KIND_ADCS_GET_ERROR_LOG_SETTING,
    REPORT_KIND_ADCS_GET_CURRENT_UNIX_TIME,
    REPORT_KIND_ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC,
    REPORT_KIND_ADCS_GET_COMMUNICATION_STATUS,
    REPORT_KIND_ADCS_GET_CONTROL_ESTIMATION_MODE,
    REPORT_KIND_ADCS_GET_REFERENCE_IRC_VECTOR,
    REPORT_KIND_ADCS_GET_REFERENCE_LLH_TARGET,
    REPORT_KIND_ADCS_GET_ORBIT_MODE,
    REPORT_KIND_ADCS_GET_HEALTH_TLM_MMT,
    REPORT_KIND_ADCS_GET_RAW_CUBESENSE_SUN,
    REPORT_KIND_ADCS_GET_REFERENCE_RPY_VALUES,
    REPORT_KIND_ADCS_GET_OPENLOOPCMD_MTQ,
    REPORT_KIND_ADCS_GET_POWER_STATE,
    REPORT_KIND_ADCS_GET_RUN_MODE,
    REPORT_KIND_ADCS_GET_CONTROL_MODE,
    REPORT_KIND_ADCS_GET_MAG0_MMT_CALIB_CONFIG,
    REPORT_KIND_ADCS_GET_MAG1_MMT_CALIB_CONFIG,
    REPORT_KIND_ADCS_GET_ESTIMATION_MODE,
    REPORT_KIND_ADCS_GET_OPERATIONAL_STATE,
    REPORT_KIND_ADCS_GET_RAW_CSS_SENSOR,
    REPORT_KIND_ADCS_GET_RAW_GYR_SENSOR,
    REPORT_KIND_ADCS_GET_CALIBRATED_GYR_SENSOR,
    REPORT_KIND_ADCS_GET_MAG_SENSING_ELM_CONFIG,
    REPORT_KIND_ADCS_GET_TLM_LOG_INCLMASK,
    REPORT_KIND_ADCS_GET_UNSOLICIT_TLM_MSG_SETUP,
    REPORT_KIND_ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP,
    REPORT_KIND_ADCS_GET_EVENT_LOG_STATUS_RESPONSE,
    REPORT_KIND_ADCS_GET_PORTMAP,
/********************************************************************************************* */

/****************************************5차 추가*************************************** */
    // 1. PAYUEL_ROMA
    REPORT_KIND_PAYUEL_ROMA_NOOP,            // CC 0, 1
    REPORT_KIND_PAYUEL_ROMA_RESETCOUNTERS,
    REPORT_KIND_PAYUEL_ROMA_COMMTEST,
    REPORT_KIND_PAYUEL_ROMA_GETSPECIFICLINE,
    REPORT_KIND_PAYUEL_ROMA_GETMULTIPLELINES,
    REPORT_KIND_PAYUEL_ROMA_GETLATESTLINE,
    REPORT_KIND_PAYUEL_ROMA_GETLATESTNLINES,
    REPORT_KIND_PAYUEL_ROMA_SETROUTEDEFAULT,
    REPORT_KIND_PAYUEL_ROMA_RESETROUTE,
    REPORT_KIND_PAYUEL_ROMA_LOADROUTE,
    REPORT_KIND_PAYUEL_ROMA_SAVEROUTE,
    REPORT_KIND_PAYUEL_ROMA_SENDROUTE,
    REPORT_KIND_PAYUEL_ROMA_SETROUTE,
    REPORT_KIND_PAYUEL_ROMA_PARGET,
    REPORT_KIND_PAYUEL_ROMA_PARSET,
    REPORT_KIND_PAYUEL_ROMA_PARDEFAULTS,
    REPORT_KIND_PAYUEL_ROMA_PARSAVE,
    REPORT_KIND_PAYUEL_ROMA_PARRESTORE,
    REPORT_KIND_PAYUEL_ROMA_PARLOAD,
    REPORT_KIND_PAYUEL_ROMA_PARSETOOB,
    REPORT_KIND_PAYUEL_ROMA_SENDCOMMAND,


    // 2. PAYUEL_LGPM
    REPORT_KIND_PAYUEL_LGPM_NOOP,               // CC 0
    REPORT_KIND_PAYUEL_LGPM_RESETCOUNTERS,
    REPORT_KIND_PAYUEL_LGPM_MCU_ALIVE,          // CC 2
    REPORT_KIND_PAYUEL_LGPM_3V3_PWR_ON,         // CC 3
    REPORT_KIND_PAYUEL_LGPM_3V3_PWR_OFF,        // CC 4
    REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_ON,      // CC 5
    REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_OFF,     // CC 6
    REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_ON,       // CC 7
    REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_OFF,      // CC 8
    REPORT_KIND_PAYUEL_LGPM_V28_MAIN_ON,        // CC 9
    REPORT_KIND_PAYUEL_LGPM_V28_MAIN_OFF,       // CC 10
    REPORT_KIND_PAYUEL_LGPM_V28_SUB_ON,         // CC 11
    REPORT_KIND_PAYUEL_LGPM_V28_SUB_OFF,        // CC 12
    REPORT_KIND_PAYUEL_LGPM_V12_MAIN_ON,        // CC 13
    REPORT_KIND_PAYUEL_LGPM_V12_MAIN_OFF,       // CC 14
    REPORT_KIND_PAYUEL_LGPM_PWR_SENSE_INFO,          // CC 15
    REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_ON,         // CC 16
    REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_OFF,        // CC 17
    REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX1,           // CC 18
    REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX2,           // CC 19
    REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX3,           // CC 20
    REPORT_KIND_PAYUEL_LGPM_RWA_PWR_ON,         // CC 21
    REPORT_KIND_PAYUEL_LGPM_RWA_PWR_OFF,        // CC 22
    REPORT_KIND_PAYUEL_LGPM_RWA_SENSE_INFO,          // CC 23
/****************************************************************************************** */

    REPORT_KIND_SC_GENERIC,

} ReportKind_t;

static ReportKind_t DetermineReportKind(uint16_t reflected_mid, uint8_t reflected_cc) {
    if (reflected_mid == UANT_APP_CMD_ID && reflected_cc == UANT_APP_GET_STATUS_CC) return REPORT_KIND_UANT_GET_STATUS_TLM;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518)  && reflected_cc == ADCS_GET_TLM_LOG_INCLMASK_CC) return REPORT_KIND_ADCS_LOG_MASK;
    
/**************************************************************************************************************************************************************************************** */
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_ERROR_LOG_SETTING_CC) return REPORT_KIND_ADCS_GET_ERROR_LOG_SETTING;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CURRENT_UNIX_TIME_CC) return REPORT_KIND_ADCS_GET_CURRENT_UNIX_TIME;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC_CC) return REPORT_KIND_ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_COMMUNICATION_STATUS_CC) return REPORT_KIND_ADCS_GET_COMMUNICATION_STATUS;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CONTROL_ESTIMATION_MODE_CC) return REPORT_KIND_ADCS_GET_CONTROL_ESTIMATION_MODE;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_REFERENCE_IRC_VECTOR_CC) return REPORT_KIND_ADCS_GET_REFERENCE_IRC_VECTOR;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_REFERENCE_LLH_TARGET_CC) return REPORT_KIND_ADCS_GET_REFERENCE_LLH_TARGET;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_ORBIT_MODE_CC) return REPORT_KIND_ADCS_GET_ORBIT_MODE;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_HEALTH_TLM_MMT_CC) return REPORT_KIND_ADCS_GET_HEALTH_TLM_MMT;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RAW_CUBESENSE_SUN_CC) return REPORT_KIND_ADCS_GET_RAW_CUBESENSE_SUN;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_REFERENCE_RPY_VALUES_CC) return REPORT_KIND_ADCS_GET_REFERENCE_RPY_VALUES;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_OPENLOOPCMD_MTQ_CC) return REPORT_KIND_ADCS_GET_OPENLOOPCMD_MTQ;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_POWER_STATE_CC) return REPORT_KIND_ADCS_GET_POWER_STATE;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RUN_MODE_CC) return REPORT_KIND_ADCS_GET_RUN_MODE;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CONTROL_MODE_CC) return REPORT_KIND_ADCS_GET_CONTROL_MODE;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_MAG0_MMT_CALIB_CONFIG_CC) return REPORT_KIND_ADCS_GET_MAG0_MMT_CALIB_CONFIG;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_MAG1_MMT_CALIB_CONFIG_CC) return REPORT_KIND_ADCS_GET_MAG1_MMT_CALIB_CONFIG;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_ESTIMATION_MODE_CC) return REPORT_KIND_ADCS_GET_ESTIMATION_MODE;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_OPERATIONAL_STATE_CC) return REPORT_KIND_ADCS_GET_OPERATIONAL_STATE;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RAW_CSS_SENSOR_CC) return REPORT_KIND_ADCS_GET_RAW_CSS_SENSOR;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RAW_GYR_SENSOR_CC) return REPORT_KIND_ADCS_GET_RAW_GYR_SENSOR;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CALIBRATED_GYR_SENSOR_CC) return REPORT_KIND_ADCS_GET_CALIBRATED_GYR_SENSOR;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_MAG_SENSING_ELM_CONFIG_CC) return REPORT_KIND_ADCS_GET_MAG_SENSING_ELM_CONFIG;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_TLM_LOG_INCLMASK_CC) return REPORT_KIND_ADCS_GET_TLM_LOG_INCLMASK;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_UNSOLICIT_TLM_MSG_SETUP_CC) return REPORT_KIND_ADCS_GET_UNSOLICIT_TLM_MSG_SETUP;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP_CC) return REPORT_KIND_ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_EVENT_LOG_STATUS_RESPONSE_CC) return REPORT_KIND_ADCS_GET_EVENT_LOG_STATUS_RESPONSE;
    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_PORTMAP_CC) return REPORT_KIND_ADCS_GET_PORTMAP;
/****************************************************************************************************************************************************************************************** */
    
    if ((reflected_mid == EPS_CMD_ID || reflected_mid == 0x7518) && reflected_cc == EPS_P60_DOCK_GET_TABLE_HK_CC) return REPORT_KIND_EPS_P60_DOCK_GET_TABLE_HK;
    if ((reflected_mid == EPS_CMD_ID || reflected_mid == 0x7518) && reflected_cc == EPS_P60_PDU_GET_TABLE_HK_CC) return REPORT_KIND_EPS_P60_PDU_GET_TABLE_HK;
    if ((reflected_mid == EPS_CMD_ID || reflected_mid == 0x7518) && reflected_cc == EPS_P60_ACU_GET_TABLE_HK_CC) return REPORT_KIND_EPS_P60_ACU_GET_TABLE_HK;

    /*********************************************5차 추가****************************************************** */
    // CC 0, 1
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_NOOP_CC)) return REPORT_KIND_PAYUEL_ROMA_NOOP;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_RESET_COUNTERS_CC)) return REPORT_KIND_PAYUEL_ROMA_RESETCOUNTERS;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_COMM_TEST_CC)) return REPORT_KIND_PAYUEL_ROMA_COMMTEST;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_SPECIFIC_LINE_CC)) return REPORT_KIND_PAYUEL_ROMA_GETSPECIFICLINE;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_MULTIPLE_LINES_CC)) return REPORT_KIND_PAYUEL_ROMA_GETMULTIPLELINES;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_LATEST_LINE_CC)) return REPORT_KIND_PAYUEL_ROMA_GETLATESTLINE;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_LATEST_N_LINES_CC)) return REPORT_KIND_PAYUEL_ROMA_GETLATESTNLINES;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SET_ROUTE_DEFAULT_CC)) return REPORT_KIND_PAYUEL_ROMA_SETROUTEDEFAULT;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_RESET_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_RESETROUTE;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_LOAD_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_LOADROUTE;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SAVE_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_SAVEROUTE;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SEND_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_SENDROUTE;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SET_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_SETROUTE;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_GET_CC)) return REPORT_KIND_PAYUEL_ROMA_PARGET;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_SET_CC)) return REPORT_KIND_PAYUEL_ROMA_PARSET;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_DEFAULTS_CC)) return REPORT_KIND_PAYUEL_ROMA_PARDEFAULTS;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_SAVE_CC)) return REPORT_KIND_PAYUEL_ROMA_PARSAVE;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_RESTORE_CC)) return REPORT_KIND_PAYUEL_ROMA_PARRESTORE;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_LOAD_CC)) return REPORT_KIND_PAYUEL_ROMA_PARLOAD;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_SET_OOB_CC)) return REPORT_KIND_PAYUEL_ROMA_PARSETOOB;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SEND_COMMAND_CC)) return REPORT_KIND_PAYUEL_ROMA_SENDCOMMAND;

    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_NOOP_CC)) return REPORT_KIND_PAYUEL_LGPM_NOOP;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RESET_COUNTERS_CC)) return REPORT_KIND_PAYUEL_LGPM_RESETCOUNTERS;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_MCU_ALIVE_CHECK_CC)) return REPORT_KIND_PAYUEL_LGPM_MCU_ALIVE;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_3V3_PWR_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_3V3_PWR_ON;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_3V3_PWR_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_3V3_PWR_OFF;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_MAIN_BOOST_SW_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_ON;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_MAIN_BOOST_SW_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_OFF;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_SUB_BOOST_SW_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_ON;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_SUB_BOOST_SW_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_OFF;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_MAIN_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_MAIN_ON;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_MAIN_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_MAIN_OFF;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_SUB_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_SUB_ON;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_SUB_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_SUB_OFF;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V12_MAIN_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_V12_MAIN_ON;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V12_MAIN_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_V12_MAIN_OFF;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_PWR_SENSE_INFO_CC)) return REPORT_KIND_PAYUEL_LGPM_PWR_SENSE_INFO;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_PWR_SEQ_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_ON;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_PWR_SEQ_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_OFF;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_CONTROL_idx1_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX1;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_CONTROL_idx2_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX2;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_CONTROL_idx3_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX3;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_PWR_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_PWR_ON;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_PWR_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_PWR_OFF;
    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_SENSE_INFO_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_SENSE_INFO;
    /**************************************************************************************************************************/
}

typedef struct
{
    uint8_t bytes[512];
} RptGenericPayload_t;



typedef struct __attribute__((__packed__)) gs_gssb_ant6_release_status_t {
    /**
       Burn state of the first channel (Burning = 1, Idle = 0)
     */
    uint8_t channel_0_state;
    /**
       Release status of the first channel (Released = 1, Not released = 0)
     */
    uint8_t channel_0_status;
    /**
       Burn time left of the first channel [s]
     */
    uint8_t channel_0_burn_time_left;
    /**
       Counter of have many burns there has been attempted
     */
    uint8_t channel_0_burn_tries;
    /**
       Burn state of the second channel (Burning = 1, Idle = 0)
     */
    uint8_t channel_1_state;
    /**
       Release status of the second channel (Released = 1, Not released = 0)
     */
    uint8_t channel_1_status;
    /**
       Burn time left of the second channel [s]
     */
    uint8_t channel_1_burn_time_left;
    /**
       Counter of have many burns there has been attempted
     */
    uint8_t channel_1_burn_tries;
} gs_gssb_ant6_release_status_t;


typedef struct EPS_P60_DOCK_GET_TABLE_HK {

    int16_t   c_out[13];
    uint16_t  v_out[13];
    uint8_t   out_en[13];

    int16_t   temp[2];

    uint32_t  bootcause;
    uint32_t  bootcnt;
    uint32_t  uptime;

    uint16_t  resetcause;

    uint8_t   batt_mode;
    uint8_t   heater_on;
    uint8_t   conv_5v_en;

    uint16_t  latchup[13];

    uint16_t  vbat_v;
    int16_t   vcc_c;
    int16_t   batt_c;
    uint16_t  batt_v;

    int16_t   batt_temp[2];

    uint8_t   device_type[8];
    uint8_t   device_status[8];

    uint8_t   dearm_status;

    uint32_t  wdt_cnt_gnd;
    uint32_t  wdt_cnt_i2c;
    uint32_t  wdt_cnt_can;
    uint32_t  wdt_cnt_csp[2];

    uint32_t  wdt_gnd_left;
    uint32_t  wdt_i2c_left;
    uint32_t  wdt_can_left;

    uint8_t   wdt_csp_left[2];

    int16_t   batt_chrg;
    int16_t   batt_dischrg;

    int8_t    ant6_depl;
    int8_t    ar6_depl;

} EPS_P60_DOCK_GET_TABLE_HK;

typedef struct EPS_P60_PDU_GET_TABLE_HK {

    int16_t   c_out[9];
    uint16_t  v_out[9];

    uint16_t  vcc;
    uint16_t  vbat;
    int16_t   temp;

    uint8_t   conv_en[3];
    uint8_t   out_en[9];

    uint32_t  bootcause;
    uint32_t  bootcnt;
    uint32_t  uptime;

    uint16_t  resetcause;

    uint8_t   batt_mode;

    uint16_t  latchup[9];

    uint8_t   device_type[8];
    uint8_t   device_status[8];

    uint32_t  wdt_cnt_gnd;
    uint32_t  wdt_cnt_i2c;
    uint32_t  wdt_cnt_can;
    uint32_t  wdt_cnt_csp[2];

    uint32_t  wdt_gnd_left;
    uint32_t  wdt_i2c_left;
    uint32_t  wdt_can_left;

    uint8_t   wdt_csp_left[2];

} EPS_P60_PDU_GET_TABLE_HK;


typedef struct EPS_P60_ACU_GET_TABLE_HK {

    int16_t   c_in[6];
    uint16_t  v_in[6];

    uint16_t  vbat;
    uint16_t  vcc;

    int16_t   temp[3];

    uint8_t   mppt_mode;

    uint16_t  vboost[6];
    uint16_t  power[6];

    uint8_t   dac_en[3];
    uint16_t  dac_val[6];

    uint32_t  bootcause;
    uint32_t  bootcnt;
    uint32_t  uptime;

    uint16_t  resetcause;

    uint16_t  mppt_time;
    uint16_t  mppt_period;

    uint8_t   device_type[8];
    uint8_t   device_status[8];

    uint32_t  wdt_cnt_gnd;
    uint32_t  wdt_gnd_left;

} EPS_P60_ACU_GET_TABLE_HK;



typedef struct
{
    bool     valid;
    uint16_t CCMessage_ID;
    uint16_t CCCount;
    uint16_t CCLength;
    uint8_t  CCTime_code[6];

    uint16_t reflected_msg_id;
    uint8_t  reflected_cc;
    uint8_t  ret_type;
    int32_t  ret_code;
    uint16_t ret_val_size;

    ReportKind_t kind;

    union
    {
        RptGenericPayload_t    generic;


        ADCS_TlmLogInclMaskTlm_Payload_t       adcs_logmask;
        ADCS_UnsolicitTlmMsgSetupTlm_Payload_t adcs_unsolicited_tlm_tlm;

        gs_gssb_ant6_release_status_t          uant_getstatus;
        EPS_P60_DOCK_GET_TABLE_HK              eps_p60dockgettablehk;
        EPS_P60_PDU_GET_TABLE_HK               eps_p60pdugettablehk;
        EPS_P60_ACU_GET_TABLE_HK               eps_p60acugettablehk;
/************************************************************************************************************************************************************* */
        ADCS_ErrorLogSettingTlm_Payload_t      adcs_errorlogsetting;
        ADCS_CurrentUnixTimeTlm_Payload_t      adcs_currentunixtime;
        ADCS_PersistConfigDiagnosticTlm_Payload_t adcs_persistconfigdiagnostic;
        ADCS_CommunicationStatusTlm_Payload_t  adcs_communicationstatus;
        ADCS_ControlEstimationModeTlm_Payload_t adcs_controlestimationmode;
        ADCS_ReferenceIRCVectorTlm_Payload_t    adcs_referenceircvector;
        ADCS_ReferenceLLHTargetTlm_Payload_t    adcs_referencellhtarget;
        ADCS_OrbitModeTlm_Payload_t             adcs_orbitmode;
        ADCS_HealthTlmMMTTlm_Payload_t          adcs_healthtlmmmt;
        ADCS_RawCubeSenseSunTlm_Payload_t       adcs_rawcubesensesun;
        ADCS_ReferenceRPYvaluesTlm_Payload_t    adcs_referencerpyvalues;
        ADCS_OpenLoopCmdMTQTlm_Payload_t        adcs_openloopcmdmtq;
        ADCS_PowerStateTlm_Payload_t            adcs_powerstate;
        ADCS_RunModeTlm_Payload_t               adcs_runmode;
        ADCS_ControlModeTlm_Payload_t           adcs_controlmode;
        ADCS_Mag0MMTCalibConfigTlm_Payload_t    adcs_mag0mmtcalibconfig;
        ADCS_Mag1MMTCalibConfigTlm_Payload_t    adcs_mag1mmtcalibconfig;
        ADCS_EstimationModeTlm_Payload_t        adcs_estimationmode;
        ADCS_OperationalStateTlm_Payload_t      adcs_operationalstate;
        ADCS_RawCSSSensorTlm_Payload_t          adcs_rawcsssensor;
        ADCS_RawGYRSensorTlm_Paylaod_t          adcs_rawgyrsensor;
        ADCS_CalibratedGYRSensorTlm_Payload_t   adcs_calibratedgyrsensor;
        ADCS_MagSensingElmConfigTlm_Payload_t   adcs_magsensingelmconfig;
        ADCS_TlmLogInclMaskTlm_Payload_t        adcs_tlmloginclmask;
        ADCS_UnsolicitTlmMsgSetupTlm_Payload_t  adcs_unsolicittlmmsgsetup;
        ADCS_UnsolicitEventMsgSetupTlm_Payload_t adcs_unsoliciteventmsgsetup;
        ADCS_EventLogStatusResponseTlm_Payload_t adcs_eventlogstatusresponse;
        ADCS_PortMapTlm_Payload_t               adcs_portmap;
/******************************************************************************************************************************************************************** */

/**********************************************5차 추가********************************************** */
        // 1. PAYUEL_ROMA
        payuel_roma_Noop_tlm_payload_t                     roma_noop;;
        payuel_roma_ResetCounters_tlm_payload_t            roma_resetcounters;
        payuel_roma_CommTest_tlm_payload_t                 roma_commtest;
        payuel_roma_GetSpecificLine_tlm_payload_t          roma_getspecificline;
        payuel_roma_GetMultipleLines_tlm_payload_t         roma_getmultiplelines;
        payuel_roma_GetLatestLine_tlm_payload_t            roma_getlatestline;
        payuel_roma_GetLatest_N_Lines_tlm_payload_t        roma_getlatestNlines;
        payuel_roma_SetRouteDefault_tlm_payload_t          roma_setroutedefault;
        payuel_roma_ResetRoute_tlm_payload_t               roma_resetroute;
        payuel_roma_LoadRoute_tlm_payload_t                roma_loadroute;
        payuel_roma_SaveRoute_tlm_payload_t                roma_saveroute;
        payuel_roma_SendRoute_tlm_payload_t                roma_sendroute;
        payuel_roma_SetRoute_tlm_payload_t                 roma_setroute;
        payuel_roma_ParGet_tlm_payload_t                   roma_parget;
        payuel_roma_ParSet_tlm_payload_t                   roma_parset;
        payuel_roma_ParDefaults_tlm_payload_t              roma_pardefaults;
        payuel_roma_ParSave_tlm_payload_t                  roma_parsave;
        payuel_roma_ParRestore_tlm_payload_t               roma_parrestore;
        payuel_roma_ParLoad_tlm_payload_t                  roma_parload;
        payuel_roma_ParSetOOB_tlm_payload_t                roma_parsetOOB;
        payuel_roma_SendCommand_tlm_payload_t              roma_sendcommand;

        PAYUEL_LGPM_Noop_tlm_payload_t                     lgpm_noop;
        PAYUEL_LGPM_ResetCounters_tlm_payload_t            lgpm_resetcounters;
        PAYUEL_LGPM_MCU_ALIVE_CHECK_Tlm_Payload            lgpm_mcualivecheck;
        PAYUEL_LGPM_3V3PwrOn_tlm_payload_t                 lgpm_3v3pwron;
        PAYUEL_LGPM_3V3PwrOff_tlm_payload_t                lgpm_3v3pwroff;
        PAYUEL_LGPM_MainBoostSwOn_tlm_payload_t            lgpm_mainboostswon;
        PAYUEL_LGPM_MainBoostSwOff_tlm_payload_t           lgpm_mainboostswoff;
        PAYUEL_LGPM_SubBoostSwOn_tlm_payload_t             lgpm_subboostswon;
        PAYUEL_LGPM_SubBoostSwOff_tlm_payload_t            lgpm_subboostswoff;
        PAYUEL_LGPM_V28MainOn_tlm_payload_t                lgpm_v28mainon;
        PAYUEL_LGPM_V28MainOff_tlm_payload_t               lgpm_v28mainoff;
        PAYUEL_LGPM_V28SubOn_tlm_payload_t                 lgpm_v28subon;
        PAYUEL_LGPM_V28SubOff_tlm_payload_t                lgpm_v28suboff;
        PAYUEL_LGPM_V12MainOn_tlm_payload_t                lgpm_v12mainon;
        PAYUEL_LGPM_V12MainOff_tlm_payload_t               lgpm_v12mainoff;
        PAYUEL_LGPM_PwrSenseInfo_tlm_payload_t             lgpm_pwrsenseinfo;
        PAYUEL_LGPM_PwrSeqOn_tlm_payload_t                 lgpm_pwrseqon;
        PAYUEL_LGPM_PwrSeqOff_tlm_payload_t                lgpm_pwrseqoff;
        PAYUEL_LGPM_RwaControlIdx1_tlm_payload_t           lgpm_rwacontrol_idx1;
        PAYUEL_LGPM_RwaControlIdx2_tlm_payload_t           lgpm_rwacontrol_idx2;
        PAYUEL_LGPM_RwaControlIdx3_tlm_payload_t           lgpm_rwacontrol_idx3;
        PAYUEL_LGPM_RwaPwrOn_tlm_payload_t                 lgpm_rwapwron;
        PAYUEL_LGPM_RwaPwrOff_tlm_payload_t                lgpm_rwapwroff;
        PAYUEL_LGPM_RwaSenseInfo_tlm_payload_t             lgpm_rwasenseinfo;
/**************************************************************************************************** */

    } u;
} ReportView_t;


extern std::mutex g_report_view_mtx;
extern ReportView_t g_report_view;


typedef struct {
    uint8 Callsign[6];
    uint8 CurrentMode;
    uint8 CurrentSubmode;
    uint8 PrevioudMode;
    uint8 PreviousSubmode;
    uint8 CurrentModeFlag;
    uint8 PreviousModeFlag;
    uint32 ApplicationRunStatus;
    uint32 SatelliteTime;
    uint16 RebootCount;
    uint8 RebootCause;

}__attribute__((packed)) FM_HK_;

typedef struct {
	uint16 DeployState_UANT;
}__attribute__((packed)) UANT_;

typedef struct {
	uint32 rxfreq;
	uint32 txfreq;
	int16 LastRssi;
    uint32 TotRxBytes;
    uint8 StatusConfiguration;
}__attribute__((packed)) UTRX_;

typedef struct {
	//EPS - P60 Dock
	uint8 out_en_dock[7]; //01458910
    int16 temp_dock[2];
    uint32 bootcause;
    uint32 bootcnt;
    uint32 uptime;
    uint16 resetcause;
    uint8 batt_mode;
    uint8 heater_on;
    uint16 latchup_dock[7]; //01458910
    uint16 vbat_v;
    int16 batt_v;
    int16 batt_temp[2];
    uint8 device_status[8];
    uint32 wdt_cnt_gnd;
    uint32 wdt_gnd_left;
    int16 batt_chrg;
    int16 batt_dischrg;
	//EPS - PDU
    int16 vbat;
    uint8 out_en_pdu[6]; //034758
    uint16 latchup_pdu[6];
    uint16 out_voltage[6]; // 034758
	//EPS - ACU
    int16 c_in[4]; //0123
    uint16 v_in[4]; //0123
}__attribute__((packed)) EPS_;

typedef struct {
	uint8 RWL0_PowerState;
	uint8 RWL1_PowerState;
	uint8 RWL2_PowerState;
	uint8 MAG0_PowerState;
	uint8 FSS0_PowerState;
	uint8 HSS0_PowerState;
	uint8 Control_Mode;
	uint16 Mag_Control_Timeout;
	float GYRO_Calib_rate_X;
	float GYRO_Calib_rate_Y;
	float GYRO_Calib_rate_Z;
}__attribute__((packed)) ADCS_;

typedef struct {
	uint8 Status;
	int16 Board_Temperature;
	int16 Battery_Current;
	int16 Battery_Voltage;
}__attribute__((packed)) STX_;

typedef struct {
	int16 temp_PAYC;
	uint16 icore;
}__attribute__((packed)) PAYC_;

typedef struct {
	uint8 DeployStatus_PAYR;
}__attribute__((packed)) PAYR_;

typedef struct {
	uint8 PAYS_State;
	uint8 PAYS_Sign;
	uint8 PAYS_Temp;
}__attribute__((packed)) PAYS_;


// typedef struct {
// 	CCSDS_Header_ CCSDS_Header;
// 	FM_HK_ FM;
// 	EPS_ EPS;
// 	TCS_ TCS;
// 	RWA_ RWA;
// 	MTQ_ MTQ;
// 	SNSR_ SNSR;
// 	UTRX_ UTRX;
// 	STX_ STX;
// 	PAY_ PAY;
// }__attribute__((packed)) HK;

// typedef struct {
// 	FM_HK_ FM;
// 	ADCS_ ADCS;
// }__attribute__((packed)) AOD;

typedef struct {
	uint32_t ExTime;
	uint32_t ExWindow;
	uint16_t EntryID;
	uint16_t GroupID;
	uint8_t cmd[];
}__attribute__((packed)) Book;



static bool ParseReportWire540(const uint8_t *buf, size_t len, Report &out);
void * TRxController(void *);
void * SignalTest(void*);
void now_rx_bytes_update();
void set_rx_bytes(uint32_t nowbytes);
uint32_t get_rx_bytes();
uint32_t * get_rx_bytes_address();
uint16_t get_boot_count();
uint16_t * get_boot_count_address();
void buf_allclear();
void CalculateChecksum(CommandHeader_t* Cmd);
int32_t GenerateCmdMsg(CommandHeader_t* Cmd, uint16_t MsgId, uint8_t FcnCode, uint32_t ArgLen);
csp_socket_t *  DL_sock_initialize();
int BeaconSaver(Beacon * bec);
void * task_downlink_onorbit(void * socketinfo);
void * task_uplink_onorbit(void * sign_);

int PacketHandler(csp_packet_t *packet, int type, int NowCursor);
packetsign * PingInit(FSWTle * FSWTleinfo);
csp_packet_t * PacketEncoder(packetsign * sign, bool freeer = true);
packetsign * PacketDecoder(csp_packet_t * packet);


class CmdGenerator_GS {
private:
    void SetHeaderWord(uint8_t* Word16, uint16_t Value, uint16_t Mask);
    void SetHeaderByte(uint8_t* Byte, uint8_t Value, uint8_t Mask);
    void GetHeaderWord(const uint8_t* Word16, uint16_t& Value, uint16_t Mask);
    uint32_t ComputeCheckSum(void);

public:
	CFE_MSG_CommandHeader* CmdHeader;
	bool Scheduled = false;
	bool Checksum = true;

    CmdGenerator_GS(void);
    ~CmdGenerator_GS(void);

    int GenerateCmdHeader(uint32_t MsgId, uint16_t FncCode, uint32_t Size, void* Data);
    void CopyCmdHeaderToBuffer(uint8_t* Buffer);

    void InitHeader(void);
    void SetHeader(const CFE_MSG_CommandHeader* Header);
    const CFE_MSG_CommandHeader* GetHeader(void) const;

    int SetHasSecondaryHeader(bool HasSec);
    int SetMsgId(uint16_t MsgId);
    int SetSize(uint16_t Size);
    int SetSegmentationFlag(uint16_t SegFlag);
    int SetFncCode(uint16_t FncCode);

    bool HasSecondaryHeader(void) const;
    uint16_t GetSize(void);
    uint16_t GetFncCode(void) const;

    int GenerateChecksum(void);
	int Scheduling(uint32_t ExecutionTime, uint32_t ExecutionWindow, uint32_t EntryID, uint16_t GroupID);
	packetsign * GenerateCMDPacket(void);
};

void * Direct_Shell(void * data);

#endif _MIMAN_COMS_H_