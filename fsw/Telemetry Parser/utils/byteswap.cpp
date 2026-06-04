#include "utils.h"


uint32_t swap32(uint32_t u32) {
    return ((u32 & 0xFF000000) >> 24) |
           ((u32 & 0x00FF0000) >> 8)  |
           ((u32 & 0x0000FF00) << 8)  |
           ((u32 & 0x000000FF) << 24);
}


uint16_t swap16(uint16_t u16) {
    return ((u16 & 0xFF00) >> 8) | ((u16 & 0x00FF) << 8);
}
