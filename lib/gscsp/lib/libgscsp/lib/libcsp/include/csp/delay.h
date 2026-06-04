#ifndef _DELAY_H_
#define _DELAY_H_

#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

void set_serial_spec_micsec(uint32_t _kiss_baud, uint32_t _switch_baud, uint32_t _can_baud, uint32_t _queue_delay, uint8_t _gs100_node, uint32_t default_delay, uint32_t guard_delay);
void set_guard_spec_micsec(uint32_t _tx_baud, uint32_t _rx_baud, uint32_t _tx_guard_gs, uint32_t _rx_guard_gs, uint32_t _tx_guard_sat, uint32_t _rx_guard_sat);
uint8_t now_gs_node();
uint32_t tx_delay(uint16_t packetlength, uint8_t node);
uint32_t rx_delay(uint16_t packetlength, uint8_t node);
uint32_t switch_delay(uint8_t node);
uint32_t tx_delay_ms(uint16_t packetlength, uint8_t node);
uint32_t rx_delay_ms(uint16_t packetlength, uint8_t node);
uint32_t switch_delay_ms(uint8_t node);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif