//Custom Headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <iostream>
#include <fstream>
#include <termios.h>
#include <alloca.h>
#include <malloc.h>

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

//Custom CSP
#include <csp/switch.h>
#include <csp/delay.h>
// include <csp/csp_custom.h>
// #include "MIMAN_CSP/csp_io_custom.h"
// #include "MIMAN_CSP/csp_conn.h"

#include "miman_config.h"
#include "miman_radial.h"
#include "miman_csp.h"
#include "miman_imgui.h"
#include "miman_orbital.h"

extern Console console;

////rparam
Ptable_0 param0;
Ptable_1 param1;
Ptable_5 param5;
extern CoordTopocentric TrackingTopo;
extern StateCheckUnit State;
extern pthread_t p_thread[16];


static int GetResult;
static int SetResult;

//Actual Result Value is here.
uint8_t val_buf_u8;
uint16_t val_buf_u16;
uint32_t val_buf_u32;
int8_t val_buf_i8;
int16_t val_buf_i16;
int32_t val_buf_i32;
float val_buf_f;
extern Setup * setup;
int16_t lastRSSI = 0;
int def_timeout = 100;
extern Ptable_0 rparam0;
extern Ptable_1 rparam1;
extern Ptable_5 rparam5;

void RadBufFree()
{
    val_buf_u8 = 0;
    val_buf_u16 = 0;
    val_buf_u32 = 0;
    val_buf_i8 = 0;
    val_buf_i16 = 0;
    val_buf_i32 = 0;
    val_buf_f = 0;
}

void * RadInitialize(void * args)
{
    Setup * setups = (Setup *) args;
    // rparam_set_quiet(RPARAM_NOT_QUIET);
    // rparam_query_reset();
    RadBufFree();

    Load_Paramtable0(NULL);
    Load_Paramtable1(NULL);
    Load_Paramtable5(NULL);
    set_guard_spec_micsec(param5.baud, param1.baud, param5.guard, param1.guard, rparam5.guard, rparam1.guard);
}

void RSSI_Monitoring()
{
    gs_rparam_get_int16(setup->gs100_node, 4, 0x0004, 0xB00B, 100, &lastRSSI);
}

int16_t Return_RSSI()
{
    return lastRSSI;
}

