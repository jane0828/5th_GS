// // // // #pragma once
// // // // #ifndef _MIMAN_COMS_H_
// // // // #define _MIMAN_COMS_H_

// // // // #include "miman_config.h"
// // // // #include "miman_orbital.h"
// // // // #include <mutex>

// // // // #pragma once
// // // // #include <mutex>






// // // // typedef struct{
// // // // 	uint16_t Identifier;
// // // // 	uint16_t PacketType;
// // // // 	uint32_t Length;
// // // // 	uint8_t Data[];

// // // // }__attribute__((packed)) packetsign;

// // // // typedef struct{
// // // // 	uint16_t target;
// // // // 	uint16_t filestatus;
// // // // 	uint32_t filenum;
// // // // 	uint32_t offset;
// // // // 	uint32_t step;
// // // // }__attribute__((packed)) dlreqdata;

// // // // typedef struct{
// // // // 	int32_t filenum;
// // // // 	uint32_t file[];
// // // // }__attribute__((packed)) filelist;

// // // // // typedef enum {
// // // // //     GS_FTP_INFO_FILE  = 0,   /**< File size and checksum */
// // // // //     GS_FTP_INFO_CRC = 1,         /**< CRC of remote and local file */
// // // // //     GS_FTP_INFO_COMPLETED = 2,   /**< Completed and total chunks */
// // // // //     GS_FTP_INFO_PROGRESS = 3,    /**< Current chunk, total_chunks and chunk_size */
// // // // // } gs_ftp_info_type_t;

// // // // typedef struct {
// // // //     FILE       * fp;
// // // //     FILE       * fp_map;
// // // //     csp_conn_t * conn;
// // // //     uint32_t   timeout;
// // // //     char       file_name[GS_FTP_PATH_LENGTH];
// // // //     uint32_t   file_size;
// // // //     uint32_t   chunks;
// // // //     int        chunk_size;
// // // //     uint32_t   checksum;
// // // //     ftp_status_element_t last_status[GS_FTP_STATUS_CHUNKS];
// // // //     uint32_t   last_entries;
// // // //     gs_ftp_info_callback_t info_callback;
// // // //     void       * info_data;
// // // // } gs_ftp_state_t;

// // // // typedef struct {
// // // //     gs_ftp_backend_type_t backend;
// // // //     const char * path;
// // // //     uint32_t addr;
// // // //     uint32_t size;
// // // // } gs_ftp_url_t;



// // // // // BEE-1000 RPT structure
// // // // typedef struct {
// // // //     bool valid;
// // // //     uint16_t CCMessage_ID;
// // // //     uint16_t CCCount;
// // // //     uint16_t CCLength;
// // // //     uint8_t CCTime_code[6];
// // // //     uint16_t msg_id;
// // // //     uint8_t cc;
// // // //     uint8_t ret_type;
// // // //     int32_t ret_code;
// // // //     uint16_t ret_val_size;
// // // //     std::vector<uint8_t> payload;
// // // // } ReportPacket_t;

// // // // extern ReportPacket_t g_last_report;


// // // // // BEE-1000 Beacon
// // // // typedef struct {

// // // // 	// CCSDS Header
// // // //     uint8 CCSDS_MID[2];
// // // //     uint8 CCSDS_Seq[2];
// // // //     uint8 CCSDS_Len[2];
// // // //     uint8 CCSDS_TimeCode[6];

// // // // 	// FSW - RPT - shoud be deprecated
// // // // 	// uint16 RPT_BootCount;
// // // //     // uint32 RPT_ScTimeSec;
// // // //     // uint32 RPT_ScTimeSubsec;
// // // //     // uint32 RPT_Sequence;
// // // //     // uint8 RPT_ResetCause;

// // // //     /* FSW - RPT - Revised (Kweon Hyeokjin) */
// // // //     uint8_t RPT_CmdCounter;
// // // //     uint8_t RPT_ErrCounter;
// // // //     uint8_t RPT_ReportCnt;
// // // //     uint8_t RPT_CriticalCnt;
// // // //     uint16_t RPT_BootCount;
// // // //     uint32_t RPT_ScTimeSec;
// // // //     uint32_t RPT_ScTimeSubsec;
// // // //     uint8_t RPT_Sequence_LSB;
// // // //     /*--------------End of BEE RPT Revision (Kweon Hyeokjin)--------------*/

// // // // 	// COMS - STX
// // // //     // all param
// // // //     uint8_t  STX_symbol_rate;
// // // //     uint8_t  STX_transmit_power;
// // // //     uint8_t  STX_modcod;
// // // //     uint8_t  STX_roll_off;
// // // //     uint8_t  STX_pilot_signal;
// // // //     uint8_t  STX_fec_frame_size;
// // // //     uint16_t STX_pretransmission_delay;
// // // //     float    STX_center_frequency;
// // // //     // modulation interface -> cpu temp 빠짐
// // // //     uint8_t STX_modulator_interface_type;  
// // // //     uint8_t STX_lvds_io_type;    
// // // // 	uint8 STX_SystemState;
// // // //     uint8 STX_StatusFlag;
// // // //     // float STX_CpuTemp;

// // // // 	// COMS - UANT
// // // //     uint8 UANT1_Chan0;
// // // //     uint8 UANT1_Chan1;
// // // //     uint8 UANT1_BackupActive;
// // // //     uint8 UANT2_Chan0;
// // // //     uint8 UANT2_Chan1;
// // // //     uint8 UANT2_BackupActive;

// // // // 	// COMS - UTRX
// // // //     uint8 UTRX_ActiveConf;
// // // //     uint16 UTRX_BootCount;
// // // //     uint32 UTRX_BootCause;
// // // //     int16 UTRX_BoardTemp;

// // // //     // PCDU - P60 Dock
// // // //     int16 P60D_Cout[9];
// // // //     uint16 P60D_Vout[9];
// // // //     uint16 P60D_OutEn;
// // // //     uint32 P60D_BootCause;
// // // //     uint32 P60D_BootCount;
// // // //     uint8 P60D_BattMode;
// // // //     uint8 P60D_HeaterOn;
// // // //     uint16 P60D_VbatV;
// // // //     int16 P60D_VccC;
// // // //     uint16 P60D_BattV;
// // // //     int16 P60D_BattTemp[2];
// // // //     uint32 P60D_WdtCanLeft;
// // // //     int16 P60D_BattChrg;
// // // //     int16 P60D_BattDischrg;

// // // //     // PCDU - P60 PDU
// // // //     int16  P60P_Cout[9];
// // // //     uint16 P60P_Vout[9];
// // // //     int16  P60P_Vcc;
// // // //     uint8  P60P_ConvEn;
// // // //     uint16 P60P_OutEn;

// // // //     // PCDU - P60 ACU
// // // //     int16  P60A_Cin[6];
// // // //     uint16 P60A_Vin[6];

// // // //     // ADCS
// // // //        /** Combined Power State
// // // //      *  | 7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
// // // //      *  +--------------------------------------+
// // // //      *  |Rsv|RWL0|RWL1|RWL2|MAG0|GYRO|FSS0|HSS0|
// // // //      *  +--------------------------------------+
// // // //      */
// // // //     uint8 ADCS_PowerState; // ID 183

// // // //     uint8 ADCS_ControlMode; // ID 185

// // // //     float ADCS_GYR0CalibratedRateXComponent;
// // // //     float ADCS_GYR0CalibratedRateYComponent;
// // // //     float ADCS_GYR0CalibratedRateZComponent; // ID 207, 12bytes

// // // // }__attribute__((packed)) BEE1000_Beacon_t;

// // // // #define a sizeof(BEE1000_Beacon_t);
// // // // static_assert(sizeof(BEE1000_Beacon_t) == 203, "BEE1000_Beacon_t size mismatch");

// // // // typedef struct {
// // // //     char call_sign[7];
// // // //     uint8_t msg_id[2];
// // // //     uint8_t sequence[2];
// // // //     uint8_t length[2];
// // // //     uint8_t time_code[6];
// // // // }__attribute__((packed)) BEE1012_BeaconHeader_t;

// // // // typedef struct {
// // // //     uint16_t boot_count;
// // // //     uint32_t sequence;
// // // //     uint8_t reset_cause;
// // // // }__attribute__((packed)) BEE1012_RPT_t;

// // // // typedef struct {
// // // //     uint8_t active_conf;
// // // //     uint16_t boot_count;
// // // //     uint32_t boot_cause;
// // // //     int16_t temp_board;
// // // // }__attribute__((packed)) BEE1012_AX100_t;

// // // // typedef struct {
// // // //     int16_t temp;
// // // //     int8_t connection_quality;
// // // //     uint16_t battery_capacity;
// // // // }__attribute__((packed)) BEE1012_LTRX_t;

// // // // typedef struct {
// // // //     BEE1012_AX100_t ax100;
// // // //     BEE1012_LTRX_t ltrx;
// // // // }__attribute__((packed)) BEE1012_COMS_t;

// // // // typedef struct {
// // // //     uint32_t bootcause;
// // // //     uint16_t resetcause;
// // // //     uint16_t bootcount;
// // // //     uint8_t out_en[6];
// // // //     int16_t temp[2];
// // // //     uint8_t batt_mode;
// // // //     int16_t batt_i;
// // // //     uint16_t batt_v;
// // // //     uint8_t sm_en;
// // // //     uint16_t gnd_wdt_cnt;
// // // //     uint16_t bus_wdt_cnt;
// // // //     uint32_t gnd_wdt_left;
// // // //     uint32_t bus_wdt_left;
// // // // }__attribute__((packed)) BEE1012_EPS_PMU_t;

// // // // typedef struct {
// // // //     int16_t out_i[12];
// // // //     uint8_t out_en[12];
// // // // }__attribute__((packed)) BEE1012_EPS_PDU_t;

// // // // typedef struct {
// // // //     int16_t input_i[6];
// // // //     uint16_t input_v[6];
// // // //     uint8_t mppt_mode;
// // // // }__attribute__((packed)) BEE1012_EPS_ACU_Unit_t;

// // // // typedef struct {
// // // //     BEE1012_EPS_ACU_Unit_t acu[2];
// // // // }__attribute__((packed)) BEE1012_EPS_ACU_t;

// // // // typedef struct {
// // // //     uint16_t bootcount;
// // // //     uint16_t bootcause;
// // // //     uint16_t resetcause;
// // // //     float soc;
// // // //     float bat_avr_temp;
// // // //     uint16_t vbat;
// // // //     float i;
// // // //     uint16_t heater_i;
// // // // }__attribute__((packed)) BEE1012_EPS_BP8_t;

// // // // typedef struct {
// // // //     uint16_t gpio_status;
// // // //     uint8_t sp_deploy_status;
// // // // }__attribute__((packed)) BEE1012_EPS_GPIO_t;

// // // // typedef struct {
// // // //     BEE1012_EPS_PMU_t pmu;
// // // //     BEE1012_EPS_PDU_t pdu;
// // // //     BEE1012_EPS_ACU_t acu;
// // // //     BEE1012_EPS_BP8_t bp8;
// // // //     BEE1012_EPS_GPIO_t gpio;
// // // // }__attribute__((packed)) BEE1012_EPS_t;

// // // // typedef struct {
// // // //     uint8_t power_state;
// // // //     uint8_t control_mode;
// // // //     float gyro0_calibrated_rate_x;
// // // //     float gyro0_calibrated_rate_y;
// // // //     float gyro0_calibrated_rate_z;
// // // //     uint8_t css[6];
// // // // }__attribute__((packed)) BEE1012_ADCS_t;

// // // // typedef struct {
// // // //     BEE1012_BeaconHeader_t header;
// // // //     BEE1012_RPT_t rpt;
// // // //     BEE1012_COMS_t coms;
// // // //     BEE1012_EPS_t eps;
// // // //     BEE1012_ADCS_t adcs;
// // // // }__attribute__((packed)) BEE1012_Beacon_t;

// // // // static_assert(sizeof(BEE1012_BeaconHeader_t) == 19, "BEE1012_BeaconHeader_t size mismatch");
// // // // static_assert(sizeof(BEE1012_RPT_t) == 7, "BEE1012_RPT_t size mismatch");
// // // // static_assert(sizeof(BEE1012_AX100_t) == 9, "BEE1012_AX100_t size mismatch");
// // // // static_assert(sizeof(BEE1012_LTRX_t) == 5, "BEE1012_LTRX_t size mismatch");
// // // // static_assert(sizeof(BEE1012_COMS_t) == 14, "BEE1012_COMS_t size mismatch");
// // // // static_assert(sizeof(BEE1012_EPS_PMU_t) == 36, "BEE1012_EPS_PMU_t size mismatch");
// // // // static_assert(sizeof(BEE1012_EPS_PDU_t) == 36, "BEE1012_EPS_PDU_t size mismatch");
// // // // static_assert(sizeof(BEE1012_EPS_ACU_Unit_t) == 25, "BEE1012_EPS_ACU_Unit_t size mismatch");
// // // // static_assert(sizeof(BEE1012_EPS_ACU_t) == 50, "BEE1012_EPS_ACU_t size mismatch");
// // // // static_assert(sizeof(BEE1012_EPS_BP8_t) == 22, "BEE1012_EPS_BP8_t size mismatch");
// // // // static_assert(sizeof(BEE1012_EPS_GPIO_t) == 3, "BEE1012_EPS_GPIO_t size mismatch");
// // // // static_assert(sizeof(BEE1012_EPS_t) == 147, "BEE1012_EPS_t size mismatch");
// // // // static_assert(sizeof(BEE1012_ADCS_t) == 20, "BEE1012_ADCS_t size mismatch");
// // // // static_assert(sizeof(BEE1012_Beacon_t) == 207, "BEE1012_Beacon_t size mismatch");

// // // // typedef struct {
// // // //     char call_sign[7];
// // // //     uint8_t msg_id[2];
// // // //     uint8_t sequence[2];
// // // //     uint8_t length[2];
// // // //     uint8_t time_code[6];
// // // // }__attribute__((packed)) UELYSYS_BeaconHeader_t;

// // // // typedef struct {
// // // //     uint16_t boot_count;
// // // //     uint32_t sequence;
// // // //     uint8_t reset_cause;
// // // // }__attribute__((packed)) UELYSYS_FSW_t;

// // // // typedef struct {
// // // //     uint8_t i2c1_0x05_channel_0_status;
// // // //     uint8_t i2c1_0x05_channel_1_status;
// // // //     uint8_t i2c1_0x06_channel_0_status;
// // // //     uint8_t i2c1_0x06_channel_1_status;
// // // //     uint8_t ax100_active_conf;
// // // //     uint16_t ax100_boot_count;
// // // //     uint32_t ax100_boot_cause;
// // // //     int16_t ax100_temp_board;
// // // // }__attribute__((packed)) UELYSYS_COMS_t;

// // // // typedef struct {
// // // //     uint32_t bootcause;
// // // //     uint16_t resetcause;
// // // //     uint16_t bootcount;
// // // //     uint8_t out_en[6];
// // // //     int16_t temp[2];
// // // //     uint8_t batt_mode;
// // // //     int16_t batt_i;
// // // //     uint16_t batt_v;
// // // //     uint8_t sm_en;
// // // //     uint16_t gnd_wdt_cnt;
// // // //     uint16_t bus_wdt_cnt;
// // // //     uint32_t gnd_wdt_left;
// // // //     uint32_t bus_wdt_left;
// // // // }__attribute__((packed)) UELYSYS_EPS_PMU_t;

// // // // typedef struct {
// // // //     int16_t out_i[12];
// // // //     uint8_t out_en[12];
// // // // }__attribute__((packed)) UELYSYS_EPS_PDU_t;

// // // // typedef struct {
// // // //     int16_t input_i[6];
// // // //     uint16_t input_v[6];
// // // //     uint8_t mppt_mode;
// // // // }__attribute__((packed)) UELYSYS_EPS_ACU_Unit_t;

// // // // typedef struct {
// // // //     UELYSYS_EPS_ACU_Unit_t acu[2];
// // // // }__attribute__((packed)) UELYSYS_EPS_ACU_t;

// // // // typedef struct {
// // // //     uint16_t bootcount;
// // // //     uint16_t bootcause;
// // // //     uint16_t resetcause;
// // // //     float soc;
// // // //     float bat_avr_temp;
// // // //     uint16_t vbat;
// // // //     float i;
// // // //     uint16_t heater_i;
// // // // }__attribute__((packed)) UELYSYS_EPS_BP8_t;

// // // // typedef struct {
// // // //     uint8_t dsp_i2c1_0x07_status;
// // // //     uint8_t dsp_i2c1_0x08_status;
// // // //     uint8_t dsp_i2c1_0x09_status;
// // // //     uint8_t dsp_i2c1_0x10_status;
// // // // }__attribute__((packed)) UELYSYS_EPS_DSP_t;

// // // // typedef struct {
// // // //     UELYSYS_EPS_PMU_t pmu;
// // // //     UELYSYS_EPS_PDU_t pdu;
// // // //     UELYSYS_EPS_ACU_t acu;
// // // //     UELYSYS_EPS_BP8_t bp8;
// // // //     UELYSYS_EPS_DSP_t dsp;
// // // // }__attribute__((packed)) UELYSYS_EPS_t;

// // // // typedef struct {
// // // //     uint8_t power_state;
// // // //     uint8_t control_mode;
// // // //     float gyro0_calibrated_rate_x;
// // // //     float gyro0_calibrated_rate_y;
// // // //     float gyro0_calibrated_rate_z;
// // // //     uint8_t css[6];
// // // // }__attribute__((packed)) UELYSYS_ADCS_t;

// // // // typedef struct {
// // // //     UELYSYS_BeaconHeader_t header;
// // // //     UELYSYS_FSW_t fsw;
// // // //     UELYSYS_COMS_t coms;
// // // //     UELYSYS_EPS_t eps;
// // // //     UELYSYS_ADCS_t adcs;
// // // // }__attribute__((packed)) UELYSYS_Beacon_t;

// // // // static_assert(sizeof(UELYSYS_BeaconHeader_t) == 19, "UELYSYS_BeaconHeader_t size mismatch");
// // // // static_assert(sizeof(UELYSYS_FSW_t) == 7, "UELYSYS_FSW_t size mismatch");
// // // // static_assert(sizeof(UELYSYS_COMS_t) == 13, "UELYSYS_COMS_t size mismatch");
// // // // static_assert(sizeof(UELYSYS_EPS_PMU_t) == 36, "UELYSYS_EPS_PMU_t size mismatch");
// // // // static_assert(sizeof(UELYSYS_EPS_PDU_t) == 36, "UELYSYS_EPS_PDU_t size mismatch");
// // // // static_assert(sizeof(UELYSYS_EPS_ACU_Unit_t) == 25, "UELYSYS_EPS_ACU_Unit_t size mismatch");
// // // // static_assert(sizeof(UELYSYS_EPS_ACU_t) == 50, "UELYSYS_EPS_ACU_t size mismatch");
// // // // static_assert(sizeof(UELYSYS_EPS_BP8_t) == 22, "UELYSYS_EPS_BP8_t size mismatch");
// // // // static_assert(sizeof(UELYSYS_EPS_DSP_t) == 4, "UELYSYS_EPS_DSP_t size mismatch");
// // // // static_assert(sizeof(UELYSYS_EPS_t) == 148, "UELYSYS_EPS_t size mismatch");
// // // // static_assert(sizeof(UELYSYS_ADCS_t) == 20, "UELYSYS_ADCS_t size mismatch");
// // // // static_assert(sizeof(UELYSYS_Beacon_t) == 207, "UELYSYS_Beacon_t size mismatch");



// // // // typedef struct CFE_SRL_HousekeepingTlm_Payload {
// // // //     uint8 CommandCounter;

// // // //     uint8 CommandErrorCounter;

// // // //     uint8 IOHandleStatus[4];

// // // //     uint16 IOHandleTxCount[4];
    
// // // // }__attribute__((packed)) CFE_SRL_HousekeepingTlm_Payload_t;

// // // // typedef struct RPT_HkTlm_Payload{
// // // //     uint8 CmdCounter;
// // // //     uint8 CmdErrCounter;

// // // //     /**
// // // //      * Queue Info
// // // //      */
// // // //     uint8 ReportQueueCnt;
// // // //     uint8 CriticalQueueCnt;

// // // //     /**
// // // //      * Operation Data
// // // //      */
// // // //     uint16 BootCount;
// // // //     uint32 TimeSec;
// // // //     uint32 TimeSubsec;
// // // //     uint32 Sequence; /* Backup data numbering */

// // // //     /**
// // // //      * Reset Cause
// // // //      */
// // // //     uint8 ResetCause;

// // // // }__attribute__((packed)) RPT_BcnTlm_Payload_t;

// // // // typedef struct PAY_BcnTlm_Payload {
// // // //     uint8 CommandCounter;
// // // //     uint8 CommandErrorCounter;

// // // //     /**
// // // //      * Else ....
// // // //      */
// // // //     /* compact beacon subset */
// // // //     int8  sys_status;         /* payload system status */
// // // //     int16 temp_ntc_0;
// // // //     int16 temp_ntc_1;
// // // //     int16 temp_ntc_2;     
// // // //     int16 temp_ntc_3;
// // // //     int16 temp_ntc_4;         /* board temp 4 */
// // // //     int16 temp_ntc_5;         /* board temp 5 */
// // // //     int16 temp_ntc_6;         /* board temp 6 */
// // // //     int16 temp_ntc_7;         /* board temp 7 */
// // // //     int16 temp_ntc_8;         /* board temp 8 */
// // // //     int16 temp_ntc_9;         /* board temp 9 */
// // // //     int16 temp_ntc_10;        /* board temp 10 */
// // // //     int16 temp_ntc_11;        /* board temp 11 */     


// // // // } PAY_BcnTlm_Payload_t;

// // // // typedef struct PAY_HkTlm_Payload {
// // // //     uint8 CommandCounter;
// // // //     uint8 CommandErrorCounter;

// // // //     int8  sys_status;         /* payload system status */
// // // //     int16 temp_ntc_0;         /* board temp 0 */
// // // //     int16 temp_ntc_1;         /* board temp 1 */
// // // //     int16 temp_ntc_2;         /* board temp 2 */
// // // //     int16 temp_ntc_3;         /* board temp 3 */
// // // //     int16 temp_ntc_4;         /* board temp 4 */
// // // //     int16 temp_ntc_5;         /* board temp 5 */
// // // //     int16 temp_ntc_6;         /* board temp 6 */
// // // //     int16 temp_ntc_7;         /* board temp 7 */
// // // //     int16 temp_ntc_8;         /* board temp 8 */
// // // //     int16 temp_ntc_9;         /* board temp 9 */
// // // //     int16 temp_ntc_10;        /* board temp 10 */
// // // //     int16 temp_ntc_11;        /* board temp 11 */
// // // //     /* Expanded TM (selected currents/sensors) */
// // // //     uint32 sen1_data_0;
// // // //     uint32 sen1_data_1;  

// // // // } PAY_HkTlm_LINPayload_t;

// // // // // BEE-1000 Mission Beacon
// // // // typedef struct {

// // // // 	// Telemetry header
// // // //     uint8 CCSDS_MID[2];
// // // //     uint8 CCSDS_Seq[2];
// // // //     uint8 CCSDS_Len[2];
// // // //     uint8 CCSDS_TimeCode[6];


// // // //     CFE_SRL_HousekeepingTlm_Payload_t srlpayload;

// // // //     RPT_BcnTlm_Payload_t    rptpayload;

// // // //     // Payload
// // // //     PAY_BcnTlm_Payload_t    paybcnpayload;



// // // //     PAY_HkTlm_LINPayload_t     payhkpayload;
    

// // // // }__attribute__((packed)) MissionBeacon;

// // // // #define a sizeof(MissionBeacon);


// // // // struct GETFILEINFO {
// // // //     // ===== TLM Header =====
// // // //     uint8 CCSDS_MID[2];
// // // //     uint8 CCSDS_Seq[2];
// // // //     uint8 CCSDS_Len[2];
// // // //     uint8 CCSDS_TimeCode[6];
// // // //     uint8 padding[4];

// // // //     // ===== Payload =====
// // // //     uint8 FileStatus;
// // // //     uint8 CRC_Computed;
// // // //     uint8 Spare[2];
// // // //     uint32 CRC;
// // // //     uint32 FileSize;
// // // //     uint32 LastModifiedTime;
// // // //     uint32 Mode;
// // // //     char Filename[64];

// // // // };

// // // // #define a sizeof(GETFILEINFO);


// // // // struct Report {
// // // //     // ===== CCSDS Header =====v  16
// // // //     uint16 CCSDS_MsgId;
// // // //     uint16 CCSDS_Seq;
// // // //     uint16 CCSDS_Len;
// // // //     uint8 CCSDS_TimeCode[6];
// // // //     uint32 CCSDS_Padding;


// // // //     // ===== Report Body =====  10     26 byte
// // // //     uint16 ReflectedMID;
// // // //     uint8  ReflectedCC;
// // // //     uint8  RetType;
// // // //     int32  RetCode;
// // // //     uint16 RetValSize;
// // // //     uint8  RetVal[512];

// // // // };

// // // // #define a sizeof(Report)

// // // // struct Event {

// // // //     uint16 CCSDS_MsgId;
// // // //     uint16 CCSDS_Seq;
// // // //     uint16 CCSDS_Len;
// // // //     uint8 CCSDS_TimeCode[6];
// // // //     uint32 CCSDS_Padding;

// // // //     char AppName[20]; /**< 20임   \cfetlmmnemonic \EVS_APPNAME
// // // //                                                 \brief Application name */
// // // //     uint16 EventID;                        /**< \cfetlmmnemonic \EVS_EVENTID
// // // //                                                 \brief Numerical event identifier */
// // // //     uint16 EventType;    /**< uint16임 \cfetlmmnemonic \EVS_EVENTTYPE  
// // // //                                                 \brief Numerical event type identifier */
// // // //     uint32 SpacecraftID;                   /**< \cfetlmmnemonic \EVS_SCID
// // // //                                                 \brief Spacecraft identifier */
// // // //     uint32 ProcessorID;                    /**< \cfetlmmnemonic \EVS_PROCESSORID
// // // //                                                 \brief Numerical processor identifier */


// // // //     char               Message[122]; /**< 122임 \cfetlmmnemonic \EVS_EVENT
// // // //                                                                  \brief Event message string */
// // // //     uint8 Spare1;                                                   /**< \cfetlmmnemonic \EVS_SPARE1
// // // //                                                                          \brief Structure padding */
// // // //     uint8 Spare2;                                                   /**< \cfetlmmnemonic \EVS_SPARE2
// // // //                                                                      \brief Structure padding */

// // // // };

// // // // #define a sizeof(Event)

// // // // typedef enum
// // // // {
// // // //     REPORT_KIND_NONE = 0,

// // // //     // ADCS Sunpointing RPT
// // // //     REPORT_KIND_ADCS_LOG_MASK,
// // // //     REPORT_KIND_ADCS_UNSOLICIT_TLM_SETUP_TLM,

// // // //     REPORT_KIND_UANT_GET_STATUS_TLM,

// // // //     REPORT_KIND_EPS_P60_DOCK_GET_TABLE_HK,
// // // //     REPORT_KIND_EPS_P60_PDU_GET_TABLE_HK,
// // // //     REPORT_KIND_EPS_P60_ACU_GET_TABLE_HK,

// // // // /****************************************************************************************** */
// // // //     REPORT_KIND_ADCS_GET_ERROR_LOG_SETTING,
// // // //     REPORT_KIND_ADCS_GET_CURRENT_UNIX_TIME,
// // // //     REPORT_KIND_ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC,
// // // //     REPORT_KIND_ADCS_GET_COMMUNICATION_STATUS,
// // // //     REPORT_KIND_ADCS_GET_CONTROL_ESTIMATION_MODE,
// // // //     REPORT_KIND_ADCS_GET_REFERENCE_IRC_VECTOR,
// // // //     REPORT_KIND_ADCS_GET_REFERENCE_LLH_TARGET,
// // // //     REPORT_KIND_ADCS_GET_ORBIT_MODE,
// // // //     REPORT_KIND_ADCS_GET_HEALTH_TLM_MMT,
// // // //     REPORT_KIND_ADCS_GET_RAW_CUBESENSE_SUN,
// // // //     REPORT_KIND_ADCS_GET_REFERENCE_RPY_VALUES,
// // // //     REPORT_KIND_ADCS_GET_OPENLOOPCMD_MTQ,
// // // //     REPORT_KIND_ADCS_GET_POWER_STATE,
// // // //     REPORT_KIND_ADCS_GET_RUN_MODE,
// // // //     REPORT_KIND_ADCS_GET_CONTROL_MODE,
// // // //     REPORT_KIND_ADCS_GET_MAG0_MMT_CALIB_CONFIG,
// // // //     REPORT_KIND_ADCS_GET_MAG1_MMT_CALIB_CONFIG,
// // // //     REPORT_KIND_ADCS_GET_ESTIMATION_MODE,
// // // //     REPORT_KIND_ADCS_GET_OPERATIONAL_STATE,
// // // //     REPORT_KIND_ADCS_GET_RAW_CSS_SENSOR,
// // // //     REPORT_KIND_ADCS_GET_RAW_GYR_SENSOR,
// // // //     REPORT_KIND_ADCS_GET_CALIBRATED_GYR_SENSOR,
// // // //     REPORT_KIND_ADCS_GET_MAG_SENSING_ELM_CONFIG,
// // // //     REPORT_KIND_ADCS_GET_TLM_LOG_INCLMASK,
// // // //     REPORT_KIND_ADCS_GET_UNSOLICIT_TLM_MSG_SETUP,
// // // //     REPORT_KIND_ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP,
// // // //     REPORT_KIND_ADCS_GET_EVENT_LOG_STATUS_RESPONSE,
// // // //     REPORT_KIND_ADCS_GET_PORTMAP,
// // // // /********************************************************************************************* */

// // // // /****************************************5차 추가*************************************** */
// // // //     // 1. PAYUEL_ROMA
// // // //     REPORT_KIND_PAYUEL_ROMA_NOOP,            // CC 0, 1
// // // //     REPORT_KIND_PAYUEL_ROMA_RESETCOUNTERS,
// // // //     REPORT_KIND_PAYUEL_ROMA_COMMTEST,
// // // //     REPORT_KIND_PAYUEL_ROMA_GETSPECIFICLINE,
// // // //     REPORT_KIND_PAYUEL_ROMA_GETMULTIPLELINES,
// // // //     REPORT_KIND_PAYUEL_ROMA_GETLATESTLINE,
// // // //     REPORT_KIND_PAYUEL_ROMA_GETLATESTNLINES,
// // // //     REPORT_KIND_PAYUEL_ROMA_SETROUTEDEFAULT,
// // // //     REPORT_KIND_PAYUEL_ROMA_RESETROUTE,
// // // //     REPORT_KIND_PAYUEL_ROMA_LOADROUTE,
// // // //     REPORT_KIND_PAYUEL_ROMA_SAVEROUTE,
// // // //     REPORT_KIND_PAYUEL_ROMA_SENDROUTE,
// // // //     REPORT_KIND_PAYUEL_ROMA_SETROUTE,
// // // //     REPORT_KIND_PAYUEL_ROMA_PARGET,
// // // //     REPORT_KIND_PAYUEL_ROMA_PARSET,
// // // //     REPORT_KIND_PAYUEL_ROMA_PARDEFAULTS,
// // // //     REPORT_KIND_PAYUEL_ROMA_PARSAVE,
// // // //     REPORT_KIND_PAYUEL_ROMA_PARRESTORE,
// // // //     REPORT_KIND_PAYUEL_ROMA_PARLOAD,
// // // //     REPORT_KIND_PAYUEL_ROMA_PARSETOOB,
// // // //     REPORT_KIND_PAYUEL_ROMA_SENDCOMMAND,


// // // //     // 2. PAYUEL_LGPM
// // // //     REPORT_KIND_PAYUEL_LGPM_NOOP,               // CC 0
// // // //     REPORT_KIND_PAYUEL_LGPM_RESETCOUNTERS,
// // // //     REPORT_KIND_PAYUEL_LGPM_MCU_ALIVE,          // CC 2
// // // //     REPORT_KIND_PAYUEL_LGPM_3V3_PWR_ON,         // CC 3
// // // //     REPORT_KIND_PAYUEL_LGPM_3V3_PWR_OFF,        // CC 4
// // // //     REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_ON,      // CC 5
// // // //     REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_OFF,     // CC 6
// // // //     REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_ON,       // CC 7
// // // //     REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_OFF,      // CC 8
// // // //     REPORT_KIND_PAYUEL_LGPM_V28_MAIN_ON,        // CC 9
// // // //     REPORT_KIND_PAYUEL_LGPM_V28_MAIN_OFF,       // CC 10
// // // //     REPORT_KIND_PAYUEL_LGPM_V28_SUB_ON,         // CC 11
// // // //     REPORT_KIND_PAYUEL_LGPM_V28_SUB_OFF,        // CC 12
// // // //     REPORT_KIND_PAYUEL_LGPM_V12_MAIN_ON,        // CC 13
// // // //     REPORT_KIND_PAYUEL_LGPM_V12_MAIN_OFF,       // CC 14
// // // //     REPORT_KIND_PAYUEL_LGPM_PWR_SENSE_INFO,          // CC 15
// // // //     REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_ON,         // CC 16
// // // //     REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_OFF,        // CC 17
// // // //     REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX1,           // CC 18
// // // //     REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX2,           // CC 19
// // // //     REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX3,           // CC 20
// // // //     REPORT_KIND_PAYUEL_LGPM_RWA_PWR_ON,         // CC 21
// // // //     REPORT_KIND_PAYUEL_LGPM_RWA_PWR_OFF,        // CC 22
// // // //     REPORT_KIND_PAYUEL_LGPM_RWA_SENSE_INFO,          // CC 23
// // // // /****************************************************************************************** */

// // // //     REPORT_KIND_SC_GENERIC,

// // // // } ReportKind_t;

// // // // static ReportKind_t DetermineReportKind(uint16_t reflected_mid, uint8_t reflected_cc) {
// // // //     if (reflected_mid == UANT_APP_CMD_ID && reflected_cc == UANT_APP_GET_STATUS_CC) return REPORT_KIND_UANT_GET_STATUS_TLM;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518)  && reflected_cc == ADCS_GET_TLM_LOG_INCLMASK_CC) return REPORT_KIND_ADCS_LOG_MASK;
    
// // // // /**************************************************************************************************************************************************************************************** */
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_ERROR_LOG_SETTING_CC) return REPORT_KIND_ADCS_GET_ERROR_LOG_SETTING;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CURRENT_UNIX_TIME_CC) return REPORT_KIND_ADCS_GET_CURRENT_UNIX_TIME;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC_CC) return REPORT_KIND_ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_COMMUNICATION_STATUS_CC) return REPORT_KIND_ADCS_GET_COMMUNICATION_STATUS;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CONTROL_ESTIMATION_MODE_CC) return REPORT_KIND_ADCS_GET_CONTROL_ESTIMATION_MODE;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_REFERENCE_IRC_VECTOR_CC) return REPORT_KIND_ADCS_GET_REFERENCE_IRC_VECTOR;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_REFERENCE_LLH_TARGET_CC) return REPORT_KIND_ADCS_GET_REFERENCE_LLH_TARGET;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_ORBIT_MODE_CC) return REPORT_KIND_ADCS_GET_ORBIT_MODE;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_HEALTH_TLM_MMT_CC) return REPORT_KIND_ADCS_GET_HEALTH_TLM_MMT;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RAW_CUBESENSE_SUN_CC) return REPORT_KIND_ADCS_GET_RAW_CUBESENSE_SUN;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_REFERENCE_RPY_VALUES_CC) return REPORT_KIND_ADCS_GET_REFERENCE_RPY_VALUES;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_OPENLOOPCMD_MTQ_CC) return REPORT_KIND_ADCS_GET_OPENLOOPCMD_MTQ;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_POWER_STATE_CC) return REPORT_KIND_ADCS_GET_POWER_STATE;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RUN_MODE_CC) return REPORT_KIND_ADCS_GET_RUN_MODE;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CONTROL_MODE_CC) return REPORT_KIND_ADCS_GET_CONTROL_MODE;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_MAG0_MMT_CALIB_CONFIG_CC) return REPORT_KIND_ADCS_GET_MAG0_MMT_CALIB_CONFIG;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_MAG1_MMT_CALIB_CONFIG_CC) return REPORT_KIND_ADCS_GET_MAG1_MMT_CALIB_CONFIG;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_ESTIMATION_MODE_CC) return REPORT_KIND_ADCS_GET_ESTIMATION_MODE;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_OPERATIONAL_STATE_CC) return REPORT_KIND_ADCS_GET_OPERATIONAL_STATE;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RAW_CSS_SENSOR_CC) return REPORT_KIND_ADCS_GET_RAW_CSS_SENSOR;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RAW_GYR_SENSOR_CC) return REPORT_KIND_ADCS_GET_RAW_GYR_SENSOR;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CALIBRATED_GYR_SENSOR_CC) return REPORT_KIND_ADCS_GET_CALIBRATED_GYR_SENSOR;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_MAG_SENSING_ELM_CONFIG_CC) return REPORT_KIND_ADCS_GET_MAG_SENSING_ELM_CONFIG;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_TLM_LOG_INCLMASK_CC) return REPORT_KIND_ADCS_GET_TLM_LOG_INCLMASK;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_UNSOLICIT_TLM_MSG_SETUP_CC) return REPORT_KIND_ADCS_GET_UNSOLICIT_TLM_MSG_SETUP;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP_CC) return REPORT_KIND_ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_EVENT_LOG_STATUS_RESPONSE_CC) return REPORT_KIND_ADCS_GET_EVENT_LOG_STATUS_RESPONSE;
// // // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_PORTMAP_CC) return REPORT_KIND_ADCS_GET_PORTMAP;
// // // // /****************************************************************************************************************************************************************************************** */

