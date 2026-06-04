#pragma once
#ifndef _MIMAN_RADIAL_H
#define _MIMAN_RADIAL_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
#include <fstream>

#include <csp/csp.h>
#include <csp/csp_error.h>
#include <csp/csp_endian.h>
#include <csp/csp_rtable.h>
#include <csp/csp_endian.h>
#include <csp/arch/csp_time.h>
#include <csp/arch/csp_queue.h>
#include <csp/arch/csp_thread.h> 
#include <csp/interfaces/csp_if_kiss.h>
#include <csp/drivers/usart.h>

//PARAM
#include <gs/param/rparam.h>
#include <gs/param/serialize.h>
#include <gs/param/table.h>
#include <gs/param/types.h>
#include <gs/util/log.h>
#include <gs/util/vmem.h>

//Custom Headers
#include "miman_config.h"

#define AX100_PORT_RPARAM			7
#define AX100_PORT_HMAC_SET                     8
#define AX100_PORT_GNDWDT_RESET		        9

/** FRAM MEMORY MAP (From 0x6000 to 0x7FFF is write-protected) */
#define AX100_FRAM_RUNNING			0x5000
#define AX100_FRAM_TELEM			0x5E00
#define AX100_FRAM_GNDWDT			0x5F00
#define AX100_FRAM_LOG				0x3000
#define AX100_FRAM_LOG_SIZE			0x1000

/** FRAM FILENAMES */
#define AX100_FNO_RUNNING			0
#define AX100_FNO_RUNNING_DFL		        24
#define AX100_FNO_RX				1
#define AX100_FNO_RX_DFL			25
#define AX100_FNO_TX				5
#define AX100_FNO_TX_DFL			29
#define AX100_FNO_HMACS                         6
#define AX100_FNO_HMACS_DFL                     30

/** PARAM INDEX MAP */
#define AX100_PARAM_RUNNING			0
#define AX100_PARAM_RX				1
#define AX100_PARAM_AX5043			3
#define AX100_PARAM_TELEM			4
#define AX100_PARAM_TX(id)			(5 + id)
#define AX100_PARAM_HMACS                       6d

void RadBufFree();
void * RadInitialize(void * args);
void RSSI_Monitoring();
int16_t Return_RSSI();

void * Load_Paramtable0(void *);
void * Load_Paramtable1(void *);
void * Load_Paramtable5(void *);
void * Save_Paramtable0(void *);
void * Save_Paramtable1(void *);
void * Save_Paramtable5(void *);
void * Save2FRAM(void* param_id);
void SetRxFreq(uint32_t freq);
void SetTxFreq(uint32_t freq);
void SetRxBaud(uint32_t baud);
void SetTxBaud(uint32_t baud);
void UpdateFreq();
void UpdateBaud();
uint32_t GetRXbaud();
uint32_t GetTXbaud();

struct Ptable_0{
    int8_t rssi_offset;
    int16_t max_temp;
    float bgndrssl_ema;
    uint8_t csp_node;
    bool i2c_en;
    bool can_en;
    bool extptt_en;
    bool led_en;
    int8_t kiss_usart;
    int8_t gosh_usart;
    uint8_t i2c_addr;
    uint16_t i2c_khz;
    uint16_t can_khz;
    uint16_t reboot_in;
    uint32_t tx_inhibit;
    uint8_t log_store;
    uint8_t tx_pwr;
    uint16_t max_tx_time;
    uint16_t max_idle_time;
    char csp_rtable[96];
    bool legacy_hmac;
    uint32_t kiss_baud;


    const uint16_t rssi_offset_addr = 0x0000;
    const uint16_t max_temp_addr = 0x0002;
    const uint16_t bgndrssl_ema_addr = 0x0004;
    const uint16_t csp_node_addr = 0x0008;
    const uint16_t i2c_en_addr = 0x0009;
    const uint16_t can_en_addr = 0x000B;
    const uint16_t extptt_en_addr = 0x000C;
    const uint16_t led_en_addr = 0x000D;
    const uint16_t kiss_usart_addr = 0x000E;
    const uint16_t gosh_usart_addr = 0x000F;
    const uint16_t i2c_addr_addr = 0x0010;
    const uint16_t i2c_khz_addr = 0x0012;
    const uint16_t can_khz_addr = 0x0014;
    const uint16_t reboot_in_addr = 0x0018;
    const uint16_t tx_inhibit_addr = 0x001C;
    const uint16_t log_store_addr = 0x0020;
    const uint16_t tx_pwr_addr = 0x0021;
    const uint16_t max_tx_time_addr = 0x002C;
    const uint16_t max_idle_time_addr = 0x002E;
    const uint16_t csp_rtable_addr = 0x0030;
    const uint16_t legacy_hmac_addr = 0x0090;
    const uint16_t kiss_baud_addr = 0x0094;
};

struct Ptable_1{
    uint32_t freq;
    uint32_t baud;
    float modindex;
    uint16_t guard;
    uint8_t pllrang;
    uint8_t mode;
    bool csp_hmac;
    bool csp_rs;
    bool csp_crc;
    bool csp_rand;
    //char* csp_hmac_key;     //need to check
    char ax25_call[6];
    uint32_t bw;
    int32_t afcrange;

    const uint16_t freq_addr = 0x0000;
    const uint16_t baud_addr = 0x0004;
    const uint16_t modindex_addr = 0x0008;
    const uint16_t guard_addr = 0x000C;
    const uint16_t pllrang_addr = 0x000E;
    const uint16_t mode_addr = 0x000F;
    const uint16_t csp_hmac_addr = 0x0010;
    const uint16_t csp_rs_addr = 0x0011;
    const uint16_t csp_crc_addr = 0x0012;
    const uint16_t csp_rand_addr = 0x0013;
    const uint16_t csp_hmac_key_addr = 0x0020;
    const uint16_t ax25_call_addr = 0x0030;
    const uint16_t bw_addr = 0x0040;
    const uint16_t afcrange_addr = 0x0044;
};

struct Ptable_5{
    uint32_t freq;
    uint32_t baud;
    float modindex;
    uint16_t guard;
    uint8_t pllrang;
    uint8_t mode;
    bool csp_hmac;
    bool csp_rs;
    bool csp_crc;
    bool csp_rand;
    //char* csp_hmac_key;     //need to check
    char ax25_call[6];
    //int8_t preamb;          //need to check
    uint8_t preamblen;
    uint8_t preambflags;
    //int8_t intfrm;          //need to check
    uint8_t intfrmlen;
    uint8_t intfrmflags;
    int16_t rssibusy;
    uint8_t kup_delay;
    int16_t pa_level;

    const uint16_t freq_addr = 0x0000;
    const uint16_t baud_addr = 0x0004;
    const uint16_t modindex_addr = 0x0008;
    const uint16_t guard_addr = 0x000C;
    const uint16_t pllrang_addr = 0x000E;
    const uint16_t mode_addr = 0x000F;
    const uint16_t csp_hmac_addr = 0x0010;
    const uint16_t csp_rs_addr = 0x0011;
    const uint16_t csp_crc_addr = 0x0012;
    const uint16_t csp_rand_addr = 0x0013;
    const uint16_t csp_hmac_key_addr = 0x0020;
    const uint16_t ax25_call_addr = 0x0030;
    const uint16_t preamb_addr = 0x0040;
    const uint16_t preamblen_addr = 0x0041;
    const uint16_t preambflags_addr = 0x0042;
    const uint16_t intfrm_addr = 0x0043;
    const uint16_t intfrmlen_addr = 0x0044;
    const uint16_t intfrmflags_addr = 0x0045;
    const uint16_t rssibusy_addr = 0x0048;
    const uint16_t kup_delay_addr = 0x004A;
    const uint16_t pa_level_addr = 0x004C;
    const uint16_t ber_addr = 0x0050;
};



#endif