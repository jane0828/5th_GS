/**
 * Build this example on linux with:
 * ./waf configure --enable-examples --enable-if-kiss --with-driver-usart=linux --enable-crc32 clean build
 */

#include <stdio.h>
#include <csp/csp.h>
#include <csp/interfaces/csp_if_kiss.h>

#include <csp/drivers/usart.h>
#include <csp/arch/csp_thread.h>

#define PORT 10
#define MY_ADDRESS 8
// #define DEST_ADDRESS 10

#define SERVER_TIDX 0
#define CLIENT_TIDX 1
#define USART_HANDLE 0

CSP_DEFINE_TASK(task_server) {
    int running = 1;
    csp_socket_t *socket = csp_socket(CSP_SO_NONE);
    csp_conn_t *conn;
    csp_packet_t *packet;
    csp_packet_t *response;

    response = csp_buffer_get(sizeof(csp_packet_t) + 2);
    if( response == NULL ) {
        fprintf(stderr, "Could not allocate memory for response packet!\n");
        return CSP_TASK_RETURN;
    }
    response->data[0] = 'O';
    response->data[1] = 'K';
    response->length = 2;

    csp_bind(socket, CSP_ANY);
    csp_listen(socket, 10);

    printf("Server task started\r\n");

    while(running) {
        if( (conn = csp_accept(socket, 10000)) == NULL )
            continue;

        while( (packet = csp_read(conn, 100)) != NULL ) {
            switch( csp_conn_dport(conn) ) {
                case PORT:
                    if( packet->data[0] == 'H' )
                        running = 0;
                    printf("Packet received on PORT: %s\r\n", (char *) packet->data);
                    csp_buffer_free(packet);
                    csp_send(conn, response, 1000);
                    break;
                default:
                    csp_service_handler(conn, packet);
                    break;
            }
        }

        csp_close(conn);
    }

    csp_buffer_free(response);

    return CSP_TASK_RETURN;
}

int main(int argc, char **argv) {
    csp_debug_toggle_level(CSP_PACKET);
    csp_debug_toggle_level(CSP_INFO);

    csp_buffer_init(400, 512);
    csp_init(MY_ADDRESS);

    struct usart_conf conf;

#if defined(CSP_WINDOWS)
    conf.device = argc != 2 ? "COM4" : argv[1];
    conf.baudrate = CBR_9600;
    conf.databits = 8;
    conf.paritysetting = NOPARITY;
    conf.stopbits = ONESTOPBIT;
    conf.checkparity = FALSE;
#elif defined(CSP_POSIX)
    conf.device = argc != 2 ? "/dev/ttyUSB0" : argv[1];
    conf.baudrate = 500000;
#elif defined(CSP_MACOSX)
    conf.device = argc != 2 ? "/dev/tty.usbserial-FTSM9EGE" : argv[1];
    conf.baudrate = 115200;
#endif

	/* Run USART init */
	usart_init(&conf);

    /* Setup CSP interface */
	static csp_iface_t csp_if_kiss;
	static csp_kiss_handle_t csp_kiss_driver;
	csp_kiss_init(&csp_if_kiss, &csp_kiss_driver, usart_putc, usart_insert, "KISS");
		
	/* Setup callback from USART RX to KISS RX */
	void my_usart_rx(uint8_t * buf, int len, void * pxTaskWoken) {
		csp_kiss_rx(&csp_if_kiss, buf, len, pxTaskWoken);
	}

	usart_set_callback(my_usart_rx);

    csp_route_set(CSP_DEFAULT_ROUTE, &csp_if_kiss, CSP_NODE_MAC);
    csp_route_start_task(0, 0);

    csp_conn_print_table();
    csp_route_print_table();
    csp_route_print_interfaces();

    csp_thread_handle_t handle_server;
    csp_thread_create(task_server, "SERVER", 1000, NULL, 0, &handle_server);

    /* Wait for program to terminate (ctrl + c) */
    while(1) {
    	csp_sleep_ms(1000000);
    }

    return 0;

}
