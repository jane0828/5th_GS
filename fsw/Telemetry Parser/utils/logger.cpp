#include "utils.h"
#include <stdarg.h>


static void SetStringSpec(log_level level) {
    uint8_t color;
    bool bold = false;
    switch (level) {
        case LL_NORMAL: color = 0; break;
        case LL_ERROR: color = 31; bold = true; break;
        case LL_WARN: color = 31; break;
        case LL_INFO: color = 32; break;
        default: color = 0;
    }
    printf("\033[%u;%um", bold, color);
}


static void UnSetStringSpec(void) {
    printf("\033[0;0m");
}


static int LoggerImpl(FILE* stream, const char* caller, log_level level, const char* str, va_list va) {
    int n = -1;
    if (stream) {
        SetStringSpec(level);
        printf("<%s> ", caller);
        n = vfprintf(stream, str, va);
        UnSetStringSpec();
        fflush(stream);
    }
    return n;
}



int Logger(const char* caller, log_level level, const char* str, ...) {
    int n;
    va_list va;
    va_start(va, str);
    n = LoggerImpl(stdout, caller, level, str, va);
    va_end(va);
    return n;
}


int Logger(FILE* stream, const char* caller, log_level level, const char* str, ...) {
    int n;
    va_list va;
    va_start(va, str);
    n = LoggerImpl(stream, caller, level, str, va);
    va_end(va);
    return n;
}

