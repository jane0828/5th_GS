/*
Cubesat Space Protocol - A small network-layer protocol designed for Cubesats
Copyright (C) 2012 GomSpace ApS (http://www.gomspace.com)
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
#include "csp_conn.h"
#include "csp_promisc.h"

/* Custom only includes */
#include "miman_imgui.h"
#include "miman_csp.h"
#include "miman_config.h"
#include "miman_radial.h"

#ifdef CSP_USE_PROMISC
extern csp_queue_handle_t csp_promisc_queue;
#endif

extern Console console;
extern StateCheckUnit State;
extern pthread_mutex_t conn_lock;
extern int PingCounter;

// csp_iface_t csp_if_kiss;
// csp_kiss_handle_t csp_kiss_driver;
int fd_transciever;
int fd_transciever_serial;

usart_callback_t mimsart_callback = NULL;

//Temp
//// GS100
csp_iface_t csp_if_kiss;
csp_kiss_handle_t csp_kiss_driver;
char WatchdogReadBuf[32];
extern Setup * setup;
struct usart_conf conf;

void miman_usart_rx(uint8_t * buf, int len, void * pxTaskWoken) {
	csp_kiss_rx(&csp_if_kiss, buf, len, pxTaskWoken);
}


int init_transceiver()
{
	// csp_debug_toggle_level(CSP_PACKET);
	// csp_debug_toggle_level(CSP_INFO);
	// csp_debug_toggle_level(CSP_PROTOCOL);
	// csp_debug_toggle_level(CSP_LOCK);
	State.GS100_Connection = true;
	csp_buffer_init(2048, 2048);
	csp_init((uint8_t)setup->kiss_node);
	csp_route_set(CSP_DEFAULT_ROUTE, &csp_if_kiss, CSP_NODE_MAC);
	// csp_route_set(setup->obc_node, &csp_if_kiss, setup->gs100_node);
	// csp_route_set(setup->eps_node, &csp_if_kiss, setup->gs100_node);
	for (int i = 1; i < 32; ++i) {
		int ret;
		if (i == setup->gs100_node || i == setup->kiss_node)
			continue;
		if ((ret = csp_route_set(i, &csp_if_kiss, setup->gs100_node)) != CSP_ERR_NONE) {
			printf("csp_rtable_set err: returned %d\n", ret);
		}
	}
	csp_kiss_init(&csp_if_kiss, &csp_kiss_driver, usart_putc, usart_insert, "KISS");
	
	struct usart_conf usart;
	usart.device = setup->Transciever_devname;
	usart.baudrate = setup->Transceiver_baud;
	usart_init(&usart);

	usart_set_callback(miman_usart_rx);

	csp_rdp_set_opt(6, 30000, 16000, 1, 8000, 3);
	csp_route_start_task(0, 0);
	
	return 0;
}



int ResetWatchdog()
{
	struct termios options;
	fd_transciever_serial = open("/dev/GS101", O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (fd_transciever_serial < 0) {
		return -1;
	}


	tcgetattr(fd_transciever_serial, &options);
	options.c_cflag = B500000;
    options.c_cflag |= (CLOCAL | CREAD); 	// Receiver and local mode
    options.c_cflag &= ~HUPCL;				// Do not change modem signals
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;					// 8 bit // @@ S대역 포지셔너(로테이터) 컨트롤러이랑 같은 옵션이듯 // @@ 시리얼 통신 설정방법: https://softtone-someday.tistory.com/15, https://blog.naver.com/choi125496/130034222760
    options.c_cflag &= ~CSTOPB;				// 1 stop bit
    options.c_cflag &= ~PARENB;				// No parity check
    options.c_cflag &= ~CRTSCTS;			// Enable hardware / software flow control

    options.c_iflag &= ~IXON;				// No handshaking

    // Raw output
    options.c_oflag &= ~OPOST; 				// Prevent special interpretation of output bytes (e.g. newline chars)

    // Canonical input
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_cc[VTIME] = 5;
	options.c_cc[VMIN] = 1;

	tcsetattr(fd_transciever_serial, TCSANOW, &options);
	if (tcgetattr(fd_transciever_serial, &options) == -1)
		perror("error setting options");
	fcntl(fd_transciever_serial, F_SETFL, 0);

	/* Flush old transmissions */
	if (tcflush(fd_transciever_serial, TCIOFLUSH) == -1)
		printf("Error flushing serial port - %s(%d).\n", strerror(errno), errno);

	write(fd_transciever_serial, "\n\r", 2);
	read(fd_transciever_serial, WatchdogReadBuf, sizeof(WatchdogReadBuf));
	printf(WatchdogReadBuf);
	printf("---Done\n");
	write(fd_transciever_serial, "\n\r", 2);
	read(fd_transciever_serial, WatchdogReadBuf, sizeof(WatchdogReadBuf));
	printf(WatchdogReadBuf);
	printf("---Done\n");
	write(fd_transciever_serial, "\n\r", 2);
	read(fd_transciever_serial, WatchdogReadBuf, sizeof(WatchdogReadBuf));
	printf(WatchdogReadBuf);
	printf("---Done\n");
	sleep(0.5);
	write(fd_transciever_serial, "config gnd_wdt 172800\n\r", 23);
	read(fd_transciever_serial, WatchdogReadBuf, sizeof(WatchdogReadBuf));
	printf(WatchdogReadBuf);
	printf("---Done\n");
	sleep(0.5);
	write(fd_transciever_serial, "reset\n\r", 7);
	read(fd_transciever_serial, WatchdogReadBuf, sizeof(WatchdogReadBuf));
	printf(WatchdogReadBuf);
	printf("---Done\n");

	close(fd_transciever_serial);

	return 0;
}

void fin_transceiver()
{
	close(fd_transciever);
}


void * csp_ping_scan(void *) {
	pthread_mutex_lock(&conn_lock);
	uint32_t timeout = 1000;
	uint16_t size = setup->pingsize;
	uint8_t conn_options = 0;
	uint8_t nownode;
	bool scanmode = false;
	uint32_t start, time = 0;
	// uint8_t node = *((uint8_t *) nodedata);
	bool dlstate = State.downlink_mode;
	if((dlstate))
        State.downlink_mode = false;
	while(!State.uplink_mode)
        continue;
	State.uplink_mode = true;
	for(uint8_t i = 0; i < 32; i++)
	{
		int pingResult = csp_ping(i, 2000, size, CSP_O_NONE);
		if(pingResult >= 0)
			console.AddLog("Scanned AX100. Node : %"PRIu8", %"PRIu32"ms.\n", i, pingResult);
	}
	State.uplink_mode = false;
	if(dlstate)
    {
        State.downlink_mode = dlstate;
    }
	pthread_mutex_unlock(&conn_lock);
}

void * csp_baud_calibration(void *) {
	console.AddLog("[OK]##Baud rate Calibration Start");
	uint32_t baud_arr[]={1200,2400,4800,9600,19200}; // baud rate candidates
	uint32_t baud_cur_tx = 0;
	uint32_t baud_cur_rx = 0;
	bool flag = false;
	if(gs_rparam_get_uint32(setup->gs100_node, 1, 0x0004,0xB00B,100, &baud_cur_tx) ==GS_OK && gs_rparam_get_uint32(setup->gs100_node,5,0x0004,0xB00B,100,&baud_cur_rx)) {
		console.AddLog("[OK]##Current Baud rate TX: %u | RX: %u",baud_cur_tx,baud_cur_rx);
	}
	else {
		console.AddLog("[ERROR]##Can not Get baud rate. Try this later.");
	}
	// Change both (Rx & Tx) baud for Calibration
	for (int i=0; i<sizeof(baud_arr)/sizeof(uint32_t); i++) {
		baud_cur_tx = baud_arr[i];
		if(gs_rparam_set_uint32(setup->gs100_node, 5, 0x0004,0xB00B,100,baud_cur_tx)==GS_OK) { // Set Tx baud
			usleep(10000); // Sleep 0.01 Second per case
			for (int j= 0; j<sizeof(baud_arr)/sizeof(uint32_t); j++) {
				baud_cur_rx = baud_arr[j];
				if(gs_rparam_set_uint32(setup->gs100_node,1,0x0004,0xB00B,100,baud_cur_rx)==GS_OK) { //Set Rx baud
					usleep(10000); // Sleep 0.01 Second per case
					console.AddLog("[OK]##Current Baud rate TX: %u | RX: %u",baud_cur_tx,baud_cur_rx);
					int pingResult = csp_ping(setup->ax100_node,1000,setup->pingsize,CSP_O_NONE);
					usleep(10000); // Sleep 0.01 Second per case
					if (pingResult>=0) { // if ping success,
						console.AddLog("[OK]##Baud rate Calibration Success. Baud TX: %u | RX: %u",baud_cur_tx,baud_cur_rx);
						flag = true;
						break; //stop process
					}
					else {
						console.AddLog("[ERROR]##Ping Fail. Trying next candidate.");
						continue; //if ping fail, try next candidate
					}
				}
				else { // if Rxbaud rate change fail, try next candidate
					console.AddLog("[ERROR]##Can not Set Rx baud rate. Try this later.");
					continue;
				}
			}
			if (flag) break; // if ping success, stop process
		}
		else { // if Tx baud rate change fail, try next candidate
			console.AddLog("[ERROR]##Can not Set baud rate. Try this later.");
			continue;
		}
	}
	console.AddLog("[OK]##Frequency Calibration Finished. Freq Rx : %u | Tx : %u", baud_cur_rx, baud_cur_tx);
}

void * csp_freq_calibration(void *) {
	console.AddLog("[OK]##Frequency Calibration Start");
	// similar to baud rate calibration
	// table #4 rssi -> get receive power
	// choose best rssi (received signal strength indicator)
	// Add check box in autopliot
	double center_freq = setup->default_freq; // 436500000 = 436.5MHz
	double offset = 100000; // step size: 0.1 MHz
	double freq_cur_rx = center_freq;
	double freq_cur_tx = center_freq;
	int16_t rssi; // dBm scale. often -100 is minimum.
	int16_t max_rssi = -100; // store current max rssi value
	double freq[2] = {0,}; // store current max Rx, Tx freq
	for (int i=-65; i<15; i++) { // Range: 430 ~ 438 MHz
		freq_cur_rx = center_freq + i*offset;
		if(gs_rparam_set_uint32(setup->gs100_node, 1, 0x0000,0xB00B,100,freq_cur_rx)==GS_OK) { //Rx
			usleep(10000); // Sleep 0.01 Second per case
			for (int j=-65; j<15; j++) { // Range: 430 ~ 438 MHz
				freq_cur_tx = center_freq + j*offset;
				if(gs_rparam_set_uint32(setup->gs100_node,5,0x0000,0xB00B,100,freq_cur_tx)==GS_OK) { //Tx
					usleep(10000); // Sleep 0.01 Second per case
					console.AddLog("[OK]##Current Frequency Tx: %u | Rx: %u",freq_cur_tx,freq_cur_rx);
					int pingResult = csp_ping(setup->ax100_node,1000,setup->pingsize,CSP_O_NONE);
					usleep(10000); // Sleep 0.01 Second per case
					if (pingResult>=0) { // if ping success,
						if(gs_rparam_get_int16(setup->gs100_node,4,0x0004,0xB00B,100,&rssi) ==GS_OK) {
							if (rssi > max_rssi) {
								freq[0] = freq_cur_rx;
								freq[1] = freq_cur_tx;
								max_rssi = rssi;
								console.AddLog("[OK]##Max RSSI Updated. Freq Tx: %u Rx: %u",freq_cur_tx,freq_cur_rx);
								continue;
							}
						}
					}
					else  {
						console.AddLog("[ERROR]##Ping Fail. Trying next candidate.");
						continue;
					}
				}
				else { // Tx setting fail
					console.AddLog("[ERROR]##Can not Set Tx Frequency. Try this later.");
					continue;
				}
				usleep(100000); // Sleep 0.1 Second per case
			}
		}
		else { // Rx setting fail
			console.AddLog("[ERROR]##Can not Set Rx Frequency. Try this later.");
			continue;
		}
	}
	// If whole loof finished, Setting frequency to Max RSSI.
	gs_rparam_set_uint32(setup->gs100_node,1,0x0000,0xB00B,100,freq[0]); // Set Rx
	gs_rparam_set_uint32(setup->gs100_node,5,0x0000,0xB00B,100,freq[1]); // Set Tx
	console.AddLog("[OK]##Frequency Calibration Finished. Freq Rx : %u | Tx : %u | RSSI : %d",freq[0],freq[1],max_rssi);
}

void * csp_ping_console(void *)
{
	int pingResult = csp_ping(setup->ax100_node, 1000, setup->pingsize, CSP_O_NONE);
	if(pingResult >= 0)
	{
		console.AddLog("[OK]##Ping Success at node %u, %dms", setup->ax100_node, pingResult);
		PingCounter++;
	}
		
	else
		console.AddLog("[ERROR]##Ping Failed.");
}

void * csp_ping_rdp_crc32(void *)
{
	int pingResult = csp_ping(setup->ax100_node, 1000, setup->pingsize, CSP_O_RDP | CSP_O_CRC32);
	if(pingResult >= 0)
		console.AddLog("[OK]##Ping Success at node %u, %dms", setup->ax100_node, pingResult);
	else
		console.AddLog("[ERROR]##Ping Failed.");
}


// For ping every node
void* csp_ping_everynode_console(void *arg)
{
    uint8_t node = *(uint8_t*)arg;  
    free(arg);                     

    int pingResult = csp_ping(node, 1500, setup->pingsize, CSP_O_NONE);

    if (pingResult >= 0) {
        console.AddLog("[OK]##Ping Success at node %u, %dms", node, pingResult);
        PingCounter++;
    } else {
        console.AddLog("[ERROR]##Ping Failed at node %u.", node);
    }

    return NULL;
}








//Cause Endian Issue, this function is modified.
void csp_debug_callback(char * filename)
{
	char cwd[4096];
	char today[64];
	getcwd(cwd, 4096);
	todaystr(today, 64);
	strcat(cwd, "/log/csp_debug_");
	strcat(cwd, today);
	strcat(cwd, ".log");
	memcpy(filename, cwd, 4096);
}