// // // //     /*********************************************5차 추가****************************************************** */
// // // //     // CC 0, 1
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_NOOP_CC)) return REPORT_KIND_PAYUEL_ROMA_NOOP;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_RESET_COUNTERS_CC)) return REPORT_KIND_PAYUEL_ROMA_RESETCOUNTERS;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_COMM_TEST_CC)) return REPORT_KIND_PAYUEL_ROMA_COMMTEST;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_SPECIFIC_LINE_CC)) return REPORT_KIND_PAYUEL_ROMA_GETSPECIFICLINE;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_MULTIPLE_LINES_CC)) return REPORT_KIND_PAYUEL_ROMA_GETMULTIPLELINES;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_LATEST_LINE_CC)) return REPORT_KIND_PAYUEL_ROMA_GETLATESTLINE;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_LATEST_N_LINES_CC)) return REPORT_KIND_PAYUEL_ROMA_GETLATESTNLINES;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SET_ROUTE_DEFAULT_CC)) return REPORT_KIND_PAYUEL_ROMA_SETROUTEDEFAULT;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_RESET_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_RESETROUTE;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_LOAD_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_LOADROUTE;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SAVE_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_SAVEROUTE;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SEND_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_SENDROUTE;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SET_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_SETROUTE;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_GET_CC)) return REPORT_KIND_PAYUEL_ROMA_PARGET;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_SET_CC)) return REPORT_KIND_PAYUEL_ROMA_PARSET;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_DEFAULTS_CC)) return REPORT_KIND_PAYUEL_ROMA_PARDEFAULTS;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_SAVE_CC)) return REPORT_KIND_PAYUEL_ROMA_PARSAVE;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_RESTORE_CC)) return REPORT_KIND_PAYUEL_ROMA_PARRESTORE;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_LOAD_CC)) return REPORT_KIND_PAYUEL_ROMA_PARLOAD;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_SET_OOB_CC)) return REPORT_KIND_PAYUEL_ROMA_PARSETOOB;
// // // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SEND_COMMAND_CC)) return REPORT_KIND_PAYUEL_ROMA_SENDCOMMAND;

// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_NOOP_CC)) return REPORT_KIND_PAYUEL_LGPM_NOOP;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RESET_COUNTERS_CC)) return REPORT_KIND_PAYUEL_LGPM_RESETCOUNTERS;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_MCU_ALIVE_CHECK_CC)) return REPORT_KIND_PAYUEL_LGPM_MCU_ALIVE;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_3V3_PWR_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_3V3_PWR_ON;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_3V3_PWR_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_3V3_PWR_OFF;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_MAIN_BOOST_SW_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_ON;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_MAIN_BOOST_SW_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_OFF;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_SUB_BOOST_SW_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_ON;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_SUB_BOOST_SW_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_OFF;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_MAIN_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_MAIN_ON;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_MAIN_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_MAIN_OFF;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_SUB_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_SUB_ON;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_SUB_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_SUB_OFF;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V12_MAIN_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_V12_MAIN_ON;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V12_MAIN_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_V12_MAIN_OFF;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_PWR_SENSE_INFO_CC)) return REPORT_KIND_PAYUEL_LGPM_PWR_SENSE_INFO;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_PWR_SEQ_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_ON;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_PWR_SEQ_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_OFF;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_CONTROL_idx1_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX1;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_CONTROL_idx2_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX2;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_CONTROL_idx3_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX3;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_PWR_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_PWR_ON;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_PWR_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_PWR_OFF;
// // // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_SENSE_INFO_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_SENSE_INFO;
// // // //     /**************************************************************************************************************************/
// // // // }

// // // // typedef struct
// // // // {
// // // //     uint8_t bytes[512];
// // // // } RptGenericPayload_t;



// // // // typedef struct __attribute__((__packed__)) gs_gssb_ant6_release_status_t {
// // // //     /**
// // // //        Burn state of the first channel (Burning = 1, Idle = 0)
// // // //      */
// // // //     uint8_t channel_0_state;
// // // //     /**
// // // //        Release status of the first channel (Released = 1, Not released = 0)
// // // //      */
// // // //     uint8_t channel_0_status;
// // // //     /**
// // // //        Burn time left of the first channel [s]
// // // //      */
// // // //     uint8_t channel_0_burn_time_left;
// // // //     /**
// // // //        Counter of have many burns there has been attempted
// // // //      */
// // // //     uint8_t channel_0_burn_tries;
// // // //     /**
// // // //        Burn state of the second channel (Burning = 1, Idle = 0)
// // // //      */
// // // //     uint8_t channel_1_state;
// // // //     /**
// // // //        Release status of the second channel (Released = 1, Not released = 0)
// // // //      */
// // // //     uint8_t channel_1_status;
// // // //     /**
// // // //        Burn time left of the second channel [s]
// // // //      */
// // // //     uint8_t channel_1_burn_time_left;
// // // //     /**
// // // //        Counter of have many burns there has been attempted
// // // //      */
// // // //     uint8_t channel_1_burn_tries;
// // // // } gs_gssb_ant6_release_status_t;


// // // // typedef struct EPS_P60_DOCK_GET_TABLE_HK {

// // // //     int16_t   c_out[13];
// // // //     uint16_t  v_out[13];
// // // //     uint8_t   out_en[13];

// // // //     int16_t   temp[2];

// // // //     uint32_t  bootcause;
// // // //     uint32_t  bootcnt;
// // // //     uint32_t  uptime;

// // // //     uint16_t  resetcause;

// // // //     uint8_t   batt_mode;
// // // //     uint8_t   heater_on;
// // // //     uint8_t   conv_5v_en;

// // // //     uint16_t  latchup[13];

// // // //     uint16_t  vbat_v;
// // // //     int16_t   vcc_c;
// // // //     int16_t   batt_c;
// // // //     uint16_t  batt_v;

// // // //     int16_t   batt_temp[2];

// // // //     uint8_t   device_type[8];
// // // //     uint8_t   device_status[8];

// // // //     uint8_t   dearm_status;

// // // //     uint32_t  wdt_cnt_gnd;
// // // //     uint32_t  wdt_cnt_i2c;
// // // //     uint32_t  wdt_cnt_can;
// // // //     uint32_t  wdt_cnt_csp[2];

// // // //     uint32_t  wdt_gnd_left;
// // // //     uint32_t  wdt_i2c_left;
// // // //     uint32_t  wdt_can_left;

// // // //     uint8_t   wdt_csp_left[2];

// // // //     int16_t   batt_chrg;
// // // //     int16_t   batt_dischrg;

// // // //     int8_t    ant6_depl;
// // // //     int8_t    ar6_depl;

// // // // } EPS_P60_DOCK_GET_TABLE_HK;

// // // // typedef struct EPS_P60_PDU_GET_TABLE_HK {

// // // //     int16_t   c_out[9];
// // // //     uint16_t  v_out[9];

// // // //     uint16_t  vcc;
// // // //     uint16_t  vbat;
// // // //     int16_t   temp;

// // // //     uint8_t   conv_en[3];
// // // //     uint8_t   out_en[9];

// // // //     uint32_t  bootcause;
// // // //     uint32_t  bootcnt;
// // // //     uint32_t  uptime;

// // // //     uint16_t  resetcause;

// // // //     uint8_t   batt_mode;

// // // //     uint16_t  latchup[9];

// // // //     uint8_t   device_type[8];
// // // //     uint8_t   device_status[8];

// // // //     uint32_t  wdt_cnt_gnd;
// // // //     uint32_t  wdt_cnt_i2c;
// // // //     uint32_t  wdt_cnt_can;
// // // //     uint32_t  wdt_cnt_csp[2];

// // // //     uint32_t  wdt_gnd_left;
// // // //     uint32_t  wdt_i2c_left;
// // // //     uint32_t  wdt_can_left;

// // // //     uint8_t   wdt_csp_left[2];

// // // // } EPS_P60_PDU_GET_TABLE_HK;


// // // // typedef struct EPS_P60_ACU_GET_TABLE_HK {

// // // //     int16_t   c_in[6];
// // // //     uint16_t  v_in[6];

// // // //     uint16_t  vbat;
// // // //     uint16_t  vcc;

// // // //     int16_t   temp[3];

// // // //     uint8_t   mppt_mode;

// // // //     uint16_t  vboost[6];
// // // //     uint16_t  power[6];

// // // //     uint8_t   dac_en[3];
// // // //     uint16_t  dac_val[6];

// // // //     uint32_t  bootcause;
// // // //     uint32_t  bootcnt;
// // // //     uint32_t  uptime;

// // // //     uint16_t  resetcause;

// // // //     uint16_t  mppt_time;
// // // //     uint16_t  mppt_period;

// // // //     uint8_t   device_type[8];
// // // //     uint8_t   device_status[8];

// // // //     uint32_t  wdt_cnt_gnd;
// // // //     uint32_t  wdt_gnd_left;

// // // // } EPS_P60_ACU_GET_TABLE_HK;



// // // // typedef struct
// // // // {
// // // //     bool     valid;
// // // //     uint16_t CCMessage_ID;
// // // //     uint16_t CCCount;
// // // //     uint16_t CCLength;
// // // //     uint8_t  CCTime_code[6];

// // // //     uint16_t reflected_msg_id;
// // // //     uint8_t  reflected_cc;
// // // //     uint8_t  ret_type;
// // // //     int32_t  ret_code;
// // // //     uint16_t ret_val_size;

// // // //     ReportKind_t kind;

// // // //     union
// // // //     {
// // // //         RptGenericPayload_t    generic;


// // // //         ADCS_TlmLogInclMaskTlm_Payload_t       adcs_logmask;
// // // //         ADCS_UnsolicitTlmMsgSetupTlm_Payload_t adcs_unsolicited_tlm_tlm;

// // // //         gs_gssb_ant6_release_status_t          uant_getstatus;
// // // //         EPS_P60_DOCK_GET_TABLE_HK              eps_p60dockgettablehk;
// // // //         EPS_P60_PDU_GET_TABLE_HK               eps_p60pdugettablehk;
// // // //         EPS_P60_ACU_GET_TABLE_HK               eps_p60acugettablehk;
// // // // /************************************************************************************************************************************************************* */
// // // //         ADCS_ErrorLogSettingTlm_Payload_t      adcs_errorlogsetting;
// // // //         ADCS_CurrentUnixTimeTlm_Payload_t      adcs_currentunixtime;
// // // //         ADCS_PersistConfigDiagnosticTlm_Payload_t adcs_persistconfigdiagnostic;
// // // //         ADCS_CommunicationStatusTlm_Payload_t  adcs_communicationstatus;
// // // //         ADCS_ControlEstimationModeTlm_Payload_t adcs_controlestimationmode;
// // // //         ADCS_ReferenceIRCVectorTlm_Payload_t    adcs_referenceircvector;
// // // //         ADCS_ReferenceLLHTargetTlm_Payload_t    adcs_referencellhtarget;
// // // //         ADCS_OrbitModeTlm_Payload_t             adcs_orbitmode;
// // // //         ADCS_HealthTlmMMTTlm_Payload_t          adcs_healthtlmmmt;
// // // //         ADCS_RawCubeSenseSunTlm_Payload_t       adcs_rawcubesensesun;
// // // //         ADCS_ReferenceRPYvaluesTlm_Payload_t    adcs_referencerpyvalues;
// // // //         ADCS_OpenLoopCmdMTQTlm_Payload_t        adcs_openloopcmdmtq;
// // // //         ADCS_PowerStateTlm_Payload_t            adcs_powerstate;
// // // //         ADCS_RunModeTlm_Payload_t               adcs_runmode;
// // // //         ADCS_ControlModeTlm_Payload_t           adcs_controlmode;
// // // //         ADCS_Mag0MMTCalibConfigTlm_Payload_t    adcs_mag0mmtcalibconfig;
// // // //         ADCS_Mag1MMTCalibConfigTlm_Payload_t    adcs_mag1mmtcalibconfig;
// // // //         ADCS_EstimationModeTlm_Payload_t        adcs_estimationmode;
// // // //         ADCS_OperationalStateTlm_Payload_t      adcs_operationalstate;
// // // //         ADCS_RawCSSSensorTlm_Payload_t          adcs_rawcsssensor;
// // // //         ADCS_RawGYRSensorTlm_Paylaod_t          adcs_rawgyrsensor;
// // // //         ADCS_CalibratedGYRSensorTlm_Payload_t   adcs_calibratedgyrsensor;
// // // //         ADCS_MagSensingElmConfigTlm_Payload_t   adcs_magsensingelmconfig;
// // // //         ADCS_TlmLogInclMaskTlm_Payload_t        adcs_tlmloginclmask;
// // // //         ADCS_UnsolicitTlmMsgSetupTlm_Payload_t  adcs_unsolicittlmmsgsetup;
// // // //         ADCS_UnsolicitEventMsgSetupTlm_Payload_t adcs_unsoliciteventmsgsetup;
// // // //         ADCS_EventLogStatusResponseTlm_Payload_t adcs_eventlogstatusresponse;
// // // //         ADCS_PortMapTlm_Payload_t               adcs_portmap;
// // // // /******************************************************************************************************************************************************************** */

// // // // /**********************************************5차 추가********************************************** */
// // // //         // 1. PAYUEL_ROMA
// // // //         payuel_roma_Noop_tlm_payload_t                     roma_noop;;
// // // //         payuel_roma_ResetCounters_tlm_payload_t            roma_resetcounters;
// // // //         payuel_roma_CommTest_tlm_payload_t                 roma_commtest;
// // // //         payuel_roma_GetSpecificLine_tlm_payload_t          roma_getspecificline;
// // // //         payuel_roma_GetMultipleLines_tlm_payload_t         roma_getmultiplelines;
// // // //         payuel_roma_GetLatestLine_tlm_payload_t            roma_getlatestline;
// // // //         payuel_roma_GetLatest_N_Lines_tlm_payload_t        roma_getlatestNlines;
// // // //         payuel_roma_SetRouteDefault_tlm_payload_t          roma_setroutedefault;
// // // //         payuel_roma_ResetRoute_tlm_payload_t               roma_resetroute;
// // // //         payuel_roma_LoadRoute_tlm_payload_t                roma_loadroute;
// // // //         payuel_roma_SaveRoute_tlm_payload_t                roma_saveroute;
// // // //         payuel_roma_SendRoute_tlm_payload_t                roma_sendroute;
// // // //         payuel_roma_SetRoute_tlm_payload_t                 roma_setroute;
// // // //         payuel_roma_ParGet_tlm_payload_t                   roma_parget;
// // // //         payuel_roma_ParSet_tlm_payload_t                   roma_parset;
// // // //         payuel_roma_ParDefaults_tlm_payload_t              roma_pardefaults;
// // // //         payuel_roma_ParSave_tlm_payload_t                  roma_parsave;
// // // //         payuel_roma_ParRestore_tlm_payload_t               roma_parrestore;
// // // //         payuel_roma_ParLoad_tlm_payload_t                  roma_parload;
// // // //         payuel_roma_ParSetOOB_tlm_payload_t                roma_parsetOOB;
// // // //         payuel_roma_SendCommand_tlm_payload_t              roma_sendcommand;

// // // //         PAYUEL_LGPM_Noop_tlm_payload_t                     lgpm_noop;
// // // //         PAYUEL_LGPM_ResetCounters_tlm_payload_t            lgpm_resetcounters;
// // // //         PAYUEL_LGPM_MCU_ALIVE_CHECK_Tlm_Payload            lgpm_mcualivecheck;
// // // //         PAYUEL_LGPM_3V3PwrOn_tlm_payload_t                 lgpm_3v3pwron;
// // // //         PAYUEL_LGPM_3V3PwrOff_tlm_payload_t                lgpm_3v3pwroff;
// // // //         PAYUEL_LGPM_MainBoostSwOn_tlm_payload_t            lgpm_mainboostswon;
// // // //         PAYUEL_LGPM_MainBoostSwOff_tlm_payload_t           lgpm_mainboostswoff;
// // // //         PAYUEL_LGPM_SubBoostSwOn_tlm_payload_t             lgpm_subboostswon;
// // // //         PAYUEL_LGPM_SubBoostSwOff_tlm_payload_t            lgpm_subboostswoff;
// // // //         PAYUEL_LGPM_V28MainOn_tlm_payload_t                lgpm_v28mainon;
// // // //         PAYUEL_LGPM_V28MainOff_tlm_payload_t               lgpm_v28mainoff;
// // // //         PAYUEL_LGPM_V28SubOn_tlm_payload_t                 lgpm_v28subon;
// // // //         PAYUEL_LGPM_V28SubOff_tlm_payload_t                lgpm_v28suboff;
// // // //         PAYUEL_LGPM_V12MainOn_tlm_payload_t                lgpm_v12mainon;
// // // //         PAYUEL_LGPM_V12MainOff_tlm_payload_t               lgpm_v12mainoff;
// // // //         PAYUEL_LGPM_PwrSenseInfo_tlm_payload_t             lgpm_pwrsenseinfo;
// // // //         PAYUEL_LGPM_PwrSeqOn_tlm_payload_t                 lgpm_pwrseqon;
// // // //         PAYUEL_LGPM_PwrSeqOff_tlm_payload_t                lgpm_pwrseqoff;
// // // //         PAYUEL_LGPM_RwaControlIdx1_tlm_payload_t           lgpm_rwacontrol_idx1;
// // // //         PAYUEL_LGPM_RwaControlIdx2_tlm_payload_t           lgpm_rwacontrol_idx2;
// // // //         PAYUEL_LGPM_RwaControlIdx3_tlm_payload_t           lgpm_rwacontrol_idx3;
// // // //         PAYUEL_LGPM_RwaPwrOn_tlm_payload_t                 lgpm_rwapwron;
// // // //         PAYUEL_LGPM_RwaPwrOff_tlm_payload_t                lgpm_rwapwroff;
// // // //         PAYUEL_LGPM_RwaSenseInfo_tlm_payload_t             lgpm_rwasenseinfo;
// // // // /**************************************************************************************************** */

// // // //     } u;
// // // // } ReportView_t;


// // // // extern std::mutex g_report_view_mtx;
// // // // extern ReportView_t g_report_view;


// // // // typedef struct {
// // // //     uint8 Callsign[6];
// // // //     uint8 CurrentMode;
// // // //     uint8 CurrentSubmode;
// // // //     uint8 PrevioudMode;
// // // //     uint8 PreviousSubmode;
// // // //     uint8 CurrentModeFlag;
// // // //     uint8 PreviousModeFlag;
// // // //     uint32 ApplicationRunStatus;
// // // //     uint32 SatelliteTime;
// // // //     uint16 RebootCount;
// // // //     uint8 RebootCause;

// // // // }__attribute__((packed)) FM_HK_;

// // // // typedef struct {
// // // // 	uint16 DeployState_UANT;
// // // // }__attribute__((packed)) UANT_;

// // // // typedef struct {
// // // // 	uint32 rxfreq;
// // // // 	uint32 txfreq;
// // // // 	int16 LastRssi;
// // // //     uint32 TotRxBytes;
// // // //     uint8 StatusConfiguration;
// // // // }__attribute__((packed)) UTRX_;

// // // // typedef struct {
// // // // 	//EPS - P60 Dock
// // // // 	uint8 out_en_dock[7]; //01458910
// // // //     int16 temp_dock[2];
// // // //     uint32 bootcause;
// // // //     uint32 bootcnt;
// // // //     uint32 uptime;
// // // //     uint16 resetcause;
// // // //     uint8 batt_mode;
// // // //     uint8 heater_on;
// // // //     uint16 latchup_dock[7]; //01458910
// // // //     uint16 vbat_v;
// // // //     int16 batt_v;
// // // //     int16 batt_temp[2];
// // // //     uint8 device_status[8];
// // // //     uint32 wdt_cnt_gnd;
// // // //     uint32 wdt_gnd_left;
// // // //     int16 batt_chrg;
// // // //     int16 batt_dischrg;
// // // // 	//EPS - PDU
// // // //     int16 vbat;
// // // //     uint8 out_en_pdu[6]; //034758
// // // //     uint16 latchup_pdu[6];
// // // //     uint16 out_voltage[6]; // 034758
// // // // 	//EPS - ACU
// // // //     int16 c_in[4]; //0123
// // // //     uint16 v_in[4]; //0123
// // // // }__attribute__((packed)) EPS_;

// // // // typedef struct {
// // // // 	uint8 RWL0_PowerState;
// // // // 	uint8 RWL1_PowerState;
// // // // 	uint8 RWL2_PowerState;
// // // // 	uint8 MAG0_PowerState;
// // // // 	uint8 FSS0_PowerState;
// // // // 	uint8 HSS0_PowerState;
// // // // 	uint8 Control_Mode;
// // // // 	uint16 Mag_Control_Timeout;
// // // // 	float GYRO_Calib_rate_X;
// // // // 	float GYRO_Calib_rate_Y;
// // // // 	float GYRO_Calib_rate_Z;
// // // // }__attribute__((packed)) ADCS_;

// // // // typedef struct {
// // // // 	uint8 Status;
// // // // 	int16 Board_Temperature;
// // // // 	int16 Battery_Current;
// // // // 	int16 Battery_Voltage;
// // // // }__attribute__((packed)) STX_;

// // // // typedef struct {
// // // // 	int16 temp_PAYC;
// // // // 	uint16 icore;
// // // // }__attribute__((packed)) PAYC_;

// // // // typedef struct {
// // // // 	uint8 DeployStatus_PAYR;
// // // // }__attribute__((packed)) PAYR_;

// // // // typedef struct {
// // // // 	uint8 PAYS_State;
// // // // 	uint8 PAYS_Sign;
// // // // 	uint8 PAYS_Temp;
// // // // }__attribute__((packed)) PAYS_;


// // // // // typedef struct {
// // // // // 	CCSDS_Header_ CCSDS_Header;
// // // // // 	FM_HK_ FM;
// // // // // 	EPS_ EPS;
// // // // // 	TCS_ TCS;
// // // // // 	RWA_ RWA;
// // // // // 	MTQ_ MTQ;
// // // // // 	SNSR_ SNSR;
// // // // // 	UTRX_ UTRX;
// // // // // 	STX_ STX;
// // // // // 	PAY_ PAY;
// // // // // }__attribute__((packed)) HK;

// // // // // typedef struct {
// // // // // 	FM_HK_ FM;
// // // // // 	ADCS_ ADCS;
// // // // // }__attribute__((packed)) AOD;

// // // // typedef struct {
// // // // 	uint32_t ExTime;
// // // // 	uint32_t ExWindow;
// // // // 	uint16_t EntryID;
// // // // 	uint16_t GroupID;
// // // // 	uint8_t cmd[];
// // // // }__attribute__((packed)) Book;



// // // // static bool ParseReportWire540(const uint8_t *buf, size_t len, Report &out);
// // // // void * TRxController(void *);
// // // // void * SignalTest(void*);
// // // // void now_rx_bytes_update();
// // // // void set_rx_bytes(uint32_t nowbytes);
// // // // uint32_t get_rx_bytes();
// // // // uint32_t * get_rx_bytes_address();
// // // // uint16_t get_boot_count();
// // // // uint16_t * get_boot_count_address();
// // // // void buf_allclear();
// // // // void CalculateChecksum(CommandHeader_t* Cmd);
// // // // int32_t GenerateCmdMsg(CommandHeader_t* Cmd, uint16_t MsgId, uint8_t FcnCode, uint32_t ArgLen);
// // // // csp_socket_t *  DL_sock_initialize();
// // // // extern BEE1000_Beacon_t* bee1000_beacon;
// // // // extern BEE1012_Beacon_t* bee1012_beacon;
// // // // extern UELYSYS_Beacon_t* uelysys_beacon;
// // // // int BEE1000BeaconSaver(BEE1000_Beacon_t * bec);
// // // // int BEE1012BeaconSaver(BEE1012_Beacon_t * bec);
// // // // int UELYSYSBeaconSaver(UELYSYS_Beacon_t * bec);
// // // // void * task_downlink_onorbit(void * socketinfo);
// // // // void * task_uplink_onorbit(void * sign_);

// // // // int PacketHandler(csp_packet_t *packet, int type, int NowCursor);
// // // // packetsign * PingInit(FSWTle * FSWTleinfo);
// // // // csp_packet_t * PacketEncoder(packetsign * sign, bool freeer = true);
// // // // packetsign * PacketDecoder(csp_packet_t * packet);


// // // // class CmdGenerator_GS {
// // // // private:
// // // //     void SetHeaderWord(uint8_t* Word16, uint16_t Value, uint16_t Mask);
// // // //     void SetHeaderByte(uint8_t* Byte, uint8_t Value, uint8_t Mask);
// // // //     void GetHeaderWord(const uint8_t* Word16, uint16_t& Value, uint16_t Mask);
// // // //     uint32_t ComputeCheckSum(void);

// // // // public:
// // // // 	CFE_MSG_CommandHeader* CmdHeader;
// // // // 	bool Scheduled = false;
// // // // 	bool Checksum = true;

// // // //     CmdGenerator_GS(void);
// // // //     ~CmdGenerator_GS(void);

// // // //     int GenerateCmdHeader(uint32_t MsgId, uint16_t FncCode, uint32_t Size, void* Data);
// // // //     void CopyCmdHeaderToBuffer(uint8_t* Buffer);

// // // //     void InitHeader(void);
// // // //     void SetHeader(const CFE_MSG_CommandHeader* Header);
// // // //     const CFE_MSG_CommandHeader* GetHeader(void) const;

// // // //     int SetHasSecondaryHeader(bool HasSec);
// // // //     int SetMsgId(uint16_t MsgId);
// // // //     int SetSize(uint16_t Size);
// // // //     int SetSegmentationFlag(uint16_t SegFlag);
// // // //     int SetFncCode(uint16_t FncCode);

// // // //     bool HasSecondaryHeader(void) const;
// // // //     uint16_t GetSize(void);
// // // //     uint16_t GetFncCode(void) const;

// // // //     int GenerateChecksum(void);
// // // // 	int Scheduling(uint32_t ExecutionTime, uint32_t ExecutionWindow, uint32_t EntryID, uint16_t GroupID);
// // // // 	packetsign * GenerateCMDPacket(void);
// // // // };

// // // // void * Direct_Shell(void * data);

// // // // #endif _MIMAN_COMS_H_



// // // #pragma once
// // // #ifndef _MIMAN_COMS_H_
// // // #define _MIMAN_COMS_H_

// // // #include "miman_config.h"
// // // #include "miman_orbital.h"
// // // #include <mutex>

// // // #pragma once
// // // #include <mutex>






// // // typedef struct{
// // // 	uint16_t Identifier;
// // // 	uint16_t PacketType;
// // // 	uint32_t Length;
// // // 	uint8_t Data[];

// // // }__attribute__((packed)) packetsign;

// // // typedef struct{
// // // 	uint16_t target;
// // // 	uint16_t filestatus;
// // // 	uint32_t filenum;
// // // 	uint32_t offset;
// // // 	uint32_t step;
// // // }__attribute__((packed)) dlreqdata;

// // // typedef struct{
// // // 	int32_t filenum;
// // // 	uint32_t file[];
// // // }__attribute__((packed)) filelist;

// // // // typedef enum {
// // // //     GS_FTP_INFO_FILE  = 0,   /**< File size and checksum */
// // // //     GS_FTP_INFO_CRC = 1,         /**< CRC of remote and local file */
// // // //     GS_FTP_INFO_COMPLETED = 2,   /**< Completed and total chunks */
// // // //     GS_FTP_INFO_PROGRESS = 3,    /**< Current chunk, total_chunks and chunk_size */
// // // // } gs_ftp_info_type_t;

// // // typedef struct {
// // //     FILE       * fp;
// // //     FILE       * fp_map;
// // //     csp_conn_t * conn;
// // //     uint32_t   timeout;
// // //     char       file_name[GS_FTP_PATH_LENGTH];
// // //     uint32_t   file_size;
// // //     uint32_t   chunks;
// // //     int        chunk_size;
// // //     uint32_t   checksum;
// // //     ftp_status_element_t last_status[GS_FTP_STATUS_CHUNKS];
// // //     uint32_t   last_entries;
// // //     gs_ftp_info_callback_t info_callback;
// // //     void       * info_data;
// // // } gs_ftp_state_t;

// // // typedef struct {
// // //     gs_ftp_backend_type_t backend;
// // //     const char * path;
// // //     uint32_t addr;
// // //     uint32_t size;
// // // } gs_ftp_url_t;



// // // // BEE-1000 RPT structure
// // // typedef struct {
// // //     bool valid;
// // //     uint16_t CCMessage_ID;
// // //     uint16_t CCCount;
// // //     uint16_t CCLength;
// // //     uint8_t CCTime_code[6];
// // //     uint16_t msg_id;
// // //     uint8_t cc;
// // //     uint8_t ret_type;
// // //     int32_t ret_code;
// // //     uint16_t ret_val_size;
// // //     std::vector<uint8_t> payload;
// // // } ReportPacket_t;

// // // extern ReportPacket_t g_last_report;


// // // // BEE-1000 Beacon
// // // typedef struct {

// // // 	// CCSDS Header
// // //     uint8 CCSDS_MID[2];
// // //     uint8 CCSDS_Seq[2];
// // //     uint8 CCSDS_Len[2];
// // //     uint8 CCSDS_TimeCode[6];

// // // 	// FSW - RPT - shoud be deprecated
// // // 	// uint16 RPT_BootCount;
// // //     // uint32 RPT_ScTimeSec;
// // //     // uint32 RPT_ScTimeSubsec;
// // //     // uint32 RPT_Sequence;
// // //     // uint8 RPT_ResetCause;

// // //     /* FSW - RPT - Revised (Kweon Hyeokjin) */
// // //     uint8_t RPT_CmdCounter;
// // //     uint8_t RPT_ErrCounter;
// // //     uint8_t RPT_ReportCnt;
// // //     uint8_t RPT_CriticalCnt;
// // //     uint16_t RPT_BootCount;
// // //     uint32_t RPT_ScTimeSec;
// // //     uint32_t RPT_ScTimeSubsec;
// // //     uint8_t RPT_Sequence_LSB;
// // //     /*--------------End of BEE RPT Revision (Kweon Hyeokjin)--------------*/

// // // 	// COMS - STX
// // //     // all param
// // //     uint8_t  STX_symbol_rate;
// // //     uint8_t  STX_transmit_power;
// // //     uint8_t  STX_modcod;
// // //     uint8_t  STX_roll_off;
// // //     uint8_t  STX_pilot_signal;
// // //     uint8_t  STX_fec_frame_size;
// // //     uint16_t STX_pretransmission_delay;
// // //     float    STX_center_frequency;
// // //     // modulation interface -> cpu temp 빠짐
// // //     uint8_t STX_modulator_interface_type;  
// // //     uint8_t STX_lvds_io_type;    
// // // 	uint8 STX_SystemState;
// // //     uint8 STX_StatusFlag;
// // //     // float STX_CpuTemp;

// // // 	// COMS - UANT
// // //     uint8 UANT1_Chan0;
// // //     uint8 UANT1_Chan1;
// // //     uint8 UANT1_BackupActive;
// // //     uint8 UANT2_Chan0;
// // //     uint8 UANT2_Chan1;
// // //     uint8 UANT2_BackupActive;

// // // 	// COMS - UTRX
// // //     uint8 UTRX_ActiveConf;
// // //     uint16 UTRX_BootCount;
// // //     uint32 UTRX_BootCause;
// // //     int16 UTRX_BoardTemp;

// // //     // PCDU - P60 Dock
// // //     int16 P60D_Cout[9];
// // //     uint16 P60D_Vout[9];
// // //     uint16 P60D_OutEn;
// // //     uint32 P60D_BootCause;
// // //     uint32 P60D_BootCount;
// // //     uint8 P60D_BattMode;
// // //     uint8 P60D_HeaterOn;
// // //     uint16 P60D_VbatV;
// // //     int16 P60D_VccC;
// // //     uint16 P60D_BattV;
// // //     int16 P60D_BattTemp[2];
// // //     uint32 P60D_WdtCanLeft;
// // //     int16 P60D_BattChrg;
// // //     int16 P60D_BattDischrg;

// // //     // PCDU - P60 PDU
// // //     int16  P60P_Cout[9];
// // //     uint16 P60P_Vout[9];
// // //     int16  P60P_Vcc;
// // //     uint8  P60P_ConvEn;
// // //     uint16 P60P_OutEn;

// // //     // PCDU - P60 ACU
// // //     int16  P60A_Cin[6];
// // //     uint16 P60A_Vin[6];

// // //     // ADCS
// // //        /** Combined Power State
// // //      *  | 7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
// // //      *  +--------------------------------------+
// // //      *  |Rsv|RWL0|RWL1|RWL2|MAG0|GYRO|FSS0|HSS0|
// // //      *  +--------------------------------------+
// // //      */
// // //     uint8 ADCS_PowerState; // ID 183

// // //     uint8 ADCS_ControlMode; // ID 185

// // //     float ADCS_GYR0CalibratedRateXComponent;
// // //     float ADCS_GYR0CalibratedRateYComponent;
// // //     float ADCS_GYR0CalibratedRateZComponent; // ID 207, 12bytes

// // // }__attribute__((packed)) BEE1000_Beacon_t;

// // // #define a sizeof(BEE1000_Beacon_t);
// // // static_assert(sizeof(BEE1000_Beacon_t) == 203, "BEE1000_Beacon_t size mismatch");

// // // typedef struct {
// // //     char call_sign[7];
// // //     uint8_t msg_id[2];
// // //     uint8_t sequence[2];
// // //     uint8_t length[2];
// // //     uint8_t time_code[6];
// // // }__attribute__((packed)) BEE1012_BeaconHeader_t;

// // // typedef struct {
// // //     uint16_t boot_count;
// // //     uint32_t sequence;
// // //     uint8_t reset_cause;
// // // }__attribute__((packed)) BEE1012_RPT_t;

// // // typedef struct {
// // //     uint8_t active_conf;
// // //     uint16_t boot_count;
// // //     uint32_t boot_cause;
// // //     int16_t temp_board;
// // // }__attribute__((packed)) BEE1012_AX100_t;

// // // typedef struct {
// // //     int16_t temp;
// // //     int8_t connection_quality;
// // //     uint16_t battery_capacity;
// // // }__attribute__((packed)) BEE1012_LTRX_t;

// // // typedef struct {
// // //     BEE1012_AX100_t ax100;
// // //     BEE1012_LTRX_t ltrx;
// // // }__attribute__((packed)) BEE1012_COMS_t;

// // // typedef struct {
// // //     uint32_t bootcause;
// // //     uint16_t resetcause;
// // //     uint16_t bootcount;
// // //     uint8_t out_en[6];
// // //     int16_t temp[2];
// // //     uint8_t batt_mode;
// // //     int16_t batt_i;
// // //     uint16_t batt_v;
// // //     uint8_t sm_en;
// // //     uint16_t gnd_wdt_cnt;
// // //     uint16_t bus_wdt_cnt;
// // //     uint32_t gnd_wdt_left;
// // //     uint32_t bus_wdt_left;
// // // }__attribute__((packed)) BEE1012_EPS_PMU_t;

// // // typedef struct {
// // //     int16_t out_i[12];
// // //     uint8_t out_en[12];
// // // }__attribute__((packed)) BEE1012_EPS_PDU_t;

// // // typedef struct {
// // //     int16_t input_i[6];
// // //     uint16_t input_v[6];
// // //     uint8_t mppt_mode;
// // // }__attribute__((packed)) BEE1012_EPS_ACU_Unit_t;

// // // typedef struct {
// // //     BEE1012_EPS_ACU_Unit_t acu[2];
// // // }__attribute__((packed)) BEE1012_EPS_ACU_t;

// // // typedef struct {
// // //     uint16_t bootcount;
// // //     uint16_t bootcause;
// // //     uint16_t resetcause;
// // //     float soc;
// // //     float bat_avr_temp;
// // //     uint16_t vbat;
// // //     float i;
// // //     uint16_t heater_i;
// // // }__attribute__((packed)) BEE1012_EPS_BP8_t;

// // // typedef struct {
// // //     uint16_t gpio_status;
// // //     uint8_t sp_deploy_status;
// // // }__attribute__((packed)) BEE1012_EPS_GPIO_t;

// // // typedef struct {
// // //     BEE1012_EPS_PMU_t pmu;
// // //     BEE1012_EPS_PDU_t pdu;
// // //     BEE1012_EPS_ACU_t acu;
// // //     BEE1012_EPS_BP8_t bp8;
// // //     BEE1012_EPS_GPIO_t gpio;
// // // }__attribute__((packed)) BEE1012_EPS_t;

// // // typedef struct {
// // //     uint8_t power_state;
// // //     uint8_t control_mode;
// // //     float gyro0_calibrated_rate_x;
// // //     float gyro0_calibrated_rate_y;
// // //     float gyro0_calibrated_rate_z;
// // //     uint8_t css[6];
// // // }__attribute__((packed)) BEE1012_ADCS_t;

// // // typedef struct {
// // //     BEE1012_BeaconHeader_t header;
// // //     BEE1012_RPT_t rpt;
// // //     BEE1012_COMS_t coms;
// // //     BEE1012_EPS_t eps;
// // //     BEE1012_ADCS_t adcs;
// // // }__attribute__((packed)) BEE1012_Beacon_t;

// // // static_assert(sizeof(BEE1012_BeaconHeader_t) == 19, "BEE1012_BeaconHeader_t size mismatch");
// // // static_assert(sizeof(BEE1012_RPT_t) == 7, "BEE1012_RPT_t size mismatch");
// // // static_assert(sizeof(BEE1012_AX100_t) == 9, "BEE1012_AX100_t size mismatch");
// // // static_assert(sizeof(BEE1012_LTRX_t) == 5, "BEE1012_LTRX_t size mismatch");
// // // static_assert(sizeof(BEE1012_COMS_t) == 14, "BEE1012_COMS_t size mismatch");
// // // static_assert(sizeof(BEE1012_EPS_PMU_t) == 36, "BEE1012_EPS_PMU_t size mismatch");
// // // static_assert(sizeof(BEE1012_EPS_PDU_t) == 36, "BEE1012_EPS_PDU_t size mismatch");
// // // static_assert(sizeof(BEE1012_EPS_ACU_Unit_t) == 25, "BEE1012_EPS_ACU_Unit_t size mismatch");
// // // static_assert(sizeof(BEE1012_EPS_ACU_t) == 50, "BEE1012_EPS_ACU_t size mismatch");
// // // static_assert(sizeof(BEE1012_EPS_BP8_t) == 22, "BEE1012_EPS_BP8_t size mismatch");
// // // static_assert(sizeof(BEE1012_EPS_GPIO_t) == 3, "BEE1012_EPS_GPIO_t size mismatch");
// // // static_assert(sizeof(BEE1012_EPS_t) == 147, "BEE1012_EPS_t size mismatch");
// // // static_assert(sizeof(BEE1012_ADCS_t) == 20, "BEE1012_ADCS_t size mismatch");
// // // static_assert(sizeof(BEE1012_Beacon_t) == 207, "BEE1012_Beacon_t size mismatch");

// // // typedef struct {
// // //     char call_sign[7];
// // //     uint8_t msg_id[2];
// // //     uint8_t sequence[2];
// // //     uint8_t length[2];
// // //     uint8_t time_code[6];
// // // }__attribute__((packed)) UELYSYS_BeaconHeader_t;

// // // typedef struct {
// // //     uint16_t boot_count;
// // //     uint32_t sequence;
// // //     uint8_t reset_cause;
// // // }__attribute__((packed)) UELYSYS_FSW_t;

