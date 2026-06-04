#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <csp/csp_autoconfig.h>
#include <csp/delay.h>
#include <csp/csp_types.h>
#include <csp/csp_debug.h>


#ifdef __cplusplus
extern "C" {
#endif

uint32_t kiss_baud = 0;
uint32_t tx_baud = 0;
uint32_t rx_baud = 0;
uint32_t switch_baud = 0;
static uint32_t tx_guard_gs;
static uint32_t rx_guard_gs;
static uint32_t tx_guard_sat;
static uint32_t rx_guard_sat;
uint32_t can_baud = 0;
static uint32_t queue_delay;
static uint8_t gs100_node;
static uint32_t default_del;

void set_serial_spec_micsec(uint32_t _kiss_baud, uint32_t _switch_baud, uint32_t _can_baud, uint32_t _queue_delay, uint8_t _gs100_node, uint32_t default_delay, uint32_t guard_delay)
{
    kiss_baud = _kiss_baud;
    switch_baud = _switch_baud;
    can_baud = _can_baud;
    queue_delay = _queue_delay * 1000;
    gs100_node = _gs100_node;
    //temp
    rx_guard_sat = guard_delay * 1000;
    default_del = default_delay * 1000;

    csp_log_info("Set Environment Delays : kiss %u switch : %u can : %u queue : %u gs100_node : %u", kiss_baud, switch_baud, can_baud, queue_delay, gs100_node);
}

void set_guard_spec_micsec(uint32_t _tx_baud, uint32_t _rx_baud, uint32_t _tx_guard_gs, uint32_t _rx_guard_gs, uint32_t _tx_guard_sat, uint32_t _rx_guard_sat)
{
    tx_baud = _tx_baud;
    rx_baud = _rx_baud;
    tx_guard_gs = _tx_guard_gs;
    rx_guard_gs = _rx_guard_gs;
    tx_guard_sat = _tx_guard_sat;
    rx_guard_sat = _rx_guard_sat;
    csp_log_info("Set Parameter Delays : tx : %u rx : %u tx_guard : %u rx_guard : %u", tx_baud, rx_baud, tx_guard_gs, rx_guard_gs);
}

uint8_t now_gs_node()
{
    return gs100_node;
}


uint32_t tx_delay(uint16_t packetlength, uint8_t node)
{
    //For RDP Packet Header, the RDP Segments would be placed in CSP_PADDING_BYTES.
    //For MIMAN Project, this value is 8.
    if(node == gs100_node)
        return 0;
    uint32_t header_bytes = CSP_PADDING_BYTES + sizeof(csp_id_t);
    uint32_t crc_bytes = sizeof(uint32_t);
    uint32_t real_length = (uint32_t)packetlength + header_bytes;

#ifdef CSP_USE_CRC32
	real_length += crc_bytes;
#endif
    uint32_t result = real_length * (8000000 / kiss_baud + 8000000 / tx_baud) + queue_delay;
    csp_log_lock("TX Delay : %u microsecond.", result);

    return result;
}

uint32_t rx_delay(uint16_t packetlength, uint8_t node)
{
    //For RDP Packet Header, the RDP Segments would be placed in CSP_PADDING_BYTES.
    //For MIMAN Project, this value is 8.
    if(node == gs100_node)
        return 0;
    uint32_t header_bytes = CSP_PADDING_BYTES + sizeof(csp_id_t);
    uint32_t crc_bytes = sizeof(uint32_t);
    uint32_t real_length = (uint32_t)packetlength + header_bytes;

#ifdef CSP_USE_CRC32
	real_length += crc_bytes;
#endif
    
    uint32_t result = real_length * (8000000 / kiss_baud + 8000000 / rx_baud) + queue_delay + rx_guard_sat + default_del;
    csp_log_info("RX Delay : %u microsecond.", result);
    return result;
}

uint32_t switch_delay(uint8_t node)
{
    if(node == gs100_node)
        return 0;
    uint32_t switch_delay_min = 25000;
    return switch_delay_min;
}

uint32_t tx_delay_ms(uint16_t packetlength, uint8_t node)
{
    uint32_t micsec_delay = tx_delay(packetlength, node);
    return micsec_delay / 1000;
}

uint32_t rx_delay_ms(uint16_t packetlength, uint8_t node)
{
    uint32_t micsec_delay = rx_delay(packetlength, node);
    return micsec_delay / 1000;
}

uint32_t switch_delay_ms(uint8_t node)
{
    uint32_t micsec_delay = switch_delay(node);
    return micsec_delay / 1000;
}

#ifdef __cplusplus
} /* extern "C" */
#endif