void * Load_Paramtable0(void*)
{
    int GetResult;
    int OKResult = 0;

    RadBufFree();
    // rparam_set_quiet(RPARAM_NOT_QUIET);
    //Get rssi_offset
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 0, param0.rssi_offset_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param0.rssi_offset = val_buf_i8;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : rssi_offset", OKResult);
    }
    
    //Get max_temp
    if ( GetResult = gs_rparam_get_int16(setup->gs100_node, 0, param0.max_temp_addr, 0xB00B, 100, &(val_buf_i16)) == GS_OK)
        param0.max_temp = val_buf_i16;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : max_temp", OKResult);
    }

    //Get bgndrssi_ema
    if ( GetResult = gs_rparam_get_float(setup->gs100_node, 0, param0.bgndrssl_ema_addr, 0xB00B, 100, &(val_buf_f)) == GS_OK)
        param0.bgndrssl_ema = val_buf_f;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : bgndrssl_ema", OKResult);
    }

    //Get csp_node
    if ( GetResult = gs_rparam_get_uint8(setup->gs100_node, 0, param0.csp_node_addr, 0xB00B, 100, &(val_buf_u8)) == GS_OK)
        param0.csp_node = val_buf_u8;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : csp_node", OKResult);
    }

    //Get i2c_en
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 0, param0.i2c_en_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param0.i2c_en = (bool)val_buf_i8;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : i2c_en", OKResult);
    }

    //Get can_en
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 0, param0.can_en_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param0.can_en = (bool)val_buf_i8;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : can_en", OKResult);
    }

    //Get extptt_en
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 0, param0.extptt_en_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param0.extptt_en = (bool)val_buf_i8;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : extptt_en", OKResult);
    }

    //Get led_en
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 0, param0.led_en_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param0.led_en = (bool)val_buf_i8;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : led_en", OKResult);
    }
    
    //Get kiss_usart
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 0, param0.kiss_usart_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param0.kiss_usart = val_buf_i8;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : kiss_usart", OKResult);
    }
    
    //Get gosh_usart
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 0, param0.gosh_usart, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param0.gosh_usart = val_buf_i8;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : gosh_usart", OKResult);
    }

    //Get i2c_addr
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 0, param0.i2c_addr_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param0.i2c_addr = val_buf_u8;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : i2c_addr", OKResult);
    }

    //Get i2c_khz
    if ( GetResult = gs_rparam_get_uint16(setup->gs100_node, 0, param0.i2c_khz_addr, 0xB00B, 100, &(val_buf_u16)) == GS_OK)
        param0.i2c_khz = val_buf_u16;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : i2c_khz", OKResult);
    }

    //Get can_khz
    if ( GetResult = gs_rparam_get_uint16(setup->gs100_node, 0, param0.can_khz_addr, 0xB00B, 100, &(val_buf_u16)) == GS_OK)
        param0.can_khz = val_buf_u16;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : can_khz", OKResult);
    }

    //Get reboot_in
    if ( GetResult = gs_rparam_get_uint16(setup->gs100_node, 0, param0.reboot_in_addr, 0xB00B, 100, &(val_buf_u16)) == GS_OK)
        param0.reboot_in = val_buf_u16;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : reboot_in", OKResult);
    }

    //Get tx_inhibit
    if ( GetResult = gs_rparam_get_uint32(setup->gs100_node, 0, param0.tx_inhibit_addr, 0xB00B, 100, &(val_buf_u32)) == GS_OK)
        param0.tx_inhibit = val_buf_u32;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : tx_inhibit", OKResult);
    }

    //Get log_store
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 0, param0.log_store_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param0.log_store = val_buf_u8;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : log_store", OKResult);
    }

    //Get tx_pwr
    if ( GetResult = gs_rparam_get_uint8(setup->gs100_node, 0, param0.tx_pwr_addr, 0xB00B, 100, &(val_buf_u8)) == GS_OK)
        param0.tx_pwr = val_buf_u8;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : tx_pwr", OKResult);
    }

    //Get max_tx_time
    if ( GetResult = gs_rparam_get_uint16(setup->gs100_node, 0, param0.max_tx_time_addr, 0xB00B, 100, &(val_buf_u16)) == GS_OK)
        param0.max_tx_time = val_buf_u16;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : max_tx_time", OKResult);
    }

    //Get max_idle_time
    if ( GetResult = gs_rparam_get_uint16(setup->gs100_node, 0, param0.max_idle_time_addr, 0xB00B, 100, &(val_buf_u16)) == GS_OK)
        param0.max_idle_time = val_buf_u16;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : max_idle_time", OKResult);
    }

    //Get max_idle_time
    if ( GetResult = gs_rparam_get_string(setup->gs100_node, 0, param0.csp_rtable_addr, 0xB00B, 100, param0.csp_rtable, sizeof(param0.csp_rtable)) == GS_OK)
    {}
        // param0.csp_rtable = param0.csp_rtable_buf;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : csp_rtable", OKResult);
    }

    RadBufFree();
    if (OKResult > 0)
        console.AddLog("[ERROR]##Cannot Read %d parameters of Table 0.", OKResult);
}

void * Load_Paramtable1(void*)
{
    int GetResult;
    int OKResult = 0;

    RadBufFree();

    //Get freq
    if ( GetResult = gs_rparam_get_uint32(setup->gs100_node, 1, param1.freq_addr, 0xB00B, 100, &(val_buf_u32)) == GS_OK)
        param1.freq = val_buf_u32;
    else
        OKResult += 1;

    //Get baud
    if ( GetResult = gs_rparam_get_uint32(setup->gs100_node, 1, param1.baud_addr, 0xB00B, 100, &(val_buf_u32)) == GS_OK)
        param1.baud = val_buf_u32;
    else
        OKResult += 1;

    //Get modindex
    if ( GetResult = gs_rparam_get_float(setup->gs100_node, 1, param1.modindex_addr, 0xB00B, 100, &(val_buf_f)) == GS_OK)
        param1.modindex = val_buf_f;
    else
        OKResult += 1;

    //Get guard
    if ( GetResult = gs_rparam_get_uint16(setup->gs100_node, 1, param1.guard_addr, 0xB00B, 100, &(val_buf_u16)) == GS_OK)
        param1.guard = val_buf_u16;
    else
        OKResult += 1;

    //Get pllrang
    if ( GetResult = gs_rparam_get_uint8(setup->gs100_node, 1, param1.pllrang_addr, 0xB00B, 100, &(val_buf_u8)) == GS_OK)
        param1.pllrang = val_buf_u8;
    else
        OKResult += 1;

    //Get mode
    if ( GetResult = gs_rparam_get_uint8(setup->gs100_node, 1, param1.mode_addr, 0xB00B, 100, &(val_buf_u8)) == GS_OK)
        param1.mode = val_buf_u8;
    else
        OKResult += 1;

    //Get csp_hmac
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 1, param1.csp_hmac_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param1.csp_hmac = (bool)val_buf_i8;
    else
        OKResult += 1;

    //Get csp_rs
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 1, param1.csp_rs_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param1.csp_rs = (bool)val_buf_i8;
    else
        OKResult += 1;

    //Get csp_crc
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 1, param1.csp_crc_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param1.csp_crc = (bool)val_buf_i8;
    else
        OKResult += 1;

    //Get csp_rand
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 1, param1.csp_rand_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param1.csp_rand = (bool)val_buf_i8;
    else
        OKResult += 1;

    //Get csp_hmac_key

    //Get ax25_call
    if ( GetResult = gs_rparam_get_string(setup->gs100_node, 1, param1.ax25_call_addr, 0xB00B, 100, param1.ax25_call, sizeof(param1.ax25_call)) == GS_OK)
    {}
        // param0.csp_rtable = param0.csp_rtable_buf;
    else
        OKResult += 1;

    //Get bw
    if ( GetResult = gs_rparam_get_uint32(setup->gs100_node, 1, param1.bw_addr, 0xB00B, 100, &(val_buf_u32)) == GS_OK)
        param1.bw = val_buf_u32;
    else
        OKResult += 1;

    //Get afcrange
    if ( GetResult = gs_rparam_get_int32(setup->gs100_node, 1, param1.afcrange_addr, 0xB00B, 100, &(val_buf_i32)) == GS_OK)
        param1.afcrange = val_buf_i32;
    else
        OKResult += 1;

    RadBufFree();
    if (OKResult > 0)
        console.AddLog("[ERROR]##Cannot Read %d parameters of Table 1.", OKResult);
}