// // // typedef struct {
// // //     uint8_t i2c1_0x05_channel_0_status;
// // //     uint8_t i2c1_0x05_channel_1_status;
// // //     uint8_t i2c1_0x06_channel_0_status;
// // //     uint8_t i2c1_0x06_channel_1_status;
// // //     uint8_t ax100_active_conf;
// // //     uint16_t ax100_boot_count;
// // //     uint32_t ax100_boot_cause;
// // //     int16_t ax100_temp_board;
// // // }__attribute__((packed)) UELYSYS_COMS_t;

// // // typedef struct {
// // //     uint32_t bootcause;
// // //     uint16_t resetcause;
// // //     uint16_t bootcount;
// // //     uint8_t out_en[6];
// // //     int16_t temp[2];
// // //     uint8_t batt_mode;
// // //     int16_t batt_i;
// // //     uint16_t batt_v;
// // //     uint8_t sm_en;
// // //     uint16_t gnd_wdt_cnt;
// // //     uint16_t bus_wdt_cnt;
// // //     uint32_t gnd_wdt_left;
// // //     uint32_t bus_wdt_left;
// // // }__attribute__((packed)) UELYSYS_EPS_PMU_t;

// // // typedef struct {
// // //     int16_t out_i[12];
// // //     uint8_t out_en[12];
// // // }__attribute__((packed)) UELYSYS_EPS_PDU_t;

// // // typedef struct {
// // //     int16_t input_i[6];
// // //     uint16_t input_v[6];
// // //     uint8_t mppt_mode;
// // // }__attribute__((packed)) UELYSYS_EPS_ACU_Unit_t;

// // // typedef struct {
// // //     UELYSYS_EPS_ACU_Unit_t acu[2];
// // // }__attribute__((packed)) UELYSYS_EPS_ACU_t;

// // // typedef struct {
// // //     uint16_t bootcount;
// // //     uint16_t bootcause;
// // //     uint16_t resetcause;
// // //     float soc;
// // //     float bat_avr_temp;
// // //     uint16_t vbat;
// // //     float i;
// // //     uint16_t heater_i;
// // // }__attribute__((packed)) UELYSYS_EPS_BP8_t;

// // // typedef struct {
// // //     uint8_t dsp_i2c1_0x07_status;
// // //     uint8_t dsp_i2c1_0x08_status;
// // //     uint8_t dsp_i2c1_0x09_status;
// // //     uint8_t dsp_i2c1_0x10_status;
// // // }__attribute__((packed)) UELYSYS_EPS_DSP_t;

// // // typedef struct {
// // //     UELYSYS_EPS_PMU_t pmu;
// // //     UELYSYS_EPS_PDU_t pdu;
// // //     UELYSYS_EPS_ACU_t acu;
// // //     UELYSYS_EPS_BP8_t bp8;
// // //     UELYSYS_EPS_DSP_t dsp;
// // // }__attribute__((packed)) UELYSYS_EPS_t;

// // // typedef struct {
// // //     uint8_t power_state;
// // //     uint8_t control_mode;
// // //     float gyro0_calibrated_rate_x;
// // //     float gyro0_calibrated_rate_y;
// // //     float gyro0_calibrated_rate_z;
// // //     uint8_t css[6];
// // // }__attribute__((packed)) UELYSYS_ADCS_t;

// // // typedef struct {
// // //     UELYSYS_BeaconHeader_t header;
// // //     UELYSYS_FSW_t fsw;
// // //     UELYSYS_COMS_t coms;
// // //     UELYSYS_EPS_t eps;
// // //     UELYSYS_ADCS_t adcs;
// // // }__attribute__((packed)) UELYSYS_Beacon_t;

// // // static_assert(sizeof(UELYSYS_BeaconHeader_t) == 19, "UELYSYS_BeaconHeader_t size mismatch");
// // // static_assert(sizeof(UELYSYS_FSW_t) == 7, "UELYSYS_FSW_t size mismatch");
// // // static_assert(sizeof(UELYSYS_COMS_t) == 13, "UELYSYS_COMS_t size mismatch");
// // // static_assert(sizeof(UELYSYS_EPS_PMU_t) == 36, "UELYSYS_EPS_PMU_t size mismatch");
// // // static_assert(sizeof(UELYSYS_EPS_PDU_t) == 36, "UELYSYS_EPS_PDU_t size mismatch");
// // // static_assert(sizeof(UELYSYS_EPS_ACU_Unit_t) == 25, "UELYSYS_EPS_ACU_Unit_t size mismatch");
// // // static_assert(sizeof(UELYSYS_EPS_ACU_t) == 50, "UELYSYS_EPS_ACU_t size mismatch");
// // // static_assert(sizeof(UELYSYS_EPS_BP8_t) == 22, "UELYSYS_EPS_BP8_t size mismatch");
// // // static_assert(sizeof(UELYSYS_EPS_DSP_t) == 4, "UELYSYS_EPS_DSP_t size mismatch");
// // // static_assert(sizeof(UELYSYS_EPS_t) == 148, "UELYSYS_EPS_t size mismatch");
// // // static_assert(sizeof(UELYSYS_ADCS_t) == 20, "UELYSYS_ADCS_t size mismatch");
// // // static_assert(sizeof(UELYSYS_Beacon_t) == 207, "UELYSYS_Beacon_t size mismatch");



// // // typedef struct CFE_SRL_HousekeepingTlm_Payload {
// // //     uint8 CommandCounter;

// // //     uint8 CommandErrorCounter;

// // //     uint8 IOHandleStatus[4];

// // //     uint16 IOHandleTxCount[4];
    
// // // }__attribute__((packed)) CFE_SRL_HousekeepingTlm_Payload_t;

// // // typedef struct RPT_HkTlm_Payload{
// // //     uint8 CmdCounter;
// // //     uint8 CmdErrCounter;

// // //     /**
// // //      * Queue Info
// // //      */
// // //     uint8 ReportQueueCnt;
// // //     uint8 CriticalQueueCnt;

// // //     /**
// // //      * Operation Data
// // //      */
// // //     uint16 BootCount;
// // //     uint32 TimeSec;
// // //     uint32 TimeSubsec;
// // //     uint32 Sequence; /* Backup data numbering */

// // //     /**
// // //      * Reset Cause
// // //      */
// // //     uint8 ResetCause;

// // // }__attribute__((packed)) RPT_BcnTlm_Payload_t;

// // // typedef struct PAY_BcnTlm_Payload {
// // //     uint8 CommandCounter;
// // //     uint8 CommandErrorCounter;

// // //     /**
// // //      * Else ....
// // //      */
// // //     /* compact beacon subset */
// // //     int8  sys_status;         /* payload system status */
// // //     int16 temp_ntc_0;
// // //     int16 temp_ntc_1;
// // //     int16 temp_ntc_2;     
// // //     int16 temp_ntc_3;
// // //     int16 temp_ntc_4;         /* board temp 4 */
// // //     int16 temp_ntc_5;         /* board temp 5 */
// // //     int16 temp_ntc_6;         /* board temp 6 */
// // //     int16 temp_ntc_7;         /* board temp 7 */
// // //     int16 temp_ntc_8;         /* board temp 8 */
// // //     int16 temp_ntc_9;         /* board temp 9 */
// // //     int16 temp_ntc_10;        /* board temp 10 */
// // //     int16 temp_ntc_11;        /* board temp 11 */     


// // // } PAY_BcnTlm_Payload_t;

// // // typedef struct PAY_HkTlm_Payload {
// // //     uint8 CommandCounter;
// // //     uint8 CommandErrorCounter;

// // //     int8  sys_status;         /* payload system status */
// // //     int16 temp_ntc_0;         /* board temp 0 */
// // //     int16 temp_ntc_1;         /* board temp 1 */
// // //     int16 temp_ntc_2;         /* board temp 2 */
// // //     int16 temp_ntc_3;         /* board temp 3 */
// // //     int16 temp_ntc_4;         /* board temp 4 */
// // //     int16 temp_ntc_5;         /* board temp 5 */
// // //     int16 temp_ntc_6;         /* board temp 6 */
// // //     int16 temp_ntc_7;         /* board temp 7 */
// // //     int16 temp_ntc_8;         /* board temp 8 */
// // //     int16 temp_ntc_9;         /* board temp 9 */
// // //     int16 temp_ntc_10;        /* board temp 10 */
// // //     int16 temp_ntc_11;        /* board temp 11 */
// // //     /* Expanded TM (selected currents/sensors) */
// // //     uint32 sen1_data_0;
// // //     uint32 sen1_data_1;  

// // // } PAY_HkTlm_LINPayload_t;

// // // // BEE-1000 Mission Beacon
// // // typedef struct {

// // // 	// Telemetry header
// // //     uint8 CCSDS_MID[2];
// // //     uint8 CCSDS_Seq[2];
// // //     uint8 CCSDS_Len[2];
// // //     uint8 CCSDS_TimeCode[6];


// // //     CFE_SRL_HousekeepingTlm_Payload_t srlpayload;

// // //     RPT_BcnTlm_Payload_t    rptpayload;

// // //     // Payload
// // //     PAY_BcnTlm_Payload_t    paybcnpayload;



// // //     PAY_HkTlm_LINPayload_t     payhkpayload;
    

// // // }__attribute__((packed)) MissionBeacon;

// // // #define a sizeof(MissionBeacon);


// // // struct GETFILEINFO {
// // //     // ===== TLM Header =====
// // //     uint8 CCSDS_MID[2];
// // //     uint8 CCSDS_Seq[2];
// // //     uint8 CCSDS_Len[2];
// // //     uint8 CCSDS_TimeCode[6];
// // //     uint8 padding[4];

// // //     // ===== Payload =====
// // //     uint8 FileStatus;
// // //     uint8 CRC_Computed;
// // //     uint8 Spare[2];
// // //     uint32 CRC;
// // //     uint32 FileSize;
// // //     uint32 LastModifiedTime;
// // //     uint32 Mode;
// // //     char Filename[64];

// // // };

// // // #define a sizeof(GETFILEINFO);


// // // struct Report {
// // //     // ===== CCSDS Header =====v  16
// // //     uint16 CCSDS_MsgId;
// // //     uint16 CCSDS_Seq;
// // //     uint16 CCSDS_Len;
// // //     uint8 CCSDS_TimeCode[6];
// // //     uint32 CCSDS_Padding;


// // //     // ===== Report Body =====  10     26 byte
// // //     uint16 ReflectedMID;
// // //     uint8  ReflectedCC;
// // //     uint8  RetType;
// // //     int32  RetCode;
// // //     uint16 RetValSize;
// // //     uint8  RetVal[512];

// // // };

// // // #define a sizeof(Report)

// // // struct Event {

// // //     uint16 CCSDS_MsgId;
// // //     uint16 CCSDS_Seq;
// // //     uint16 CCSDS_Len;
// // //     uint8 CCSDS_TimeCode[6];
// // //     uint32 CCSDS_Padding;

// // //     char AppName[20]; /**< 20임   \cfetlmmnemonic \EVS_APPNAME
// // //                                                 \brief Application name */
// // //     uint16 EventID;                        /**< \cfetlmmnemonic \EVS_EVENTID
// // //                                                 \brief Numerical event identifier */
// // //     uint16 EventType;    /**< uint16임 \cfetlmmnemonic \EVS_EVENTTYPE  
// // //                                                 \brief Numerical event type identifier */
// // //     uint32 SpacecraftID;                   /**< \cfetlmmnemonic \EVS_SCID
// // //                                                 \brief Spacecraft identifier */
// // //     uint32 ProcessorID;                    /**< \cfetlmmnemonic \EVS_PROCESSORID
// // //                                                 \brief Numerical processor identifier */


// // //     char               Message[122]; /**< 122임 \cfetlmmnemonic \EVS_EVENT
// // //                                                                  \brief Event message string */
// // //     uint8 Spare1;                                                   /**< \cfetlmmnemonic \EVS_SPARE1
// // //                                                                          \brief Structure padding */
// // //     uint8 Spare2;                                                   /**< \cfetlmmnemonic \EVS_SPARE2
// // //                                                                      \brief Structure padding */

// // // };

// // // #define a sizeof(Event)

// // // typedef enum
// // // {
// // //     REPORT_KIND_NONE = 0,

// // //     // ADCS Sunpointing RPT
// // //     REPORT_KIND_ADCS_LOG_MASK,
// // //     REPORT_KIND_ADCS_UNSOLICIT_TLM_SETUP_TLM,

// // //     REPORT_KIND_UANT_GET_STATUS_TLM,

// // // 	    REPORT_KIND_EPS_P60_DOCK_GET_TABLE_HK,
// // // 	    REPORT_KIND_EPS_P60_PDU_GET_TABLE_HK,
// // // 	    REPORT_KIND_EPS_P60_ACU_GET_TABLE_HK,
// // // 	    REPORT_KIND_EPS_QUERY_REPORT,
// // // 	    REPORT_KIND_EPS_POWER_IF_STATUS,
// // // 	    REPORT_KIND_EPS_POWER_IF_LIST,
// // // 	    REPORT_KIND_EPS_P80_PMU_HK,
// // // 	    REPORT_KIND_EPS_P80_PDU_HK,
// // // 	    REPORT_KIND_EPS_P80_ACU1_HK,
// // // 	    REPORT_KIND_EPS_P80_ACU2_HK,
// // // 	    REPORT_KIND_EPS_BP8_HK,

// // // /****************************************************************************************** */
// // //     REPORT_KIND_ADCS_GET_ERROR_LOG_SETTING,
// // //     REPORT_KIND_ADCS_GET_CURRENT_UNIX_TIME,
// // //     REPORT_KIND_ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC,
// // //     REPORT_KIND_ADCS_GET_COMMUNICATION_STATUS,
// // //     REPORT_KIND_ADCS_GET_CONTROL_ESTIMATION_MODE,
// // //     REPORT_KIND_ADCS_GET_REFERENCE_IRC_VECTOR,
// // //     REPORT_KIND_ADCS_GET_REFERENCE_LLH_TARGET,
// // //     REPORT_KIND_ADCS_GET_ORBIT_MODE,
// // //     REPORT_KIND_ADCS_GET_HEALTH_TLM_MMT,
// // //     REPORT_KIND_ADCS_GET_RAW_CUBESENSE_SUN,
// // //     REPORT_KIND_ADCS_GET_REFERENCE_RPY_VALUES,
// // //     REPORT_KIND_ADCS_GET_OPENLOOPCMD_MTQ,
// // //     REPORT_KIND_ADCS_GET_POWER_STATE,
// // //     REPORT_KIND_ADCS_GET_RUN_MODE,
// // //     REPORT_KIND_ADCS_GET_CONTROL_MODE,
// // //     REPORT_KIND_ADCS_GET_MAG0_MMT_CALIB_CONFIG,
// // //     REPORT_KIND_ADCS_GET_MAG1_MMT_CALIB_CONFIG,
// // //     REPORT_KIND_ADCS_GET_ESTIMATION_MODE,
// // //     REPORT_KIND_ADCS_GET_OPERATIONAL_STATE,
// // //     REPORT_KIND_ADCS_GET_RAW_CSS_SENSOR,
// // //     REPORT_KIND_ADCS_GET_RAW_GYR_SENSOR,
// // //     REPORT_KIND_ADCS_GET_CALIBRATED_GYR_SENSOR,
// // //     REPORT_KIND_ADCS_GET_MAG_SENSING_ELM_CONFIG,
// // //     REPORT_KIND_ADCS_GET_TLM_LOG_INCLMASK,
// // //     REPORT_KIND_ADCS_GET_UNSOLICIT_TLM_MSG_SETUP,
// // //     REPORT_KIND_ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP,
// // //     REPORT_KIND_ADCS_GET_EVENT_LOG_STATUS_RESPONSE,
// // //     REPORT_KIND_ADCS_GET_PORTMAP,
// // // /********************************************************************************************* */

// // // /****************************************5차 추가*************************************** */
// // //     // 1. PAYUEL_ROMA
// // //     REPORT_KIND_PAYUEL_ROMA_NOOP,            // CC 0, 1
// // //     REPORT_KIND_PAYUEL_ROMA_RESETCOUNTERS,
// // //     REPORT_KIND_PAYUEL_ROMA_COMMTEST,
// // //     REPORT_KIND_PAYUEL_ROMA_GETSPECIFICLINE,
// // //     REPORT_KIND_PAYUEL_ROMA_GETMULTIPLELINES,
// // //     REPORT_KIND_PAYUEL_ROMA_GETLATESTLINE,
// // //     REPORT_KIND_PAYUEL_ROMA_GETLATESTNLINES,
// // //     REPORT_KIND_PAYUEL_ROMA_SETROUTEDEFAULT,
// // //     REPORT_KIND_PAYUEL_ROMA_RESETROUTE,
// // //     REPORT_KIND_PAYUEL_ROMA_LOADROUTE,
// // //     REPORT_KIND_PAYUEL_ROMA_SAVEROUTE,
// // //     REPORT_KIND_PAYUEL_ROMA_SENDROUTE,
// // //     REPORT_KIND_PAYUEL_ROMA_SETROUTE,
// // //     REPORT_KIND_PAYUEL_ROMA_PARGET,
// // //     REPORT_KIND_PAYUEL_ROMA_PARSET,
// // //     REPORT_KIND_PAYUEL_ROMA_PARDEFAULTS,
// // //     REPORT_KIND_PAYUEL_ROMA_PARSAVE,
// // //     REPORT_KIND_PAYUEL_ROMA_PARRESTORE,
// // //     REPORT_KIND_PAYUEL_ROMA_PARLOAD,
// // //     REPORT_KIND_PAYUEL_ROMA_PARSETOOB,
// // //     REPORT_KIND_PAYUEL_ROMA_SENDCOMMAND,


// // //     // 2. PAYUEL_LGPM
// // //     REPORT_KIND_PAYUEL_LGPM_NOOP,               // CC 0
// // //     REPORT_KIND_PAYUEL_LGPM_RESETCOUNTERS,
// // //     REPORT_KIND_PAYUEL_LGPM_MCU_ALIVE,          // CC 2
// // //     REPORT_KIND_PAYUEL_LGPM_3V3_PWR_ON,         // CC 3
// // //     REPORT_KIND_PAYUEL_LGPM_3V3_PWR_OFF,        // CC 4
// // //     REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_ON,      // CC 5
// // //     REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_OFF,     // CC 6
// // //     REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_ON,       // CC 7
// // //     REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_OFF,      // CC 8
// // //     REPORT_KIND_PAYUEL_LGPM_V28_MAIN_ON,        // CC 9
// // //     REPORT_KIND_PAYUEL_LGPM_V28_MAIN_OFF,       // CC 10
// // //     REPORT_KIND_PAYUEL_LGPM_V28_SUB_ON,         // CC 11
// // //     REPORT_KIND_PAYUEL_LGPM_V28_SUB_OFF,        // CC 12
// // //     REPORT_KIND_PAYUEL_LGPM_V12_MAIN_ON,        // CC 13
// // //     REPORT_KIND_PAYUEL_LGPM_V12_MAIN_OFF,       // CC 14
// // //     REPORT_KIND_PAYUEL_LGPM_PWR_SENSE_INFO,          // CC 15
// // //     REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_ON,         // CC 16
// // //     REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_OFF,        // CC 17
// // //     REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX1,           // CC 18
// // //     REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX2,           // CC 19
// // //     REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX3,           // CC 20
// // //     REPORT_KIND_PAYUEL_LGPM_RWA_PWR_ON,         // CC 21
// // //     REPORT_KIND_PAYUEL_LGPM_RWA_PWR_OFF,        // CC 22
// // //     REPORT_KIND_PAYUEL_LGPM_RWA_SENSE_INFO,          // CC 23
// // // /****************************************************************************************** */

// // //     REPORT_KIND_SC_GENERIC,

// // // } ReportKind_t;

// // // static ReportKind_t DetermineReportKind(uint16_t reflected_mid, uint8_t reflected_cc) {
// // //     if (reflected_mid == UANT_APP_CMD_ID && reflected_cc == UANT_APP_GET_STATUS_CC) return REPORT_KIND_UANT_GET_STATUS_TLM;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518)  && reflected_cc == ADCS_GET_TLM_LOG_INCLMASK_CC) return REPORT_KIND_ADCS_LOG_MASK;
    
// // // /**************************************************************************************************************************************************************************************** */
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_ERROR_LOG_SETTING_CC) return REPORT_KIND_ADCS_GET_ERROR_LOG_SETTING;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CURRENT_UNIX_TIME_CC) return REPORT_KIND_ADCS_GET_CURRENT_UNIX_TIME;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC_CC) return REPORT_KIND_ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_COMMUNICATION_STATUS_CC) return REPORT_KIND_ADCS_GET_COMMUNICATION_STATUS;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CONTROL_ESTIMATION_MODE_CC) return REPORT_KIND_ADCS_GET_CONTROL_ESTIMATION_MODE;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_REFERENCE_IRC_VECTOR_CC) return REPORT_KIND_ADCS_GET_REFERENCE_IRC_VECTOR;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_REFERENCE_LLH_TARGET_CC) return REPORT_KIND_ADCS_GET_REFERENCE_LLH_TARGET;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_ORBIT_MODE_CC) return REPORT_KIND_ADCS_GET_ORBIT_MODE;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_HEALTH_TLM_MMT_CC) return REPORT_KIND_ADCS_GET_HEALTH_TLM_MMT;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RAW_CUBESENSE_SUN_CC) return REPORT_KIND_ADCS_GET_RAW_CUBESENSE_SUN;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_REFERENCE_RPY_VALUES_CC) return REPORT_KIND_ADCS_GET_REFERENCE_RPY_VALUES;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_OPENLOOPCMD_MTQ_CC) return REPORT_KIND_ADCS_GET_OPENLOOPCMD_MTQ;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_POWER_STATE_CC) return REPORT_KIND_ADCS_GET_POWER_STATE;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RUN_MODE_CC) return REPORT_KIND_ADCS_GET_RUN_MODE;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CONTROL_MODE_CC) return REPORT_KIND_ADCS_GET_CONTROL_MODE;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_MAG0_MMT_CALIB_CONFIG_CC) return REPORT_KIND_ADCS_GET_MAG0_MMT_CALIB_CONFIG;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_MAG1_MMT_CALIB_CONFIG_CC) return REPORT_KIND_ADCS_GET_MAG1_MMT_CALIB_CONFIG;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_ESTIMATION_MODE_CC) return REPORT_KIND_ADCS_GET_ESTIMATION_MODE;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_OPERATIONAL_STATE_CC) return REPORT_KIND_ADCS_GET_OPERATIONAL_STATE;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RAW_CSS_SENSOR_CC) return REPORT_KIND_ADCS_GET_RAW_CSS_SENSOR;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RAW_GYR_SENSOR_CC) return REPORT_KIND_ADCS_GET_RAW_GYR_SENSOR;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CALIBRATED_GYR_SENSOR_CC) return REPORT_KIND_ADCS_GET_CALIBRATED_GYR_SENSOR;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_MAG_SENSING_ELM_CONFIG_CC) return REPORT_KIND_ADCS_GET_MAG_SENSING_ELM_CONFIG;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_TLM_LOG_INCLMASK_CC) return REPORT_KIND_ADCS_GET_TLM_LOG_INCLMASK;
// // //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_UNSOLICIT_TLM_MSG_SETUP_CC) return REPORT_KIND_ADCS_GET_UNSOLICIT_TLM_MSG_SETUP;
// // // 	    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP_CC) return REPORT_KIND_ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP;
// // // 	    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_EVENT_LOG_STATUS_RESPONSE_CC) return REPORT_KIND_ADCS_GET_EVENT_LOG_STATUS_RESPONSE;
// // // 	    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_PORTMAP_CC) return REPORT_KIND_ADCS_GET_PORTMAP;
// // // 	/****************************************************************************************************************************************************************************************** */

// // // 	    if ((reflected_mid == EPS_CMD_ID || reflected_mid == 0x7518) &&
// // // 	        (reflected_cc == EPS_P80_POWER_IF_GET_CC ||
// // // 	         reflected_cc == EPS_P80_POWER_IF_LIST_CC ||
// // // 	         reflected_cc == EPS_GET_HK_ALL_CC)) return REPORT_KIND_EPS_QUERY_REPORT;

// // // 	    /*********************************************5차 추가****************************************************** */
// // //     // CC 0, 1
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_NOOP_CC)) return REPORT_KIND_PAYUEL_ROMA_NOOP;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_RESET_COUNTERS_CC)) return REPORT_KIND_PAYUEL_ROMA_RESETCOUNTERS;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_COMM_TEST_CC)) return REPORT_KIND_PAYUEL_ROMA_COMMTEST;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_SPECIFIC_LINE_CC)) return REPORT_KIND_PAYUEL_ROMA_GETSPECIFICLINE;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_MULTIPLE_LINES_CC)) return REPORT_KIND_PAYUEL_ROMA_GETMULTIPLELINES;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_LATEST_LINE_CC)) return REPORT_KIND_PAYUEL_ROMA_GETLATESTLINE;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_LATEST_N_LINES_CC)) return REPORT_KIND_PAYUEL_ROMA_GETLATESTNLINES;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SET_ROUTE_DEFAULT_CC)) return REPORT_KIND_PAYUEL_ROMA_SETROUTEDEFAULT;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_RESET_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_RESETROUTE;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_LOAD_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_LOADROUTE;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SAVE_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_SAVEROUTE;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SEND_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_SENDROUTE;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SET_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_SETROUTE;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_GET_CC)) return REPORT_KIND_PAYUEL_ROMA_PARGET;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_SET_CC)) return REPORT_KIND_PAYUEL_ROMA_PARSET;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_DEFAULTS_CC)) return REPORT_KIND_PAYUEL_ROMA_PARDEFAULTS;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_SAVE_CC)) return REPORT_KIND_PAYUEL_ROMA_PARSAVE;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_RESTORE_CC)) return REPORT_KIND_PAYUEL_ROMA_PARRESTORE;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_LOAD_CC)) return REPORT_KIND_PAYUEL_ROMA_PARLOAD;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_SET_OOB_CC)) return REPORT_KIND_PAYUEL_ROMA_PARSETOOB;
// // //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SEND_COMMAND_CC)) return REPORT_KIND_PAYUEL_ROMA_SENDCOMMAND;

// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_NOOP_CC)) return REPORT_KIND_PAYUEL_LGPM_NOOP;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RESET_COUNTERS_CC)) return REPORT_KIND_PAYUEL_LGPM_RESETCOUNTERS;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_MCU_ALIVE_CHECK_CC)) return REPORT_KIND_PAYUEL_LGPM_MCU_ALIVE;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_3V3_PWR_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_3V3_PWR_ON;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_3V3_PWR_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_3V3_PWR_OFF;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_MAIN_BOOST_SW_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_ON;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_MAIN_BOOST_SW_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_OFF;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_SUB_BOOST_SW_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_ON;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_SUB_BOOST_SW_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_OFF;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_MAIN_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_MAIN_ON;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_MAIN_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_MAIN_OFF;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_SUB_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_SUB_ON;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_SUB_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_SUB_OFF;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V12_MAIN_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_V12_MAIN_ON;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V12_MAIN_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_V12_MAIN_OFF;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_PWR_SENSE_INFO_CC)) return REPORT_KIND_PAYUEL_LGPM_PWR_SENSE_INFO;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_PWR_SEQ_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_ON;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_PWR_SEQ_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_OFF;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_CONTROL_idx1_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX1;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_CONTROL_idx2_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX2;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_CONTROL_idx3_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX3;
// // //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_PWR_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_PWR_ON;
// // // 	    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_PWR_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_PWR_OFF;
// // // 	    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_SENSE_INFO_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_SENSE_INFO;
// // // 	    /**************************************************************************************************************************/
// // // 	    return REPORT_KIND_SC_GENERIC;
// // // 	}

// // // typedef struct
// // // {
// // //     uint8_t bytes[512];
// // // } RptGenericPayload_t;

// // // static_assert(sizeof(EPS_Query_Report_Payload_t) == 512, "EPS_Query_Report_Payload_t size mismatch");

// // // typedef struct __attribute__((packed)) {
// // //     uint8_t  ch_idx;
// // //     uint8_t  mode;
// // //     uint16_t on_cnt;
// // //     uint16_t off_cnt;
// // //     uint16_t cur_lu_lim;
// // //     uint16_t cur_lim;
// // //     uint16_t voltage;
// // //     int16_t  current;
// // //     uint16_t latchup;
// // //     char     name[8];
// // // } EPS_PowerIfStatus_Report_t;

// // // typedef struct __attribute__((packed)) {
// // //     uint8_t ch_idx;
// // //     uint8_t mode;
// // //     char    name[8];
// // // } EPS_PowerIfListItem_Report_t;

// // // typedef struct __attribute__((packed)) {
// // //     uint8_t cmd;
// // //     uint8_t status;
// // //     uint8_t count;
// // //     EPS_PowerIfListItem_Report_t list[24];
// // // } EPS_PowerIfList_Report_t;

// // // typedef struct __attribute__((packed)) {
// // //     uint32_t uptime;
// // //     uint32_t bootcause;
// // //     uint16_t resetcause;
// // //     uint16_t bootcount;
// // //     uint16_t batt_v;
// // //     int16_t  batt_i;
// // //     uint8_t  batt_mode;
// // //     uint8_t  spare1;
// // //     uint16_t vbat_v;
// // //     uint16_t vcc_v;
// // //     int16_t  temp[2];
// // //     uint8_t  out_en[6];
// // //     int16_t  out_i[6];
// // //     uint8_t  sm_en[8];
// // //     uint16_t gnd_wdt_cnt;
// // //     uint16_t bus_wdt_cnt;
// // //     uint32_t gnd_wdt_left;
// // //     uint32_t bus_wdt_left;
// // // } EPS_P80_PMU_HK_Report_t;

// // // typedef struct __attribute__((packed)) {
// // //     uint32_t uptime;
// // //     uint32_t bootcause;
// // //     uint32_t bootcount;
// // //     uint16_t resetcause;
// // //     uint16_t vcc_v;
// // //     uint16_t vcc_i;
// // //     uint16_t vbat_v;
// // //     int16_t  temp;
// // //     uint8_t  batt_mode;
// // //     uint8_t  out_en[24];
// // //     uint8_t  spare1;
// // //     int16_t  out_i[24];
// // //     uint32_t gnd_wdt_cnt;
// // //     uint32_t bus_wdt_cnt;
// // //     uint32_t gnd_wdt_left;
// // //     uint32_t bus_wdt_left;
// // // } EPS_P80_PDU_HK_Report_t;

// // // typedef struct __attribute__((packed)) {
// // //     uint32_t uptime;
// // //     uint32_t bootcause;
// // //     uint32_t bootcount;
// // //     uint16_t resetcause;
// // //     int16_t  input_i[6];
// // //     uint16_t input_v[6];
// // //     uint16_t vcc_v;
// // //     uint16_t vbat_v;
// // //     int16_t  temp[3];
// // //     uint8_t  mppt_mode;
// // //     uint8_t  spare1;
// // //     uint32_t gnd_wdt_cnt;
// // //     uint32_t gnd_wdt_left;
// // // } EPS_P80_ACU_HK_Report_t;

// // // typedef struct __attribute__((packed)) {
// // //     uint32_t Uptime;
// // //     uint16_t BootCount;
// // //     uint16_t BootCause;
// // //     uint16_t ResetCause;
// // //     uint16_t Vbat;
// // //     float    Soc;
// // //     float    Current;
// // //     uint16_t InCurrent;
// // //     uint16_t OutCurrent;
// // //     uint16_t HeaterCurrent;
// // //     int16_t  IntTemp;
// // //     float    BatAvrTemp;
// // //     int16_t  BatTemp[4];
// // //     uint16_t OVoltCount;
// // //     uint8_t  BatFault;
// // //     uint8_t  spare2;
// // // } EPS_BP8_HK_Report_t;

// // // static_assert(sizeof(EPS_PowerIfStatus_Report_t) == 26, "EPS_PowerIfStatus_Report_t size mismatch");
// // // static_assert(sizeof(EPS_PowerIfListItem_Report_t) == 10, "EPS_PowerIfListItem_Report_t size mismatch");
// // // static_assert(sizeof(EPS_PowerIfList_Report_t) == 243, "EPS_PowerIfList_Report_t size mismatch");
// // // static_assert(sizeof(EPS_P80_PMU_HK_Report_t) == 64, "EPS_P80_PMU_HK_Report_t size mismatch");
// // // static_assert(sizeof(EPS_P80_PDU_HK_Report_t) == 112, "EPS_P80_PDU_HK_Report_t size mismatch");
// // // static_assert(sizeof(EPS_P80_ACU_HK_Report_t) == 58, "EPS_P80_ACU_HK_Report_t size mismatch");
// // // static_assert(sizeof(EPS_BP8_HK_Report_t) == 44, "EPS_BP8_HK_Report_t size mismatch");

// // // typedef struct {
// // //     bool     valid;
// // //     uint8_t  command_code;
// // //     int32_t  return_code;
// // //     uint16_t return_data_size;
// // //     uint8_t  source;
// // //     uint8_t  data_id;
// // //     uint8_t  arg0;
// // //     uint8_t  arg1;
// // //     uint16_t sequence;
// // //     uint16_t offset;
// // //     uint16_t total_size;
// // //     uint16_t chunk_size;
// // // } EPS_Report_Metadata_t;

// // // typedef struct {
// // //     bool power_if_status_valid;
// // //     bool power_if_list_valid;
// // //     bool pmu_hk_valid;
// // //     bool pdu_hk_valid;
// // //     bool acu1_hk_valid;
// // //     bool acu2_hk_valid;
// // //     bool bp8_hk_valid;

// // //     EPS_Report_Metadata_t power_if_status_meta;
// // //     EPS_Report_Metadata_t power_if_list_meta;
// // //     EPS_Report_Metadata_t pmu_hk_meta;
// // //     EPS_Report_Metadata_t pdu_hk_meta;
// // //     EPS_Report_Metadata_t acu1_hk_meta;
// // //     EPS_Report_Metadata_t acu2_hk_meta;
// // //     EPS_Report_Metadata_t bp8_hk_meta;

// // //     EPS_PowerIfStatus_Report_t power_if_status;
// // //     EPS_PowerIfList_Report_t   power_if_list;
// // //     EPS_P80_PMU_HK_Report_t    pmu_hk;
// // //     EPS_P80_PDU_HK_Report_t    pdu_hk;
// // //     EPS_P80_ACU_HK_Report_t    acu1_hk;
// // //     EPS_P80_ACU_HK_Report_t    acu2_hk;
// // //     EPS_BP8_HK_Report_t        bp8_hk;
// // // } EPS_Report_State_t;



// // // typedef struct __attribute__((__packed__)) gs_gssb_ant6_release_status_t {
// // //     /**
// // //        Burn state of the first channel (Burning = 1, Idle = 0)
// // //      */
// // //     uint8_t channel_0_state;
// // //     /**
// // //        Release status of the first channel (Released = 1, Not released = 0)
// // //      */
// // //     uint8_t channel_0_status;
// // //     /**
// // //        Burn time left of the first channel [s]
// // //      */
// // //     uint8_t channel_0_burn_time_left;
// // //     /**
// // //        Counter of have many burns there has been attempted
// // //      */
// // //     uint8_t channel_0_burn_tries;
// // //     /**
// // //        Burn state of the second channel (Burning = 1, Idle = 0)
// // //      */
// // //     uint8_t channel_1_state;
// // //     /**
// // //        Release status of the second channel (Released = 1, Not released = 0)
// // //      */
// // //     uint8_t channel_1_status;
// // //     /**
// // //        Burn time left of the second channel [s]
// // //      */
// // //     uint8_t channel_1_burn_time_left;
// // //     /**
// // //        Counter of have many burns there has been attempted
// // //      */
// // //     uint8_t channel_1_burn_tries;
// // // } gs_gssb_ant6_release_status_t;


// // // typedef struct EPS_P60_DOCK_GET_TABLE_HK {

// // //     int16_t   c_out[13];
// // //     uint16_t  v_out[13];
// // //     uint8_t   out_en[13];

// // //     int16_t   temp[2];

// // //     uint32_t  bootcause;
// // //     uint32_t  bootcnt;
// // //     uint32_t  uptime;

// // //     uint16_t  resetcause;

// // //     uint8_t   batt_mode;
// // //     uint8_t   heater_on;
// // //     uint8_t   conv_5v_en;

// // //     uint16_t  latchup[13];

// // //     uint16_t  vbat_v;
// // //     int16_t   vcc_c;
// // //     int16_t   batt_c;
// // //     uint16_t  batt_v;

// // //     int16_t   batt_temp[2];

// // //     uint8_t   device_type[8];
// // //     uint8_t   device_status[8];

// // //     uint8_t   dearm_status;

// // //     uint32_t  wdt_cnt_gnd;
// // //     uint32_t  wdt_cnt_i2c;
// // //     uint32_t  wdt_cnt_can;
// // //     uint32_t  wdt_cnt_csp[2];

// // //     uint32_t  wdt_gnd_left;
// // //     uint32_t  wdt_i2c_left;
// // //     uint32_t  wdt_can_left;

// // //     uint8_t   wdt_csp_left[2];

// // //     int16_t   batt_chrg;
// // //     int16_t   batt_dischrg;

// // //     int8_t    ant6_depl;
// // //     int8_t    ar6_depl;

// // // } EPS_P60_DOCK_GET_TABLE_HK;

// // // typedef struct EPS_P60_PDU_GET_TABLE_HK {

// // //     int16_t   c_out[9];
// // //     uint16_t  v_out[9];

// // //     uint16_t  vcc;
// // //     uint16_t  vbat;
// // //     int16_t   temp;

// // //     uint8_t   conv_en[3];
// // //     uint8_t   out_en[9];

// // //     uint32_t  bootcause;
// // //     uint32_t  bootcnt;
// // //     uint32_t  uptime;

// // //     uint16_t  resetcause;

// // //     uint8_t   batt_mode;

// // //     uint16_t  latchup[9];

// // //     uint8_t   device_type[8];
// // //     uint8_t   device_status[8];

// // //     uint32_t  wdt_cnt_gnd;
// // //     uint32_t  wdt_cnt_i2c;
// // //     uint32_t  wdt_cnt_can;
// // //     uint32_t  wdt_cnt_csp[2];

// // //     uint32_t  wdt_gnd_left;
// // //     uint32_t  wdt_i2c_left;
// // //     uint32_t  wdt_can_left;

// // //     uint8_t   wdt_csp_left[2];

// // // } EPS_P60_PDU_GET_TABLE_HK;


// // // typedef struct EPS_P60_ACU_GET_TABLE_HK {

