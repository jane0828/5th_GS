// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <stdint.h>
// #include <iostream>
// #include <fstream>
// #include <ctime>
// #include <unistd.h>
// #include <termios.h>
// #include <fcntl.h>
// #include <locale.h>


// const char * devname = "/dev/ttyACM0"; //arduino device
// struct termios new_gpio_tio;
// int gpio = 0;
// int duration = 60*14;
// char on = '1'; //amp on cmd
// char off = '0'; //ampf off cmd
// int status  = 0;

// static int gpio_set_on(char cmd) {
//     tcflush(gpio, TCIOFLUSH);
//     char buf[64];
//     sprintf(buf, "%c", cmd);
//     buf[1] = 13;
//     printf("%c \n", buf[0]);
//     if(write(gpio, buf, 1) > 0)
//     {
//         return 0;
//     }
//     else
//     {
//         return -1;
//     }
//     memset(buf, 0, sizeof(buf));

// }

// static int gpio_set_off(char cmd) {
//     tcflush(gpio, TCIOFLUSH);
//     char buf[64];
//     sprintf(buf, "%c", cmd);
//     buf[1] = 13;
//     if(write(gpio, buf, 1) > 0)
//     {
//         return 0;
//     }
//     else
//     {
//         return -1;
//     }
//     memset(buf, 0, sizeof(buf));
// }

// int init_gpio() {
//     gpio = open(devname, O_RDWR | O_NOCTTY | O_NDELAY); // Or O_RDWR | O_NOCTTY? // @@ 로테이터 열기
    
//     // open 함수 https://www.it-note.kr/19
//     // O_RDWR은 파일 디스크립터인 fd를 읽기와 쓰기 모드로 열기 위한 지정이며
//     // O_NOCCTY와 O_NONBLOCK는 시리얼 통신 장치에 맞추어 추가했습니다.

//     if (gpio < 0)
//     {
//         return -1;
//     }
//     // tcgetattr(rotator, &new_gpio_tio);
//     tcflush(gpio, TCIOFLUSH);
//     new_gpio_tio.c_cflag = B9600;
//     new_gpio_tio.c_cflag |= (CLOCAL | CREAD);   // Receiver and local mode
//     new_gpio_tio.c_cflag &= ~HUPCL;             // Do not change modem signals
//     new_gpio_tio.c_cflag &= ~CSIZE;
//     new_gpio_tio.c_cflag |= CS8;                    // 8 bit // @@ S대역 포지셔너(로테이터) 컨트롤러이랑 같은 옵션이듯 // @@ 시리얼 통신 설정방법: https://softtone-someday.tistory.com/15, https://blog.naver.com/choi125496/130034222760
//     new_gpio_tio.c_cflag &= ~CSTOPB;                // 1 stop bit
//     new_gpio_tio.c_cflag &= ~PARENB;                // No parity check
//     new_gpio_tio.c_cflag &= ~CRTSCTS;           // Enable hardware / software flow control
//     new_gpio_tio.c_iflag &= ~IXON;              // No handshaking

//     // Raw output
//     new_gpio_tio.c_oflag &= ~OPOST;                 // Prevent special interpretation of output bytes (e.g. newline chars)
//     // Canonical input
//     new_gpio_tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);    // 

//     // Initialize control characters
//     new_gpio_tio.c_cc[VTIME]    = 5;
//     new_gpio_tio.c_cc[VMIN]     = 1;        // Block 0.5(VTIME) sec after read first(VMIN) character

//     tcflush(gpio, TCIOFLUSH);
    


//     if (tcsetattr(gpio, TCSANOW, &new_gpio_tio) != 0)
//     {
        
//         return -1;
//     }
        
//     return 0;
// }

// int main() {
//     status = init_gpio();
//     printf("%d \n", status);
//     if(status)
//     {
//         printf("error!!");
//         printf("Failed to Initialize GPIO.\n");
//         return -1;
//     }
//     else
//     {
//         printf("GPIO Init Success.\n");
//     }

//     printf("Device name : %s\n", devname);
//     printf("Filestream : %d\n", gpio);
//     printf("Default Max ON Duration : %dseconds\n", duration);
//     //printf("Selected port : %d\n", port);
//     printf("Start GPIO Safety Process.\n");

//     time_t ref, now;
//     struct tm * tm_info;
//     char buffer [64];
//     time(&ref);
//     tm_info = localtime(&ref);
//     strftime(buffer, sizeof(buffer), "%H:%M:%S", tm_info);

//     if(gpio_set_on(on) == 0){
//         printf("Command on success at %s", buffer);
//     }
//     else{
//         printf("Command on fail at %s", buffer);
//     };
//     tcflush(gpio, TCIOFLUSH);

//     memset(buffer, 0, sizeof(buffer));

//     while(1)
//     {
//         time(&now);
//         if(now - ref < duration)
//         {
//             sleep(1);
//             continue;
            
//         }
//         else
//             break;
        
//     }
//     time(&now);
//     gpio_set_off(off);
//     tcflush(gpio, TCIOFLUSH);

//     tm_info = localtime(&now);
//     strftime(buffer, sizeof(buffer), "%H:%M:%S", tm_info);
//     memset(buffer, 0, sizeof(buffer));

//     printf("Now Finish GPIO Task.\n");

//     close(gpio);
    
//     return 0;
// }


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cstdint>
#include <ctime>
#include <csignal>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

// ==============================
// Config
// ==============================
static const char* DEVNAME = "/dev/ttyACM0";  // Arduino CDC ACM
static const int   BAUD     = B9600;
static const int   DURATION_SEC = 60 * 14;    // 14 minutes

static const char  CMD_ON  = '1';
static const char  CMD_OFF = '0';

