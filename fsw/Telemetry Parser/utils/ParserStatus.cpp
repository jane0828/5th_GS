#include "ParserStatus.h"
#include <map>


static std::map<int, std::string> mdesc = { 
    {ParserStatus::TLM_GOOD,                    "TLM loaded and good"},
    {ParserStatus::TLM_NOT_LOADED,              "No TLM file loaded"},
    {ParserStatus::TLM_TOO_SHORT,               "TLM file is too small"},
    {ParserStatus::TLM_MISMATCH_CONTENT_TYPE,   "Content type verification failed"},
    {ParserStatus::TLM_MISMATCH_SPACECRAFT_ID,  "Spacecraft ID verification failed"},
    {ParserStatus::TLM_ERR_SEEK,                "File seek error"},
    {ParserStatus::TLM_ERR_READ,                "File read error"},
    {ParserStatus::TLM_ERR_RANGE,               "Index or value is out of range"},
    {ParserStatus::TLM_ERR_MEM,                 "No memory available for allocation"},
    {ParserStatus::TLM_GOOD,                    "Null pointer was passed"},
    {ParserStatus::TLM_ERR_UNKNOWN,             "Unknown error"},
    {ParserStatus::TLM_STATUS_UNSET,            "Status code is not initialized"},
};


bool ParserStatus::IsGood(void) const {
    return this->statusCode == TLM_GOOD;
}

bool ParserStatus::IsBad(void) const {
    return !this->IsGood();
}

std::string ParserStatus::GetDescription(void) const {
    return mdesc[this->statusCode];
}


ParserStatus ParserStatus::SetStatus(uint32_t value) {
    this->statusCode = value;
    return *this;
}

ParserStatus ParserStatus::SetStatus(const ParserStatus& status) {
    this->statusCode = status.GetStatus();
    return *this;
}

uint32_t ParserStatus::GetStatus(void) const {
    return this->statusCode;
}

bool ParserStatus::operator == (const ParserStatus& status) const {
    return this->statusCode == status.GetStatus();
}

bool ParserStatus::operator != (const ParserStatus& status) const {
    return this->statusCode != status.GetStatus();
}

ParserStatus ParserStatus::operator = (const ParserStatus& status) {
    this->statusCode = status.GetStatus();
    return status;
}