// // //     int16_t   c_in[6];
// // //     uint16_t  v_in[6];

// // //     uint16_t  vbat;
// // //     uint16_t  vcc;

// // //     int16_t   temp[3];

// // //     uint8_t   mppt_mode;

// // //     uint16_t  vboost[6];
// // //     uint16_t  power[6];

// // //     uint8_t   dac_en[3];
// // //     uint16_t  dac_val[6];

// // //     uint32_t  bootcause;
// // //     uint32_t  bootcnt;
// // //     uint32_t  uptime;

// // //     uint16_t  resetcause;

// // //     uint16_t  mppt_time;
// // //     uint16_t  mppt_period;

// // //     uint8_t   device_type[8];
// // //     uint8_t   device_status[8];

// // //     uint32_t  wdt_cnt_gnd;
// // //     uint32_t  wdt_gnd_left;

// // // } EPS_P60_ACU_GET_TABLE_HK;



// // // typedef struct
// // // {
// // //     bool     valid;
// // //     uint16_t CCMessage_ID;
// // //     uint16_t CCCount;
// // //     uint16_t CCLength;
// // //     uint8_t  CCTime_code[6];

// // //     uint16_t reflected_msg_id;
// // //     uint8_t  reflected_cc;
// // //     uint8_t  ret_type;
// // //     int32_t  ret_code;
// // // 	    uint16_t ret_val_size;

// // // 	    ReportKind_t kind;
// // // 	    EPS_Report_Metadata_t eps_meta;

// // // 	    union
// // // 	    {
// // //         RptGenericPayload_t    generic;


// // //         ADCS_TlmLogInclMaskTlm_Payload_t       adcs_logmask;
// // //         ADCS_UnsolicitTlmMsgSetupTlm_Payload_t adcs_unsolicited_tlm_tlm;

// // // 	        gs_gssb_ant6_release_status_t          uant_getstatus;
// // // 	        EPS_P60_DOCK_GET_TABLE_HK              eps_p60dockgettablehk;
// // // 	        EPS_P60_PDU_GET_TABLE_HK               eps_p60pdugettablehk;
// // // 	        EPS_P60_ACU_GET_TABLE_HK               eps_p60acugettablehk;
// // // 	        EPS_PowerIfStatus_Report_t             eps_power_if_status;
// // // 	        EPS_PowerIfList_Report_t               eps_power_if_list;
// // // 	        EPS_P80_PMU_HK_Report_t                eps_p80_pmu_hk;
// // // 	        EPS_P80_PDU_HK_Report_t                eps_p80_pdu_hk;
// // // 	        EPS_P80_ACU_HK_Report_t                eps_p80_acu_hk;
// // // 	        EPS_BP8_HK_Report_t                    eps_bp8_hk;
// // // /************************************************************************************************************************************************************* */
// // //         ADCS_ErrorLogSettingTlm_Payload_t      adcs_errorlogsetting;
// // //         ADCS_CurrentUnixTimeTlm_Payload_t      adcs_currentunixtime;
// // //         ADCS_PersistConfigDiagnosticTlm_Payload_t adcs_persistconfigdiagnostic;
// // //         ADCS_CommunicationStatusTlm_Payload_t  adcs_communicationstatus;
// // //         ADCS_ControlEstimationModeTlm_Payload_t adcs_controlestimationmode;
// // //         ADCS_ReferenceIRCVectorTlm_Payload_t    adcs_referenceircvector;
// // //         ADCS_ReferenceLLHTargetTlm_Payload_t    adcs_referencellhtarget;
// // //         ADCS_OrbitModeTlm_Payload_t             adcs_orbitmode;
// // //         ADCS_HealthTlmMMTTlm_Payload_t          adcs_healthtlmmmt;
// // //         ADCS_RawCubeSenseSunTlm_Payload_t       adcs_rawcubesensesun;
// // //         ADCS_ReferenceRPYvaluesTlm_Payload_t    adcs_referencerpyvalues;
// // //         ADCS_OpenLoopCmdMTQTlm_Payload_t        adcs_openloopcmdmtq;
// // //         ADCS_PowerStateTlm_Payload_t            adcs_powerstate;
// // //         ADCS_RunModeTlm_Payload_t               adcs_runmode;
// // //         ADCS_ControlModeTlm_Payload_t           adcs_controlmode;
// // //         ADCS_Mag0MMTCalibConfigTlm_Payload_t    adcs_mag0mmtcalibconfig;
// // //         ADCS_Mag1MMTCalibConfigTlm_Payload_t    adcs_mag1mmtcalibconfig;
// // //         ADCS_EstimationModeTlm_Payload_t        adcs_estimationmode;
// // //         ADCS_OperationalStateTlm_Payload_t      adcs_operationalstate;
// // //         ADCS_RawCSSSensorTlm_Payload_t          adcs_rawcsssensor;
// // //         ADCS_RawGYRSensorTlm_Paylaod_t          adcs_rawgyrsensor;
// // //         ADCS_CalibratedGYRSensorTlm_Payload_t   adcs_calibratedgyrsensor;
// // //         ADCS_MagSensingElmConfigTlm_Payload_t   adcs_magsensingelmconfig;
// // //         ADCS_TlmLogInclMaskTlm_Payload_t        adcs_tlmloginclmask;
// // //         ADCS_UnsolicitTlmMsgSetupTlm_Payload_t  adcs_unsolicittlmmsgsetup;
// // //         ADCS_UnsolicitEventMsgSetupTlm_Payload_t adcs_unsoliciteventmsgsetup;
// // //         ADCS_EventLogStatusResponseTlm_Payload_t adcs_eventlogstatusresponse;
// // //         ADCS_PortMapTlm_Payload_t               adcs_portmap;
// // // /******************************************************************************************************************************************************************** */

// // // /**********************************************5차 추가********************************************** */
// // //         // 1. PAYUEL_ROMA
// // //         payuel_roma_Noop_tlm_payload_t                     roma_noop;;
// // //         payuel_roma_ResetCounters_tlm_payload_t            roma_resetcounters;
// // //         payuel_roma_CommTest_tlm_payload_t                 roma_commtest;
// // //         payuel_roma_GetSpecificLine_tlm_payload_t          roma_getspecificline;
// // //         payuel_roma_GetMultipleLines_tlm_payload_t         roma_getmultiplelines;
// // //         payuel_roma_GetLatestLine_tlm_payload_t            roma_getlatestline;
// // //         payuel_roma_GetLatest_N_Lines_tlm_payload_t        roma_getlatestNlines;
// // //         payuel_roma_SetRouteDefault_tlm_payload_t          roma_setroutedefault;
// // //         payuel_roma_ResetRoute_tlm_payload_t               roma_resetroute;
// // //         payuel_roma_LoadRoute_tlm_payload_t                roma_loadroute;
// // //         payuel_roma_SaveRoute_tlm_payload_t                roma_saveroute;
// // //         payuel_roma_SendRoute_tlm_payload_t                roma_sendroute;
// // //         payuel_roma_SetRoute_tlm_payload_t                 roma_setroute;
// // //         payuel_roma_ParGet_tlm_payload_t                   roma_parget;
// // //         payuel_roma_ParSet_tlm_payload_t                   roma_parset;
// // //         payuel_roma_ParDefaults_tlm_payload_t              roma_pardefaults;
// // //         payuel_roma_ParSave_tlm_payload_t                  roma_parsave;
// // //         payuel_roma_ParRestore_tlm_payload_t               roma_parrestore;
// // //         payuel_roma_ParLoad_tlm_payload_t                  roma_parload;
// // //         payuel_roma_ParSetOOB_tlm_payload_t                roma_parsetOOB;
// // //         payuel_roma_SendCommand_tlm_payload_t              roma_sendcommand;

// // //         PAYUEL_LGPM_Noop_tlm_payload_t                     lgpm_noop;
// // //         PAYUEL_LGPM_ResetCounters_tlm_payload_t            lgpm_resetcounters;
// // //         PAYUEL_LGPM_MCU_ALIVE_CHECK_Tlm_Payload            lgpm_mcualivecheck;
// // //         PAYUEL_LGPM_3V3PwrOn_tlm_payload_t                 lgpm_3v3pwron;
// // //         PAYUEL_LGPM_3V3PwrOff_tlm_payload_t                lgpm_3v3pwroff;
// // //         PAYUEL_LGPM_MainBoostSwOn_tlm_payload_t            lgpm_mainboostswon;
// // //         PAYUEL_LGPM_MainBoostSwOff_tlm_payload_t           lgpm_mainboostswoff;
// // //         PAYUEL_LGPM_SubBoostSwOn_tlm_payload_t             lgpm_subboostswon;
// // //         PAYUEL_LGPM_SubBoostSwOff_tlm_payload_t            lgpm_subboostswoff;
// // //         PAYUEL_LGPM_V28MainOn_tlm_payload_t                lgpm_v28mainon;
// // //         PAYUEL_LGPM_V28MainOff_tlm_payload_t               lgpm_v28mainoff;
// // //         PAYUEL_LGPM_V28SubOn_tlm_payload_t                 lgpm_v28subon;
// // //         PAYUEL_LGPM_V28SubOff_tlm_payload_t                lgpm_v28suboff;
// // //         PAYUEL_LGPM_V12MainOn_tlm_payload_t                lgpm_v12mainon;
// // //         PAYUEL_LGPM_V12MainOff_tlm_payload_t               lgpm_v12mainoff;
// // //         PAYUEL_LGPM_PwrSenseInfo_tlm_payload_t             lgpm_pwrsenseinfo;
// // //         PAYUEL_LGPM_PwrSeqOn_tlm_payload_t                 lgpm_pwrseqon;
// // //         PAYUEL_LGPM_PwrSeqOff_tlm_payload_t                lgpm_pwrseqoff;
// // //         PAYUEL_LGPM_RwaControlIdx1_tlm_payload_t           lgpm_rwacontrol_idx1;
// // //         PAYUEL_LGPM_RwaControlIdx2_tlm_payload_t           lgpm_rwacontrol_idx2;
// // //         PAYUEL_LGPM_RwaControlIdx3_tlm_payload_t           lgpm_rwacontrol_idx3;
// // //         PAYUEL_LGPM_RwaPwrOn_tlm_payload_t                 lgpm_rwapwron;
// // //         PAYUEL_LGPM_RwaPwrOff_tlm_payload_t                lgpm_rwapwroff;
// // //         PAYUEL_LGPM_RwaSenseInfo_tlm_payload_t             lgpm_rwasenseinfo;
// // // /**************************************************************************************************** */

// // //     } u;
// // // } ReportView_t;


// // // extern std::mutex g_report_view_mtx;
// // // extern ReportView_t g_report_view;
// // // extern EPS_Report_State_t g_eps_report_state;


// // // typedef struct {
// // //     uint8 Callsign[6];
// // //     uint8 CurrentMode;
// // //     uint8 CurrentSubmode;
// // //     uint8 PrevioudMode;
// // //     uint8 PreviousSubmode;
// // //     uint8 CurrentModeFlag;
// // //     uint8 PreviousModeFlag;
// // //     uint32 ApplicationRunStatus;
// // //     uint32 SatelliteTime;
// // //     uint16 RebootCount;
// // //     uint8 RebootCause;

// // // }__attribute__((packed)) FM_HK_;

// // // typedef struct {
// // // 	uint16 DeployState_UANT;
// // // }__attribute__((packed)) UANT_;

// // // typedef struct {
// // // 	uint32 rxfreq;
// // // 	uint32 txfreq;
// // // 	int16 LastRssi;
// // //     uint32 TotRxBytes;
// // //     uint8 StatusConfiguration;
// // // }__attribute__((packed)) UTRX_;

// // // typedef struct {
// // // 	//EPS - P60 Dock
// // // 	uint8 out_en_dock[7]; //01458910
// // //     int16 temp_dock[2];
// // //     uint32 bootcause;
// // //     uint32 bootcnt;
// // //     uint32 uptime;
// // //     uint16 resetcause;
// // //     uint8 batt_mode;
// // //     uint8 heater_on;
// // //     uint16 latchup_dock[7]; //01458910
// // //     uint16 vbat_v;
// // //     int16 batt_v;
// // //     int16 batt_temp[2];
// // //     uint8 device_status[8];
// // //     uint32 wdt_cnt_gnd;
// // //     uint32 wdt_gnd_left;
// // //     int16 batt_chrg;
// // //     int16 batt_dischrg;
// // // 	//EPS - PDU
// // //     int16 vbat;
// // //     uint8 out_en_pdu[6]; //034758
// // //     uint16 latchup_pdu[6];
// // //     uint16 out_voltage[6]; // 034758
// // // 	//EPS - ACU
// // //     int16 c_in[4]; //0123
// // //     uint16 v_in[4]; //0123
// // // }__attribute__((packed)) EPS_;

// // // typedef struct {
// // // 	uint8 RWL0_PowerState;
// // // 	uint8 RWL1_PowerState;
// // // 	uint8 RWL2_PowerState;
// // // 	uint8 MAG0_PowerState;
// // // 	uint8 FSS0_PowerState;
// // // 	uint8 HSS0_PowerState;
// // // 	uint8 Control_Mode;
// // // 	uint16 Mag_Control_Timeout;
// // // 	float GYRO_Calib_rate_X;
// // // 	float GYRO_Calib_rate_Y;
// // // 	float GYRO_Calib_rate_Z;
// // // }__attribute__((packed)) ADCS_;

// // // typedef struct {
// // // 	uint8 Status;
// // // 	int16 Board_Temperature;
// // // 	int16 Battery_Current;
// // // 	int16 Battery_Voltage;
// // // }__attribute__((packed)) STX_;

// // // typedef struct {
// // // 	int16 temp_PAYC;
// // // 	uint16 icore;
// // // }__attribute__((packed)) PAYC_;

// // // typedef struct {
// // // 	uint8 DeployStatus_PAYR;
// // // }__attribute__((packed)) PAYR_;

// // // typedef struct {
// // // 	uint8 PAYS_State;
// // // 	uint8 PAYS_Sign;
// // // 	uint8 PAYS_Temp;
// // // }__attribute__((packed)) PAYS_;


// // // // typedef struct {
// // // // 	CCSDS_Header_ CCSDS_Header;
// // // // 	FM_HK_ FM;
// // // // 	EPS_ EPS;
// // // // 	TCS_ TCS;
// // // // 	RWA_ RWA;
// // // // 	MTQ_ MTQ;
// // // // 	SNSR_ SNSR;
// // // // 	UTRX_ UTRX;
// // // // 	STX_ STX;
// // // // 	PAY_ PAY;
// // // // }__attribute__((packed)) HK;

// // // // typedef struct {
// // // // 	FM_HK_ FM;
// // // // 	ADCS_ ADCS;
// // // // }__attribute__((packed)) AOD;

// // // typedef struct {
// // // 	uint32_t ExTime;
// // // 	uint32_t ExWindow;
// // // 	uint16_t EntryID;
// // // 	uint16_t GroupID;
// // // 	uint8_t cmd[];
// // // }__attribute__((packed)) Book;



// // // static bool ParseReportWire540(const uint8_t *buf, size_t len, Report &out);
// // // void * TRxController(void *);
// // // void * SignalTest(void*);
// // // void now_rx_bytes_update();
// // // void set_rx_bytes(uint32_t nowbytes);
// // // uint32_t get_rx_bytes();
// // // uint32_t * get_rx_bytes_address();
// // // uint16_t get_boot_count();
// // // uint16_t * get_boot_count_address();
// // // void buf_allclear();
// // // void CalculateChecksum(CommandHeader_t* Cmd);
// // // int32_t GenerateCmdMsg(CommandHeader_t* Cmd, uint16_t MsgId, uint8_t FcnCode, uint32_t ArgLen);
// // // csp_socket_t *  DL_sock_initialize();
// // // extern BEE1000_Beacon_t* bee1000_beacon;
// // // extern BEE1012_Beacon_t* bee1012_beacon;
// // // extern UELYSYS_Beacon_t* uelysys_beacon;
// // // int BEE1000BeaconSaver(BEE1000_Beacon_t * bec);
// // // int BEE1012BeaconSaver(BEE1012_Beacon_t * bec);
// // // int UELYSYSBeaconSaver(UELYSYS_Beacon_t * bec);
// // // void * task_downlink_onorbit(void * socketinfo);
// // // void * task_uplink_onorbit(void * sign_);

// // // int PacketHandler(csp_packet_t *packet, int type, int NowCursor);
// // // packetsign * PingInit(FSWTle * FSWTleinfo);
// // // csp_packet_t * PacketEncoder(packetsign * sign, bool freeer = true);
// // // packetsign * PacketDecoder(csp_packet_t * packet);


// // // class CmdGenerator_GS {
// // // private:
// // //     void SetHeaderWord(uint8_t* Word16, uint16_t Value, uint16_t Mask);
// // //     void SetHeaderByte(uint8_t* Byte, uint8_t Value, uint8_t Mask);
// // //     void GetHeaderWord(const uint8_t* Word16, uint16_t& Value, uint16_t Mask);
// // //     uint32_t ComputeCheckSum(void);

// // // public:
// // // 	CFE_MSG_CommandHeader* CmdHeader;
// // // 	bool Scheduled = false;
// // // 	bool Checksum = true;

// // //     CmdGenerator_GS(void);
// // //     ~CmdGenerator_GS(void);

// // //     int GenerateCmdHeader(uint32_t MsgId, uint16_t FncCode, uint32_t Size, void* Data);
// // //     void CopyCmdHeaderToBuffer(uint8_t* Buffer);

// // //     void InitHeader(void);
// // //     void SetHeader(const CFE_MSG_CommandHeader* Header);
// // //     const CFE_MSG_CommandHeader* GetHeader(void) const;

// // //     int SetHasSecondaryHeader(bool HasSec);
// // //     int SetMsgId(uint16_t MsgId);
// // //     int SetSize(uint16_t Size);
// // //     int SetSegmentationFlag(uint16_t SegFlag);
// // //     int SetFncCode(uint16_t FncCode);

// // //     bool HasSecondaryHeader(void) const;
// // //     uint16_t GetSize(void);
// // //     uint16_t GetFncCode(void) const;

// // //     int GenerateChecksum(void);
// // // 	int Scheduling(uint32_t ExecutionTime, uint32_t ExecutionWindow, uint32_t EntryID, uint16_t GroupID);
// // // 	packetsign * GenerateCMDPacket(void);
// // // };

// // // void * Direct_Shell(void * data);

// // // #endif _MIMAN_COMS_H_

// // #pragma once
// // #ifndef _MIMAN_COMS_H_
// // #define _MIMAN_COMS_H_

// // #include "miman_config.h"
// // #include "miman_orbital.h"
// // #include <mutex>

// // #pragma once
// // #include <mutex>






// // typedef struct{
// // 	uint16_t Identifier;
// // 	uint16_t PacketType;
// // 	uint32_t Length;
// // 	uint8_t Data[];

// // }__attribute__((packed)) packetsign;

// // typedef struct{
// // 	uint16_t target;
// // 	uint16_t filestatus;
// // 	uint32_t filenum;
// // 	uint32_t offset;
// // 	uint32_t step;
// // }__attribute__((packed)) dlreqdata;

// // typedef struct{
// // 	int32_t filenum;
// // 	uint32_t file[];
// // }__attribute__((packed)) filelist;

// // // typedef enum {
// // //     GS_FTP_INFO_FILE  = 0,   /**< File size and checksum */
// // //     GS_FTP_INFO_CRC = 1,         /**< CRC of remote and local file */
// // //     GS_FTP_INFO_COMPLETED = 2,   /**< Completed and total chunks */
// // //     GS_FTP_INFO_PROGRESS = 3,    /**< Current chunk, total_chunks and chunk_size */
// // // } gs_ftp_info_type_t;

// // typedef struct {
// //     FILE       * fp;
// //     FILE       * fp_map;
// //     csp_conn_t * conn;
// //     uint32_t   timeout;
// //     char       file_name[GS_FTP_PATH_LENGTH];
// //     uint32_t   file_size;
// //     uint32_t   chunks;
// //     int        chunk_size;
// //     uint32_t   checksum;
// //     ftp_status_element_t last_status[GS_FTP_STATUS_CHUNKS];
// //     uint32_t   last_entries;
// //     gs_ftp_info_callback_t info_callback;
// //     void       * info_data;
// // } gs_ftp_state_t;

// // typedef struct {
// //     gs_ftp_backend_type_t backend;
// //     const char * path;
// //     uint32_t addr;
// //     uint32_t size;
// // } gs_ftp_url_t;



// // // BEE-1000 RPT structure
// // typedef struct {
// //     bool valid;
// //     uint16_t CCMessage_ID;
// //     uint16_t CCCount;
// //     uint16_t CCLength;
// //     uint8_t CCTime_code[6];
// //     uint16_t msg_id;
// //     uint8_t cc;
// //     uint8_t ret_type;
// //     int32_t ret_code;
// //     uint16_t ret_val_size;
// //     std::vector<uint8_t> payload;
// // } ReportPacket_t;

// // extern ReportPacket_t g_last_report;


// // // BEE-1000 Beacon
// // typedef struct {

// // 	// CCSDS Header
// //     uint8 CCSDS_MID[2];
// //     uint8 CCSDS_Seq[2];
// //     uint8 CCSDS_Len[2];
// //     uint8 CCSDS_TimeCode[6];

// // 	// FSW - RPT - shoud be deprecated
// // 	// uint16 RPT_BootCount;
// //     // uint32 RPT_ScTimeSec;
// //     // uint32 RPT_ScTimeSubsec;
// //     // uint32 RPT_Sequence;
// //     // uint8 RPT_ResetCause;

// //     /* FSW - RPT - Revised (Kweon Hyeokjin) */
// //     uint8_t RPT_CmdCounter;
// //     uint8_t RPT_ErrCounter;
// //     uint8_t RPT_ReportCnt;
// //     uint8_t RPT_CriticalCnt;
// //     uint16_t RPT_BootCount;
// //     uint32_t RPT_ScTimeSec;
// //     uint32_t RPT_ScTimeSubsec;
// //     uint8_t RPT_Sequence_LSB;
// //     /*--------------End of BEE RPT Revision (Kweon Hyeokjin)--------------*/

// // 	// COMS - STX
// //     // all param
// //     uint8_t  STX_symbol_rate;
// //     uint8_t  STX_transmit_power;
// //     uint8_t  STX_modcod;
// //     uint8_t  STX_roll_off;
// //     uint8_t  STX_pilot_signal;
// //     uint8_t  STX_fec_frame_size;
// //     uint16_t STX_pretransmission_delay;
// //     float    STX_center_frequency;
// //     // modulation interface -> cpu temp 빠짐
// //     uint8_t STX_modulator_interface_type;  
// //     uint8_t STX_lvds_io_type;    
// // 	uint8 STX_SystemState;
// //     uint8 STX_StatusFlag;
// //     // float STX_CpuTemp;

// // 	// COMS - UANT
// //     uint8 UANT1_Chan0;
// //     uint8 UANT1_Chan1;
// //     uint8 UANT1_BackupActive;
// //     uint8 UANT2_Chan0;
// //     uint8 UANT2_Chan1;
// //     uint8 UANT2_BackupActive;

// // 	// COMS - UTRX
// //     uint8 UTRX_ActiveConf;
// //     uint16 UTRX_BootCount;
// //     uint32 UTRX_BootCause;
// //     int16 UTRX_BoardTemp;

// //     // PCDU - P60 Dock
// //     int16 P60D_Cout[9];
// //     uint16 P60D_Vout[9];
// //     uint16 P60D_OutEn;
// //     uint32 P60D_BootCause;
// //     uint32 P60D_BootCount;
// //     uint8 P60D_BattMode;
// //     uint8 P60D_HeaterOn;
// //     uint16 P60D_VbatV;
// //     int16 P60D_VccC;
// //     uint16 P60D_BattV;
// //     int16 P60D_BattTemp[2];
// //     uint32 P60D_WdtCanLeft;
// //     int16 P60D_BattChrg;
// //     int16 P60D_BattDischrg;

// //     // PCDU - P60 PDU
// //     int16  P60P_Cout[9];
// //     uint16 P60P_Vout[9];
// //     int16  P60P_Vcc;
// //     uint8  P60P_ConvEn;
// //     uint16 P60P_OutEn;

// //     // PCDU - P60 ACU
// //     int16  P60A_Cin[6];
// //     uint16 P60A_Vin[6];

// //     // ADCS
// //        /** Combined Power State
// //      *  | 7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
// //      *  +--------------------------------------+
// //      *  |Rsv|RWL0|RWL1|RWL2|MAG0|GYRO|FSS0|HSS0|
// //      *  +--------------------------------------+
// //      */
// //     uint8 ADCS_PowerState; // ID 183

// //     uint8 ADCS_ControlMode; // ID 185

// //     float ADCS_GYR0CalibratedRateXComponent;
// //     float ADCS_GYR0CalibratedRateYComponent;
// //     float ADCS_GYR0CalibratedRateZComponent; // ID 207, 12bytes

// // }__attribute__((packed)) BEE1000_Beacon_t;

// // #define a sizeof(BEE1000_Beacon_t);
// // static_assert(sizeof(BEE1000_Beacon_t) == 203, "BEE1000_Beacon_t size mismatch");

// // typedef struct {
// //     char call_sign[7];
// //     uint8_t msg_id[2];
// //     uint8_t sequence[2];
// //     uint8_t length[2];
// //     uint8_t time_code[6];
// // }__attribute__((packed)) BEE1012_BeaconHeader_t;

// // typedef struct {
// //     uint16_t boot_count;
// //     uint32_t sequence;
// //     uint8_t reset_cause;
// // }__attribute__((packed)) BEE1012_RPT_t;

// // typedef struct {
// //     uint8_t active_conf;
// //     uint16_t boot_count;
// //     uint32_t boot_cause;
// //     int16_t temp_board;
// // }__attribute__((packed)) BEE1012_AX100_t;

// // typedef struct {
// //     int16_t temp;
// //     int8_t connection_quality;
// //     uint16_t battery_capacity;
// // }__attribute__((packed)) BEE1012_LTRX_t;

// // typedef struct {
// //     BEE1012_AX100_t ax100;
// //     BEE1012_LTRX_t ltrx;
// // }__attribute__((packed)) BEE1012_COMS_t;

// // typedef struct {
// //     uint32_t bootcause;
// //     uint16_t resetcause;
// //     uint16_t bootcount;
// //     uint8_t out_en[6];
// //     int16_t temp[2];
// //     uint8_t batt_mode;
// //     int16_t batt_i;
// //     uint16_t batt_v;
// //     uint8_t sm_en;
// //     uint16_t gnd_wdt_cnt;
// //     uint16_t bus_wdt_cnt;
// //     uint32_t gnd_wdt_left;
// //     uint32_t bus_wdt_left;
// // }__attribute__((packed)) BEE1012_EPS_PMU_t;

// // typedef struct {
// //     int16_t out_i[12];
// //     uint8_t out_en[12];
// // }__attribute__((packed)) BEE1012_EPS_PDU_t;

// // typedef struct {
// //     int16_t input_i[6];
// //     uint16_t input_v[6];
// //     uint8_t mppt_mode;
// // }__attribute__((packed)) BEE1012_EPS_ACU_Unit_t;

// // typedef struct {
// //     BEE1012_EPS_ACU_Unit_t acu[2];
// // }__attribute__((packed)) BEE1012_EPS_ACU_t;

// // typedef struct {
// //     uint16_t bootcount;
// //     uint16_t bootcause;
// //     uint16_t resetcause;
// //     float soc;
// //     float bat_avr_temp;
// //     uint16_t vbat;
// //     float i;
// //     uint16_t heater_i;
// // }__attribute__((packed)) BEE1012_EPS_BP8_t;

// // typedef struct {
// //     uint16_t gpio_status;
// //     uint8_t sp_deploy_status;
// // }__attribute__((packed)) BEE1012_EPS_GPIO_t;

// // typedef struct {
// //     BEE1012_EPS_PMU_t pmu;
// //     BEE1012_EPS_PDU_t pdu;
// //     BEE1012_EPS_ACU_t acu;
// //     BEE1012_EPS_BP8_t bp8;
// //     BEE1012_EPS_GPIO_t gpio;
// // }__attribute__((packed)) BEE1012_EPS_t;

// // typedef struct {
// //     uint8_t power_state;
// //     uint8_t control_mode;
// //     float gyro0_calibrated_rate_x;
// //     float gyro0_calibrated_rate_y;
// //     float gyro0_calibrated_rate_z;
// //     uint8_t css[6];
// // }__attribute__((packed)) BEE1012_ADCS_t;

// // typedef struct {
// //     BEE1012_BeaconHeader_t header;
// //     BEE1012_RPT_t rpt;
// //     BEE1012_COMS_t coms;
// //     BEE1012_EPS_t eps;
// //     BEE1012_ADCS_t adcs;
// // }__attribute__((packed)) BEE1012_Beacon_t;

// // static_assert(sizeof(BEE1012_BeaconHeader_t) == 19, "BEE1012_BeaconHeader_t size mismatch");
// // static_assert(sizeof(BEE1012_RPT_t) == 7, "BEE1012_RPT_t size mismatch");
// // static_assert(sizeof(BEE1012_AX100_t) == 9, "BEE1012_AX100_t size mismatch");
// // static_assert(sizeof(BEE1012_LTRX_t) == 5, "BEE1012_LTRX_t size mismatch");
// // static_assert(sizeof(BEE1012_COMS_t) == 14, "BEE1012_COMS_t size mismatch");
// // static_assert(sizeof(BEE1012_EPS_PMU_t) == 36, "BEE1012_EPS_PMU_t size mismatch");
// // static_assert(sizeof(BEE1012_EPS_PDU_t) == 36, "BEE1012_EPS_PDU_t size mismatch");
// // static_assert(sizeof(BEE1012_EPS_ACU_Unit_t) == 25, "BEE1012_EPS_ACU_Unit_t size mismatch");
// // static_assert(sizeof(BEE1012_EPS_ACU_t) == 50, "BEE1012_EPS_ACU_t size mismatch");
// // static_assert(sizeof(BEE1012_EPS_BP8_t) == 22, "BEE1012_EPS_BP8_t size mismatch");
// // static_assert(sizeof(BEE1012_EPS_GPIO_t) == 3, "BEE1012_EPS_GPIO_t size mismatch");
// // static_assert(sizeof(BEE1012_EPS_t) == 147, "BEE1012_EPS_t size mismatch");
// // static_assert(sizeof(BEE1012_ADCS_t) == 20, "BEE1012_ADCS_t size mismatch");
// // static_assert(sizeof(BEE1012_Beacon_t) == 207, "BEE1012_Beacon_t size mismatch");

// // typedef struct {
// //     char call_sign[7];
// //     uint8_t msg_id[2];
// //     uint8_t sequence[2];
// //     uint8_t length[2];
// //     uint8_t time_code[6];
// // }__attribute__((packed)) UELYSYS_BeaconHeader_t;

// // typedef struct {
// //     uint16_t boot_count;
// //     uint32_t sequence;
// //     uint8_t reset_cause;
// // }__attribute__((packed)) UELYSYS_FSW_t;

// // typedef struct {
// //     uint8_t i2c1_0x05_channel_0_status;
// //     uint8_t i2c1_0x05_channel_1_status;
// //     uint8_t i2c1_0x06_channel_0_status;
// //     uint8_t i2c1_0x06_channel_1_status;
// //     uint8_t ax100_active_conf;
// //     uint16_t ax100_boot_count;
// //     uint32_t ax100_boot_cause;
// //     int16_t ax100_temp_board;
// // }__attribute__((packed)) UELYSYS_COMS_t;

// // typedef struct {
// //     uint32_t bootcause;
// //     uint16_t resetcause;
// //     uint16_t bootcount;
// //     uint8_t out_en[6];
// //     int16_t temp[2];
// //     uint8_t batt_mode;
// //     int16_t batt_i;
// //     uint16_t batt_v;
// //     uint8_t sm_en;
// //     uint16_t gnd_wdt_cnt;
// //     uint16_t bus_wdt_cnt;
// //     uint32_t gnd_wdt_left;
// //     uint32_t bus_wdt_left;
// // }__attribute__((packed)) UELYSYS_EPS_PMU_t;

// // typedef struct {
// //     int16_t out_i[12];
// //     uint8_t out_en[12];
// // }__attribute__((packed)) UELYSYS_EPS_PDU_t;

// // typedef struct {
// //     int16_t input_i[6];
// //     uint16_t input_v[6];
// //     uint8_t mppt_mode;
// // }__attribute__((packed)) UELYSYS_EPS_ACU_Unit_t;

// // typedef struct {
// //     UELYSYS_EPS_ACU_Unit_t acu[2];
// // }__attribute__((packed)) UELYSYS_EPS_ACU_t;

// // typedef struct {
// //     uint16_t bootcount;
// //     uint16_t bootcause;
// //     uint16_t resetcause;
// //     float soc;
// //     float bat_avr_temp;
// //     uint16_t vbat;
// //     float i;
// //     uint16_t heater_i;
// // }__attribute__((packed)) UELYSYS_EPS_BP8_t;

// // typedef struct {
// //     uint8_t dsp_i2c1_0x07_status;
// //     uint8_t dsp_i2c1_0x08_status;
// //     uint8_t dsp_i2c1_0x09_status;
// //     uint8_t dsp_i2c1_0x10_status;
// // }__attribute__((packed)) UELYSYS_EPS_DSP_t;

// // typedef struct {
// //     UELYSYS_EPS_PMU_t pmu;
// //     UELYSYS_EPS_PDU_t pdu;
// //     UELYSYS_EPS_ACU_t acu;
// //     UELYSYS_EPS_BP8_t bp8;
// //     UELYSYS_EPS_DSP_t dsp;
// // }__attribute__((packed)) UELYSYS_EPS_t;

// // typedef struct {
// //     uint8_t power_state;
// //     uint8_t control_mode;
// //     float gyro0_calibrated_rate_x;
// //     float gyro0_calibrated_rate_y;
// //     float gyro0_calibrated_rate_z;
// //     uint8_t css[6];
// // }__attribute__((packed)) UELYSYS_ADCS_t;

// // typedef struct {
// //     UELYSYS_BeaconHeader_t header;
// //     UELYSYS_FSW_t fsw;
// //     UELYSYS_COMS_t coms;
// //     UELYSYS_EPS_t eps;
// //     UELYSYS_ADCS_t adcs;
// // }__attribute__((packed)) UELYSYS_Beacon_t;

// // static_assert(sizeof(UELYSYS_BeaconHeader_t) == 19, "UELYSYS_BeaconHeader_t size mismatch");
// // static_assert(sizeof(UELYSYS_FSW_t) == 7, "UELYSYS_FSW_t size mismatch");
// // static_assert(sizeof(UELYSYS_COMS_t) == 13, "UELYSYS_COMS_t size mismatch");
// // static_assert(sizeof(UELYSYS_EPS_PMU_t) == 36, "UELYSYS_EPS_PMU_t size mismatch");
// // static_assert(sizeof(UELYSYS_EPS_PDU_t) == 36, "UELYSYS_EPS_PDU_t size mismatch");
// // static_assert(sizeof(UELYSYS_EPS_ACU_Unit_t) == 25, "UELYSYS_EPS_ACU_Unit_t size mismatch");
// // static_assert(sizeof(UELYSYS_EPS_ACU_t) == 50, "UELYSYS_EPS_ACU_t size mismatch");
// // static_assert(sizeof(UELYSYS_EPS_BP8_t) == 22, "UELYSYS_EPS_BP8_t size mismatch");
// // static_assert(sizeof(UELYSYS_EPS_DSP_t) == 4, "UELYSYS_EPS_DSP_t size mismatch");
// // static_assert(sizeof(UELYSYS_EPS_t) == 148, "UELYSYS_EPS_t size mismatch");
// // static_assert(sizeof(UELYSYS_ADCS_t) == 20, "UELYSYS_ADCS_t size mismatch");
// // static_assert(sizeof(UELYSYS_Beacon_t) == 207, "UELYSYS_Beacon_t size mismatch");



// // typedef struct CFE_SRL_HousekeepingTlm_Payload {
// //     uint8 CommandCounter;

// //     uint8 CommandErrorCounter;

// //     uint8 IOHandleStatus[4];

// //     uint16 IOHandleTxCount[4];
    
// // }__attribute__((packed)) CFE_SRL_HousekeepingTlm_Payload_t;

// // typedef struct RPT_HkTlm_Payload{
// //     uint8 CmdCounter;
// //     uint8 CmdErrCounter;

// //     /**
// //      * Queue Info
// //      */
// //     uint8 ReportQueueCnt;
// //     uint8 CriticalQueueCnt;

// //     /**
// //      * Operation Data
// //      */
// //     uint16 BootCount;
// //     uint32 TimeSec;
// //     uint32 TimeSubsec;
// //     uint32 Sequence; /* Backup data numbering */

// //     /**
// //      * Reset Cause
// //      */
// //     uint8 ResetCause;

// // }__attribute__((packed)) RPT_BcnTlm_Payload_t;

// // typedef struct PAY_BcnTlm_Payload {
// //     uint8 CommandCounter;
// //     uint8 CommandErrorCounter;

// //     /**
// //      * Else ....
// //      */
// //     /* compact beacon subset */
// //     int8  sys_status;         /* payload system status */
// //     int16 temp_ntc_0;
// //     int16 temp_ntc_1;
// //     int16 temp_ntc_2;     
// //     int16 temp_ntc_3;
// //     int16 temp_ntc_4;         /* board temp 4 */
// //     int16 temp_ntc_5;         /* board temp 5 */
// //     int16 temp_ntc_6;         /* board temp 6 */
// //     int16 temp_ntc_7;         /* board temp 7 */
// //     int16 temp_ntc_8;         /* board temp 8 */
// //     int16 temp_ntc_9;         /* board temp 9 */
// //     int16 temp_ntc_10;        /* board temp 10 */
// //     int16 temp_ntc_11;        /* board temp 11 */     


// // } PAY_BcnTlm_Payload_t;

// // typedef struct PAY_HkTlm_Payload {
// //     uint8 CommandCounter;
// //     uint8 CommandErrorCounter;

// //     int8  sys_status;         /* payload system status */
// //     int16 temp_ntc_0;         /* board temp 0 */
// //     int16 temp_ntc_1;         /* board temp 1 */
// //     int16 temp_ntc_2;         /* board temp 2 */
// //     int16 temp_ntc_3;         /* board temp 3 */
// //     int16 temp_ntc_4;         /* board temp 4 */
// //     int16 temp_ntc_5;         /* board temp 5 */
// //     int16 temp_ntc_6;         /* board temp 6 */
// //     int16 temp_ntc_7;         /* board temp 7 */
// //     int16 temp_ntc_8;         /* board temp 8 */
// //     int16 temp_ntc_9;         /* board temp 9 */
// //     int16 temp_ntc_10;        /* board temp 10 */
// //     int16 temp_ntc_11;        /* board temp 11 */
// //     /* Expanded TM (selected currents/sensors) */
// //     uint32 sen1_data_0;
// //     uint32 sen1_data_1;  

