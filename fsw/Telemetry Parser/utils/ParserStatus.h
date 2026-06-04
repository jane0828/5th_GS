#pragma once

#include <string>

class ParserStatus {
 
public:

    enum status_enum {

        TLM_GOOD                    = 1,

        TLM_NOT_LOADED              = 2,
        TLM_TOO_SHORT               = 10,
        TLM_MISMATCH_CONTENT_TYPE   = 11,
        TLM_MISMATCH_SPACECRAFT_ID  = 12,

        TLM_ERR_SEEK                = 30,
        TLM_ERR_READ                = 31,
        TLM_ERR_RANGE               = 32,
        TLM_ERR_MEM                 = 33,
        TLM_ERR_NULL                = 34,

        TLM_ERR_UNKNOWN             = 55,

        TLM_STATUS_UNSET            = 0,

    };

    ParserStatus(): statusCode(TLM_NOT_LOADED) {};
    ParserStatus(uint32_t status): statusCode(status) {};
    ParserStatus(const ParserStatus& status): statusCode(status.GetStatus()) {};

    bool IsGood(void) const;
    bool IsBad(void) const;

    std::string GetDescription(void) const;

    ParserStatus SetStatus(uint32_t value);
    ParserStatus SetStatus(const ParserStatus& status);
    uint32_t GetStatus(void) const;

    bool operator == (const ParserStatus& status) const;
    bool operator != (const ParserStatus& status) const;
    ParserStatus operator = (const ParserStatus& status);

private:

    uint32_t statusCode;

};