void * Load_Paramtable5(void*)
{
    int GetResult;
    int OKResult = 0;

    RadBufFree();

    //Get freq
    if ( GetResult = gs_rparam_get_uint32(setup->gs100_node, 5, param5.freq_addr, 0xB00B, 100, &(val_buf_u32)) == GS_OK)
        param5.freq = val_buf_u32;
    else
        OKResult += 1;

    //Get baud
    if ( GetResult = gs_rparam_get_uint32(setup->gs100_node, 5, param5.baud_addr, 0xB00B, 100, &(val_buf_u32)) == GS_OK)
        param5.baud = val_buf_u32;
    else
        OKResult += 1;

    //Get modindex
    if ( GetResult = gs_rparam_get_float(setup->gs100_node, 5, param5.modindex_addr, 0xB00B, 100, &(val_buf_f)) == GS_OK)
        param5.modindex = val_buf_f;
    else
        OKResult += 1;

    //Get guard
    if ( GetResult = gs_rparam_get_uint16(setup->gs100_node, 5, param5.guard_addr, 0xB00B, 100, &(val_buf_u16)) == GS_OK)
        param5.guard = val_buf_u16;
    else
        OKResult += 1;

    //Get pllrang
    if ( GetResult = gs_rparam_get_uint8(setup->gs100_node, 5, param5.pllrang_addr, 0xB00B, 100, &(val_buf_u8)) == GS_OK)
        param5.pllrang = val_buf_u8;
    else
        OKResult += 1;

    //Get mode
    if ( GetResult = gs_rparam_get_uint8(setup->gs100_node, 5, param5.mode_addr, 0xB00B, 100, &(val_buf_u8)) == GS_OK)
        param5.mode = val_buf_u8;
    else
        OKResult += 1;

    //Get csp_hmac
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 5, param5.csp_hmac_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param5.csp_hmac = (bool)val_buf_i8;
    else
        OKResult += 1;

    //Get csp_rs
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 5, param5.csp_rs_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param5.csp_rs = (bool)val_buf_i8;
    else
        OKResult += 1;

    //Get csp_crc
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 5, param5.csp_crc_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param5.csp_crc = (bool)val_buf_i8;
    else
        OKResult += 1;

    //Get csp_rand
    if ( GetResult = gs_rparam_get_int8(setup->gs100_node, 5, param5.csp_rand_addr, 0xB00B, 100, &(val_buf_i8)) == GS_OK)
        param5.csp_rand = (bool)val_buf_i8;
    else
        OKResult += 1;

    //Get csp_hmac_key

    //Get ax25_call
    if ( GetResult = gs_rparam_get_string(setup->gs100_node, 5, param5.ax25_call_addr, 0xB00B, 100, param5.ax25_call, sizeof(param5.ax25_call)) == GS_OK)
    {}
        // param0.csp_rtable = param0.csp_rtable_buf;
    else
        OKResult += 1;

    //Get preamblen
    if ( GetResult = gs_rparam_get_uint8(setup->gs100_node, 5, param5.preamblen_addr, 0xB00B, 100, &(val_buf_u8)) == GS_OK)
        param5.preamblen = val_buf_u8;
    else
        OKResult += 1;

    //Get preambflags
    if ( GetResult = gs_rparam_get_uint8(setup->gs100_node, 5, param5.preambflags_addr, 0xB00B, 100, &(val_buf_u8)) == GS_OK)
        param5.preambflags = val_buf_u8;
    else
        OKResult += 1;

    //Get intfrmlen
    if ( GetResult = gs_rparam_get_uint8(setup->gs100_node, 5, param5.intfrmlen_addr, 0xB00B, 100, &(val_buf_u8)) == GS_OK)
        param5.intfrmlen = val_buf_u8;
    else
        OKResult += 1;

    //Get intfrmflags
    if ( GetResult = gs_rparam_get_uint8(setup->gs100_node, 5, param5.intfrmflags_addr, 0xB00B, 100, &(val_buf_u8)) == GS_OK)
        param5.intfrmflags = val_buf_u8;
    else
        OKResult += 1;

    //Get rssibusy
    if ( GetResult = gs_rparam_get_int16(setup->gs100_node, 5, param5.rssibusy_addr, 0xB00B, 100, &(val_buf_i16)) == GS_OK)
        param5.rssibusy = val_buf_i16;
    else
        OKResult += 1;

    //Get kup_delay
    if ( GetResult = gs_rparam_get_uint8(setup->gs100_node, 5, param5.kup_delay_addr, 0xB00B, 100, &(val_buf_u8)) == GS_OK)
        param5.kup_delay = val_buf_u8;
    else
        OKResult += 1;

    //Get pa_level
    if ( GetResult = gs_rparam_get_int16(setup->gs100_node, 5, param5.pa_level_addr, 0xB00B, 100, &(val_buf_i16)) == GS_OK)
        param5.pa_level = val_buf_i16;
    else
        OKResult += 1;

    RadBufFree();
    if (OKResult > 0)
        console.AddLog("[ERROR]##Cannot Read %d parameters of Table 5.", OKResult);
}