// // } PAY_HkTlm_LINPayload_t;

// // // BEE-1000 Mission Beacon
// // typedef struct {

// // 	// Telemetry header
// //     uint8 CCSDS_MID[2];
// //     uint8 CCSDS_Seq[2];
// //     uint8 CCSDS_Len[2];
// //     uint8 CCSDS_TimeCode[6];


// //     CFE_SRL_HousekeepingTlm_Payload_t srlpayload;

// //     RPT_BcnTlm_Payload_t    rptpayload;

// //     // Payload
// //     PAY_BcnTlm_Payload_t    paybcnpayload;



// //     PAY_HkTlm_LINPayload_t     payhkpayload;
    

// // }__attribute__((packed)) MissionBeacon;

// // #define a sizeof(MissionBeacon);


// // struct GETFILEINFO {
// //     // ===== TLM Header =====
// //     uint8 CCSDS_MID[2];
// //     uint8 CCSDS_Seq[2];
// //     uint8 CCSDS_Len[2];
// //     uint8 CCSDS_TimeCode[6];
// //     uint8 padding[4];

// //     // ===== Payload =====
// //     uint8 FileStatus;
// //     uint8 CRC_Computed;
// //     uint8 Spare[2];
// //     uint32 CRC;
// //     uint32 FileSize;
// //     uint32 LastModifiedTime;
// //     uint32 Mode;
// //     char Filename[64];

// // };

// // #define a sizeof(GETFILEINFO);


// // struct Report {
// //     // ===== CCSDS Header =====v  16
// //     uint16 CCSDS_MsgId;
// //     uint16 CCSDS_Seq;
// //     uint16 CCSDS_Len;
// //     uint8 CCSDS_TimeCode[6];
// //     uint32 CCSDS_Padding;


// //     // ===== Report Body =====  10     26 byte
// //     uint16 ReflectedMID;
// //     uint8  ReflectedCC;
// //     uint8  RetType;
// //     int32  RetCode;
// //     uint16 RetValSize;
// //     uint8  RetVal[512];

// // };

// // #define a sizeof(Report)

// // struct Event {

// //     uint16 CCSDS_MsgId;
// //     uint16 CCSDS_Seq;
// //     uint16 CCSDS_Len;
// //     uint8 CCSDS_TimeCode[6];
// //     uint32 CCSDS_Padding;

// //     char AppName[20]; /**< 20임   \cfetlmmnemonic \EVS_APPNAME
// //                                                 \brief Application name */
// //     uint16 EventID;                        /**< \cfetlmmnemonic \EVS_EVENTID
// //                                                 \brief Numerical event identifier */
// //     uint16 EventType;    /**< uint16임 \cfetlmmnemonic \EVS_EVENTTYPE  
// //                                                 \brief Numerical event type identifier */
// //     uint32 SpacecraftID;                   /**< \cfetlmmnemonic \EVS_SCID
// //                                                 \brief Spacecraft identifier */
// //     uint32 ProcessorID;                    /**< \cfetlmmnemonic \EVS_PROCESSORID
// //                                                 \brief Numerical processor identifier */


// //     char               Message[122]; /**< 122임 \cfetlmmnemonic \EVS_EVENT
// //                                                                  \brief Event message string */
// //     uint8 Spare1;                                                   /**< \cfetlmmnemonic \EVS_SPARE1
// //                                                                          \brief Structure padding */
// //     uint8 Spare2;                                                   /**< \cfetlmmnemonic \EVS_SPARE2
// //                                                                      \brief Structure padding */

// // };

// // #define a sizeof(Event)

// // typedef enum
// // {
// //     REPORT_KIND_NONE = 0,

// //     // ADCS Sunpointing RPT
// //     REPORT_KIND_ADCS_LOG_MASK,
// //     REPORT_KIND_ADCS_UNSOLICIT_TLM_SETUP_TLM,

// //     REPORT_KIND_UANT_GET_STATUS_TLM,

// // 	    REPORT_KIND_EPS_P60_DOCK_GET_TABLE_HK,
// // 	    REPORT_KIND_EPS_P60_PDU_GET_TABLE_HK,
// // 	    REPORT_KIND_EPS_P60_ACU_GET_TABLE_HK,
// // 	    REPORT_KIND_EPS_QUERY_REPORT,
// // 	    REPORT_KIND_EPS_POWER_IF_STATUS,
// // 	    REPORT_KIND_EPS_POWER_IF_LIST,
// // 	    REPORT_KIND_EPS_P80_PMU_HK,
// // 	    REPORT_KIND_EPS_P80_PDU_HK,
// // 	    REPORT_KIND_EPS_P80_ACU1_HK,
// // 	    REPORT_KIND_EPS_P80_ACU2_HK,
// // 	    REPORT_KIND_EPS_BP8_HK,

// // /****************************************************************************************** */
// //     REPORT_KIND_ADCS_GET_ERROR_LOG_SETTING,
// //     REPORT_KIND_ADCS_GET_CURRENT_UNIX_TIME,
// //     REPORT_KIND_ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC,
// //     REPORT_KIND_ADCS_GET_COMMUNICATION_STATUS,
// //     REPORT_KIND_ADCS_GET_CONTROL_ESTIMATION_MODE,
// //     REPORT_KIND_ADCS_GET_REFERENCE_IRC_VECTOR,
// //     REPORT_KIND_ADCS_GET_REFERENCE_LLH_TARGET,
// //     REPORT_KIND_ADCS_GET_ORBIT_MODE,
// //     REPORT_KIND_ADCS_GET_HEALTH_TLM_MMT,
// //     REPORT_KIND_ADCS_GET_RAW_CUBESENSE_SUN,
// //     REPORT_KIND_ADCS_GET_REFERENCE_RPY_VALUES,
// //     REPORT_KIND_ADCS_GET_OPENLOOPCMD_MTQ,
// //     REPORT_KIND_ADCS_GET_POWER_STATE,
// //     REPORT_KIND_ADCS_GET_RUN_MODE,
// //     REPORT_KIND_ADCS_GET_CONTROL_MODE,
// //     REPORT_KIND_ADCS_GET_MAG0_MMT_CALIB_CONFIG,
// //     REPORT_KIND_ADCS_GET_MAG1_MMT_CALIB_CONFIG,
// //     REPORT_KIND_ADCS_GET_ESTIMATION_MODE,
// //     REPORT_KIND_ADCS_GET_OPERATIONAL_STATE,
// //     REPORT_KIND_ADCS_GET_RAW_CSS_SENSOR,
// //     REPORT_KIND_ADCS_GET_RAW_GYR_SENSOR,
// //     REPORT_KIND_ADCS_GET_CALIBRATED_GYR_SENSOR,
// //     REPORT_KIND_ADCS_GET_MAG_SENSING_ELM_CONFIG,
// //     REPORT_KIND_ADCS_GET_TLM_LOG_INCLMASK,
// //     REPORT_KIND_ADCS_GET_UNSOLICIT_TLM_MSG_SETUP,
// //     REPORT_KIND_ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP,
// //     REPORT_KIND_ADCS_GET_EVENT_LOG_STATUS_RESPONSE,
// //     REPORT_KIND_ADCS_GET_PORTMAP,
// // /********************************************************************************************* */

// // /****************************************5차 추가*************************************** */
// //     // 1. PAYUEL_ROMA
// //     REPORT_KIND_PAYUEL_ROMA_NOOP,            // CC 0, 1
// //     REPORT_KIND_PAYUEL_ROMA_RESETCOUNTERS,
// //     REPORT_KIND_PAYUEL_ROMA_COMMTEST,
// //     REPORT_KIND_PAYUEL_ROMA_GETSPECIFICLINE,
// //     REPORT_KIND_PAYUEL_ROMA_GETMULTIPLELINES,
// //     REPORT_KIND_PAYUEL_ROMA_GETLATESTLINE,
// //     REPORT_KIND_PAYUEL_ROMA_GETLATESTNLINES,
// //     REPORT_KIND_PAYUEL_ROMA_SETROUTEDEFAULT,
// //     REPORT_KIND_PAYUEL_ROMA_RESETROUTE,
// //     REPORT_KIND_PAYUEL_ROMA_LOADROUTE,
// //     REPORT_KIND_PAYUEL_ROMA_SAVEROUTE,
// //     REPORT_KIND_PAYUEL_ROMA_SENDROUTE,
// //     REPORT_KIND_PAYUEL_ROMA_SETROUTE,
// //     REPORT_KIND_PAYUEL_ROMA_PARGET,
// //     REPORT_KIND_PAYUEL_ROMA_PARSET,
// //     REPORT_KIND_PAYUEL_ROMA_PARDEFAULTS,
// //     REPORT_KIND_PAYUEL_ROMA_PARSAVE,
// //     REPORT_KIND_PAYUEL_ROMA_PARRESTORE,
// //     REPORT_KIND_PAYUEL_ROMA_PARLOAD,
// //     REPORT_KIND_PAYUEL_ROMA_PARSETOOB,
// //     REPORT_KIND_PAYUEL_ROMA_SENDCOMMAND,


// //     // 2. PAYUEL_LGPM
// //     REPORT_KIND_PAYUEL_LGPM_NOOP,               // CC 0
// //     REPORT_KIND_PAYUEL_LGPM_RESETCOUNTERS,
// //     REPORT_KIND_PAYUEL_LGPM_MCU_ALIVE,          // CC 2
// //     REPORT_KIND_PAYUEL_LGPM_3V3_PWR_ON,         // CC 3
// //     REPORT_KIND_PAYUEL_LGPM_3V3_PWR_OFF,        // CC 4
// //     REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_ON,      // CC 5
// //     REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_OFF,     // CC 6
// //     REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_ON,       // CC 7
// //     REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_OFF,      // CC 8
// //     REPORT_KIND_PAYUEL_LGPM_V28_MAIN_ON,        // CC 9
// //     REPORT_KIND_PAYUEL_LGPM_V28_MAIN_OFF,       // CC 10
// //     REPORT_KIND_PAYUEL_LGPM_V28_SUB_ON,         // CC 11
// //     REPORT_KIND_PAYUEL_LGPM_V28_SUB_OFF,        // CC 12
// //     REPORT_KIND_PAYUEL_LGPM_V12_MAIN_ON,        // CC 13
// //     REPORT_KIND_PAYUEL_LGPM_V12_MAIN_OFF,       // CC 14
// //     REPORT_KIND_PAYUEL_LGPM_PWR_SENSE_INFO,          // CC 15
// //     REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_ON,         // CC 16
// //     REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_OFF,        // CC 17
// //     REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX1,           // CC 18
// //     REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX2,           // CC 19
// //     REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX3,           // CC 20
// //     REPORT_KIND_PAYUEL_LGPM_RWA_PWR_ON,         // CC 21
// //     REPORT_KIND_PAYUEL_LGPM_RWA_PWR_OFF,        // CC 22
// //     REPORT_KIND_PAYUEL_LGPM_RWA_SENSE_INFO,          // CC 23
// // /****************************************************************************************** */

// //     REPORT_KIND_SC_GENERIC,

// // } ReportKind_t;

// // static ReportKind_t DetermineReportKind(uint16_t reflected_mid, uint8_t reflected_cc) {
// //     if (reflected_mid == UANT_APP_CMD_ID && reflected_cc == UANT_APP_GET_STATUS_CC) return REPORT_KIND_UANT_GET_STATUS_TLM;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518)  && reflected_cc == ADCS_GET_TLM_LOG_INCLMASK_CC) return REPORT_KIND_ADCS_LOG_MASK;
    
// // /**************************************************************************************************************************************************************************************** */
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_ERROR_LOG_SETTING_CC) return REPORT_KIND_ADCS_GET_ERROR_LOG_SETTING;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CURRENT_UNIX_TIME_CC) return REPORT_KIND_ADCS_GET_CURRENT_UNIX_TIME;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC_CC) return REPORT_KIND_ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_COMMUNICATION_STATUS_CC) return REPORT_KIND_ADCS_GET_COMMUNICATION_STATUS;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CONTROL_ESTIMATION_MODE_CC) return REPORT_KIND_ADCS_GET_CONTROL_ESTIMATION_MODE;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_REFERENCE_IRC_VECTOR_CC) return REPORT_KIND_ADCS_GET_REFERENCE_IRC_VECTOR;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_REFERENCE_LLH_TARGET_CC) return REPORT_KIND_ADCS_GET_REFERENCE_LLH_TARGET;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_ORBIT_MODE_CC) return REPORT_KIND_ADCS_GET_ORBIT_MODE;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_HEALTH_TLM_MMT_CC) return REPORT_KIND_ADCS_GET_HEALTH_TLM_MMT;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RAW_CUBESENSE_SUN_CC) return REPORT_KIND_ADCS_GET_RAW_CUBESENSE_SUN;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_REFERENCE_RPY_VALUES_CC) return REPORT_KIND_ADCS_GET_REFERENCE_RPY_VALUES;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_OPENLOOPCMD_MTQ_CC) return REPORT_KIND_ADCS_GET_OPENLOOPCMD_MTQ;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_POWER_STATE_CC) return REPORT_KIND_ADCS_GET_POWER_STATE;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RUN_MODE_CC) return REPORT_KIND_ADCS_GET_RUN_MODE;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CONTROL_MODE_CC) return REPORT_KIND_ADCS_GET_CONTROL_MODE;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_MAG0_MMT_CALIB_CONFIG_CC) return REPORT_KIND_ADCS_GET_MAG0_MMT_CALIB_CONFIG;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_MAG1_MMT_CALIB_CONFIG_CC) return REPORT_KIND_ADCS_GET_MAG1_MMT_CALIB_CONFIG;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_ESTIMATION_MODE_CC) return REPORT_KIND_ADCS_GET_ESTIMATION_MODE;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_OPERATIONAL_STATE_CC) return REPORT_KIND_ADCS_GET_OPERATIONAL_STATE;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RAW_CSS_SENSOR_CC) return REPORT_KIND_ADCS_GET_RAW_CSS_SENSOR;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RAW_GYR_SENSOR_CC) return REPORT_KIND_ADCS_GET_RAW_GYR_SENSOR;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CALIBRATED_GYR_SENSOR_CC) return REPORT_KIND_ADCS_GET_CALIBRATED_GYR_SENSOR;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_MAG_SENSING_ELM_CONFIG_CC) return REPORT_KIND_ADCS_GET_MAG_SENSING_ELM_CONFIG;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_TLM_LOG_INCLMASK_CC) return REPORT_KIND_ADCS_GET_TLM_LOG_INCLMASK;
// //     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_UNSOLICIT_TLM_MSG_SETUP_CC) return REPORT_KIND_ADCS_GET_UNSOLICIT_TLM_MSG_SETUP;
// // 	    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP_CC) return REPORT_KIND_ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP;
// // 	    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_EVENT_LOG_STATUS_RESPONSE_CC) return REPORT_KIND_ADCS_GET_EVENT_LOG_STATUS_RESPONSE;
// // 	    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_PORTMAP_CC) return REPORT_KIND_ADCS_GET_PORTMAP;
// // 	/****************************************************************************************************************************************************************************************** */

// // 	    if ((reflected_mid == EPS_CMD_ID || reflected_mid == 0x7518) &&
// // 	        (reflected_cc == EPS_P80_POWER_IF_GET_CC ||
// // 	         reflected_cc == EPS_P80_POWER_IF_LIST_CC ||
// // 	         reflected_cc == EPS_GET_HK_ALL_CC)) return REPORT_KIND_EPS_QUERY_REPORT;

// // 	    /*********************************************5차 추가****************************************************** */
// //     // CC 0, 1
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_NOOP_CC)) return REPORT_KIND_PAYUEL_ROMA_NOOP;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_RESET_COUNTERS_CC)) return REPORT_KIND_PAYUEL_ROMA_RESETCOUNTERS;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_COMM_TEST_CC)) return REPORT_KIND_PAYUEL_ROMA_COMMTEST;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_SPECIFIC_LINE_CC)) return REPORT_KIND_PAYUEL_ROMA_GETSPECIFICLINE;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_MULTIPLE_LINES_CC)) return REPORT_KIND_PAYUEL_ROMA_GETMULTIPLELINES;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_LATEST_LINE_CC)) return REPORT_KIND_PAYUEL_ROMA_GETLATESTLINE;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_LATEST_N_LINES_CC)) return REPORT_KIND_PAYUEL_ROMA_GETLATESTNLINES;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SET_ROUTE_DEFAULT_CC)) return REPORT_KIND_PAYUEL_ROMA_SETROUTEDEFAULT;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_RESET_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_RESETROUTE;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_LOAD_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_LOADROUTE;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SAVE_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_SAVEROUTE;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SEND_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_SENDROUTE;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SET_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_SETROUTE;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_GET_CC)) return REPORT_KIND_PAYUEL_ROMA_PARGET;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_SET_CC)) return REPORT_KIND_PAYUEL_ROMA_PARSET;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_DEFAULTS_CC)) return REPORT_KIND_PAYUEL_ROMA_PARDEFAULTS;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_SAVE_CC)) return REPORT_KIND_PAYUEL_ROMA_PARSAVE;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_RESTORE_CC)) return REPORT_KIND_PAYUEL_ROMA_PARRESTORE;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_LOAD_CC)) return REPORT_KIND_PAYUEL_ROMA_PARLOAD;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_SET_OOB_CC)) return REPORT_KIND_PAYUEL_ROMA_PARSETOOB;
// //     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SEND_COMMAND_CC)) return REPORT_KIND_PAYUEL_ROMA_SENDCOMMAND;

// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_NOOP_CC)) return REPORT_KIND_PAYUEL_LGPM_NOOP;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RESET_COUNTERS_CC)) return REPORT_KIND_PAYUEL_LGPM_RESETCOUNTERS;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_MCU_ALIVE_CHECK_CC)) return REPORT_KIND_PAYUEL_LGPM_MCU_ALIVE;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_3V3_PWR_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_3V3_PWR_ON;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_3V3_PWR_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_3V3_PWR_OFF;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_MAIN_BOOST_SW_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_ON;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_MAIN_BOOST_SW_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_OFF;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_SUB_BOOST_SW_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_ON;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_SUB_BOOST_SW_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_OFF;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_MAIN_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_MAIN_ON;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_MAIN_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_MAIN_OFF;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_SUB_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_SUB_ON;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_SUB_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_SUB_OFF;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V12_MAIN_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_V12_MAIN_ON;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V12_MAIN_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_V12_MAIN_OFF;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_PWR_SENSE_INFO_CC)) return REPORT_KIND_PAYUEL_LGPM_PWR_SENSE_INFO;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_PWR_SEQ_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_ON;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_PWR_SEQ_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_OFF;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_CONTROL_idx1_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX1;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_CONTROL_idx2_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX2;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_CONTROL_idx3_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX3;
// //     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_PWR_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_PWR_ON;
// // 	    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_PWR_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_PWR_OFF;
// // 	    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_SENSE_INFO_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_SENSE_INFO;
// // 	    /**************************************************************************************************************************/
// // 	    return REPORT_KIND_SC_GENERIC;
// // 	}

// // typedef struct
// // {
// //     uint8_t bytes[512];
// // } RptGenericPayload_t;

// // static_assert(sizeof(EPS_Query_Report_Payload_t) == 512, "EPS_Query_Report_Payload_t size mismatch");

// // #pragma pack(push, 1)

// // typedef struct {
// //     uint8_t  ch_idx;
// //     uint8_t  mode;
// //     uint16_t on_cnt;
// //     uint16_t off_cnt;
// //     uint16_t cur_lu_lim;
// //     uint16_t cur_lim;
// //     uint16_t voltage;
// //     int16_t  current;
// //     uint16_t latchup;
// //     char     name[8];
// // }__attribute__((__packed__)) EPS_PowerIfStatus_Report_t;

// // typedef struct {
// //     uint8_t ch_idx;
// //     uint8_t mode;
// //     char    name[8];
// // } EPS_PowerIfListItem_Report_t;

// // typedef struct {
// //     uint8_t cmd;
// //     uint8_t status;
// //     uint8_t count;
// //     EPS_PowerIfListItem_Report_t list[24];
// // } EPS_PowerIfList_Report_t;

// // typedef struct {
// //     uint32_t uptime;
// //     uint32_t bootcause;
// //     uint16_t resetcause;
// //     uint16_t bootcount;
// //     uint16_t batt_v;
// //     int16_t  batt_i;
// //     uint8_t  batt_mode;
// //     uint8_t  spare1;
// //     uint16_t vbat_v;
// //     uint16_t vcc_v;
// //     int16_t  temp[2];
// //     uint8_t  out_en[6];
// //     int16_t  out_i[6];
// //     uint8_t  sm_en[8];
// //     uint16_t gnd_wdt_cnt;
// //     uint16_t bus_wdt_cnt;
// //     uint32_t gnd_wdt_left;
// //     uint32_t bus_wdt_left;
// // } EPS_P80_PMU_HK_Report_t;

// // typedef struct {
// //     uint32_t uptime;
// //     uint32_t bootcause;
// //     uint32_t bootcount;
// //     uint16_t resetcause;
// //     uint16_t vcc_v;
// //     uint16_t vcc_i;
// //     uint16_t vbat_v;
// //     int16_t  temp;
// //     uint8_t  batt_mode;
// //     uint8_t  out_en[24];
// //     uint8_t  spare1;
// //     int16_t  out_i[24];
// //     uint32_t gnd_wdt_cnt;
// //     uint32_t bus_wdt_cnt;
// //     uint32_t gnd_wdt_left;
// //     uint32_t bus_wdt_left;
// // } EPS_P80_PDU_HK_Report_t;

// // typedef struct {
// //     uint32_t uptime;
// //     uint32_t bootcause;
// //     uint32_t bootcount;
// //     uint16_t resetcause;
// //     int16_t  input_i[6];
// //     uint16_t input_v[6];
// //     uint16_t vcc_v;
// //     uint16_t vbat_v;
// //     int16_t  temp[3];
// //     uint8_t  mppt_mode;
// //     uint8_t  spare1;
// //     uint32_t gnd_wdt_cnt;
// //     uint32_t gnd_wdt_left;
// // } EPS_P80_ACU_HK_Report_t;

// // typedef struct {
// //     uint32_t Uptime;
// //     uint16_t BootCount;
// //     uint16_t BootCause;
// //     uint16_t ResetCause;
// //     uint16_t Vbat;
// //     float    Soc;
// //     float    Current;
// //     uint16_t InCurrent;
// //     uint16_t OutCurrent;
// //     uint16_t HeaterCurrent;
// //     int16_t  IntTemp;
// //     float    BatAvrTemp;
// //     int16_t  BatTemp[4];
// //     uint16_t OVoltCount;
// //     uint8_t  BatFault;
// //     uint8_t  spare2;
// // } EPS_BP8_HK_Report_t;

// // #pragma pack(pop)

// // static_assert(sizeof(EPS_PowerIfStatus_Report_t) == 24, "EPS_PowerIfStatus_Report_t size mismatch");
// // static_assert(sizeof(EPS_PowerIfListItem_Report_t) == 10, "EPS_PowerIfListItem_Report_t size mismatch");
// // static_assert(sizeof(EPS_PowerIfList_Report_t) == 243, "EPS_PowerIfList_Report_t size mismatch");
// // static_assert(sizeof(EPS_P80_PMU_HK_Report_t) == 64, "EPS_P80_PMU_HK_Report_t size mismatch");
// // static_assert(sizeof(EPS_P80_PDU_HK_Report_t) == 112, "EPS_P80_PDU_HK_Report_t size mismatch");
// // static_assert(sizeof(EPS_P80_ACU_HK_Report_t) == 58, "EPS_P80_ACU_HK_Report_t size mismatch");
// // static_assert(sizeof(EPS_BP8_HK_Report_t) == 44, "EPS_BP8_HK_Report_t size mismatch");

// // typedef struct {
// //     bool     valid;
// //     uint8_t  command_code;
// //     int32_t  return_code;
// //     uint16_t return_data_size;
// //     uint8_t  source;
// //     uint8_t  data_id;
// //     uint8_t  arg0;
// //     uint8_t  arg1;
// //     uint16_t sequence;
// //     uint16_t offset;
// //     uint16_t total_size;
// //     uint16_t chunk_size;
// // } EPS_Report_Metadata_t;

// // typedef struct {
// //     bool power_if_status_valid;
// //     bool power_if_list_valid;
// //     bool pmu_hk_valid;
// //     bool pdu_hk_valid;
// //     bool acu1_hk_valid;
// //     bool acu2_hk_valid;
// //     bool bp8_hk_valid;

// //     EPS_Report_Metadata_t power_if_status_meta;
// //     EPS_Report_Metadata_t power_if_list_meta;
// //     EPS_Report_Metadata_t pmu_hk_meta;
// //     EPS_Report_Metadata_t pdu_hk_meta;
// //     EPS_Report_Metadata_t acu1_hk_meta;
// //     EPS_Report_Metadata_t acu2_hk_meta;
// //     EPS_Report_Metadata_t bp8_hk_meta;

// //     EPS_PowerIfStatus_Report_t power_if_status;
// //     EPS_PowerIfList_Report_t   power_if_list;
// //     EPS_P80_PMU_HK_Report_t    pmu_hk;
// //     EPS_P80_PDU_HK_Report_t    pdu_hk;
// //     EPS_P80_ACU_HK_Report_t    acu1_hk;
// //     EPS_P80_ACU_HK_Report_t    acu2_hk;
// //     EPS_BP8_HK_Report_t        bp8_hk;
// // } EPS_Report_State_t;



// // typedef struct __attribute__((__packed__)) gs_gssb_ant6_release_status_t {
// //     /**
// //        Burn state of the first channel (Burning = 1, Idle = 0)
// //      */
// //     uint8_t channel_0_state;
// //     /**
// //        Release status of the first channel (Released = 1, Not released = 0)
// //      */
// //     uint8_t channel_0_status;
// //     /**
// //        Burn time left of the first channel [s]
// //      */
// //     uint8_t channel_0_burn_time_left;
// //     /**
// //        Counter of have many burns there has been attempted
// //      */
// //     uint8_t channel_0_burn_tries;
// //     /**
// //        Burn state of the second channel (Burning = 1, Idle = 0)
// //      */
// //     uint8_t channel_1_state;
// //     /**
// //        Release status of the second channel (Released = 1, Not released = 0)
// //      */
// //     uint8_t channel_1_status;
// //     /**
// //        Burn time left of the second channel [s]
// //      */
// //     uint8_t channel_1_burn_time_left;
// //     /**
// //        Counter of have many burns there has been attempted
// //      */
// //     uint8_t channel_1_burn_tries;
// // } gs_gssb_ant6_release_status_t;


// // typedef struct EPS_P60_DOCK_GET_TABLE_HK {

// //     int16_t   c_out[13];
// //     uint16_t  v_out[13];
// //     uint8_t   out_en[13];

// //     int16_t   temp[2];

// //     uint32_t  bootcause;
// //     uint32_t  bootcnt;
// //     uint32_t  uptime;

// //     uint16_t  resetcause;

// //     uint8_t   batt_mode;
// //     uint8_t   heater_on;
// //     uint8_t   conv_5v_en;

// //     uint16_t  latchup[13];

// //     uint16_t  vbat_v;
// //     int16_t   vcc_c;
// //     int16_t   batt_c;
// //     uint16_t  batt_v;

// //     int16_t   batt_temp[2];

// //     uint8_t   device_type[8];
// //     uint8_t   device_status[8];

// //     uint8_t   dearm_status;

// //     uint32_t  wdt_cnt_gnd;
// //     uint32_t  wdt_cnt_i2c;
// //     uint32_t  wdt_cnt_can;
// //     uint32_t  wdt_cnt_csp[2];

// //     uint32_t  wdt_gnd_left;
// //     uint32_t  wdt_i2c_left;
// //     uint32_t  wdt_can_left;

// //     uint8_t   wdt_csp_left[2];

// //     int16_t   batt_chrg;
// //     int16_t   batt_dischrg;

// //     int8_t    ant6_depl;
// //     int8_t    ar6_depl;

// // } EPS_P60_DOCK_GET_TABLE_HK;

// // typedef struct EPS_P60_PDU_GET_TABLE_HK {

// //     int16_t   c_out[9];
// //     uint16_t  v_out[9];

// //     uint16_t  vcc;
// //     uint16_t  vbat;
// //     int16_t   temp;

// //     uint8_t   conv_en[3];
// //     uint8_t   out_en[9];

// //     uint32_t  bootcause;
// //     uint32_t  bootcnt;
// //     uint32_t  uptime;

// //     uint16_t  resetcause;

// //     uint8_t   batt_mode;

// //     uint16_t  latchup[9];

// //     uint8_t   device_type[8];
// //     uint8_t   device_status[8];

// //     uint32_t  wdt_cnt_gnd;
// //     uint32_t  wdt_cnt_i2c;
// //     uint32_t  wdt_cnt_can;
// //     uint32_t  wdt_cnt_csp[2];

// //     uint32_t  wdt_gnd_left;
// //     uint32_t  wdt_i2c_left;
// //     uint32_t  wdt_can_left;

// //     uint8_t   wdt_csp_left[2];

// // } EPS_P60_PDU_GET_TABLE_HK;


// // typedef struct EPS_P60_ACU_GET_TABLE_HK {

// //     int16_t   c_in[6];
// //     uint16_t  v_in[6];

// //     uint16_t  vbat;
// //     uint16_t  vcc;

// //     int16_t   temp[3];

// //     uint8_t   mppt_mode;

// //     uint16_t  vboost[6];
// //     uint16_t  power[6];

// //     uint8_t   dac_en[3];
// //     uint16_t  dac_val[6];

// //     uint32_t  bootcause;
// //     uint32_t  bootcnt;
// //     uint32_t  uptime;

// //     uint16_t  resetcause;

// //     uint16_t  mppt_time;
// //     uint16_t  mppt_period;

// //     uint8_t   device_type[8];
// //     uint8_t   device_status[8];

// //     uint32_t  wdt_cnt_gnd;
// //     uint32_t  wdt_gnd_left;

// // } EPS_P60_ACU_GET_TABLE_HK;



// // typedef struct
// // {
// //     bool     valid;
// //     uint16_t CCMessage_ID;
// //     uint16_t CCCount;
// //     uint16_t CCLength;
// //     uint8_t  CCTime_code[6];

// //     uint16_t reflected_msg_id;
// //     uint8_t  reflected_cc;
// //     uint8_t  ret_type;
// //     int32_t  ret_code;
// // 	    uint16_t ret_val_size;

// // 	    ReportKind_t kind;
// // 	    EPS_Report_Metadata_t eps_meta;

// // 	    union
// // 	    {
// //         RptGenericPayload_t    generic;


// //         ADCS_TlmLogInclMaskTlm_Payload_t       adcs_logmask;
// //         ADCS_UnsolicitTlmMsgSetupTlm_Payload_t adcs_unsolicited_tlm_tlm;

// // 	        gs_gssb_ant6_release_status_t          uant_getstatus;
// // 	        EPS_P60_DOCK_GET_TABLE_HK              eps_p60dockgettablehk;
// // 	        EPS_P60_PDU_GET_TABLE_HK               eps_p60pdugettablehk;
// // 	        EPS_P60_ACU_GET_TABLE_HK               eps_p60acugettablehk;
// // 	        EPS_PowerIfStatus_Report_t             eps_power_if_status;
// // 	        EPS_PowerIfList_Report_t               eps_power_if_list;
// // 	        EPS_P80_PMU_HK_Report_t                eps_p80_pmu_hk;
// // 	        EPS_P80_PDU_HK_Report_t                eps_p80_pdu_hk;
// // 	        EPS_P80_ACU_HK_Report_t                eps_p80_acu_hk;
// // 	        EPS_BP8_HK_Report_t                    eps_bp8_hk;
// // /************************************************************************************************************************************************************* */
// //         ADCS_ErrorLogSettingTlm_Payload_t      adcs_errorlogsetting;
// //         ADCS_CurrentUnixTimeTlm_Payload_t      adcs_currentunixtime;
// //         ADCS_PersistConfigDiagnosticTlm_Payload_t adcs_persistconfigdiagnostic;
// //         ADCS_CommunicationStatusTlm_Payload_t  adcs_communicationstatus;
// //         ADCS_ControlEstimationModeTlm_Payload_t adcs_controlestimationmode;
// //         ADCS_ReferenceIRCVectorTlm_Payload_t    adcs_referenceircvector;
// //         ADCS_ReferenceLLHTargetTlm_Payload_t    adcs_referencellhtarget;
// //         ADCS_OrbitModeTlm_Payload_t             adcs_orbitmode;
// //         ADCS_HealthTlmMMTTlm_Payload_t          adcs_healthtlmmmt;
// //         ADCS_RawCubeSenseSunTlm_Payload_t       adcs_rawcubesensesun;
// //         ADCS_ReferenceRPYvaluesTlm_Payload_t    adcs_referencerpyvalues;
// //         ADCS_OpenLoopCmdMTQTlm_Payload_t        adcs_openloopcmdmtq;
// //         ADCS_PowerStateTlm_Payload_t            adcs_powerstate;
// //         ADCS_RunModeTlm_Payload_t               adcs_runmode;
// //         ADCS_ControlModeTlm_Payload_t           adcs_controlmode;
// //         ADCS_Mag0MMTCalibConfigTlm_Payload_t    adcs_mag0mmtcalibconfig;
// //         ADCS_Mag1MMTCalibConfigTlm_Payload_t    adcs_mag1mmtcalibconfig;
// //         ADCS_EstimationModeTlm_Payload_t        adcs_estimationmode;
// //         ADCS_OperationalStateTlm_Payload_t      adcs_operationalstate;
// //         ADCS_RawCSSSensorTlm_Payload_t          adcs_rawcsssensor;
// //         ADCS_RawGYRSensorTlm_Paylaod_t          adcs_rawgyrsensor;
// //         ADCS_CalibratedGYRSensorTlm_Payload_t   adcs_calibratedgyrsensor;
// //         ADCS_MagSensingElmConfigTlm_Payload_t   adcs_magsensingelmconfig;
// //         ADCS_TlmLogInclMaskTlm_Payload_t        adcs_tlmloginclmask;
// //         ADCS_UnsolicitTlmMsgSetupTlm_Payload_t  adcs_unsolicittlmmsgsetup;
// //         ADCS_UnsolicitEventMsgSetupTlm_Payload_t adcs_unsoliciteventmsgsetup;
// //         ADCS_EventLogStatusResponseTlm_Payload_t adcs_eventlogstatusresponse;
// //         ADCS_PortMapTlm_Payload_t               adcs_portmap;
// // /******************************************************************************************************************************************************************** */

// // /**********************************************5차 추가********************************************** */
// //         // 1. PAYUEL_ROMA
// //         payuel_roma_Noop_tlm_payload_t                     roma_noop;;
// //         payuel_roma_ResetCounters_tlm_payload_t            roma_resetcounters;
// //         payuel_roma_CommTest_tlm_payload_t                 roma_commtest;
// //         payuel_roma_GetSpecificLine_tlm_payload_t          roma_getspecificline;
// //         payuel_roma_GetMultipleLines_tlm_payload_t         roma_getmultiplelines;
// //         payuel_roma_GetLatestLine_tlm_payload_t            roma_getlatestline;
// //         payuel_roma_GetLatest_N_Lines_tlm_payload_t        roma_getlatestNlines;
// //         payuel_roma_SetRouteDefault_tlm_payload_t          roma_setroutedefault;
// //         payuel_roma_ResetRoute_tlm_payload_t               roma_resetroute;
// //         payuel_roma_LoadRoute_tlm_payload_t                roma_loadroute;
// //         payuel_roma_SaveRoute_tlm_payload_t                roma_saveroute;
// //         payuel_roma_SendRoute_tlm_payload_t                roma_sendroute;
// //         payuel_roma_SetRoute_tlm_payload_t                 roma_setroute;
// //         payuel_roma_ParGet_tlm_payload_t                   roma_parget;
// //         payuel_roma_ParSet_tlm_payload_t                   roma_parset;
// //         payuel_roma_ParDefaults_tlm_payload_t              roma_pardefaults;
// //         payuel_roma_ParSave_tlm_payload_t                  roma_parsave;
// //         payuel_roma_ParRestore_tlm_payload_t               roma_parrestore;
// //         payuel_roma_ParLoad_tlm_payload_t                  roma_parload;
// //         payuel_roma_ParSetOOB_tlm_payload_t                roma_parsetOOB;
// //         payuel_roma_SendCommand_tlm_payload_t              roma_sendcommand;

// //         PAYUEL_LGPM_Noop_tlm_payload_t                     lgpm_noop;
// //         PAYUEL_LGPM_ResetCounters_tlm_payload_t            lgpm_resetcounters;
// //         PAYUEL_LGPM_MCU_ALIVE_CHECK_Tlm_Payload            lgpm_mcualivecheck;
// //         PAYUEL_LGPM_3V3PwrOn_tlm_payload_t                 lgpm_3v3pwron;
// //         PAYUEL_LGPM_3V3PwrOff_tlm_payload_t                lgpm_3v3pwroff;
// //         PAYUEL_LGPM_MainBoostSwOn_tlm_payload_t            lgpm_mainboostswon;
// //         PAYUEL_LGPM_MainBoostSwOff_tlm_payload_t           lgpm_mainboostswoff;
// //         PAYUEL_LGPM_SubBoostSwOn_tlm_payload_t             lgpm_subboostswon;
// //         PAYUEL_LGPM_SubBoostSwOff_tlm_payload_t            lgpm_subboostswoff;
// //         PAYUEL_LGPM_V28MainOn_tlm_payload_t                lgpm_v28mainon;
// //         PAYUEL_LGPM_V28MainOff_tlm_payload_t               lgpm_v28mainoff;
// //         PAYUEL_LGPM_V28SubOn_tlm_payload_t                 lgpm_v28subon;
// //         PAYUEL_LGPM_V28SubOff_tlm_payload_t                lgpm_v28suboff;
// //         PAYUEL_LGPM_V12MainOn_tlm_payload_t                lgpm_v12mainon;
// //         PAYUEL_LGPM_V12MainOff_tlm_payload_t               lgpm_v12mainoff;
// //         PAYUEL_LGPM_PwrSenseInfo_tlm_payload_t             lgpm_pwrsenseinfo;
// //         PAYUEL_LGPM_PwrSeqOn_tlm_payload_t                 lgpm_pwrseqon;
// //         PAYUEL_LGPM_PwrSeqOff_tlm_payload_t                lgpm_pwrseqoff;
// //         PAYUEL_LGPM_RwaControlIdx1_tlm_payload_t           lgpm_rwacontrol_idx1;
// //         PAYUEL_LGPM_RwaControlIdx2_tlm_payload_t           lgpm_rwacontrol_idx2;
// //         PAYUEL_LGPM_RwaControlIdx3_tlm_payload_t           lgpm_rwacontrol_idx3;
// //         PAYUEL_LGPM_RwaPwrOn_tlm_payload_t                 lgpm_rwapwron;
// //         PAYUEL_LGPM_RwaPwrOff_tlm_payload_t                lgpm_rwapwroff;
// //         PAYUEL_LGPM_RwaSenseInfo_tlm_payload_t             lgpm_rwasenseinfo;
// // /**************************************************************************************************** */