// ==============================
// Globals (for signal-safe cleanup)
// ==============================
static int g_fd = -1;
static volatile sig_atomic_t g_stop_requested = 0;

// ==============================
// Utility
// ==============================
static void log_time_prefix(const char* tag)
{
    std::time_t t = std::time(nullptr);
    std::tm tm {};
    localtime_r(&t, &tm);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
    std::printf("[%s] %s ", buf, tag);
}

static void die_perror(const char* msg)
{
    log_time_prefix("ERROR");
    std::fprintf(stderr, "%s: %s\n", msg, std::strerror(errno));
}

static int set_blocking(int fd, bool blocking)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;

    if (blocking) flags &= ~O_NONBLOCK;
    else          flags |=  O_NONBLOCK;

    return fcntl(fd, F_SETFL, flags);
}

static int write_all(int fd, const void* data, size_t len)
{
    const uint8_t* p = static_cast<const uint8_t*>(data);
    size_t sent = 0;

    while (sent < len)
    {
        ssize_t n = ::write(fd, p + sent, len - sent);
        if (n > 0)
        {
            sent += static_cast<size_t>(n);
            continue;
        }
        if (n == 0) continue;

        // n < 0
        if (errno == EINTR) continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // Shouldn't happen in blocking mode, but handle anyway
            usleep(1000);
            continue;
        }
        return -1;
    }
    return 0;
}

static int serial_open_and_configure(const char* devname)
{
    // Open in blocking mode
    int fd = ::open(devname, O_RDWR | O_NOCTTY);
    if (fd < 0) return -1;

    // Ensure blocking
    if (set_blocking(fd, true) != 0)
    {
        ::close(fd);
        errno = EINVAL;
        return -1;
    }

    // Get current settings first (IMPORTANT)
    struct termios tio {};
    if (tcgetattr(fd, &tio) != 0)
    {
        ::close(fd);
        return -1;
    }

    // Raw-ish mode: easiest and robust for simple command bytes
    cfmakeraw(&tio);

    // Baud rate
    if (cfsetispeed(&tio, BAUD) != 0 || cfsetospeed(&tio, BAUD) != 0)
    {
        ::close(fd);
        return -1;
    }

    // 8N1
    tio.c_cflag |= (CLOCAL | CREAD);
    tio.c_cflag &= ~PARENB;
    tio.c_cflag &= ~CSTOPB;
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8;
    tio.c_cflag &= ~CRTSCTS;

    // No software flow control
    tio.c_iflag &= ~(IXON | IXOFF | IXANY);

    // Read timeout behavior (not critical here, but sane defaults)
    tio.c_cc[VTIME] = 5;  // 0.5 sec
    tio.c_cc[VMIN]  = 0;

    // Apply settings
    if (tcsetattr(fd, TCSANOW, &tio) != 0)
    {
        ::close(fd);
        return -1;
    }

    // Flush any pending I/O
    tcflush(fd, TCIOFLUSH);

    return fd;
}

static int send_cmd(char cmd)
{
    if (g_fd < 0)
    {
        errno = EBADF;
        return -1;
    }

    // Send "<cmd>\r" (2 bytes)
    char pkt[2];
    pkt[0] = cmd;
    pkt[1] = '\r';

    // Flush before sending to avoid stale buffered reads/writes
    tcflush(g_fd, TCIOFLUSH);

    if (write_all(g_fd, pkt, sizeof(pkt)) != 0)
        return -1;

    // Optional: flush after sending (depends on device; harmless)
    tcdrain(g_fd);

    return 0;
}

static void safe_amp_off()
{
    if (g_fd >= 0)
    {
        if (send_cmd(CMD_OFF) == 0)
        {
            log_time_prefix("OK");
            std::printf("AMP OFF sent\n");
        }
        else
        {
            die_perror("Failed to send AMP OFF");
        }
    }
}

static void handle_signal(int)
{
    g_stop_requested = 1;
}

// ==============================
// Main
// ==============================
int main(int argc, char** argv)
{
    int duration = DURATION_SEC;

    // Allow override: ./ampcontrol 600  (seconds)
    if (argc >= 2)
    {
        int v = std::atoi(argv[1]);
        if (v > 0) duration = v;
    }

    // Install signal handlers (Ctrl+C / kill)
    std::signal(SIGINT,  handle_signal);
    std::signal(SIGTERM, handle_signal);

    g_fd = serial_open_and_configure(DEVNAME);
    if (g_fd < 0)
    {
        die_perror("Failed to open/configure serial device");
        return 1;
    }

    log_time_prefix("INFO");
    std::printf("Device: %s, Duration: %d sec\n", DEVNAME, duration);

    // AMP ON
    if (send_cmd(CMD_ON) == 0)
    {
        log_time_prefix("OK");
        std::printf("AMP ON sent\n");
    }
    else
    {
        die_perror("Failed to send AMP ON");
        ::close(g_fd);
        g_fd = -1;
        return 1;
    }

    // Wait loop with ability to stop early by signal
    std::time_t t0 = std::time(nullptr);
    while (!g_stop_requested)
    {
        std::time_t now = std::time(nullptr);
        int elapsed = static_cast<int>(now - t0);
        if (elapsed >= duration) break;
        sleep(1);
    }

    if (g_stop_requested)
    {
        log_time_prefix("INFO");
        std::printf("Stop requested (signal). Turning AMP OFF...\n");
    }
    else
    {
        log_time_prefix("INFO");
        std::printf("Duration elapsed. Turning AMP OFF...\n");
    }

    // AMP OFF (always)
    safe_amp_off();

    // Cleanup
    ::close(g_fd);
    g_fd = -1;

    log_time_prefix("INFO");
    std::printf("Finished.\n");
    return 0;
}