void * Save_Paramtable0(void *)
{
    int setResult;
    int OKResult = 0;

    // rparam_set_quiet(RPARAM_NOT_QUIET);
    //set rssi_offset
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 0, param0.rssi_offset_addr, 0xB00B, 100, param0.rssi_offset) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : rssi_offset", OKResult);
    }
    
    //set max_temp
    if ( setResult = gs_rparam_set_int16(setup->gs100_node, 0, param0.max_temp_addr, 0xB00B, 100, param0.max_temp) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : max_temp", OKResult);
    }

    //set bgndrssi_ema
    if ( setResult = gs_rparam_set_float(setup->gs100_node, 0, param0.bgndrssl_ema_addr, 0xB00B, 100, param0.bgndrssl_ema) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : bgndrssl_ema", OKResult);
    }

    //set csp_node
    if ( setResult = gs_rparam_set_uint8(setup->gs100_node, 0, param0.csp_node_addr, 0xB00B, 100, param0.csp_node) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : csp_node", OKResult);
    }

    //set i2c_en
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 0, param0.i2c_en_addr, 0xB00B, 100, param0.i2c_en) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : i2c_en", OKResult);
    }

    //set can_en
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 0, param0.can_en_addr, 0xB00B, 100, param0.can_en) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : can_en", OKResult);
    }

    //set extptt_en
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 0, param0.extptt_en_addr, 0xB00B, 100, param0.extptt_en) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : extptt_en", OKResult);
    }

    //set led_en
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 0, param0.led_en_addr, 0xB00B, 100, param0.led_en) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : led_en", OKResult);
    }
    
    //set kiss_usart
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 0, param0.kiss_usart_addr, 0xB00B, 100, param0.kiss_usart) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : kiss_usart", OKResult);
    }
    
    //set gosh_usart
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 0, param0.gosh_usart, 0xB00B, 100, param0.gosh_usart) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : gosh_usart", OKResult);
    }

    //set i2c_addr
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 0, param0.i2c_addr_addr, 0xB00B, 100, param0.i2c_addr) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : i2c_addr", OKResult);
    }

    //set i2c_khz
    if ( setResult = gs_rparam_set_uint16(setup->gs100_node, 0, param0.i2c_khz_addr, 0xB00B, 100, param0.i2c_khz) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : i2c_khz", OKResult);
    }

    //set can_khz
    if ( setResult = gs_rparam_set_uint16(setup->gs100_node, 0, param0.can_khz_addr, 0xB00B, 100, param0.can_khz) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : can_khz", OKResult);
    }

    //set reboot_in
    if ( setResult = gs_rparam_set_uint16(setup->gs100_node, 0, param0.reboot_in_addr, 0xB00B, 100, param0.reboot_in) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : reboot_in", OKResult);
    }

    //set tx_inhibit
    if ( setResult = gs_rparam_set_uint32(setup->gs100_node, 0, param0.tx_inhibit_addr, 0xB00B, 100, param0.tx_inhibit) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : tx_inhibit", OKResult);
    }

    //set log_store
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 0, param0.log_store_addr, 0xB00B, 100, param0.log_store) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : log_store", OKResult);
    }

    //set tx_pwr
    if ( setResult = gs_rparam_set_uint8(setup->gs100_node, 0, param0.tx_pwr_addr, 0xB00B, 100, param0.tx_pwr) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : tx_pwr", OKResult);
    }

    //set max_tx_time
    if ( setResult = gs_rparam_set_uint16(setup->gs100_node, 0, param0.max_tx_time_addr, 0xB00B, 100, param0.max_tx_time) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : max_tx_time", OKResult);
    }

    //set max_idle_time
    if ( setResult = gs_rparam_set_uint16(setup->gs100_node, 0, param0.max_idle_time_addr, 0xB00B, 100, param0.max_idle_time) == GS_OK)
    {}
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : max_idle_time", OKResult);
    }

    //set max_idle_time
    if ( setResult = gs_rparam_set_string(setup->gs100_node, 0, param0.csp_rtable_addr, 0xB00B, 100, param0.csp_rtable, sizeof(param0.csp_rtable)) == GS_OK)
    {}
        // param0.csp_rtable = param0.csp_rtable_buf;
    else
    {
        OKResult += 1;
        //console.AddLog("[ERROR]##Read Error : csp_rtable", OKResult);
    }

    
    if (OKResult > 0)
        console.AddLog("[ERROR]##Cannot Set %d parameters of Table 0.", OKResult);
}