// //     } u;
// // } ReportView_t;


// // extern std::mutex g_report_view_mtx;
// // extern ReportView_t g_report_view;
// // extern EPS_Report_State_t g_eps_report_state;


// // typedef struct {
// //     uint8 Callsign[6];
// //     uint8 CurrentMode;
// //     uint8 CurrentSubmode;
// //     uint8 PrevioudMode;
// //     uint8 PreviousSubmode;
// //     uint8 CurrentModeFlag;
// //     uint8 PreviousModeFlag;
// //     uint32 ApplicationRunStatus;
// //     uint32 SatelliteTime;
// //     uint16 RebootCount;
// //     uint8 RebootCause;

// // }__attribute__((packed)) FM_HK_;

// // typedef struct {
// // 	uint16 DeployState_UANT;
// // }__attribute__((packed)) UANT_;

// // typedef struct {
// // 	uint32 rxfreq;
// // 	uint32 txfreq;
// // 	int16 LastRssi;
// //     uint32 TotRxBytes;
// //     uint8 StatusConfiguration;
// // }__attribute__((packed)) UTRX_;

// // typedef struct {
// // 	//EPS - P60 Dock
// // 	uint8 out_en_dock[7]; //01458910
// //     int16 temp_dock[2];
// //     uint32 bootcause;
// //     uint32 bootcnt;
// //     uint32 uptime;
// //     uint16 resetcause;
// //     uint8 batt_mode;
// //     uint8 heater_on;
// //     uint16 latchup_dock[7]; //01458910
// //     uint16 vbat_v;
// //     int16 batt_v;
// //     int16 batt_temp[2];
// //     uint8 device_status[8];
// //     uint32 wdt_cnt_gnd;
// //     uint32 wdt_gnd_left;
// //     int16 batt_chrg;
// //     int16 batt_dischrg;
// // 	//EPS - PDU
// //     int16 vbat;
// //     uint8 out_en_pdu[6]; //034758
// //     uint16 latchup_pdu[6];
// //     uint16 out_voltage[6]; // 034758
// // 	//EPS - ACU
// //     int16 c_in[4]; //0123
// //     uint16 v_in[4]; //0123
// // }__attribute__((packed)) EPS_;

// // typedef struct {
// // 	uint8 RWL0_PowerState;
// // 	uint8 RWL1_PowerState;
// // 	uint8 RWL2_PowerState;
// // 	uint8 MAG0_PowerState;
// // 	uint8 FSS0_PowerState;
// // 	uint8 HSS0_PowerState;
// // 	uint8 Control_Mode;
// // 	uint16 Mag_Control_Timeout;
// // 	float GYRO_Calib_rate_X;
// // 	float GYRO_Calib_rate_Y;
// // 	float GYRO_Calib_rate_Z;
// // }__attribute__((packed)) ADCS_;

// // typedef struct {
// // 	uint8 Status;
// // 	int16 Board_Temperature;
// // 	int16 Battery_Current;
// // 	int16 Battery_Voltage;
// // }__attribute__((packed)) STX_;

// // typedef struct {
// // 	int16 temp_PAYC;
// // 	uint16 icore;
// // }__attribute__((packed)) PAYC_;

// // typedef struct {
// // 	uint8 DeployStatus_PAYR;
// // }__attribute__((packed)) PAYR_;

// // typedef struct {
// // 	uint8 PAYS_State;
// // 	uint8 PAYS_Sign;
// // 	uint8 PAYS_Temp;
// // }__attribute__((packed)) PAYS_;


// // // typedef struct {
// // // 	CCSDS_Header_ CCSDS_Header;
// // // 	FM_HK_ FM;
// // // 	EPS_ EPS;
// // // 	TCS_ TCS;
// // // 	RWA_ RWA;
// // // 	MTQ_ MTQ;
// // // 	SNSR_ SNSR;
// // // 	UTRX_ UTRX;
// // // 	STX_ STX;
// // // 	PAY_ PAY;
// // // }__attribute__((packed)) HK;

// // // typedef struct {
// // // 	FM_HK_ FM;
// // // 	ADCS_ ADCS;
// // // }__attribute__((packed)) AOD;

// // typedef struct {
// // 	uint32_t ExTime;
// // 	uint32_t ExWindow;
// // 	uint16_t EntryID;
// // 	uint16_t GroupID;
// // 	uint8_t cmd[];
// // }__attribute__((packed)) Book;



// // static bool ParseReportWire540(const uint8_t *buf, size_t len, Report &out);
// // void * TRxController(void *);
// // void * SignalTest(void*);
// // void now_rx_bytes_update();
// // void set_rx_bytes(uint32_t nowbytes);
// // uint32_t get_rx_bytes();
// // uint32_t * get_rx_bytes_address();
// // uint16_t get_boot_count();
// // uint16_t * get_boot_count_address();
// // void buf_allclear();
// // void CalculateChecksum(CommandHeader_t* Cmd);
// // int32_t GenerateCmdMsg(CommandHeader_t* Cmd, uint16_t MsgId, uint8_t FcnCode, uint32_t ArgLen);
// // csp_socket_t *  DL_sock_initialize();
// // extern BEE1000_Beacon_t* bee1000_beacon;
// // extern BEE1012_Beacon_t* bee1012_beacon;
// // extern UELYSYS_Beacon_t* uelysys_beacon;
// // int BEE1000BeaconSaver(BEE1000_Beacon_t * bec);
// // int BEE1012BeaconSaver(BEE1012_Beacon_t * bec);
// // int UELYSYSBeaconSaver(UELYSYS_Beacon_t * bec);
// // void * task_downlink_onorbit(void * socketinfo);
// // void * task_uplink_onorbit(void * sign_);

// // int PacketHandler(csp_packet_t *packet, int type, int NowCursor);
// // packetsign * PingInit(FSWTle * FSWTleinfo);
// // csp_packet_t * PacketEncoder(packetsign * sign, bool freeer = true);
// // packetsign * PacketDecoder(csp_packet_t * packet);


// // class CmdGenerator_GS {
// // private:
// //     void SetHeaderWord(uint8_t* Word16, uint16_t Value, uint16_t Mask);
// //     void SetHeaderByte(uint8_t* Byte, uint8_t Value, uint8_t Mask);
// //     void GetHeaderWord(const uint8_t* Word16, uint16_t& Value, uint16_t Mask);
// //     uint32_t ComputeCheckSum(void);

// // public:
// // 	CFE_MSG_CommandHeader* CmdHeader;
// // 	bool Scheduled = false;
// // 	bool Checksum = true;

// //     CmdGenerator_GS(void);
// //     ~CmdGenerator_GS(void);

// //     int GenerateCmdHeader(uint32_t MsgId, uint16_t FncCode, uint32_t Size, void* Data);
// //     void CopyCmdHeaderToBuffer(uint8_t* Buffer);

// //     void InitHeader(void);
// //     void SetHeader(const CFE_MSG_CommandHeader* Header);
// //     const CFE_MSG_CommandHeader* GetHeader(void) const;

// //     int SetHasSecondaryHeader(bool HasSec);
// //     int SetMsgId(uint16_t MsgId);
// //     int SetSize(uint16_t Size);
// //     int SetSegmentationFlag(uint16_t SegFlag);
// //     int SetFncCode(uint16_t FncCode);

// //     bool HasSecondaryHeader(void) const;
// //     uint16_t GetSize(void);
// //     uint16_t GetFncCode(void) const;

// //     int GenerateChecksum(void);
// // 	int Scheduling(uint32_t ExecutionTime, uint32_t ExecutionWindow, uint32_t EntryID, uint16_t GroupID);
// // 	packetsign * GenerateCMDPacket(void);
// // };

// // void * Direct_Shell(void * data);

// // #endif _MIMAN_COMS_H_


// #pragma once
// #ifndef _MIMAN_COMS_H_
// #define _MIMAN_COMS_H_

// #include "miman_config.h"
// #include "miman_orbital.h"
// #include <mutex>

// #pragma once
// #include <mutex>






// typedef struct{
// 	uint16_t Identifier;
// 	uint16_t PacketType;
// 	uint32_t Length;
// 	uint8_t Data[];

// }__attribute__((packed)) packetsign;

// typedef struct{
// 	uint16_t target;
// 	uint16_t filestatus;
// 	uint32_t filenum;
// 	uint32_t offset;
// 	uint32_t step;
// }__attribute__((packed)) dlreqdata;

// typedef struct{
// 	int32_t filenum;
// 	uint32_t file[];
// }__attribute__((packed)) filelist;

// // typedef enum {
// //     GS_FTP_INFO_FILE  = 0,   /**< File size and checksum */
// //     GS_FTP_INFO_CRC = 1,         /**< CRC of remote and local file */
// //     GS_FTP_INFO_COMPLETED = 2,   /**< Completed and total chunks */
// //     GS_FTP_INFO_PROGRESS = 3,    /**< Current chunk, total_chunks and chunk_size */
// // } gs_ftp_info_type_t;

// typedef struct {
//     FILE       * fp;
//     FILE       * fp_map;
//     csp_conn_t * conn;
//     uint32_t   timeout;
//     char       file_name[GS_FTP_PATH_LENGTH];
//     uint32_t   file_size;
//     uint32_t   chunks;
//     int        chunk_size;
//     uint32_t   checksum;
//     ftp_status_element_t last_status[GS_FTP_STATUS_CHUNKS];
//     uint32_t   last_entries;
//     gs_ftp_info_callback_t info_callback;
//     void       * info_data;
// } gs_ftp_state_t;

// typedef struct {
//     gs_ftp_backend_type_t backend;
//     const char * path;
//     uint32_t addr;
//     uint32_t size;
// } gs_ftp_url_t;



// // BEE-1000 RPT structure
// typedef struct {
//     bool valid;
//     uint16_t CCMessage_ID;
//     uint16_t CCCount;
//     uint16_t CCLength;
//     uint8_t CCTime_code[6];
//     uint16_t msg_id;
//     uint8_t cc;
//     uint8_t ret_type;
//     int32_t ret_code;
//     uint16_t ret_val_size;
//     std::vector<uint8_t> payload;
// } ReportPacket_t;

// extern ReportPacket_t g_last_report;


// // BEE-1000 Beacon
// typedef struct {

// 	// CCSDS Header
//     uint8 CCSDS_MID[2];
//     uint8 CCSDS_Seq[2];
//     uint8 CCSDS_Len[2];
//     uint8 CCSDS_TimeCode[6];

// 	// FSW - RPT - shoud be deprecated
// 	// uint16 RPT_BootCount;
//     // uint32 RPT_ScTimeSec;
//     // uint32 RPT_ScTimeSubsec;
//     // uint32 RPT_Sequence;
//     // uint8 RPT_ResetCause;

//     /* FSW - RPT - Revised (Kweon Hyeokjin) */
//     uint8_t RPT_CmdCounter;
//     uint8_t RPT_ErrCounter;
//     uint8_t RPT_ReportCnt;
//     uint8_t RPT_CriticalCnt;
//     uint16_t RPT_BootCount;
//     uint32_t RPT_ScTimeSec;
//     uint32_t RPT_ScTimeSubsec;
//     uint8_t RPT_Sequence_LSB;
//     /*--------------End of BEE RPT Revision (Kweon Hyeokjin)--------------*/

// 	// COMS - STX
//     // all param
//     uint8_t  STX_symbol_rate;
//     uint8_t  STX_transmit_power;
//     uint8_t  STX_modcod;
//     uint8_t  STX_roll_off;
//     uint8_t  STX_pilot_signal;
//     uint8_t  STX_fec_frame_size;
//     uint16_t STX_pretransmission_delay;
//     float    STX_center_frequency;
//     // modulation interface -> cpu temp 빠짐
//     uint8_t STX_modulator_interface_type;  
//     uint8_t STX_lvds_io_type;    
// 	uint8 STX_SystemState;
//     uint8 STX_StatusFlag;
//     // float STX_CpuTemp;

// 	// COMS - UANT
//     uint8 UANT1_Chan0;
//     uint8 UANT1_Chan1;
//     uint8 UANT1_BackupActive;
//     uint8 UANT2_Chan0;
//     uint8 UANT2_Chan1;
//     uint8 UANT2_BackupActive;

// 	// COMS - UTRX
//     uint8 UTRX_ActiveConf;
//     uint16 UTRX_BootCount;
//     uint32 UTRX_BootCause;
//     int16 UTRX_BoardTemp;

//     // PCDU - P60 Dock
//     int16 P60D_Cout[9];
//     uint16 P60D_Vout[9];
//     uint16 P60D_OutEn;
//     uint32 P60D_BootCause;
//     uint32 P60D_BootCount;
//     uint8 P60D_BattMode;
//     uint8 P60D_HeaterOn;
//     uint16 P60D_VbatV;
//     int16 P60D_VccC;
//     uint16 P60D_BattV;
//     int16 P60D_BattTemp[2];
//     uint32 P60D_WdtCanLeft;
//     int16 P60D_BattChrg;
//     int16 P60D_BattDischrg;

//     // PCDU - P60 PDU
//     int16  P60P_Cout[9];
//     uint16 P60P_Vout[9];
//     int16  P60P_Vcc;
//     uint8  P60P_ConvEn;
//     uint16 P60P_OutEn;

//     // PCDU - P60 ACU
//     int16  P60A_Cin[6];
//     uint16 P60A_Vin[6];

//     // ADCS
//        /** Combined Power State
//      *  | 7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
//      *  +--------------------------------------+
//      *  |Rsv|RWL0|RWL1|RWL2|MAG0|GYRO|FSS0|HSS0|
//      *  +--------------------------------------+
//      */
//     uint8 ADCS_PowerState; // ID 183

//     uint8 ADCS_ControlMode; // ID 185

//     float ADCS_GYR0CalibratedRateXComponent;
//     float ADCS_GYR0CalibratedRateYComponent;
//     float ADCS_GYR0CalibratedRateZComponent; // ID 207, 12bytes

// }__attribute__((packed)) BEE1000_Beacon_t;

// #define a sizeof(BEE1000_Beacon_t);
// static_assert(sizeof(BEE1000_Beacon_t) == 203, "BEE1000_Beacon_t size mismatch");

// typedef struct {
//     char call_sign[7];
//     uint8_t msg_id[2];
//     uint8_t sequence[2];
//     uint8_t length[2];
//     uint8_t time_code[6];
// }__attribute__((packed)) BEE1012_BeaconHeader_t;

// typedef struct {
//     uint16_t boot_count;
//     uint32_t sequence;
//     uint8_t reset_cause;
// }__attribute__((packed)) BEE1012_RPT_t;

// typedef struct {
//     uint8_t active_conf;
//     uint16_t boot_count;
//     uint32_t boot_cause;
//     int16_t temp_board;
// }__attribute__((packed)) BEE1012_AX100_t;

// typedef struct {
//     int16_t temp;
//     int8_t connection_quality;
//     uint16_t battery_capacity;
// }__attribute__((packed)) BEE1012_LTRX_t;

// typedef struct {
//     BEE1012_AX100_t ax100;
//     BEE1012_LTRX_t ltrx;
// }__attribute__((packed)) BEE1012_COMS_t;

// typedef struct {
//     uint32_t bootcause;
//     uint16_t resetcause;
//     uint16_t bootcount;
//     uint8_t out_en[6];
//     int16_t temp[2];
//     uint8_t batt_mode;
//     int16_t batt_i;
//     uint16_t batt_v;
//     uint8_t sm_en;
//     uint16_t gnd_wdt_cnt;
//     uint16_t bus_wdt_cnt;
//     uint32_t gnd_wdt_left;
//     uint32_t bus_wdt_left;
// }__attribute__((packed)) BEE1012_EPS_PMU_t;

// typedef struct {
//     int16_t out_i[12];
//     uint8_t out_en[12];
// }__attribute__((packed)) BEE1012_EPS_PDU_t;

// typedef struct {
//     int16_t input_i[6];
//     uint16_t input_v[6];
//     uint8_t mppt_mode;
// }__attribute__((packed)) BEE1012_EPS_ACU_Unit_t;

// typedef struct {
//     BEE1012_EPS_ACU_Unit_t acu[2];
// }__attribute__((packed)) BEE1012_EPS_ACU_t;

// typedef struct {
//     uint16_t bootcount;
//     uint16_t bootcause;
//     uint16_t resetcause;
//     float soc;
//     float bat_avr_temp;
//     uint16_t vbat;
//     float i;
//     uint16_t heater_i;
// }__attribute__((packed)) BEE1012_EPS_BP8_t;

// typedef struct {
//     uint16_t gpio_status;
//     uint8_t sp_deploy_status;
// }__attribute__((packed)) BEE1012_EPS_GPIO_t;

// typedef struct {
//     BEE1012_EPS_PMU_t pmu;
//     BEE1012_EPS_PDU_t pdu;
//     BEE1012_EPS_ACU_t acu;
//     BEE1012_EPS_BP8_t bp8;
//     BEE1012_EPS_GPIO_t gpio;
// }__attribute__((packed)) BEE1012_EPS_t;

// typedef struct {
//     uint8_t power_state;
//     uint8_t control_mode;
//     float gyro0_calibrated_rate_x;
//     float gyro0_calibrated_rate_y;
//     float gyro0_calibrated_rate_z;
//     uint8_t css[6];
// }__attribute__((packed)) BEE1012_ADCS_t;

// typedef struct {
//     BEE1012_BeaconHeader_t header;
//     BEE1012_RPT_t rpt;
//     BEE1012_COMS_t coms;
//     BEE1012_EPS_t eps;
//     BEE1012_ADCS_t adcs;
// }__attribute__((packed)) BEE1012_Beacon_t;

// static_assert(sizeof(BEE1012_BeaconHeader_t) == 19, "BEE1012_BeaconHeader_t size mismatch");
// static_assert(sizeof(BEE1012_RPT_t) == 7, "BEE1012_RPT_t size mismatch");
// static_assert(sizeof(BEE1012_AX100_t) == 9, "BEE1012_AX100_t size mismatch");
// static_assert(sizeof(BEE1012_LTRX_t) == 5, "BEE1012_LTRX_t size mismatch");
// static_assert(sizeof(BEE1012_COMS_t) == 14, "BEE1012_COMS_t size mismatch");
// static_assert(sizeof(BEE1012_EPS_PMU_t) == 36, "BEE1012_EPS_PMU_t size mismatch");
// static_assert(sizeof(BEE1012_EPS_PDU_t) == 36, "BEE1012_EPS_PDU_t size mismatch");
// static_assert(sizeof(BEE1012_EPS_ACU_Unit_t) == 25, "BEE1012_EPS_ACU_Unit_t size mismatch");
// static_assert(sizeof(BEE1012_EPS_ACU_t) == 50, "BEE1012_EPS_ACU_t size mismatch");
// static_assert(sizeof(BEE1012_EPS_BP8_t) == 22, "BEE1012_EPS_BP8_t size mismatch");
// static_assert(sizeof(BEE1012_EPS_GPIO_t) == 3, "BEE1012_EPS_GPIO_t size mismatch");
// static_assert(sizeof(BEE1012_EPS_t) == 147, "BEE1012_EPS_t size mismatch");
// static_assert(sizeof(BEE1012_ADCS_t) == 20, "BEE1012_ADCS_t size mismatch");
// static_assert(sizeof(BEE1012_Beacon_t) == 207, "BEE1012_Beacon_t size mismatch");

// typedef struct {
//     char call_sign[7];
//     uint8_t msg_id[2];
//     uint8_t sequence[2];
//     uint8_t length[2];
//     uint8_t time_code[6];
// }__attribute__((packed)) UELYSYS_BeaconHeader_t;

// typedef struct {
//     uint16_t boot_count;
//     uint32_t sequence;
//     uint8_t reset_cause;
// }__attribute__((packed)) UELYSYS_FSW_t;

// typedef struct {
//     uint8_t i2c1_0x05_channel_0_status;
//     uint8_t i2c1_0x05_channel_1_status;
//     uint8_t i2c1_0x06_channel_0_status;
//     uint8_t i2c1_0x06_channel_1_status;
//     uint8_t ax100_active_conf;
//     uint16_t ax100_boot_count;
//     uint32_t ax100_boot_cause;
//     int16_t ax100_temp_board;
// }__attribute__((packed)) UELYSYS_COMS_t;

// typedef struct {
//     uint32_t bootcause;
//     uint16_t resetcause;
//     uint16_t bootcount;
//     uint8_t out_en[6];
//     int16_t temp[2];
//     uint8_t batt_mode;
//     int16_t batt_i;
//     uint16_t batt_v;
//     uint8_t sm_en;
//     uint16_t gnd_wdt_cnt;
//     uint16_t bus_wdt_cnt;
//     uint32_t gnd_wdt_left;
//     uint32_t bus_wdt_left;
// }__attribute__((packed)) UELYSYS_EPS_PMU_t;

// typedef struct {
//     int16_t out_i[12];
//     uint8_t out_en[12];
// }__attribute__((packed)) UELYSYS_EPS_PDU_t;

// typedef struct {
//     int16_t input_i[6];
//     uint16_t input_v[6];
//     uint8_t mppt_mode;
// }__attribute__((packed)) UELYSYS_EPS_ACU_Unit_t;

// typedef struct {
//     UELYSYS_EPS_ACU_Unit_t acu[2];
// }__attribute__((packed)) UELYSYS_EPS_ACU_t;

// typedef struct {
//     uint16_t bootcount;
//     uint16_t bootcause;
//     uint16_t resetcause;
//     float soc;
//     float bat_avr_temp;
//     uint16_t vbat;
//     float i;
//     uint16_t heater_i;
// }__attribute__((packed)) UELYSYS_EPS_BP8_t;

// typedef struct {
//     uint8_t dsp_i2c1_0x07_status;
//     uint8_t dsp_i2c1_0x08_status;
//     uint8_t dsp_i2c1_0x09_status;
//     uint8_t dsp_i2c1_0x10_status;
// }__attribute__((packed)) UELYSYS_EPS_DSP_t;

// typedef struct {
//     UELYSYS_EPS_PMU_t pmu;
//     UELYSYS_EPS_PDU_t pdu;
//     UELYSYS_EPS_ACU_t acu;
//     UELYSYS_EPS_BP8_t bp8;
//     UELYSYS_EPS_DSP_t dsp;
// }__attribute__((packed)) UELYSYS_EPS_t;

// typedef struct {
//     uint8_t power_state;
//     uint8_t control_mode;
//     float gyro0_calibrated_rate_x;
//     float gyro0_calibrated_rate_y;
//     float gyro0_calibrated_rate_z;
//     uint8_t css[6];
// }__attribute__((packed)) UELYSYS_ADCS_t;

// typedef struct {
//     UELYSYS_BeaconHeader_t header;
//     UELYSYS_FSW_t fsw;
//     UELYSYS_COMS_t coms;
//     UELYSYS_EPS_t eps;
//     UELYSYS_ADCS_t adcs;
// }__attribute__((packed)) UELYSYS_Beacon_t;

// static_assert(sizeof(UELYSYS_BeaconHeader_t) == 19, "UELYSYS_BeaconHeader_t size mismatch");
// static_assert(sizeof(UELYSYS_FSW_t) == 7, "UELYSYS_FSW_t size mismatch");
// static_assert(sizeof(UELYSYS_COMS_t) == 13, "UELYSYS_COMS_t size mismatch");
// static_assert(sizeof(UELYSYS_EPS_PMU_t) == 36, "UELYSYS_EPS_PMU_t size mismatch");
// static_assert(sizeof(UELYSYS_EPS_PDU_t) == 36, "UELYSYS_EPS_PDU_t size mismatch");
// static_assert(sizeof(UELYSYS_EPS_ACU_Unit_t) == 25, "UELYSYS_EPS_ACU_Unit_t size mismatch");
// static_assert(sizeof(UELYSYS_EPS_ACU_t) == 50, "UELYSYS_EPS_ACU_t size mismatch");
// static_assert(sizeof(UELYSYS_EPS_BP8_t) == 22, "UELYSYS_EPS_BP8_t size mismatch");
// static_assert(sizeof(UELYSYS_EPS_DSP_t) == 4, "UELYSYS_EPS_DSP_t size mismatch");
// static_assert(sizeof(UELYSYS_EPS_t) == 148, "UELYSYS_EPS_t size mismatch");
// static_assert(sizeof(UELYSYS_ADCS_t) == 20, "UELYSYS_ADCS_t size mismatch");
// static_assert(sizeof(UELYSYS_Beacon_t) == 207, "UELYSYS_Beacon_t size mismatch");



// typedef struct CFE_SRL_HousekeepingTlm_Payload {
//     uint8 CommandCounter;

//     uint8 CommandErrorCounter;

//     uint8 IOHandleStatus[4];

//     uint16 IOHandleTxCount[4];
    
// }__attribute__((packed)) CFE_SRL_HousekeepingTlm_Payload_t;

// typedef struct RPT_HkTlm_Payload{
//     uint8 CmdCounter;
//     uint8 CmdErrCounter;

//     /**
//      * Queue Info
//      */
//     uint8 ReportQueueCnt;
//     uint8 CriticalQueueCnt;

//     /**
//      * Operation Data
//      */
//     uint16 BootCount;
//     uint32 TimeSec;
//     uint32 TimeSubsec;
//     uint32 Sequence; /* Backup data numbering */

//     /**
//      * Reset Cause
//      */
//     uint8 ResetCause;

// }__attribute__((packed)) RPT_BcnTlm_Payload_t;

// typedef struct PAY_BcnTlm_Payload {
//     uint8 CommandCounter;
//     uint8 CommandErrorCounter;

//     /**
//      * Else ....
//      */
//     /* compact beacon subset */
//     int8  sys_status;         /* payload system status */
//     int16 temp_ntc_0;
//     int16 temp_ntc_1;
//     int16 temp_ntc_2;     
//     int16 temp_ntc_3;
//     int16 temp_ntc_4;         /* board temp 4 */
//     int16 temp_ntc_5;         /* board temp 5 */
//     int16 temp_ntc_6;         /* board temp 6 */
//     int16 temp_ntc_7;         /* board temp 7 */
//     int16 temp_ntc_8;         /* board temp 8 */
//     int16 temp_ntc_9;         /* board temp 9 */
//     int16 temp_ntc_10;        /* board temp 10 */
//     int16 temp_ntc_11;        /* board temp 11 */     


// } PAY_BcnTlm_Payload_t;

// typedef struct PAY_HkTlm_Payload {
//     uint8 CommandCounter;
//     uint8 CommandErrorCounter;

//     int8  sys_status;         /* payload system status */
//     int16 temp_ntc_0;         /* board temp 0 */
//     int16 temp_ntc_1;         /* board temp 1 */
//     int16 temp_ntc_2;         /* board temp 2 */
//     int16 temp_ntc_3;         /* board temp 3 */
//     int16 temp_ntc_4;         /* board temp 4 */
//     int16 temp_ntc_5;         /* board temp 5 */
//     int16 temp_ntc_6;         /* board temp 6 */
//     int16 temp_ntc_7;         /* board temp 7 */
//     int16 temp_ntc_8;         /* board temp 8 */
//     int16 temp_ntc_9;         /* board temp 9 */
//     int16 temp_ntc_10;        /* board temp 10 */
//     int16 temp_ntc_11;        /* board temp 11 */
//     /* Expanded TM (selected currents/sensors) */
//     uint32 sen1_data_0;
//     uint32 sen1_data_1;  

// } PAY_HkTlm_LINPayload_t;

// // BEE-1000 Mission Beacon
// typedef struct {

// 	// Telemetry header
//     uint8 CCSDS_MID[2];
//     uint8 CCSDS_Seq[2];
//     uint8 CCSDS_Len[2];
//     uint8 CCSDS_TimeCode[6];


//     CFE_SRL_HousekeepingTlm_Payload_t srlpayload;

//     RPT_BcnTlm_Payload_t    rptpayload;

//     // Payload
//     PAY_BcnTlm_Payload_t    paybcnpayload;



//     PAY_HkTlm_LINPayload_t     payhkpayload;
    

// }__attribute__((packed)) MissionBeacon;

// #define a sizeof(MissionBeacon);


// struct GETFILEINFO {
//     // ===== TLM Header =====
//     uint8 CCSDS_MID[2];
//     uint8 CCSDS_Seq[2];
//     uint8 CCSDS_Len[2];
//     uint8 CCSDS_TimeCode[6];
//     uint8 padding[4];

//     // ===== Payload =====
//     uint8 FileStatus;
//     uint8 CRC_Computed;
//     uint8 Spare[2];
//     uint32 CRC;
//     uint32 FileSize;
//     uint32 LastModifiedTime;
//     uint32 Mode;
//     char Filename[64];

// };

// #define a sizeof(GETFILEINFO);


// struct Report {
//     // ===== CCSDS Header =====v  16
//     uint16 CCSDS_MsgId;
//     uint16 CCSDS_Seq;
//     uint16 CCSDS_Len;
//     uint8 CCSDS_TimeCode[6];
//     uint32 CCSDS_Padding;


//     // ===== Report Body =====  10     26 byte
//     uint16 ReflectedMID;
//     uint8  ReflectedCC;
//     uint8  RetType;
//     int32  RetCode;
//     uint16 RetValSize;
//     uint8  RetVal[512];

// };

// #define a sizeof(Report)

// struct Event {

//     uint16 CCSDS_MsgId;
//     uint16 CCSDS_Seq;
//     uint16 CCSDS_Len;
//     uint8 CCSDS_TimeCode[6];
//     uint32 CCSDS_Padding;

//     char AppName[20]; /**< 20임   \cfetlmmnemonic \EVS_APPNAME
//                                                 \brief Application name */
//     uint16 EventID;                        /**< \cfetlmmnemonic \EVS_EVENTID
//                                                 \brief Numerical event identifier */
//     uint16 EventType;    /**< uint16임 \cfetlmmnemonic \EVS_EVENTTYPE  
//                                                 \brief Numerical event type identifier */
//     uint32 SpacecraftID;                   /**< \cfetlmmnemonic \EVS_SCID
//                                                 \brief Spacecraft identifier */
//     uint32 ProcessorID;                    /**< \cfetlmmnemonic \EVS_PROCESSORID
//                                                 \brief Numerical processor identifier */


//     char               Message[122]; /**< 122임 \cfetlmmnemonic \EVS_EVENT
//                                                                  \brief Event message string */
//     uint8 Spare1;                                                   /**< \cfetlmmnemonic \EVS_SPARE1
//                                                                          \brief Structure padding */
//     uint8 Spare2;                                                   /**< \cfetlmmnemonic \EVS_SPARE2
//                                                                      \brief Structure padding */

// };

// #define a sizeof(Event)

// typedef enum
// {
//     REPORT_KIND_NONE = 0,

//     // ADCS Sunpointing RPT
//     REPORT_KIND_ADCS_LOG_MASK,
//     REPORT_KIND_ADCS_UNSOLICIT_TLM_SETUP_TLM,

//     REPORT_KIND_UANT_GET_STATUS_TLM,

// 	    REPORT_KIND_EPS_P60_DOCK_GET_TABLE_HK,
// 	    REPORT_KIND_EPS_P60_PDU_GET_TABLE_HK,
// 	    REPORT_KIND_EPS_P60_ACU_GET_TABLE_HK,
// 	    REPORT_KIND_EPS_QUERY_REPORT,
// 	    REPORT_KIND_EPS_POWER_IF_STATUS,
// 	    REPORT_KIND_EPS_POWER_IF_LIST,
// 	    REPORT_KIND_EPS_P80_PMU_HK,
// 	    REPORT_KIND_EPS_P80_PDU_HK,
// 	    REPORT_KIND_EPS_P80_ACU1_HK,
// 	    REPORT_KIND_EPS_P80_ACU2_HK,
// 	    REPORT_KIND_EPS_BP8_HK,
// 	    REPORT_KIND_GPIO_DEP1_EN_ON,
// 	    REPORT_KIND_GPIO_DEP1_EN_OFF,
// 	    REPORT_KIND_GPIO_DEP2_EN_ON,
// 	    REPORT_KIND_GPIO_DEP2_EN_OFF,
// 	    REPORT_KIND_GPIO_SP_IN_READ_5S,

// /****************************************************************************************** */
//     REPORT_KIND_ADCS_GET_ERROR_LOG_SETTING,
//     REPORT_KIND_ADCS_GET_CURRENT_UNIX_TIME,
//     REPORT_KIND_ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC,
//     REPORT_KIND_ADCS_GET_COMMUNICATION_STATUS,
//     REPORT_KIND_ADCS_GET_CONTROL_ESTIMATION_MODE,
//     REPORT_KIND_ADCS_GET_REFERENCE_IRC_VECTOR,
//     REPORT_KIND_ADCS_GET_REFERENCE_LLH_TARGET,
//     REPORT_KIND_ADCS_GET_ORBIT_MODE,
//     REPORT_KIND_ADCS_GET_HEALTH_TLM_MMT,
//     REPORT_KIND_ADCS_GET_RAW_CUBESENSE_SUN,
//     REPORT_KIND_ADCS_GET_REFERENCE_RPY_VALUES,
//     REPORT_KIND_ADCS_GET_OPENLOOPCMD_MTQ,
//     REPORT_KIND_ADCS_GET_POWER_STATE,
//     REPORT_KIND_ADCS_GET_RUN_MODE,
//     REPORT_KIND_ADCS_GET_CONTROL_MODE,
//     REPORT_KIND_ADCS_GET_MAG0_MMT_CALIB_CONFIG,
//     REPORT_KIND_ADCS_GET_MAG1_MMT_CALIB_CONFIG,
//     REPORT_KIND_ADCS_GET_ESTIMATION_MODE,
//     REPORT_KIND_ADCS_GET_OPERATIONAL_STATE,
//     REPORT_KIND_ADCS_GET_RAW_CSS_SENSOR,
//     REPORT_KIND_ADCS_GET_RAW_GYR_SENSOR,
//     REPORT_KIND_ADCS_GET_CALIBRATED_GYR_SENSOR,
//     REPORT_KIND_ADCS_GET_MAG_SENSING_ELM_CONFIG,
//     REPORT_KIND_ADCS_GET_TLM_LOG_INCLMASK,
//     REPORT_KIND_ADCS_GET_UNSOLICIT_TLM_MSG_SETUP,
//     REPORT_KIND_ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP,
//     REPORT_KIND_ADCS_GET_EVENT_LOG_STATUS_RESPONSE,
//     REPORT_KIND_ADCS_GET_PORTMAP,
// /********************************************************************************************* */

// /****************************************5차 추가*************************************** */
//     // 1. PAYUEL_ROMA
//     REPORT_KIND_PAYUEL_ROMA_NOOP,            // CC 0, 1
//     REPORT_KIND_PAYUEL_ROMA_RESETCOUNTERS,
//     REPORT_KIND_PAYUEL_ROMA_COMMTEST,
//     REPORT_KIND_PAYUEL_ROMA_GETSPECIFICLINE,
//     REPORT_KIND_PAYUEL_ROMA_GETMULTIPLELINES,
//     REPORT_KIND_PAYUEL_ROMA_GETLATESTLINE,
//     REPORT_KIND_PAYUEL_ROMA_GETLATESTNLINES,
//     REPORT_KIND_PAYUEL_ROMA_SETROUTEDEFAULT,
//     REPORT_KIND_PAYUEL_ROMA_RESETROUTE,
//     REPORT_KIND_PAYUEL_ROMA_LOADROUTE,
//     REPORT_KIND_PAYUEL_ROMA_SAVEROUTE,
//     REPORT_KIND_PAYUEL_ROMA_SENDROUTE,
//     REPORT_KIND_PAYUEL_ROMA_SETROUTE,
//     REPORT_KIND_PAYUEL_ROMA_PARGET,
//     REPORT_KIND_PAYUEL_ROMA_PARSET,
//     REPORT_KIND_PAYUEL_ROMA_PARDEFAULTS,
//     REPORT_KIND_PAYUEL_ROMA_PARSAVE,
//     REPORT_KIND_PAYUEL_ROMA_PARRESTORE,
//     REPORT_KIND_PAYUEL_ROMA_PARLOAD,
//     REPORT_KIND_PAYUEL_ROMA_PARSETOOB,
//     REPORT_KIND_PAYUEL_ROMA_SENDCOMMAND,


//     // 2. PAYUEL_LGPM
//     REPORT_KIND_PAYUEL_LGPM_NOOP,               // CC 0
//     REPORT_KIND_PAYUEL_LGPM_RESETCOUNTERS,
//     REPORT_KIND_PAYUEL_LGPM_MCU_ALIVE,          // CC 2
//     REPORT_KIND_PAYUEL_LGPM_3V3_PWR_ON,         // CC 3
//     REPORT_KIND_PAYUEL_LGPM_3V3_PWR_OFF,        // CC 4
//     REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_ON,      // CC 5
//     REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_OFF,     // CC 6
//     REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_ON,       // CC 7
//     REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_OFF,      // CC 8
//     REPORT_KIND_PAYUEL_LGPM_V28_MAIN_ON,        // CC 9
//     REPORT_KIND_PAYUEL_LGPM_V28_MAIN_OFF,       // CC 10
//     REPORT_KIND_PAYUEL_LGPM_V28_SUB_ON,         // CC 11
//     REPORT_KIND_PAYUEL_LGPM_V28_SUB_OFF,        // CC 12
//     REPORT_KIND_PAYUEL_LGPM_V12_MAIN_ON,        // CC 13
//     REPORT_KIND_PAYUEL_LGPM_V12_MAIN_OFF,       // CC 14
//     REPORT_KIND_PAYUEL_LGPM_PWR_SENSE_INFO,          // CC 15
//     REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_ON,         // CC 16
//     REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_OFF,        // CC 17
//     REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX1,           // CC 18
//     REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX2,           // CC 19
//     REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX3,           // CC 20
//     REPORT_KIND_PAYUEL_LGPM_RWA_PWR_ON,         // CC 21
//     REPORT_KIND_PAYUEL_LGPM_RWA_PWR_OFF,        // CC 22
//     REPORT_KIND_PAYUEL_LGPM_RWA_SENSE_INFO,          // CC 23
// /****************************************************************************************** */

