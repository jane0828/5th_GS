#include "cfe.h"
#include <unistd.h>

int32 io_csp_ftp_init(void);

int main(void) {
    io_csp_ftp_init();
    while (1) sleep(1000);
    return 0;
}
