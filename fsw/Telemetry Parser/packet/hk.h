#ifndef _HK_H_
#define _HK_H_


#include "components/fm.h"
#include "components/eps.h"
#include "components/rwa.h"
#include "components/mtq.h"
#include "components/snsr.h"
#include "components/pay.h"
#include "components/utrx.h"
#include "components/stx.h"
#include "components/ts.h"
#include "components/ccsds.h"


typedef struct __attribute__((packed)) {
    ccsds_tlm_header_t      tlmHeader;
    FM_HkTlm_Payload_t      fm;
    EPS_HkTlm_Payload_t     eps;
    uint8_t padding[6];
    RWA_HkTlm_Payload_t     rwa;
    MTQ_HkTlm_Payload_t     mtq;
    SNSR_HkTlm_Payload_t    snsr;
    UTRX_HkTlm_Payload_t    utrx;
    STX_HkTlm_Payload_t     stx;
    //PAY_HkTlm_Payload_t     pay;
    //TS_HkTlm_Payload_t      ts;
    //uint8_t padding2[16];
} hk_packet_t;
#define a sizeof(TS_HkTlm_Payload_t)

#endif