//     REPORT_KIND_SC_GENERIC,

// } ReportKind_t;

// static ReportKind_t DetermineReportKind(uint16_t reflected_mid, uint8_t reflected_cc) {
//     if (reflected_mid == UANT_APP_CMD_ID && reflected_cc == UANT_APP_GET_STATUS_CC) return REPORT_KIND_UANT_GET_STATUS_TLM;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518)  && reflected_cc == ADCS_GET_TLM_LOG_INCLMASK_CC) return REPORT_KIND_ADCS_LOG_MASK;
    
// /**************************************************************************************************************************************************************************************** */
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_ERROR_LOG_SETTING_CC) return REPORT_KIND_ADCS_GET_ERROR_LOG_SETTING;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CURRENT_UNIX_TIME_CC) return REPORT_KIND_ADCS_GET_CURRENT_UNIX_TIME;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC_CC) return REPORT_KIND_ADCS_GET_PERSIST_CONFIG_DIAGNOSTIC;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_COMMUNICATION_STATUS_CC) return REPORT_KIND_ADCS_GET_COMMUNICATION_STATUS;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CONTROL_ESTIMATION_MODE_CC) return REPORT_KIND_ADCS_GET_CONTROL_ESTIMATION_MODE;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_REFERENCE_IRC_VECTOR_CC) return REPORT_KIND_ADCS_GET_REFERENCE_IRC_VECTOR;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_REFERENCE_LLH_TARGET_CC) return REPORT_KIND_ADCS_GET_REFERENCE_LLH_TARGET;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_ORBIT_MODE_CC) return REPORT_KIND_ADCS_GET_ORBIT_MODE;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_HEALTH_TLM_MMT_CC) return REPORT_KIND_ADCS_GET_HEALTH_TLM_MMT;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RAW_CUBESENSE_SUN_CC) return REPORT_KIND_ADCS_GET_RAW_CUBESENSE_SUN;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_REFERENCE_RPY_VALUES_CC) return REPORT_KIND_ADCS_GET_REFERENCE_RPY_VALUES;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_OPENLOOPCMD_MTQ_CC) return REPORT_KIND_ADCS_GET_OPENLOOPCMD_MTQ;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_POWER_STATE_CC) return REPORT_KIND_ADCS_GET_POWER_STATE;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RUN_MODE_CC) return REPORT_KIND_ADCS_GET_RUN_MODE;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CONTROL_MODE_CC) return REPORT_KIND_ADCS_GET_CONTROL_MODE;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_MAG0_MMT_CALIB_CONFIG_CC) return REPORT_KIND_ADCS_GET_MAG0_MMT_CALIB_CONFIG;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_MAG1_MMT_CALIB_CONFIG_CC) return REPORT_KIND_ADCS_GET_MAG1_MMT_CALIB_CONFIG;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_ESTIMATION_MODE_CC) return REPORT_KIND_ADCS_GET_ESTIMATION_MODE;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_OPERATIONAL_STATE_CC) return REPORT_KIND_ADCS_GET_OPERATIONAL_STATE;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RAW_CSS_SENSOR_CC) return REPORT_KIND_ADCS_GET_RAW_CSS_SENSOR;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_RAW_GYR_SENSOR_CC) return REPORT_KIND_ADCS_GET_RAW_GYR_SENSOR;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_CALIBRATED_GYR_SENSOR_CC) return REPORT_KIND_ADCS_GET_CALIBRATED_GYR_SENSOR;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_MAG_SENSING_ELM_CONFIG_CC) return REPORT_KIND_ADCS_GET_MAG_SENSING_ELM_CONFIG;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_TLM_LOG_INCLMASK_CC) return REPORT_KIND_ADCS_GET_TLM_LOG_INCLMASK;
//     if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_UNSOLICIT_TLM_MSG_SETUP_CC) return REPORT_KIND_ADCS_GET_UNSOLICIT_TLM_MSG_SETUP;
// 	    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP_CC) return REPORT_KIND_ADCS_GET_UNSOLICIT_EVENT_MSG_SETUP;
// 	    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_EVENT_LOG_STATUS_RESPONSE_CC) return REPORT_KIND_ADCS_GET_EVENT_LOG_STATUS_RESPONSE;
// 	    if ((reflected_mid == ADCS_CMD_ID || reflected_mid == 0x6518) && reflected_cc == ADCS_GET_PORTMAP_CC) return REPORT_KIND_ADCS_GET_PORTMAP;
// 	/****************************************************************************************************************************************************************************************** */

// 	    if ((reflected_mid == EPS_CMD_ID || reflected_mid == 0x7518) &&
// 	        (reflected_cc == EPS_P80_POWER_IF_GET_CC ||
// 	         reflected_cc == EPS_P80_POWER_IF_LIST_CC ||
// 	         reflected_cc == EPS_GET_HK_ALL_CC)) return REPORT_KIND_EPS_QUERY_REPORT;

// 	    if ((reflected_mid == GPIO_CMD_ID || reflected_mid == 0x9218) && reflected_cc == GPIO_DEP1_EN_ON_CC) return REPORT_KIND_GPIO_DEP1_EN_ON;
// 	    if ((reflected_mid == GPIO_CMD_ID || reflected_mid == 0x9218) && reflected_cc == GPIO_DEP1_EN_OFF_CC) return REPORT_KIND_GPIO_DEP1_EN_OFF;
// 	    if ((reflected_mid == GPIO_CMD_ID || reflected_mid == 0x9218) && reflected_cc == GPIO_DEP2_EN_ON_CC) return REPORT_KIND_GPIO_DEP2_EN_ON;
// 	    if ((reflected_mid == GPIO_CMD_ID || reflected_mid == 0x9218) && reflected_cc == GPIO_DEP2_EN_OFF_CC) return REPORT_KIND_GPIO_DEP2_EN_OFF;
// 	    if ((reflected_mid == GPIO_CMD_ID || reflected_mid == 0x9218) && reflected_cc == GPIO_SP_IN_READ_5S_CC) return REPORT_KIND_GPIO_SP_IN_READ_5S;

// 	    /*********************************************5차 추가****************************************************** */
//     // CC 0, 1
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_NOOP_CC)) return REPORT_KIND_PAYUEL_ROMA_NOOP;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_RESET_COUNTERS_CC)) return REPORT_KIND_PAYUEL_ROMA_RESETCOUNTERS;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_COMM_TEST_CC)) return REPORT_KIND_PAYUEL_ROMA_COMMTEST;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_SPECIFIC_LINE_CC)) return REPORT_KIND_PAYUEL_ROMA_GETSPECIFICLINE;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_MULTIPLE_LINES_CC)) return REPORT_KIND_PAYUEL_ROMA_GETMULTIPLELINES;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_LATEST_LINE_CC)) return REPORT_KIND_PAYUEL_ROMA_GETLATESTLINE;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_GET_LATEST_N_LINES_CC)) return REPORT_KIND_PAYUEL_ROMA_GETLATESTNLINES;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SET_ROUTE_DEFAULT_CC)) return REPORT_KIND_PAYUEL_ROMA_SETROUTEDEFAULT;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_RESET_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_RESETROUTE;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_LOAD_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_LOADROUTE;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SAVE_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_SAVEROUTE;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SEND_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_SENDROUTE;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SET_ROUTE_CC)) return REPORT_KIND_PAYUEL_ROMA_SETROUTE;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_GET_CC)) return REPORT_KIND_PAYUEL_ROMA_PARGET;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_SET_CC)) return REPORT_KIND_PAYUEL_ROMA_PARSET;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_DEFAULTS_CC)) return REPORT_KIND_PAYUEL_ROMA_PARDEFAULTS;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_SAVE_CC)) return REPORT_KIND_PAYUEL_ROMA_PARSAVE;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_RESTORE_CC)) return REPORT_KIND_PAYUEL_ROMA_PARRESTORE;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_LOAD_CC)) return REPORT_KIND_PAYUEL_ROMA_PARLOAD;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAR_SET_OOB_CC)) return REPORT_KIND_PAYUEL_ROMA_PARSETOOB;
//     if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SEND_COMMAND_CC)) return REPORT_KIND_PAYUEL_ROMA_SENDCOMMAND;

//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_NOOP_CC)) return REPORT_KIND_PAYUEL_LGPM_NOOP;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RESET_COUNTERS_CC)) return REPORT_KIND_PAYUEL_LGPM_RESETCOUNTERS;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_MCU_ALIVE_CHECK_CC)) return REPORT_KIND_PAYUEL_LGPM_MCU_ALIVE;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_3V3_PWR_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_3V3_PWR_ON;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_3V3_PWR_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_3V3_PWR_OFF;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_MAIN_BOOST_SW_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_ON;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_MAIN_BOOST_SW_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_MAIN_BOOST_SW_OFF;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_SUB_BOOST_SW_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_ON;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_SUB_BOOST_SW_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_SUB_BOOST_SW_OFF;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_MAIN_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_MAIN_ON;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_MAIN_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_MAIN_OFF;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_SUB_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_SUB_ON;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V28_SUB_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_V28_SUB_OFF;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V12_MAIN_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_V12_MAIN_ON;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_V12_MAIN_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_V12_MAIN_OFF;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_PWR_SENSE_INFO_CC)) return REPORT_KIND_PAYUEL_LGPM_PWR_SENSE_INFO;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_PWR_SEQ_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_ON;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_PWR_SEQ_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_PWR_SEQ_OFF;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_CONTROL_idx1_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX1;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_CONTROL_idx2_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX2;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_CONTROL_idx3_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_CONTROL_IDX3;
//     if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_PWR_ON_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_PWR_ON;
// 	    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_PWR_OFF_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_PWR_OFF;
// 	    if ((reflected_mid == PAYUEL_LGPM_CMD_ID || reflected_mid == 0x3518) && (reflected_cc == PAYUEL_LGPM_RWA_SENSE_INFO_CC)) return REPORT_KIND_PAYUEL_LGPM_RWA_SENSE_INFO;
// 	    /**************************************************************************************************************************/
// 	    return REPORT_KIND_SC_GENERIC;
// 	}

// typedef struct
// {
//     uint8_t bytes[512];
// } RptGenericPayload_t;

// static_assert(sizeof(EPS_Query_Report_Payload_t) == 512, "EPS_Query_Report_Payload_t size mismatch");

// #pragma pack(push, 1)

// typedef struct {
//     uint8_t  ch_idx;
//     uint8_t  mode;
//     uint16_t on_cnt;
//     uint16_t off_cnt;
//     uint16_t cur_lu_lim;
//     uint16_t cur_lim;
//     uint16_t voltage;
//     int16_t  current;
//     uint16_t latchup;
//     char     name[8];
// } __attribute__((packed)) EPS_PowerIfStatus_Report_t;

// typedef struct {
//     uint8_t ch_idx;
//     uint8_t mode;
//     char    name[8];
// } __attribute__((packed)) EPS_PowerIfListItem_Report_t;

// typedef struct {
//     uint8_t cmd;
//     uint8_t status;
//     uint8_t count;
//     EPS_PowerIfListItem_Report_t list[24];
// } __attribute__((packed)) EPS_PowerIfList_Report_t;

// typedef struct {
//     uint32_t uptime;
//     uint32_t bootcause;
//     uint16_t resetcause;
//     uint16_t bootcount;
//     uint16_t batt_v;
//     int16_t  batt_i;
//     uint8_t  batt_mode;
//     uint8_t  spare1;
//     uint16_t vbat_v;
//     uint16_t vcc_v;
//     int16_t  temp[2];
//     uint8_t  out_en[6];
//     int16_t  out_i[6];
//     uint8_t  sm_en[8];
//     uint16_t gnd_wdt_cnt;
//     uint16_t bus_wdt_cnt;
//     uint32_t gnd_wdt_left;
//     uint32_t bus_wdt_left;
// } __attribute__((packed)) EPS_P80_PMU_HK_Report_t;

// typedef struct {
//     uint32_t uptime;
//     uint32_t bootcause;
//     uint32_t bootcount;
//     uint16_t resetcause;
//     uint16_t vcc_v;
//     uint16_t vcc_i;
//     uint16_t vbat_v;
//     int16_t  temp;
//     uint8_t  batt_mode;
//     uint8_t  out_en[24];
//     uint8_t  spare1;
//     int16_t  out_i[24];
//     uint32_t gnd_wdt_cnt;
//     uint32_t bus_wdt_cnt;
//     uint32_t gnd_wdt_left;
//     uint32_t bus_wdt_left;
// } __attribute__((packed)) EPS_P80_PDU_HK_Report_t;

// typedef struct {
//     uint32_t uptime;
//     uint32_t bootcause;
//     uint32_t bootcount;
//     uint16_t resetcause;
//     int16_t  input_i[6];
//     uint16_t input_v[6];
//     uint16_t vcc_v;
//     uint16_t vbat_v;
//     int16_t  temp[3];
//     uint8_t  mppt_mode;
//     uint8_t  spare1;
//     uint32_t gnd_wdt_cnt;
//     uint32_t gnd_wdt_left;
// } __attribute__((packed)) EPS_P80_ACU_HK_Report_t;

// typedef struct {
//     uint32_t Uptime;
//     uint16_t BootCount;
//     uint16_t BootCause;
//     uint16_t ResetCause;
//     uint16_t Vbat;
//     float    Soc;
//     float    Current;
//     uint16_t InCurrent;
//     uint16_t OutCurrent;
//     uint16_t HeaterCurrent;
//     int16_t  IntTemp;
//     float    BatAvrTemp;
//     int16_t  BatTemp[4];
//     uint16_t OVoltCount;
//     uint8_t  BatFault;
//     uint8_t  spare2;
// } __attribute__((packed)) EPS_BP8_HK_Report_t;

// #pragma pack(pop)

// static_assert(sizeof(EPS_PowerIfStatus_Report_t) == 24, "EPS_PowerIfStatus_Report_t size mismatch");
// static_assert(sizeof(EPS_PowerIfListItem_Report_t) == 10, "EPS_PowerIfListItem_Report_t size mismatch");
// static_assert(sizeof(EPS_PowerIfList_Report_t) == 243, "EPS_PowerIfList_Report_t size mismatch");
// static_assert(sizeof(EPS_P80_PMU_HK_Report_t) == 64, "EPS_P80_PMU_HK_Report_t size mismatch");
// static_assert(sizeof(EPS_P80_PDU_HK_Report_t) == 112, "EPS_P80_PDU_HK_Report_t size mismatch");
// static_assert(sizeof(EPS_P80_ACU_HK_Report_t) == 58, "EPS_P80_ACU_HK_Report_t size mismatch");
// static_assert(sizeof(EPS_BP8_HK_Report_t) == 44, "EPS_BP8_HK_Report_t size mismatch");

// typedef struct {
//     bool     valid;
//     uint8_t  command_code;
//     int32_t  return_code;
//     uint16_t return_data_size;
//     uint8_t  source;
//     uint8_t  data_id;
//     uint8_t  arg0;
//     uint8_t  arg1;
//     uint16_t sequence;
//     uint16_t offset;
//     uint16_t total_size;
//     uint16_t chunk_size;
// } EPS_Report_Metadata_t;

// typedef struct {
//     bool power_if_status_valid;
//     bool power_if_list_valid;
//     bool pmu_hk_valid;
//     bool pdu_hk_valid;
//     bool acu1_hk_valid;
//     bool acu2_hk_valid;
//     bool bp8_hk_valid;

//     EPS_Report_Metadata_t power_if_status_meta;
//     EPS_Report_Metadata_t power_if_list_meta;
//     EPS_Report_Metadata_t pmu_hk_meta;
//     EPS_Report_Metadata_t pdu_hk_meta;
//     EPS_Report_Metadata_t acu1_hk_meta;
//     EPS_Report_Metadata_t acu2_hk_meta;
//     EPS_Report_Metadata_t bp8_hk_meta;

//     EPS_PowerIfStatus_Report_t power_if_status;
//     EPS_PowerIfList_Report_t   power_if_list;
//     EPS_P80_PMU_HK_Report_t    pmu_hk;
//     EPS_P80_PDU_HK_Report_t    pdu_hk;
//     EPS_P80_ACU_HK_Report_t    acu1_hk;
//     EPS_P80_ACU_HK_Report_t    acu2_hk;
//     EPS_BP8_HK_Report_t        bp8_hk;
// } EPS_Report_State_t;



// typedef struct __attribute__((__packed__)) gs_gssb_ant6_release_status_t {
//     /**
//        Burn state of the first channel (Burning = 1, Idle = 0)
//      */
//     uint8_t channel_0_state;
//     /**
//        Release status of the first channel (Released = 1, Not released = 0)
//      */
//     uint8_t channel_0_status;
//     /**
//        Burn time left of the first channel [s]
//      */
//     uint8_t channel_0_burn_time_left;
//     /**
//        Counter of have many burns there has been attempted
//      */
//     uint8_t channel_0_burn_tries;
//     /**
//        Burn state of the second channel (Burning = 1, Idle = 0)
//      */
//     uint8_t channel_1_state;
//     /**
//        Release status of the second channel (Released = 1, Not released = 0)
//      */
//     uint8_t channel_1_status;
//     /**
//        Burn time left of the second channel [s]
//      */
//     uint8_t channel_1_burn_time_left;
//     /**
//        Counter of have many burns there has been attempted
//      */
//     uint8_t channel_1_burn_tries;
// } gs_gssb_ant6_release_status_t;


// typedef struct EPS_P60_DOCK_GET_TABLE_HK {

//     int16_t   c_out[13];
//     uint16_t  v_out[13];
//     uint8_t   out_en[13];

//     int16_t   temp[2];

//     uint32_t  bootcause;
//     uint32_t  bootcnt;
//     uint32_t  uptime;

//     uint16_t  resetcause;

//     uint8_t   batt_mode;
//     uint8_t   heater_on;
//     uint8_t   conv_5v_en;

//     uint16_t  latchup[13];

//     uint16_t  vbat_v;
//     int16_t   vcc_c;
//     int16_t   batt_c;
//     uint16_t  batt_v;

//     int16_t   batt_temp[2];

//     uint8_t   device_type[8];
//     uint8_t   device_status[8];

//     uint8_t   dearm_status;

//     uint32_t  wdt_cnt_gnd;
//     uint32_t  wdt_cnt_i2c;
//     uint32_t  wdt_cnt_can;
//     uint32_t  wdt_cnt_csp[2];

//     uint32_t  wdt_gnd_left;
//     uint32_t  wdt_i2c_left;
//     uint32_t  wdt_can_left;

//     uint8_t   wdt_csp_left[2];

//     int16_t   batt_chrg;
//     int16_t   batt_dischrg;

//     int8_t    ant6_depl;
//     int8_t    ar6_depl;

// } EPS_P60_DOCK_GET_TABLE_HK;

// typedef struct EPS_P60_PDU_GET_TABLE_HK {

//     int16_t   c_out[9];
//     uint16_t  v_out[9];

//     uint16_t  vcc;
//     uint16_t  vbat;
//     int16_t   temp;

//     uint8_t   conv_en[3];
//     uint8_t   out_en[9];

//     uint32_t  bootcause;
//     uint32_t  bootcnt;
//     uint32_t  uptime;

//     uint16_t  resetcause;

//     uint8_t   batt_mode;

//     uint16_t  latchup[9];

//     uint8_t   device_type[8];
//     uint8_t   device_status[8];

//     uint32_t  wdt_cnt_gnd;
//     uint32_t  wdt_cnt_i2c;
//     uint32_t  wdt_cnt_can;
//     uint32_t  wdt_cnt_csp[2];

//     uint32_t  wdt_gnd_left;
//     uint32_t  wdt_i2c_left;
//     uint32_t  wdt_can_left;

//     uint8_t   wdt_csp_left[2];

// } EPS_P60_PDU_GET_TABLE_HK;


// typedef struct EPS_P60_ACU_GET_TABLE_HK {

//     int16_t   c_in[6];
//     uint16_t  v_in[6];

//     uint16_t  vbat;
//     uint16_t  vcc;

//     int16_t   temp[3];

//     uint8_t   mppt_mode;

//     uint16_t  vboost[6];
//     uint16_t  power[6];

//     uint8_t   dac_en[3];
//     uint16_t  dac_val[6];

//     uint32_t  bootcause;
//     uint32_t  bootcnt;
//     uint32_t  uptime;

//     uint16_t  resetcause;

//     uint16_t  mppt_time;
//     uint16_t  mppt_period;

//     uint8_t   device_type[8];
//     uint8_t   device_status[8];

//     uint32_t  wdt_cnt_gnd;
//     uint32_t  wdt_gnd_left;

// } EPS_P60_ACU_GET_TABLE_HK;



// typedef struct
// {
//     bool     valid;
//     uint16_t CCMessage_ID;
//     uint16_t CCCount;
//     uint16_t CCLength;
//     uint8_t  CCTime_code[6];

//     uint16_t reflected_msg_id;
//     uint8_t  reflected_cc;
//     uint8_t  ret_type;
//     int32_t  ret_code;
// 	    uint16_t ret_val_size;

// 	    ReportKind_t kind;
// 	    EPS_Report_Metadata_t eps_meta;

// 	    union
// 	    {
//         RptGenericPayload_t    generic;


//         ADCS_TlmLogInclMaskTlm_Payload_t       adcs_logmask;
//         ADCS_UnsolicitTlmMsgSetupTlm_Payload_t adcs_unsolicited_tlm_tlm;

// 	        gs_gssb_ant6_release_status_t          uant_getstatus;
// 	        EPS_P60_DOCK_GET_TABLE_HK              eps_p60dockgettablehk;
// 	        EPS_P60_PDU_GET_TABLE_HK               eps_p60pdugettablehk;
// 	        EPS_P60_ACU_GET_TABLE_HK               eps_p60acugettablehk;
// 	        EPS_PowerIfStatus_Report_t             eps_power_if_status;
// 	        EPS_PowerIfList_Report_t               eps_power_if_list;
// 	        EPS_P80_PMU_HK_Report_t                eps_p80_pmu_hk;
// 	        EPS_P80_PDU_HK_Report_t                eps_p80_pdu_hk;
// 	        EPS_P80_ACU_HK_Report_t                eps_p80_acu_hk;
// 	        EPS_BP8_HK_Report_t                    eps_bp8_hk;
// /************************************************************************************************************************************************************* */
//         ADCS_ErrorLogSettingTlm_Payload_t      adcs_errorlogsetting;
//         ADCS_CurrentUnixTimeTlm_Payload_t      adcs_currentunixtime;
//         ADCS_PersistConfigDiagnosticTlm_Payload_t adcs_persistconfigdiagnostic;
//         ADCS_CommunicationStatusTlm_Payload_t  adcs_communicationstatus;
//         ADCS_ControlEstimationModeTlm_Payload_t adcs_controlestimationmode;
//         ADCS_ReferenceIRCVectorTlm_Payload_t    adcs_referenceircvector;
//         ADCS_ReferenceLLHTargetTlm_Payload_t    adcs_referencellhtarget;
//         ADCS_OrbitModeTlm_Payload_t             adcs_orbitmode;
//         ADCS_HealthTlmMMTTlm_Payload_t          adcs_healthtlmmmt;
//         ADCS_RawCubeSenseSunTlm_Payload_t       adcs_rawcubesensesun;
//         ADCS_ReferenceRPYvaluesTlm_Payload_t    adcs_referencerpyvalues;
//         ADCS_OpenLoopCmdMTQTlm_Payload_t        adcs_openloopcmdmtq;
//         ADCS_PowerStateTlm_Payload_t            adcs_powerstate;
//         ADCS_RunModeTlm_Payload_t               adcs_runmode;
//         ADCS_ControlModeTlm_Payload_t           adcs_controlmode;
//         ADCS_Mag0MMTCalibConfigTlm_Payload_t    adcs_mag0mmtcalibconfig;
//         ADCS_Mag1MMTCalibConfigTlm_Payload_t    adcs_mag1mmtcalibconfig;
//         ADCS_EstimationModeTlm_Payload_t        adcs_estimationmode;
//         ADCS_OperationalStateTlm_Payload_t      adcs_operationalstate;
//         ADCS_RawCSSSensorTlm_Payload_t          adcs_rawcsssensor;
//         ADCS_RawGYRSensorTlm_Paylaod_t          adcs_rawgyrsensor;
//         ADCS_CalibratedGYRSensorTlm_Payload_t   adcs_calibratedgyrsensor;
//         ADCS_MagSensingElmConfigTlm_Payload_t   adcs_magsensingelmconfig;
//         ADCS_TlmLogInclMaskTlm_Payload_t        adcs_tlmloginclmask;
//         ADCS_UnsolicitTlmMsgSetupTlm_Payload_t  adcs_unsolicittlmmsgsetup;
//         ADCS_UnsolicitEventMsgSetupTlm_Payload_t adcs_unsoliciteventmsgsetup;
//         ADCS_EventLogStatusResponseTlm_Payload_t adcs_eventlogstatusresponse;
//         ADCS_PortMapTlm_Payload_t               adcs_portmap;
// /******************************************************************************************************************************************************************** */

// /**********************************************5차 추가********************************************** */
//         // 1. PAYUEL_ROMA
//         payuel_roma_Noop_tlm_payload_t                     roma_noop;;
//         payuel_roma_ResetCounters_tlm_payload_t            roma_resetcounters;
//         payuel_roma_CommTest_tlm_payload_t                 roma_commtest;
//         payuel_roma_GetSpecificLine_tlm_payload_t          roma_getspecificline;
//         payuel_roma_GetMultipleLines_tlm_payload_t         roma_getmultiplelines;
//         payuel_roma_GetLatestLine_tlm_payload_t            roma_getlatestline;
//         payuel_roma_GetLatest_N_Lines_tlm_payload_t        roma_getlatestNlines;
//         payuel_roma_SetRouteDefault_tlm_payload_t          roma_setroutedefault;
//         payuel_roma_ResetRoute_tlm_payload_t               roma_resetroute;
//         payuel_roma_LoadRoute_tlm_payload_t                roma_loadroute;
//         payuel_roma_SaveRoute_tlm_payload_t                roma_saveroute;
//         payuel_roma_SendRoute_tlm_payload_t                roma_sendroute;
//         payuel_roma_SetRoute_tlm_payload_t                 roma_setroute;
//         payuel_roma_ParGet_tlm_payload_t                   roma_parget;
//         payuel_roma_ParSet_tlm_payload_t                   roma_parset;
//         payuel_roma_ParDefaults_tlm_payload_t              roma_pardefaults;
//         payuel_roma_ParSave_tlm_payload_t                  roma_parsave;
//         payuel_roma_ParRestore_tlm_payload_t               roma_parrestore;
//         payuel_roma_ParLoad_tlm_payload_t                  roma_parload;
//         payuel_roma_ParSetOOB_tlm_payload_t                roma_parsetOOB;
//         payuel_roma_SendCommand_tlm_payload_t              roma_sendcommand;

//         PAYUEL_LGPM_Noop_tlm_payload_t                     lgpm_noop;
//         PAYUEL_LGPM_ResetCounters_tlm_payload_t            lgpm_resetcounters;
//         PAYUEL_LGPM_MCU_ALIVE_CHECK_Tlm_Payload            lgpm_mcualivecheck;
//         PAYUEL_LGPM_3V3PwrOn_tlm_payload_t                 lgpm_3v3pwron;
//         PAYUEL_LGPM_3V3PwrOff_tlm_payload_t                lgpm_3v3pwroff;
//         PAYUEL_LGPM_MainBoostSwOn_tlm_payload_t            lgpm_mainboostswon;
//         PAYUEL_LGPM_MainBoostSwOff_tlm_payload_t           lgpm_mainboostswoff;
//         PAYUEL_LGPM_SubBoostSwOn_tlm_payload_t             lgpm_subboostswon;
//         PAYUEL_LGPM_SubBoostSwOff_tlm_payload_t            lgpm_subboostswoff;
//         PAYUEL_LGPM_V28MainOn_tlm_payload_t                lgpm_v28mainon;
//         PAYUEL_LGPM_V28MainOff_tlm_payload_t               lgpm_v28mainoff;
//         PAYUEL_LGPM_V28SubOn_tlm_payload_t                 lgpm_v28subon;
//         PAYUEL_LGPM_V28SubOff_tlm_payload_t                lgpm_v28suboff;
//         PAYUEL_LGPM_V12MainOn_tlm_payload_t                lgpm_v12mainon;
//         PAYUEL_LGPM_V12MainOff_tlm_payload_t               lgpm_v12mainoff;
//         PAYUEL_LGPM_PwrSenseInfo_tlm_payload_t             lgpm_pwrsenseinfo;
//         PAYUEL_LGPM_PwrSeqOn_tlm_payload_t                 lgpm_pwrseqon;
//         PAYUEL_LGPM_PwrSeqOff_tlm_payload_t                lgpm_pwrseqoff;
//         PAYUEL_LGPM_RwaControlIdx1_tlm_payload_t           lgpm_rwacontrol_idx1;
//         PAYUEL_LGPM_RwaControlIdx2_tlm_payload_t           lgpm_rwacontrol_idx2;
//         PAYUEL_LGPM_RwaControlIdx3_tlm_payload_t           lgpm_rwacontrol_idx3;
//         PAYUEL_LGPM_RwaPwrOn_tlm_payload_t                 lgpm_rwapwron;
//         PAYUEL_LGPM_RwaPwrOff_tlm_payload_t                lgpm_rwapwroff;
//         PAYUEL_LGPM_RwaSenseInfo_tlm_payload_t             lgpm_rwasenseinfo;
// /**************************************************************************************************** */

//     } u;
// } ReportView_t;


// extern std::mutex g_report_view_mtx;
// extern ReportView_t g_report_view;
// extern EPS_Report_State_t g_eps_report_state;


// typedef struct {
//     uint8 Callsign[6];
//     uint8 CurrentMode;
//     uint8 CurrentSubmode;
//     uint8 PrevioudMode;
//     uint8 PreviousSubmode;
//     uint8 CurrentModeFlag;
//     uint8 PreviousModeFlag;
//     uint32 ApplicationRunStatus;
//     uint32 SatelliteTime;
//     uint16 RebootCount;
//     uint8 RebootCause;

// }__attribute__((packed)) FM_HK_;

// typedef struct {
// 	uint16 DeployState_UANT;
// }__attribute__((packed)) UANT_;

// typedef struct {
// 	uint32 rxfreq;
// 	uint32 txfreq;
// 	int16 LastRssi;
//     uint32 TotRxBytes;
//     uint8 StatusConfiguration;
// }__attribute__((packed)) UTRX_;

// typedef struct {
// 	//EPS - P60 Dock
// 	uint8 out_en_dock[7]; //01458910
//     int16 temp_dock[2];
//     uint32 bootcause;
//     uint32 bootcnt;
//     uint32 uptime;
//     uint16 resetcause;
//     uint8 batt_mode;
//     uint8 heater_on;
//     uint16 latchup_dock[7]; //01458910
//     uint16 vbat_v;
//     int16 batt_v;
//     int16 batt_temp[2];
//     uint8 device_status[8];
//     uint32 wdt_cnt_gnd;
//     uint32 wdt_gnd_left;
//     int16 batt_chrg;
//     int16 batt_dischrg;
// 	//EPS - PDU
//     int16 vbat;
//     uint8 out_en_pdu[6]; //034758
//     uint16 latchup_pdu[6];
//     uint16 out_voltage[6]; // 034758
// 	//EPS - ACU
//     int16 c_in[4]; //0123
//     uint16 v_in[4]; //0123
// }__attribute__((packed)) EPS_;

// typedef struct {
// 	uint8 RWL0_PowerState;
// 	uint8 RWL1_PowerState;
// 	uint8 RWL2_PowerState;
// 	uint8 MAG0_PowerState;
// 	uint8 FSS0_PowerState;
// 	uint8 HSS0_PowerState;
// 	uint8 Control_Mode;
// 	uint16 Mag_Control_Timeout;
// 	float GYRO_Calib_rate_X;
// 	float GYRO_Calib_rate_Y;
// 	float GYRO_Calib_rate_Z;
// }__attribute__((packed)) ADCS_;

// typedef struct {
// 	uint8 Status;
// 	int16 Board_Temperature;
// 	int16 Battery_Current;
// 	int16 Battery_Voltage;
// }__attribute__((packed)) STX_;

// typedef struct {
// 	int16 temp_PAYC;
// 	uint16 icore;
// }__attribute__((packed)) PAYC_;

// typedef struct {
// 	uint8 DeployStatus_PAYR;
// }__attribute__((packed)) PAYR_;

// typedef struct {
// 	uint8 PAYS_State;
// 	uint8 PAYS_Sign;
// 	uint8 PAYS_Temp;
// }__attribute__((packed)) PAYS_;


// // typedef struct {
// // 	CCSDS_Header_ CCSDS_Header;
// // 	FM_HK_ FM;
// // 	EPS_ EPS;
// // 	TCS_ TCS;
// // 	RWA_ RWA;
// // 	MTQ_ MTQ;
// // 	SNSR_ SNSR;
// // 	UTRX_ UTRX;
// // 	STX_ STX;
// // 	PAY_ PAY;
// // }__attribute__((packed)) HK;

// // typedef struct {
// // 	FM_HK_ FM;
// // 	ADCS_ ADCS;
// // }__attribute__((packed)) AOD;

// typedef struct {
// 	uint32_t ExTime;
// 	uint32_t ExWindow;
// 	uint16_t EntryID;
// 	uint16_t GroupID;
// 	uint8_t cmd[];
// }__attribute__((packed)) Book;



// static bool ParseReportWire540(const uint8_t *buf, size_t len, Report &out);
// void * TRxController(void *);
// void * SignalTest(void*);
// void now_rx_bytes_update();
// void set_rx_bytes(uint32_t nowbytes);
// uint32_t get_rx_bytes();
// uint32_t * get_rx_bytes_address();
// uint16_t get_boot_count();
// uint16_t * get_boot_count_address();
// void buf_allclear();
// void CalculateChecksum(CommandHeader_t* Cmd);
// int32_t GenerateCmdMsg(CommandHeader_t* Cmd, uint16_t MsgId, uint8_t FcnCode, uint32_t ArgLen);
// csp_socket_t *  DL_sock_initialize();
// extern BEE1000_Beacon_t* bee1000_beacon;
// extern BEE1012_Beacon_t* bee1012_beacon;
// extern UELYSYS_Beacon_t* uelysys_beacon;
// int BEE1000BeaconSaver(BEE1000_Beacon_t * bec);
// int BEE1012BeaconSaver(BEE1012_Beacon_t * bec);
// int UELYSYSBeaconSaver(UELYSYS_Beacon_t * bec);
// void * task_downlink_onorbit(void * socketinfo);
// void * task_uplink_onorbit(void * sign_);

// int PacketHandler(csp_packet_t *packet, int type, int NowCursor);
// packetsign * PingInit(FSWTle * FSWTleinfo);
// csp_packet_t * PacketEncoder(packetsign * sign, bool freeer = true);
// packetsign * PacketDecoder(csp_packet_t * packet);


// class CmdGenerator_GS {
// private:
//     void SetHeaderWord(uint8_t* Word16, uint16_t Value, uint16_t Mask);
//     void SetHeaderByte(uint8_t* Byte, uint8_t Value, uint8_t Mask);
//     void GetHeaderWord(const uint8_t* Word16, uint16_t& Value, uint16_t Mask);
//     uint32_t ComputeCheckSum(void);

// public:
// 	CFE_MSG_CommandHeader* CmdHeader;
// 	bool Scheduled = false;
// 	bool Checksum = true;

//     CmdGenerator_GS(void);
//     ~CmdGenerator_GS(void);

//     int GenerateCmdHeader(uint32_t MsgId, uint16_t FncCode, uint32_t Size, void* Data);
//     void CopyCmdHeaderToBuffer(uint8_t* Buffer);

//     void InitHeader(void);
//     void SetHeader(const CFE_MSG_CommandHeader* Header);
//     const CFE_MSG_CommandHeader* GetHeader(void) const;

//     int SetHasSecondaryHeader(bool HasSec);
//     int SetMsgId(uint16_t MsgId);
//     int SetSize(uint16_t Size);
//     int SetSegmentationFlag(uint16_t SegFlag);
//     int SetFncCode(uint16_t FncCode);

//     bool HasSecondaryHeader(void) const;
//     uint16_t GetSize(void);
//     uint16_t GetFncCode(void) const;

//     int GenerateChecksum(void);
// 	int Scheduling(uint32_t ExecutionTime, uint32_t ExecutionWindow, uint32_t EntryID, uint16_t GroupID);
// 	packetsign * GenerateCMDPacket(void);
// };

// void * Direct_Shell(void * data);

// #endif _MIMAN_COMS_H_


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

}__attribute__((packed)) BEE1000_Beacon_t;

#define a sizeof(BEE1000_Beacon_t);
static_assert(sizeof(BEE1000_Beacon_t) == 203, "BEE1000_Beacon_t size mismatch");

typedef struct {
    char call_sign[7];
    uint8_t msg_id[2];
    uint8_t sequence[2];
    uint8_t length[2];
    uint8_t time_code[6];
}__attribute__((packed)) BEE1012_BeaconHeader_t;

typedef struct {
    uint16_t boot_count;
    uint32_t sequence;
    uint8_t reset_cause;
}__attribute__((packed)) BEE1012_RPT_t;

typedef struct {
    uint8_t active_conf;
    uint16_t boot_count;
    uint32_t boot_cause;
    int16_t temp_board;
}__attribute__((packed)) BEE1012_AX100_t;

typedef struct {
    int16_t temp;
    int8_t connection_quality;
    uint16_t battery_capacity;
}__attribute__((packed)) BEE1012_LTRX_t;

typedef struct {
    BEE1012_AX100_t ax100;
    BEE1012_LTRX_t ltrx;
}__attribute__((packed)) BEE1012_COMS_t;

typedef struct {
    uint32_t bootcause;
    uint16_t resetcause;
    uint16_t bootcount;
    uint8_t out_en[6];
    int16_t temp[2];
    uint8_t batt_mode;
    int16_t batt_i;
    uint16_t batt_v;
    uint8_t sm_en;
    uint16_t gnd_wdt_cnt;
    uint16_t bus_wdt_cnt;
    uint32_t gnd_wdt_left;
    uint32_t bus_wdt_left;
}__attribute__((packed)) BEE1012_EPS_PMU_t;

typedef struct {
    int16_t out_i[12];
    uint8_t out_en[12];
}__attribute__((packed)) BEE1012_EPS_PDU_t;

typedef struct {
    int16_t input_i[6];
    uint16_t input_v[6];
    uint8_t mppt_mode;
}__attribute__((packed)) BEE1012_EPS_ACU_Unit_t;