void * Save_Paramtable1(void *)
{
    int setResult;
    int OKResult = 0;

    //set freq
    if ( setResult = gs_rparam_set_uint32(setup->gs100_node, 1, param1.freq_addr, 0xB00B, 100, param1.freq) == GS_OK)
    {}
    else
        OKResult += 1;

    //set baud
    if ( setResult = gs_rparam_set_uint32(setup->gs100_node, 1, param1.baud_addr, 0xB00B, 100, param1.baud) == GS_OK)
    {}
    else
        OKResult += 1;

    //set modindex
    if ( setResult = gs_rparam_set_float(setup->gs100_node, 1, param1.modindex_addr, 0xB00B, 100, param1.modindex) == GS_OK)
    {}
    else
        OKResult += 1;

    //set guard
    if ( setResult = gs_rparam_set_uint16(setup->gs100_node, 1, param1.guard_addr, 0xB00B, 100, param1.guard) == GS_OK)
    {}
    else
        OKResult += 1;

    //set pllrang
    if ( setResult = gs_rparam_set_uint8(setup->gs100_node, 1, param1.pllrang_addr, 0xB00B, 100, param1.pllrang) == GS_OK)
    {}
    else
        OKResult += 1;

    //set mode
    if ( setResult = gs_rparam_set_uint8(setup->gs100_node, 1, param1.mode_addr, 0xB00B, 100, param1.mode) == GS_OK)
    {}
    else
        OKResult += 1;

    //set csp_hmac
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 1, param1.csp_hmac_addr, 0xB00B, 100, param1.csp_hmac) == GS_OK)
    {}
    else
        OKResult += 1;

    //set csp_rs
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 1, param1.csp_rs_addr, 0xB00B, 100, param1.csp_rs) == GS_OK)
    {}
    else
        OKResult += 1;

    //set csp_crc
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 1, param1.csp_crc_addr, 0xB00B, 100, param1.csp_crc) == GS_OK)
    {}
    else
        OKResult += 1;

    //set csp_rand
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 1, param1.csp_rand_addr, 0xB00B, 100, param1.csp_rand) == GS_OK)
    {}
    else
        OKResult += 1;

    //set csp_hmac_key

    //set ax25_call
    if ( setResult = gs_rparam_set_string(setup->gs100_node, 1, param1.ax25_call_addr, 0xB00B, 100, param1.ax25_call, sizeof(param1.ax25_call)) == GS_OK)
    {}
        // param0.csp_rtable = param0.csp_rtable_buf;
    else
        OKResult += 1;

    //set bw
    if ( setResult = gs_rparam_set_uint32(setup->gs100_node, 1, param1.bw_addr, 0xB00B, 100, param1.bw) == GS_OK)
    {}
    else
        OKResult += 1;

    //set afcrange
    if ( setResult = gs_rparam_set_int32(setup->gs100_node, 1, param1.afcrange_addr, 0xB00B, 100, param1.afcrange) == GS_OK)
    {}
    else
        OKResult += 1;
    
    if (OKResult > 0)
        console.AddLog("[ERROR]##Cannot Set %d parameters of Table 1.", OKResult);
}

