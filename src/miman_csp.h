#pragma once
/*
Cubesat Space Protocol - A small network-layer protocol designed for Cubesats
Copyright (C) 2012 Gomspace ApS (http://www.gomspace.com)
Copyright (C) 2012 AAUSAT3 Project (http://aausat3.space.aau.dk) 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/* Includes */
#include "miman_config.h"


#ifndef _MIMAN_CSP_H_
#define _MIMAN_CSP_H_

#ifdef __cplusplus
extern "C"{
#endif



void miman_usart_rx(uint8_t * buf, int len, void * pxTaskWoken);
int init_transceiver();
int ResetWatchdog();
void fin_transceiver();

int csp_transaction_persistent_switch(csp_conn_t * conn, uint32_t timeout, void * outbuf, int outlen, void * inbuf, int inlen);
int csp_transaction_persistent_custom(csp_conn_t * conn, uint32_t timeout, void * outbuf, int outlen, void * inbuf, int inlen);
int csp_transaction2_custom(uint8_t prio, uint8_t dest, uint8_t port, uint32_t timeout, void * outbuf, int outlen, void * inbuf, int inlen, uint32_t opts);
int csp_transaction_custom(uint8_t prio, uint8_t dest, uint8_t port, uint32_t timeout, void * outbuf, int outlen, void * inbuf, int inlen);

void * csp_ping_scan(void *);
void * csp_baud_calibration(void*);
void * csp_freq_calibration(void*);
void * csp_ping_console(void * param);
void * csp_ping_rdp_crc32(void *);
void* csp_ping_everynode_console(void *arg);
void csp_debug_callback(char * filename);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