typedef struct {
    BEE1012_EPS_ACU_Unit_t acu[2];
}__attribute__((packed)) BEE1012_EPS_ACU_t;

typedef struct {
    uint16_t bootcount;
    uint16_t bootcause;
    uint16_t resetcause;
    float soc;
    float bat_avr_temp;
    uint16_t vbat;
    float i;
    uint16_t heater_i;
}__attribute__((packed)) BEE1012_EPS_BP8_t;

typedef struct {
    uint16_t gpio_status;
    uint8_t sp_deploy_status;
}__attribute__((packed)) BEE1012_EPS_GPIO_t;

typedef struct {
    BEE1012_EPS_PMU_t pmu;
    BEE1012_EPS_PDU_t pdu;
    BEE1012_EPS_ACU_t acu;
    BEE1012_EPS_BP8_t bp8;
    BEE1012_EPS_GPIO_t gpio;
}__attribute__((packed)) BEE1012_EPS_t;

typedef struct {
    uint8_t power_state;
    uint8_t control_mode;
    float gyro0_calibrated_rate_x;
    float gyro0_calibrated_rate_y;
    float gyro0_calibrated_rate_z;
    uint8_t css[6];
}__attribute__((packed)) BEE1012_ADCS_t;

typedef struct {
    BEE1012_BeaconHeader_t header;
    BEE1012_RPT_t rpt;
    BEE1012_COMS_t coms;
    BEE1012_EPS_t eps;
    BEE1012_ADCS_t adcs;
}__attribute__((packed)) BEE1012_Beacon_t;

static_assert(sizeof(BEE1012_BeaconHeader_t) == 19, "BEE1012_BeaconHeader_t size mismatch");
static_assert(sizeof(BEE1012_RPT_t) == 7, "BEE1012_RPT_t size mismatch");
static_assert(sizeof(BEE1012_AX100_t) == 9, "BEE1012_AX100_t size mismatch");
static_assert(sizeof(BEE1012_LTRX_t) == 5, "BEE1012_LTRX_t size mismatch");
static_assert(sizeof(BEE1012_COMS_t) == 14, "BEE1012_COMS_t size mismatch");
static_assert(sizeof(BEE1012_EPS_PMU_t) == 36, "BEE1012_EPS_PMU_t size mismatch");
static_assert(sizeof(BEE1012_EPS_PDU_t) == 36, "BEE1012_EPS_PDU_t size mismatch");
static_assert(sizeof(BEE1012_EPS_ACU_Unit_t) == 25, "BEE1012_EPS_ACU_Unit_t size mismatch");
static_assert(sizeof(BEE1012_EPS_ACU_t) == 50, "BEE1012_EPS_ACU_t size mismatch");
static_assert(sizeof(BEE1012_EPS_BP8_t) == 22, "BEE1012_EPS_BP8_t size mismatch");
static_assert(sizeof(BEE1012_EPS_GPIO_t) == 3, "BEE1012_EPS_GPIO_t size mismatch");
static_assert(sizeof(BEE1012_EPS_t) == 147, "BEE1012_EPS_t size mismatch");
static_assert(sizeof(BEE1012_ADCS_t) == 20, "BEE1012_ADCS_t size mismatch");
static_assert(sizeof(BEE1012_Beacon_t) == 207, "BEE1012_Beacon_t size mismatch");

typedef struct {
    char call_sign[7];
    uint8_t msg_id[2];
    uint8_t sequence[2];
    uint8_t length[2];
    uint8_t time_code[6];
}__attribute__((packed)) UELYSYS_BeaconHeader_t;

typedef struct {
    uint16_t boot_count;
    uint32_t sequence;
    uint8_t reset_cause;
}__attribute__((packed)) UELYSYS_FSW_t;

typedef struct {
    uint8_t i2c1_0x05_channel_0_status;
    uint8_t i2c1_0x05_channel_1_status;
    uint8_t i2c1_0x06_channel_0_status;
    uint8_t i2c1_0x06_channel_1_status;
    uint8_t ax100_active_conf;
    uint16_t ax100_boot_count;
    uint32_t ax100_boot_cause;
    int16_t ax100_temp_board;
}__attribute__((packed)) UELYSYS_COMS_t;

typedef struct {
    uint32_t bootcause;
    uint16_t resetcause;
    uint16_t bootcount;
    uint8_t out_en[6];
    int16_t temp[2];
    uint8_t batt_mode;
    int16_t batt_i;
    uint16_t batt_v;
    uint8_t sm_en;
    uint16_t gnd_wdt_cnt;
    uint16_t bus_wdt_cnt;
    uint32_t gnd_wdt_left;
    uint32_t bus_wdt_left;
}__attribute__((packed)) UELYSYS_EPS_PMU_t;

typedef struct {
    int16_t out_i[12];
    uint8_t out_en[12];
}__attribute__((packed)) UELYSYS_EPS_PDU_t;

typedef struct {
    int16_t input_i[6];
    uint16_t input_v[6];
    uint8_t mppt_mode;
}__attribute__((packed)) UELYSYS_EPS_ACU_Unit_t;

typedef struct {
    UELYSYS_EPS_ACU_Unit_t acu[2];
}__attribute__((packed)) UELYSYS_EPS_ACU_t;

typedef struct {
    uint16_t bootcount;
    uint16_t bootcause;
    uint16_t resetcause;
    float soc;
    float bat_avr_temp;
    uint16_t vbat;
    float i;
    uint16_t heater_i;
}__attribute__((packed)) UELYSYS_EPS_BP8_t;

typedef struct {
    uint8_t dsp_i2c1_0x07_status;
    uint8_t dsp_i2c1_0x08_status;
    uint8_t dsp_i2c1_0x09_status;
    uint8_t dsp_i2c1_0x10_status;
}__attribute__((packed)) UELYSYS_EPS_DSP_t;

typedef struct {
    UELYSYS_EPS_PMU_t pmu;
    UELYSYS_EPS_PDU_t pdu;
    UELYSYS_EPS_ACU_t acu;
    UELYSYS_EPS_BP8_t bp8;
    UELYSYS_EPS_DSP_t dsp;
}__attribute__((packed)) UELYSYS_EPS_t;

typedef struct {
    uint8_t power_state;
    uint8_t control_mode;
    float gyro0_calibrated_rate_x;
    float gyro0_calibrated_rate_y;
    float gyro0_calibrated_rate_z;
    uint8_t css[6];
}__attribute__((packed)) UELYSYS_ADCS_t;

typedef struct {
    UELYSYS_BeaconHeader_t header;
    UELYSYS_FSW_t fsw;
    UELYSYS_COMS_t coms;
    UELYSYS_EPS_t eps;
    UELYSYS_ADCS_t adcs;
}__attribute__((packed)) UELYSYS_Beacon_t;

static_assert(sizeof(UELYSYS_BeaconHeader_t) == 19, "UELYSYS_BeaconHeader_t size mismatch");
static_assert(sizeof(UELYSYS_FSW_t) == 7, "UELYSYS_FSW_t size mismatch");
static_assert(sizeof(UELYSYS_COMS_t) == 13, "UELYSYS_COMS_t size mismatch");
static_assert(sizeof(UELYSYS_EPS_PMU_t) == 36, "UELYSYS_EPS_PMU_t size mismatch");
static_assert(sizeof(UELYSYS_EPS_PDU_t) == 36, "UELYSYS_EPS_PDU_t size mismatch");
static_assert(sizeof(UELYSYS_EPS_ACU_Unit_t) == 25, "UELYSYS_EPS_ACU_Unit_t size mismatch");
static_assert(sizeof(UELYSYS_EPS_ACU_t) == 50, "UELYSYS_EPS_ACU_t size mismatch");
static_assert(sizeof(UELYSYS_EPS_BP8_t) == 22, "UELYSYS_EPS_BP8_t size mismatch");
static_assert(sizeof(UELYSYS_EPS_DSP_t) == 4, "UELYSYS_EPS_DSP_t size mismatch");
static_assert(sizeof(UELYSYS_EPS_t) == 148, "UELYSYS_EPS_t size mismatch");
static_assert(sizeof(UELYSYS_ADCS_t) == 20, "UELYSYS_ADCS_t size mismatch");
static_assert(sizeof(UELYSYS_Beacon_t) == 207, "UELYSYS_Beacon_t size mismatch");



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
	    REPORT_KIND_EPS_QUERY_REPORT,
	    REPORT_KIND_EPS_QUERY_RAW,
	    REPORT_KIND_EPS_BCN_REPORT,
	    REPORT_KIND_EPS_POWER_IF_STATUS,
	    REPORT_KIND_EPS_POWER_IF_LIST,
	    REPORT_KIND_EPS_P80_PMU_HK,
	    REPORT_KIND_EPS_P80_PDU_HK,
	    REPORT_KIND_EPS_P80_ACU1_HK,
	    REPORT_KIND_EPS_P80_ACU2_HK,
	    REPORT_KIND_EPS_BP8_HK,
	    REPORT_KIND_GPIO_DEP1_EN_ON,
	    REPORT_KIND_GPIO_DEP1_EN_OFF,
	    REPORT_KIND_GPIO_DEP2_EN_ON,
	    REPORT_KIND_GPIO_DEP2_EN_OFF,
	    REPORT_KIND_GPIO_SP_IN_READ_5S,

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
    REPORT_KIND_PAYUEL_ROMA_SENDMSG,
    REPORT_KIND_PAYUEL_ROMA_PAYINIT,


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

    REPORT_KIND_UELYSYS_TTC,
    REPORT_KIND_UELYSYS_MEOW,
    REPORT_KIND_UELYSYS_STX,
    REPORT_KIND_UELYSYS_UTRX,
    REPORT_KIND_UELYSYS_UANT,
    REPORT_KIND_UELYSYS_GPIO,
    REPORT_KIND_UELYSYS_PAYUEL_OBC,
    REPORT_KIND_UELYSYS_PAYUEL_CAM,
    REPORT_KIND_UELYSYS_LGBAT,
    REPORT_KIND_UELYSYS_AOS,

    REPORT_KIND_SC_GENERIC,

} ReportKind_t;

static ReportKind_t DetermineReportKind(uint16_t reflected_mid, uint8_t reflected_cc) {
    if (reflected_mid == UELYSYS_TTC_CMD_ID || reflected_mid == 0x8018) return REPORT_KIND_UELYSYS_TTC;
    if (reflected_mid == UELYSYS_MEOW_CMD_ID || reflected_mid == 0x8618) return REPORT_KIND_UELYSYS_MEOW;
    if (reflected_mid == UELYSYS_STX_CMD_ID || reflected_mid == 0x5418) return REPORT_KIND_UELYSYS_STX;
    if (reflected_mid == UELYSYS_UTRX_CMD_ID || reflected_mid == 0x5018) return REPORT_KIND_UELYSYS_UTRX;
    if (reflected_mid == UELYSYS_UANT_CMD_ID || reflected_mid == 0x4018) return REPORT_KIND_UELYSYS_UANT;
    if (reflected_mid == UELYSYS_GPIO_CMD_ID || reflected_mid == 0x8A18) return REPORT_KIND_UELYSYS_GPIO;
    if (reflected_mid == UELYSYS_PAYUEL_OBC_CMD_ID || reflected_mid == 0xC018) return REPORT_KIND_UELYSYS_PAYUEL_OBC;
    if (reflected_mid == UELYSYS_PAYUEL_CAM_CMD_ID || reflected_mid == 0xC318) return REPORT_KIND_UELYSYS_PAYUEL_CAM;
    if (reflected_mid == UELYSYS_LGBAT_CMD_ID || reflected_mid == 0xC618) return REPORT_KIND_UELYSYS_LGBAT;
    if (reflected_mid == UELYSYS_PAYUEL_AOS_CMD_ID || reflected_mid == 0x4718) return REPORT_KIND_UELYSYS_AOS;
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

	    if ((reflected_mid == EPS_CMD_ID || reflected_mid == 0x7518) &&
	        (reflected_cc == EPS_P80_POWER_IF_GET_CC ||
	         reflected_cc == EPS_P80_POWER_IF_LIST_CC ||
	         reflected_cc == EPS_GET_HK_CC ||
	         reflected_cc == EPS_GET_HK_ALL_CC ||
	         reflected_cc == EPS_RPARAM_GET_FULL_TABLE_CC ||
	         reflected_cc == EPS_CSP_PING_CC)) return REPORT_KIND_EPS_QUERY_REPORT;
	    if ((reflected_mid == EPS_CMD_ID || reflected_mid == 0x7518) &&
	        reflected_cc == EPS_REPORT_BCN_CC) return REPORT_KIND_EPS_BCN_REPORT;

	    if ((reflected_mid == GPIO_CMD_ID || reflected_mid == 0x9218) && reflected_cc == GPIO_DEP1_EN_ON_CC) return REPORT_KIND_GPIO_DEP1_EN_ON;
	    if ((reflected_mid == GPIO_CMD_ID || reflected_mid == 0x9218) && reflected_cc == GPIO_DEP1_EN_OFF_CC) return REPORT_KIND_GPIO_DEP1_EN_OFF;
	    if ((reflected_mid == GPIO_CMD_ID || reflected_mid == 0x9218) && reflected_cc == GPIO_DEP2_EN_ON_CC) return REPORT_KIND_GPIO_DEP2_EN_ON;
	    if ((reflected_mid == GPIO_CMD_ID || reflected_mid == 0x9218) && reflected_cc == GPIO_DEP2_EN_OFF_CC) return REPORT_KIND_GPIO_DEP2_EN_OFF;
	    if ((reflected_mid == GPIO_CMD_ID || reflected_mid == 0x9218) && reflected_cc == GPIO_SP_IN_READ_5S_CC) return REPORT_KIND_GPIO_SP_IN_READ_5S;

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
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_SEND_MSG_CC)) return REPORT_KIND_PAYUEL_ROMA_SENDMSG;
    if ((reflected_mid == PAYUEL_ROMA_CMD_MID || reflected_mid == 0x3018) && (reflected_cc == PAYUEL_ROMA_PAY_INIT_CC)) return REPORT_KIND_PAYUEL_ROMA_PAYINIT;

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
	    return REPORT_KIND_SC_GENERIC;
	}

typedef struct
{
    uint8_t bytes[512];
} RptGenericPayload_t;

typedef struct
{
    uint16_t size;
    uint8_t bytes[512];
} UELYSYS_ReportPayloadView_t;

typedef struct __attribute__((packed))
{
    UELYSYS_EPS_PMU_t PMU;
    UELYSYS_EPS_PDU_t PDU;
    UELYSYS_EPS_ACU_Unit_t ACU[2];
    UELYSYS_EPS_BP8_t BP8;
} EPS_Bcn_Report_t;

static_assert(sizeof(EPS_Bcn_Report_t) == 144, "EPS_Bcn_Report_t size mismatch");

#pragma pack(push, 1)
typedef struct { uint16_t counters[2]; } UELYSYS_TTC_Counters_Report_t;
typedef struct {
    uint16_t group_id;
    uint16_t first_entry_id;
    uint16_t scheduled_entries;
    uint16_t fail_count;
} UELYSYS_TTC_BeaconBurst_Report_t;
typedef struct { uint16_t entry_id; uint16_t group_id; uint8_t has_entry; } UELYSYS_TTC_NextEntry_Report_t;
typedef struct { uint32_t next_execution_time; uint8_t has_entry; } UELYSYS_TTC_NextExecution_Report_t;
typedef struct {
    uint32_t execution_time_absolute;
    uint16_t staleness_threshold;
    uint16_t entry_id;
    uint16_t group_id;
    uint8_t execution_type;
    uint16_t command_size;
} UELYSYS_TTC_InsertAbs_Report_t;
typedef struct {
    uint16_t execution_time_relative;
    uint16_t staleness_threshold;
    uint16_t entry_id;
    uint16_t group_id;
    uint8_t execution_type;
    uint16_t command_size;
} UELYSYS_TTC_InsertRel_Report_t;
typedef struct { uint16_t entry_id; uint16_t group_id; } UELYSYS_TTC_EntryGroup_Report_t;
typedef struct { uint16_t group_id; uint16_t delete_count; } UELYSYS_TTC_DeleteGroup_Report_t;
typedef struct { uint16_t group_id; uint16_t entry_id; uint8_t persistent; } UELYSYS_TTC_ExecuteEntry_Report_t;
typedef struct { uint16_t group_id; uint8_t persistent; } UELYSYS_TTC_ExecuteGroup_Report_t;
typedef struct {
    uint16_t entry_id;
    uint16_t group_id;
    uint16_t offset;
    uint16_t chunk_size;
    uint8_t data[220];
} UELYSYS_TTC_PlumbWrite_Report_t;
typedef struct {
    uint16_t entry_id;
    uint16_t group_id;
    uint32_t time_tag;
    uint16_t staleness_threshold;
    uint16_t command_size;
    uint8_t time_tag_type;
    uint8_t execution_type;
} UELYSYS_TTC_PlumbFinalize_Report_t;

typedef struct { int32_t exit_code; int32_t pid; } UELYSYS_MEOW_ShellResult_Report_t;
typedef struct { int32_t value; int32_t os_errno; } UELYSYS_MEOW_ValueErrno_Report_t;
typedef struct {
    uint64_t size;
    int64_t mtime;
    int64_t ctime;
    uint32_t mode;
    int32_t is_dir;
    uint8_t preview[16];
    uint64_t preview_len;
} UELYSYS_MEOW_FileStat_Report_t;
typedef struct { uint64_t total_bytes; uint64_t avail_bytes; } UELYSYS_MEOW_DiskStat_Report_t;
typedef struct {
    uint32_t uptime;
    uint32_t load1;
    uint32_t load5;
    uint32_t load15;
    uint64_t total_ram;
    uint64_t free_ram;
    uint16_t num_procs;
    uint8_t padding[6];
} UELYSYS_MEOW_SysInfo_Report_t;
typedef struct { int64_t sec; uint32_t nsec; uint32_t padding; } UELYSYS_MEOW_SysTime_Report_t;
typedef struct {
    uint32_t rx;
    uint32_t rx_error;
    uint32_t tx;
    uint32_t tx_error;
    uint32_t drop;
} UELYSYS_MEOW_CspIfStats_Report_t;

typedef struct { uint8_t symbol_rate; uint8_t transmit_power; uint8_t modcod; uint8_t rolloff;
    uint8_t pilot_signal; uint8_t fec_frame_size; uint16_t pretx_delay; float center_frequency;
} UELYSYS_STX_AllParameters_Report_t;
typedef struct { uint8_t command_status; uint32_t file_handle; } UELYSYS_STX_CreateFile_Report_t;
typedef struct { int32_t result; uint32_t packet_number; uint8_t command_status; } UELYSYS_STX_WriteFile_Report_t;
typedef struct { uint8_t command_status; uint8_t flag_more; uint16_t file_count; } UELYSYS_STX_DirHeader_Report_t;

typedef struct { uint32_t seconds_since_boot; uint8_t reboot_count; } UELYSYS_UANT_BoardStatus_Report_t;
typedef struct { int8_t result; } UELYSYS_UANT_Autodeploy_Report_t;

typedef struct { uint32_t baud; } UELYSYS_UTRX_RxBaud_Report_t;
typedef struct { uint8_t status; } UELYSYS_PayloadStatus_Report_t;
typedef struct {
    uint8_t object_id;
    uint8_t image_index;
    uint8_t chunk_count_be[2];
    uint8_t last_chunk_size;
    uint8_t file_crc32_be[4];
} UELYSYS_ImageMeta_Report_t;
typedef struct {
    uint8_t object_id;
    uint8_t image_index;
} UELYSYS_DownloadRequest_Report_t;
typedef struct {
    uint8_t command_id;
    uint8_t status;
    uint8_t crc16_be[2];
} UELYSYS_PayloadError_Report_t;

typedef struct { uint8_t ID; uint16_t Pack_Voltage; int16_t Pack_Current; uint16_t Average_Time_To_Empty; uint16_t Average_Time_To_Full; uint8_t CheckSum; } UELYSYS_LGBAT_Data01_t;
typedef struct { uint8_t ID; uint8_t Power_Supply_Status; uint8_t SOH; uint16_t SOC; uint16_t RC_Remaining_Capacity; uint16_t AE_Available_Energy; uint8_t CheckSum; } UELYSYS_LGBAT_Data02_t;
typedef struct { uint8_t ID; uint16_t System_Max_Voltage; uint8_t Power_Supply_Health; uint16_t FETTestRequiredVoltage; uint8_t Battery_Information_Reserved[3]; uint8_t CheckSum; } UELYSYS_LGBAT_Data03_t;
typedef struct { uint8_t ID; uint16_t Percentage; uint16_t Design_Capacity; uint16_t Capacity; uint16_t Charge; uint8_t CheckSum; } UELYSYS_LGBAT_Data04_t;
typedef struct { uint8_t ID; uint16_t Cell_Voltage_Max; uint16_t Cell_Voltage_Min; int16_t Cell_Temperature_Max; int16_t Cell_Temperature_Min; uint8_t CheckSum; } UELYSYS_LGBAT_Data05_t;
typedef struct { uint8_t ID; uint16_t Cell_Voltage_01; uint16_t Cell_Voltage_02; uint16_t Reserved_01; uint16_t Reserved_02; uint8_t CheckSum; } UELYSYS_LGBAT_Data06_t;
typedef struct { uint8_t ID; int16_t Cell_Temperature_02; int16_t Cell_Temperature_01; int16_t Balancing_R_Temperature; int16_t PreCharge_R_Temperature; uint8_t CheckSum; } UELYSYS_LGBAT_Data07_t;
typedef struct { uint8_t ID; int16_t FET_Down_Temperature; int16_t FET_Up_Temperature; uint16_t CtrlCBStatus; uint8_t Temperature_002_Reserved; uint8_t SoftVersion; uint8_t CheckSum; } UELYSYS_LGBAT_Data08_t;
typedef struct { uint8_t ID; uint8_t BMS_Wakeup; uint8_t WakeupHoldStatus; uint8_t VoltCurrDiag; uint8_t TempFailLevel; uint8_t FETStatus; uint8_t Reserved[3]; uint8_t CheckSum; } UELYSYS_LGBAT_Data09_t;
typedef struct { uint8_t ID; uint8_t FailStatus2; uint8_t FailStatus3; uint8_t Reserved[6]; uint8_t CheckSum; } UELYSYS_LGBAT_Data0A_t;
typedef struct { uint8_t ID; uint16_t BOOST_Out_Voltage; uint16_t MCU_B_Plus_Volt; uint16_t MCU_P_Plus_Volt; uint16_t MCU_BMIC_REG_Out_Volt; uint8_t CheckSum; } UELYSYS_LGBAT_Data0B_t;
typedef struct { uint8_t ID; int16_t BMS_Current; int16_t PCB_Temperature; int16_t BOOST_Temperature; uint16_t MCU_SBC_AMUX_Voltage; uint8_t CheckSum; } UELYSYS_LGBAT_Data0C_t;
typedef struct {
    UELYSYS_LGBAT_Data01_t Data01; UELYSYS_LGBAT_Data02_t Data02;
    UELYSYS_LGBAT_Data03_t Data03; UELYSYS_LGBAT_Data04_t Data04;
    UELYSYS_LGBAT_Data05_t Data05; UELYSYS_LGBAT_Data06_t Data06;
    UELYSYS_LGBAT_Data07_t Data07; UELYSYS_LGBAT_Data08_t Data08;
    UELYSYS_LGBAT_Data09_t Data09; UELYSYS_LGBAT_Data0A_t Data0A;
    UELYSYS_LGBAT_Data0B_t Data0B; UELYSYS_LGBAT_Data0C_t Data0C;
} UELYSYS_LGBAT_AllData_Report_t;
typedef union {
    UELYSYS_LGBAT_Data01_t Data01; UELYSYS_LGBAT_Data02_t Data02;
    UELYSYS_LGBAT_Data03_t Data03; UELYSYS_LGBAT_Data04_t Data04;
    UELYSYS_LGBAT_Data05_t Data05; UELYSYS_LGBAT_Data06_t Data06;
    UELYSYS_LGBAT_Data07_t Data07; UELYSYS_LGBAT_Data08_t Data08;
    UELYSYS_LGBAT_Data09_t Data09; UELYSYS_LGBAT_Data0A_t Data0A;
    UELYSYS_LGBAT_Data0B_t Data0B; UELYSYS_LGBAT_Data0C_t Data0C;
    uint8_t raw[10];
} UELYSYS_LGBAT_Block_Report_t;

typedef struct { uint8_t msb; uint8_t lsb; } UELYSYS_AOS_ReadRegister_Report_t;
typedef struct { float resistance[4]; } UELYSYS_AOS_ReadAll_Report_t;
#pragma pack(pop)

static_assert(sizeof(UELYSYS_TTC_Counters_Report_t) == 4, "TTC counters report size mismatch");
static_assert(sizeof(UELYSYS_TTC_BeaconBurst_Report_t) == 8, "TTC beacon burst report size mismatch");
static_assert(sizeof(UELYSYS_TTC_NextEntry_Report_t) == 5, "TTC next entry report size mismatch");
static_assert(sizeof(UELYSYS_TTC_NextExecution_Report_t) == 5, "TTC next execution report size mismatch");
static_assert(sizeof(UELYSYS_TTC_InsertAbs_Report_t) == 13, "TTC insert abs report size mismatch");
static_assert(sizeof(UELYSYS_TTC_InsertRel_Report_t) == 11, "TTC insert rel report size mismatch");
static_assert(sizeof(UELYSYS_TTC_PlumbWrite_Report_t) == 228, "TTC plumb write report size mismatch");
static_assert(sizeof(UELYSYS_TTC_PlumbFinalize_Report_t) == 14, "TTC plumb finalize report size mismatch");
static_assert(sizeof(UELYSYS_MEOW_FileStat_Report_t) == 56, "MEOW file stat report size mismatch");
static_assert(sizeof(UELYSYS_MEOW_DiskStat_Report_t) == 16, "MEOW disk stat report size mismatch");
static_assert(sizeof(UELYSYS_MEOW_SysInfo_Report_t) == 40, "MEOW sys info report size mismatch");
static_assert(sizeof(UELYSYS_MEOW_SysTime_Report_t) == 16, "MEOW sys time report size mismatch");
static_assert(sizeof(UELYSYS_MEOW_CspIfStats_Report_t) == 20, "MEOW CSP ifstats report size mismatch");
static_assert(sizeof(UELYSYS_STX_AllParameters_Report_t) == 12, "STX all parameters report size mismatch");
static_assert(sizeof(UELYSYS_STX_CreateFile_Report_t) == 5, "STX create file report size mismatch");
static_assert(sizeof(UELYSYS_STX_WriteFile_Report_t) == 9, "STX write file report size mismatch");
static_assert(sizeof(UELYSYS_STX_DirHeader_Report_t) == 4, "STX DIR header size mismatch");
static_assert(sizeof(UELYSYS_UANT_BoardStatus_Report_t) == 5, "UANT board status report size mismatch");
static_assert(sizeof(UELYSYS_UTRX_RxBaud_Report_t) == 4, "UTRX RX baud report size mismatch");
static_assert(sizeof(UELYSYS_PayloadStatus_Report_t) == 1, "payload status report size mismatch");
static_assert(sizeof(UELYSYS_ImageMeta_Report_t) == 9, "image metadata report size mismatch");
static_assert(sizeof(UELYSYS_DownloadRequest_Report_t) == 2, "download request report size mismatch");
static_assert(sizeof(UELYSYS_PayloadError_Report_t) == 4, "payload error report size mismatch");
static_assert(sizeof(UELYSYS_LGBAT_Data01_t) == 10, "LGBAT Data01 size mismatch");
static_assert(sizeof(UELYSYS_LGBAT_Data02_t) == 10, "LGBAT Data02 size mismatch");
static_assert(sizeof(UELYSYS_LGBAT_Data03_t) == 10, "LGBAT Data03 size mismatch");
static_assert(sizeof(UELYSYS_LGBAT_Data04_t) == 10, "LGBAT Data04 size mismatch");
static_assert(sizeof(UELYSYS_LGBAT_Data05_t) == 10, "LGBAT Data05 size mismatch");
static_assert(sizeof(UELYSYS_LGBAT_Data06_t) == 10, "LGBAT Data06 size mismatch");
static_assert(sizeof(UELYSYS_LGBAT_Data07_t) == 10, "LGBAT Data07 size mismatch");
static_assert(sizeof(UELYSYS_LGBAT_Data08_t) == 10, "LGBAT Data08 size mismatch");
static_assert(sizeof(UELYSYS_LGBAT_Data09_t) == 10, "LGBAT Data09 size mismatch");
static_assert(sizeof(UELYSYS_LGBAT_Data0A_t) == 10, "LGBAT Data0A size mismatch");
static_assert(sizeof(UELYSYS_LGBAT_Data0B_t) == 10, "LGBAT Data0B size mismatch");
static_assert(sizeof(UELYSYS_LGBAT_Data0C_t) == 10, "LGBAT Data0C size mismatch");
static_assert(sizeof(UELYSYS_LGBAT_AllData_Report_t) == 120, "LGBAT all data report size mismatch");
static_assert(sizeof(UELYSYS_LGBAT_Block_Report_t) == 10, "LGBAT block report size mismatch");
static_assert(sizeof(UELYSYS_AOS_ReadRegister_Report_t) == 2, "AOS read register report size mismatch");
static_assert(sizeof(UELYSYS_AOS_ReadAll_Report_t) == 16, "AOS read all report size mismatch");

static_assert(sizeof(EPS_Query_Report_Payload_t) == 512, "EPS_Query_Report_Payload_t size mismatch");

#pragma pack(push, 1)

typedef struct {
    uint8_t  ch_idx;
    uint8_t  mode;
    uint16_t on_cnt;
    uint16_t off_cnt;
    uint16_t cur_lu_lim;
    uint16_t cur_lim;
    uint16_t voltage;
    int16_t  current;
    uint16_t latchup;
    char     name[8];
} __attribute__((packed)) EPS_PowerIfStatus_Report_t;

typedef struct {
    uint8_t ch_idx;
    uint8_t mode;
    char    name[8];
} __attribute__((packed)) EPS_PowerIfListItem_Report_t;

typedef struct {
    uint8_t cmd;
    uint8_t status;
    uint8_t count;
    EPS_PowerIfListItem_Report_t list[24];
} __attribute__((packed)) EPS_PowerIfList_Report_t;

typedef struct {
    uint32_t uptime;
    uint32_t bootcause;
    uint16_t resetcause;
    uint16_t bootcount;
    uint16_t batt_v;
    int16_t  batt_i;
    uint8_t  batt_mode;
    uint8_t  spare1;
    uint16_t vbat_v;
    uint16_t vcc_v;
    int16_t  temp[2];
    uint8_t  out_en[6];
    int16_t  out_i[6];
    uint8_t  sm_en[8];
    uint16_t gnd_wdt_cnt;
    uint16_t bus_wdt_cnt;
    uint32_t gnd_wdt_left;
    uint32_t bus_wdt_left;
} __attribute__((packed)) EPS_P80_PMU_HK_Report_t;

typedef struct {
    uint32_t uptime;
    uint32_t bootcause;
    uint32_t bootcount;
    uint16_t resetcause;
    uint16_t vcc_v;
    uint16_t vcc_i;
    uint16_t vbat_v;
    int16_t  temp;
    uint8_t  batt_mode;
    uint8_t  out_en[24];
    uint8_t  spare1;
    int16_t  out_i[24];
    uint32_t gnd_wdt_cnt;
    uint32_t bus_wdt_cnt;
    uint32_t gnd_wdt_left;
    uint32_t bus_wdt_left;
} __attribute__((packed)) EPS_P80_PDU_HK_Report_t;

typedef struct {
    uint32_t uptime;
    uint32_t bootcause;
    uint32_t bootcount;
    uint16_t resetcause;
    int16_t  input_i[6];
    uint16_t input_v[6];
    uint16_t vcc_v;
    uint16_t vbat_v;
    int16_t  temp[3];
    uint8_t  mppt_mode;
    uint8_t  spare1;
    uint32_t gnd_wdt_cnt;
    uint32_t gnd_wdt_left;
} __attribute__((packed)) EPS_P80_ACU_HK_Report_t;

typedef struct {
    uint32_t Uptime;
    uint16_t BootCount;
    uint16_t BootCause;
    uint16_t ResetCause;
    uint16_t Vbat;
    float    Soc;
    float    Current;
    uint16_t InCurrent;
    uint16_t OutCurrent;
    uint16_t HeaterCurrent;
    int16_t  IntTemp;
    float    BatAvrTemp;
    int16_t  BatTemp[4];
    uint16_t OVoltCount;
    uint8_t  BatFault;
    uint8_t  spare2;
} __attribute__((packed)) EPS_BP8_HK_Report_t;

#pragma pack(pop)

static_assert(sizeof(EPS_PowerIfStatus_Report_t) == 24, "EPS_PowerIfStatus_Report_t size mismatch");
static_assert(sizeof(EPS_PowerIfListItem_Report_t) == 10, "EPS_PowerIfListItem_Report_t size mismatch");
static_assert(sizeof(EPS_PowerIfList_Report_t) == 243, "EPS_PowerIfList_Report_t size mismatch");
static_assert(sizeof(EPS_P80_PMU_HK_Report_t) == 64, "EPS_P80_PMU_HK_Report_t size mismatch");
static_assert(sizeof(EPS_P80_PDU_HK_Report_t) == 112, "EPS_P80_PDU_HK_Report_t size mismatch");
static_assert(sizeof(EPS_P80_ACU_HK_Report_t) == 58, "EPS_P80_ACU_HK_Report_t size mismatch");
static_assert(sizeof(EPS_BP8_HK_Report_t) == 44, "EPS_BP8_HK_Report_t size mismatch");

typedef struct {
    bool     valid;
    uint8_t  command_code;
    int32_t  return_code;
    uint16_t return_data_size;
    uint8_t  source;
    uint8_t  data_id;
    uint8_t  arg0;
    uint8_t  arg1;
    uint16_t sequence;
    uint16_t offset;
    uint16_t total_size;
    uint16_t chunk_size;
} EPS_Report_Metadata_t;

typedef struct {
    bool power_if_status_valid;
    bool power_if_list_valid;
    bool pmu_hk_valid;
    bool pdu_hk_valid;
    bool acu1_hk_valid;
    bool acu2_hk_valid;
    bool bp8_hk_valid;

    EPS_Report_Metadata_t power_if_status_meta;
    EPS_Report_Metadata_t power_if_list_meta;
    EPS_Report_Metadata_t pmu_hk_meta;
    EPS_Report_Metadata_t pdu_hk_meta;
    EPS_Report_Metadata_t acu1_hk_meta;
    EPS_Report_Metadata_t acu2_hk_meta;
    EPS_Report_Metadata_t bp8_hk_meta;

    EPS_PowerIfStatus_Report_t power_if_status;
    EPS_PowerIfList_Report_t   power_if_list;
    EPS_P80_PMU_HK_Report_t    pmu_hk;
    EPS_P80_PDU_HK_Report_t    pdu_hk;
    EPS_P80_ACU_HK_Report_t    acu1_hk;
    EPS_P80_ACU_HK_Report_t    acu2_hk;
    EPS_BP8_HK_Report_t        bp8_hk;
} EPS_Report_State_t;

#define EPS_RPARAM_REASSEMBLY_CAPACITY 65535
typedef struct {
    bool active;
    bool valid;
    bool complete;
    bool error;
    bool duplicate_seen;
    bool missing_sequence_seen;
    uint8_t source;
    uint8_t data_id;
    uint8_t arg0;
    uint8_t arg1;
    uint16_t expected_sequence;
    uint16_t last_sequence;
    uint16_t total_size;
    uint16_t received_size;
    uint16_t last_offset;
    uint16_t last_chunk_size;
    uint8_t buffer[EPS_RPARAM_REASSEMBLY_CAPACITY];
    uint8_t coverage[EPS_RPARAM_REASSEMBLY_CAPACITY];
} EPS_RParam_Reassembly_State_t;



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
        bool payload_valid;
        int8_t roma_parget_index;
        uint16_t raw_size;
        uint8_t raw_bytes[512];

	    ReportKind_t kind;
	    EPS_Report_Metadata_t eps_meta;

	    union
	    {
        RptGenericPayload_t    generic;
        UELYSYS_ReportPayloadView_t uelysys;
        EPS_Bcn_Report_t eps_bcn;
        UELYSYS_STX_DirHeader_Report_t stx_dir;
        UELYSYS_UTRX_RxBaud_Report_t utrx_rx_baud;
        UELYSYS_PayloadStatus_Report_t payload_status;
        UELYSYS_ImageMeta_Report_t image_meta;
        UELYSYS_DownloadRequest_Report_t download_request;
        UELYSYS_PayloadError_Report_t payload_error;
        UELYSYS_LGBAT_Block_Report_t lgbat_block;
        UELYSYS_LGBAT_AllData_Report_t lgbat_all;
        UELYSYS_AOS_ReadRegister_Report_t aos_read_register;
        UELYSYS_AOS_ReadAll_Report_t aos_read_all;


        ADCS_TlmLogInclMaskTlm_Payload_t       adcs_logmask;
        ADCS_UnsolicitTlmMsgSetupTlm_Payload_t adcs_unsolicited_tlm_tlm;

	        gs_gssb_ant6_release_status_t          uant_getstatus;
	        EPS_P60_DOCK_GET_TABLE_HK              eps_p60dockgettablehk;
	        EPS_P60_PDU_GET_TABLE_HK               eps_p60pdugettablehk;
	        EPS_P60_ACU_GET_TABLE_HK               eps_p60acugettablehk;
	        EPS_PowerIfStatus_Report_t             eps_power_if_status;
	        EPS_PowerIfList_Report_t               eps_power_if_list;
	        EPS_P80_PMU_HK_Report_t                eps_p80_pmu_hk;
	        EPS_P80_PDU_HK_Report_t                eps_p80_pdu_hk;
	        EPS_P80_ACU_HK_Report_t                eps_p80_acu_hk;
	        EPS_BP8_HK_Report_t                    eps_bp8_hk;
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
extern EPS_Report_State_t g_eps_report_state;
extern EPS_RParam_Reassembly_State_t g_eps_rparam_reassembly;
extern int8_t g_roma_parget_requested_index;


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
extern BEE1000_Beacon_t* bee1000_beacon;
extern BEE1012_Beacon_t* bee1012_beacon;
extern UELYSYS_Beacon_t* uelysys_beacon;
int BEE1000BeaconSaver(BEE1000_Beacon_t * bec);
int BEE1012BeaconSaver(BEE1012_Beacon_t * bec);
int UELYSYSBeaconSaver(UELYSYS_Beacon_t * bec);
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