void * Save_Paramtable5(void *)
{
    int setResult;
    int OKResult = 0;

    //set freq
    if ( setResult = gs_rparam_set_uint32(setup->gs100_node, 5, param5.freq_addr, 0xB00B, 100, param5.freq) == GS_OK)
    {}
    else
        OKResult += 1;

    //set baud
    if ( setResult = gs_rparam_set_uint32(setup->gs100_node, 5, param5.baud_addr, 0xB00B, 100, param5.baud) == GS_OK)
    {}
    else
        OKResult += 1;

    //set modindex
    if ( setResult = gs_rparam_set_float(setup->gs100_node, 5, param5.modindex_addr, 0xB00B, 100, param5.modindex) == GS_OK)
    {}
    else
        OKResult += 1;

    //set guard
    if ( setResult = gs_rparam_set_uint16(setup->gs100_node, 5, param5.guard_addr, 0xB00B, 100, param5.guard) == GS_OK)
    {}
    else
        OKResult += 1;

    //set pllrang
    if ( setResult = gs_rparam_set_uint8(setup->gs100_node, 5, param5.pllrang_addr, 0xB00B, 100, param5.pllrang) == GS_OK)
    {}
    else
        OKResult += 1;

    //set mode
    if ( setResult = gs_rparam_set_uint8(setup->gs100_node, 5, param5.mode_addr, 0xB00B, 100, param5.mode) == GS_OK)
    {}
    else
        OKResult += 1;

    //set csp_hmac
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 5, param5.csp_hmac_addr, 0xB00B, 100, param5.csp_hmac) == GS_OK)
    {}
    else
        OKResult += 1;

    //set csp_rs
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 5, param5.csp_rs_addr, 0xB00B, 100, param5.csp_rs) == GS_OK)
    {}
    else
        OKResult += 1;

    //set csp_crc
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 5, param5.csp_crc_addr, 0xB00B, 100, param5.csp_crc) == GS_OK)
    {}
    else
        OKResult += 1;

    //set csp_rand
    if ( setResult = gs_rparam_set_int8(setup->gs100_node, 5, param5.csp_rand_addr, 0xB00B, 100, param5.csp_rand) == GS_OK)
    {}
    else
        OKResult += 1;

    //set csp_hmac_key

    //set ax25_call
    if ( setResult = gs_rparam_set_string(setup->gs100_node, 5, param5.ax25_call_addr, 0xB00B, 100, param5.ax25_call, sizeof(param5.ax25_call)) == GS_OK)
    {}
        // param0.csp_rtable = param0.csp_rtable_buf;
    else
        OKResult += 1;

    //set preamblen
    if ( setResult = gs_rparam_set_uint8(setup->gs100_node, 5, param5.preamblen_addr, 0xB00B, 100, param5.preamblen) == GS_OK)
    {}
    else
        OKResult += 1;

    //set preambflags
    if ( setResult = gs_rparam_set_uint8(setup->gs100_node, 5, param5.preambflags_addr, 0xB00B, 100, param5.preambflags) == GS_OK)
    {}
    else
        OKResult += 1;

    //set intfrmlen
    if ( setResult = gs_rparam_set_uint8(setup->gs100_node, 5, param5.intfrmlen_addr, 0xB00B, 100, param5.intfrmlen) == GS_OK)
    {}
    else
        OKResult += 1;

    //set intfrmflags
    if ( setResult = gs_rparam_set_uint8(setup->gs100_node, 5, param5.intfrmflags_addr, 0xB00B, 100, param5.intfrmflags) == GS_OK)
    {}
    else
        OKResult += 1;

    //set rssibusy
    if ( setResult = gs_rparam_set_int16(setup->gs100_node, 5, param5.rssibusy_addr, 0xB00B, 100, param5.rssibusy) == GS_OK)
    {}
    else
        OKResult += 1;

    //set kup_delay
    if ( setResult = gs_rparam_set_uint8(setup->gs100_node, 5, param5.kup_delay_addr, 0xB00B, 100, param5.kup_delay) == GS_OK)
    {}
    else
        OKResult += 1;

    //set pa_level
    if ( setResult = gs_rparam_set_int16(setup->gs100_node, 5, param5.pa_level_addr, 0xB00B, 100, param5.pa_level) == GS_OK)
    {}
    else
        OKResult += 1;


    
    if (OKResult > 0)
        console.AddLog("[ERROR]##Cannot Set %d parameters of Table 5.", OKResult);
}

void * Save2FRAM(void* param_id)
{
    int8_t table_id = *(int8_t*) param_id;
    uint8_t node = (uint8_t)setup->gs100_node;
    uint8_t port = AX100_PORT_RPARAM;
    gs_rparam_save(node, def_timeout, table_id, (uint8_t)table_id);
    set_guard_spec_micsec(param5.baud, param1.baud, param5.guard, param1.guard, rparam5.guard, rparam1.guard);
    usleep(500);
    // csp_reboot(setup->gs100_node);
}

void SetRxFreq(uint32_t freq)
{
    //Set freq
    gs_rparam_set_uint32(setup->gs100_node, 1, param1.freq_addr, 0xB00B, 100, freq);
}

void SetTxFreq(uint32_t freq)
{
    //Set freq
    gs_rparam_set_uint32(setup->gs100_node, 5, param5.freq_addr, 0xB00B, 100, freq);
}

void SetRxBaud(uint32_t baud)
{
    //Set Baud
    gs_rparam_set_uint32(setup->gs100_node, 1, param1.baud_addr, 0xB00B, 100, baud);
}

void SetTxBaud(uint32_t baud)
{
    //Set Baud
    gs_rparam_set_uint32(setup->gs100_node, 5, param5.baud_addr, 0xB00B, 100, baud);
}

void UpdateFreq()
{
    gs_rparam_get_uint32(setup->gs100_node, 1, param1.freq_addr, 0xB00B, 100, &param1.freq);
    gs_rparam_get_uint32(setup->gs100_node, 5, param5.freq_addr, 0xB00B, 100, &param5.freq);
}

void UpdateBaud()
{
    gs_rparam_get_uint32(setup->gs100_node, 1, param1.baud_addr, 0xB00B, 100, &param1.baud);
    gs_rparam_get_uint32(setup->gs100_node, 5, param5.baud_addr, 0xB00B, 100, &param5.baud);
}

uint32_t GetRXbaud()
{
    return param1.baud;
}

uint32_t GetTXbaud()
{
    return param5.baud;
}