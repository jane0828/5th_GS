#ifndef _SWTICH_H_
#define _SWITCH_H_

#include <stdint.h>
#include <inttypes.h>
#include <termios.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

void sig_handler(int signo, siginfo_t *si, void *context);
int init_signal();
int init_switch(const char * devname, int baudrate);
void fin_switch();
int switch_to_rx(uint8_t node);
int switch_to_tx(uint8_t node);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif