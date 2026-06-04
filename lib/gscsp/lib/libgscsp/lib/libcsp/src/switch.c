#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#include <csp/csp_debug.h>
#include <csp/delay.h>
#include <csp/switch.h>

#ifdef __cplusplus
extern "C" {
#endif

int fd_switch = -1;
struct termios switch_tio;
struct sigaction sa;
char SwitchReadBuf[8];

void sig_handler(int signo, siginfo_t *si, void *context)
{
	exit(EXIT_FAILURE);
}

int init_signal()
{
	sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = sig_handler;
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGFPE,  &sa, NULL);
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
	return 0;
}

int init_switch(const char * devname, int baudrate)
{
	init_signal();
	fd_switch = open(devname, O_RDWR | O_NOCTTY | O_NDELAY); // Or O_RDWR | O_NOCTTY?
	if (fd_switch < 0) {
		//csp_log_error("Fail to open switch");
		return -1;
	}
	else
	{
		tcgetattr(fd_switch, &switch_tio); /* save current serial port settings */

		if (baudrate == 4800) {
			cfsetispeed(&switch_tio, B4800);
			cfsetospeed(&switch_tio, B4800);
		}
		else if (baudrate == 9600) {
			cfsetispeed(&switch_tio, B9600);
			cfsetospeed(&switch_tio, B9600);
		}
		else if (baudrate == 19200) {
			cfsetispeed(&switch_tio, B19200);
			cfsetospeed(&switch_tio, B19200);
		}
		else if (baudrate == 38400) {
			cfsetispeed(&switch_tio, B38400);
			cfsetospeed(&switch_tio, B38400);
		}
		else if (baudrate == 57600) {
			cfsetispeed(&switch_tio, B57600);
			cfsetospeed(&switch_tio, B57600);
		}
		else if (baudrate == 115200) {
			cfsetispeed(&switch_tio, B115200);
			cfsetospeed(&switch_tio, B115200);
		}
		else {
			close(fd_switch);
			fd_switch = -1;
			return -1;
		}
		
		switch_tio.c_cflag |= (CLOCAL); 	// Receiver and local mode
		switch_tio.c_cflag &= ~HUPCL;				// Do not change modem signals
		switch_tio.c_cflag &= ~CSIZE;
		switch_tio.c_cflag |= CS8;					// 8 bit
		switch_tio.c_cflag &= ~CSTOPB;				// 1 stop bit
		switch_tio.c_cflag &= ~PARENB;				// No parity check
		switch_tio.c_cflag &= ~CRTSCTS;			// No hardware / software flow control

		// Raw output
		switch_tio.c_oflag &= ~OPOST; 				// Prevent special interpretation of output bytes (e.g. newline chars)

		tcflush(fd_switch, TCIFLUSH);
		if (tcsetattr(fd_switch, TCSANOW, &switch_tio) != 0){
		//csp_log_error("Fail to open switch");
		return -1;
		}
	}
	return 0;
}

void fin_switch()
{
	if (fd_switch > 0)
    {
        tcflush(fd_switch, TCIOFLUSH);
        tcsetattr(fd_switch, TCSANOW, &switch_tio);
        tcflush(fd_switch, TCIOFLUSH);
        close(fd_switch);
    }
}


int switch_to_rx(uint8_t node) {
	if(node == now_gs_node())
	{
		return 0;
	}
	if (fd_switch == -1){
		csp_log_info("Switch does not connected");
		return -1;
	}
		
	// Turn on relay 2 only: 00000010
	if (write(fd_switch, "@00WR2\r", 7) < 1){
		csp_log_error("[ERROR]##Fail to turn on RX");
		return -2;
	}
	tcflush(fd_switch, TCIOFLUSH);
	usleep(switch_delay(node));
	csp_log_info("SWITCH : RX");

	return 0;
}

int switch_to_tx(uint8_t node) {
	if(node == now_gs_node())
	{
		return 0;
	}
	if (fd_switch == -1) {
		csp_log_info("[ERROR]##Switch does not connected");
		return -1;
	}

	// Turn on relay 2 only: 00000100
	if (write(fd_switch, "@00WR4\r", 7) < 1) {
		csp_log_error("[ERROR]##Fail to turn on TX");
		return -2;
	}
	tcflush(fd_switch, TCIOFLUSH);
	usleep(switch_delay(node));
	csp_log_info("SWITCH : TX");
	return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif