#pragma once


#include <stdint.h>


typedef union {
    struct __attribute__((packed)) {
        uint8_t priHdr[6];
        uint32_t sec;
        uint16_t subsec;
        uint8_t padding[4];
    };
    uint8_t byte[16];
} ccsds_tlm_header_t;
