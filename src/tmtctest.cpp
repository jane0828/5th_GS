#include "miman_config.h"
#include "miman_csp.h"
#include "miman_coms.h"
#include "miman_imgui.h"
#include "miman_config.h"
#include "miman_orbital.h"
#include "miman_radial.h"
#include "miman_ftp.h"

int uplink(void * sign_);
int downlink();

int main() {
    packetsign * sign;
    csp_packet_t * packet;
    csp_conn_t* txconn;
    int mode;
    printf("Select Mode: 1:Uplink 2: Downlink");
    scanf("%d",&mode);
    switch (mode) {
        case 1:
            uplink();
            break;
        case 2:
            downlink();
            break;
    }
    return 0;
}
int uplink(void * sign_) {
    
    return 0;
}

int downlink() {
    return 0;
}