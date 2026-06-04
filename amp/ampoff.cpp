#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <locale.h>

const char * devname = "/dev/ttyACM0";
struct termios new_gpio_tio;
int gpio = 0;
int duration = 60 * 14;
char off = '0';
int status  = 0;

static int gpio_set_off(char cmd) {
    tcflush(gpio, TCIOFLUSH);
    char buf[64];
    sprintf(buf, "%c", cmd);
    buf[1] = 13;
    if(write(gpio, buf, 1) > 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
    memset(buf, 0, sizeof(buf));
}

int init_gpio() {
    gpio = open(devname, O_RDWR | O_NOCTTY | O_NDELAY); // Or O_RDWR | O_NOCTTY? // @@ 로테이터 열기
    // open 함수 https://www.it-note.kr/19
    // O_RDWR은 파일 디스크립터인 fd를 읽기와 쓰기 모드로 열기 위한 지정이며
    // O_NOCCTY와 O_NONBLOCK는 시리얼 통신 장치에 맞추어 추가했습니다.

    if (gpio < 0)
    {
		return -1;
	}
    // tcgetattr(rotator, &new_gpio_tio);
    tcflush(gpio, TCIOFLUSH);
    new_gpio_tio.c_cflag = B9600;
    new_gpio_tio.c_cflag |= (CLOCAL | CREAD); 	// Receiver and local mode
    new_gpio_tio.c_cflag &= ~HUPCL;				// Do not change modem signals
    new_gpio_tio.c_cflag &= ~CSIZE;
    new_gpio_tio.c_cflag |= CS8;					// 8 bit // @@ S대역 포지셔너(로테이터) 컨트롤러이랑 같은 옵션이듯 // @@ 시리얼 통신 설정방법: https://softtone-someday.tistory.com/15, https://blog.naver.com/choi125496/130034222760
    new_gpio_tio.c_cflag &= ~CSTOPB;				// 1 stop bit
    new_gpio_tio.c_cflag &= ~PARENB;				// No parity check
    new_gpio_tio.c_cflag &= ~CRTSCTS;			// Enable hardware / software flow control
    new_gpio_tio.c_iflag &= ~IXON;				// No handshaking

    // Raw output
    new_gpio_tio.c_oflag &= ~OPOST; 				// Prevent special interpretation of output bytes (e.g. newline chars)

    // Canonical input
    new_gpio_tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);	// Non-canonical read

    // Initialize control characters
    new_gpio_tio.c_cc[VTIME]    = 5;
    new_gpio_tio.c_cc[VMIN]     = 1;		// Block 0.5(VTIME) sec after read first(VMIN) character

    tcflush(gpio, TCIOFLUSH);
    


    if (tcsetattr(gpio, TCSANOW, &new_gpio_tio) != 0)
    {
        
        return -1;
    }
    	
    return 0;
}

int main() {
    status = init_gpio();
    printf("\n");
    if(status)
    {
        printf("Failed to Initialize GPIO.\n");
        return -1;
    }
    else
    {
        printf("GPIO Init Success.\n");
    }

    time_t ref, now;
    struct tm * tm_info;
    char buffer [64];
    time(&ref);
    tm_info = localtime(&ref);
    strftime(buffer, sizeof(buffer), "%H:%M:%S", tm_info);
    
    if(gpio_set_off(off) == 0){
        printf("Command off success at %s \n", buffer);
    }
    else{
        printf("Command off fail at %s \n", buffer);
    }
   
    memset(buffer, 0, sizeof(buffer));


    printf("Now Finish GPIO Task.\n");

    close(gpio);

    return 0;
}
