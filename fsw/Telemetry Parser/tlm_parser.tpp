/*******************************************************************************************
 * @file tlm_parser.tpp
 * 
 * @author Han-Gyeol Ryu (morimn21@gmail.com)
 * 
 * @brief Parser template implementation.
 * 
 * @version 2.0
 * 
 * @date 2022-07-01
 * 
 * @copyright Copyright (c) 2021 Astrodynamics & Control Lab. Yonsei Univ.
 * 
 ******************************************************************************************/
#include <iostream>
#include <fstream>
#include <string.h>
#include <errno.h>

#include "packet/components/ccsds.h"
#include "utils/utils.h"
#include "utils/ParserStatus.h"
#include "tlm_parser.h"

const int mimanSpaceCraftID = 0x00000042;
const int mimanContentType  = 0x63464531;
static const ParserStatus goodStat(ParserStatus::TLM_GOOD);
static const ParserStatus notLoaded(ParserStatus::TLM_NOT_LOADED);


template<typename T>
TlmParser<T>::~TlmParser(void) {
    if (this->packetData) {
        delete[] this->packetData;
    }
}


template<typename T>
ParserStatus TlmParser<T>::LoadFromFile(std::string filename) {

    std::streamsize bytesRead;
    std::ifstream is(filename, std::ifstream::binary);
    size_t headerLength, totalBytes;

    if (!is.is_open() || is.bad()) {
        LogError("Failed to open file %s: %s\n", filename.c_str(), strerror(errno));
        return this->lastStatus.SetStatus(notLoaded);
    }

    if (!is.seekg(0, is.end)) {
        LogError("File seek error: %s\n", strerror(errno));
        is.close();
        return this->lastStatus.SetStatus(ParserStatus::TLM_ERR_SEEK);
    }

    totalBytes = is.tellg();
    headerLength = sizeof(fileHeader) + sizeof(secHeader);
    if (totalBytes < headerLength) {
        LogError("Total bytes %d is too small for a TLM file\n", totalBytes);
        is.close();
        return this->lastStatus.SetStatus(ParserStatus::TLM_TOO_SHORT);
    }


    if (!is.seekg(0, is.beg)) {
        LogError("File seek error: %s\n", strerror(errno));
        is.close();
        return this->lastStatus.SetStatus(ParserStatus::TLM_ERR_SEEK);
    }

    is.read((char*)&this->fileHeader, sizeof(this->fileHeader));
    bytesRead = is.gcount();
    if (bytesRead != sizeof(this->fileHeader)) {
        LogError("File header read error: only %d bytes out of %d have been read (%s)\n", bytesRead, strerror(errno));
        is.close();
        return this->lastStatus.SetStatus(ParserStatus::TLM_ERR_READ);
    }

    /* Some data has been first read at this point. Override specs now (if any had been). */
    if (this->packetData) {
        delete[] this->packetData;
    }
    this->status.SetStatus(notLoaded);
    this->totalBytes = totalBytes;
    this->dataBytes = totalBytes - headerLength;

    if (this->ValidateFileHeader() != true)  {
        if (!this->ignoreHeaderValidation) {
            LogError("Header validation failed (Try IgnoreHeaderValidation() to override).\n");
            is.close();
            return this->lastStatus;
        }
    }

    is.read((char*)&this->secHeader, sizeof(this->secHeader));
    bytesRead = is.gcount();
    if (bytesRead != sizeof(this->secHeader)) {
        LogError("Secondary header read error: only %d bytes out of %d have been read (%s)\n", bytesRead, strerror(errno));
        is.close();
        return this->lastStatus.SetStatus(ParserStatus::TLM_ERR_READ);
    }

    this->packetSize = sizeof(T);
    this->packetCount =  this->dataBytes / this->packetSize;
    if (this->dataBytes % this->packetSize) {
        LogWarning("Data byte count %d is not divisible by the packet size %d (%d stray bytes left)\n", 
                   this->dataBytes, this->packetSize, this->dataBytes % this->packetSize);
    }

    this->packetData = new T[this->packetCount];
    if (!this->packetData) {
        LogError("Memory allocation failed\n");
        is.close();
        return this->lastStatus.SetStatus(ParserStatus::TLM_ERR_MEM);
    }

    is.read((char*)this->packetData, this->dataBytes);
    bytesRead = is.gcount();
    if (bytesRead != this->dataBytes) {
        LogError("Data read error: only %d bytes out of %d have been read (%s)\n", bytesRead, strerror(errno));
        is.close();
        return this->lastStatus.SetStatus(ParserStatus::TLM_ERR_READ);
    }

    LogInfo("File loaded successfully: %s\n", filename.c_str());
    is.close();
    return this->lastStatus.SetStatus(goodStat);

}


template<typename T>
bool TlmParser<T>::IsValidTlm(void) const {
    return (this->status.IsGood()) 
    || (this->ignoreHeaderValidation && (this->status == ParserStatus::TLM_MISMATCH_CONTENT_TYPE || this->status == ParserStatus::TLM_MISMATCH_SPACECRAFT_ID));
}


template<typename T>
bool TlmParser<T>::IsValidTlmVerbose(const char* caller) const {
    bool ret = this->IsValidTlm();
    if (!ret) {
        Logger(caller, LL_ERROR, "Operation on invalid TLM file: %s\n", this->status.GetDescription().c_str());
    }
    return ret;
}
#define NUMNULL                 0
#define SANITY_CHECK(ret)       (this->IsValidTlmVerbose(__func__) ? (ret) : NUMNULL)



template<typename T>
ParserStatus TlmParser<T>::GetPacket(T& packet, int index) {
    if (SANITY_CHECK(true) != true) {
        return this->status;
    }
    if (index >= this->packetCount) {
        LogError("Index %d is out of bound (packet count is %d)\n", index, this->packetCount);
        return this->lastStatus.SetStatus(ParserStatus::TLM_ERR_RANGE);
    }
    packet = this->packetData[index];
    return this->lastStatus.SetStatus(goodStat);
}


template<typename T>
ParserStatus TlmParser<T>::GetFileHeader(CFE_FS_Header_t& header) {
    if (SANITY_CHECK(true) != true) {
        return this->lastStatus.SetStatus(this->status);
    }
    header = this->fileHeader;
    return this->lastStatus.SetStatus(goodStat);
}


template<typename T>
ParserStatus TlmParser<T>::GetSecHeader(DS_FileHeader_t& header) {
    if (SANITY_CHECK(true) != true) {
        return this->lastStatus.SetStatus(this->status);
    }
    header = this->secHeader;
    return this->lastStatus.SetStatus(goodStat);
}


template<typename T>
int TlmParser<T>::GetPacketCount(void) const {
    return SANITY_CHECK(this->packetCount);
}

template<typename T>
size_t TlmParser<T>::GetPacketLength(void) const {
    return SANITY_CHECK(this->packetSize);
}

template<typename T>
size_t TlmParser<T>::GetTotalBytes(void) const {
    return SANITY_CHECK(this->totalBytes);
}

template<typename T>
size_t TlmParser<T>::GetDataBytes(void) const {
    return SANITY_CHECK(this->dataBytes);
}

template<typename T>
ParserStatus TlmParser<T>::GetLastStatus(void) const {
    return SANITY_CHECK(this->lastStatus);
}



template<typename T>
uint32_t TlmParser<T>::GetPacketTimeStamp(int index) const {

    uint32_t timeStamp;

    if (SANITY_CHECK(true) != true) {
        return 0;
    }

    if (index >= this->GetPacketCount()) {
        LogError("Index %d is out of bound (packet count is %d)\n", index, this->packetCount);
        return 0;
    }

    timeStamp = reinterpret_cast<ccsds_tlm_header_t*>(&this->packetData[index])->sec;

    return this->isHostBigEndian ? timeStamp : swap32(timeStamp);
}

template<typename T>
uint16_t TlmParser<T>::GetPacketTimeStampSubSeconds(int index) const {

    uint16_t timeStamp;

    if (SANITY_CHECK(true) != true) {
        return 0;
    }

    if (index >= this->GetPacketCount()) {
        LogError("Index %d is out of bound (packet count is %d)\n", index, this->packetCount);
        return 0;
    }

    timeStamp = reinterpret_cast<ccsds_tlm_header_t>(this->packetData[index]).subsec;

    return this->isHostBigEndian ? timeStamp : swap16(timeStamp);

}

template<typename T>
void TlmParser<T>::IgnoreHeaderValidation(void) {
    this->ignoreHeaderValidation = true; 
}

template<typename T>
void TlmParser<T>::SetHostEndianToBigEndian(void) {
    this->isHostBigEndian = true;
}

template<typename T>
void TlmParser<T>::SetHostEndianToLittleEndian(void) {
    this->isHostBigEndian = false;
}




template<typename T>
uint32_t TlmParser<T>::GetCreateSeconds(void) const {
    return SANITY_CHECK(this->isHostBigEndian ? this->fileHeader.TimeSeconds : swap32(this->fileHeader.TimeSeconds));
}

template<typename T>
uint32_t TlmParser<T>::GetCreateSubseconds(void) const {
     return SANITY_CHECK(this->isHostBigEndian ? this->fileHeader.TimeSubSeconds : swap32(this->fileHeader.TimeSubSeconds));
}

template<typename T>
double TlmParser<T>::GetCreateMilliseconds(void) const {
    double Millisec;
    if (SANITY_CHECK(true) != true) {
        Millisec = this->GetCreateSeconds();
        Millisec += this->GetCreateSubseconds() / 4294967296.;
    }
    else Millisec = 0;
    return Millisec;
}

template<typename T>
uint32_t TlmParser<T>::GetCloseSeconds(void) const {
     return SANITY_CHECK(this->isHostBigEndian ? this->SecHeader.CloseSeconds : swap32(this->SecHeader.CloseSeconds));
}

template<typename T>
uint32_t TlmParser<T>::GetCloseSubseconds(void) const {
     return SANITY_CHECK(this->isHostBigEndian ? this->SecHeader.CloseSubsecs : swap32(this->SecHeader.CloseSubsecs));
}

template<typename T>
double TlmParser<T>::GetCloseMilliseconds(void) const {
    double Millisec;
    if (SANITY_CHECK(true) != true) {
        Millisec = this->GetCloseSeconds();
        Millisec += this->GetCloseSubseconds() / 4294967296.;
    }
    else Millisec = 0;
    return Millisec;
}

template<typename T>
uint16_t TlmParser<T>::GetFileTableIndex(void) const {
    return SANITY_CHECK(this->isHostBigEndian ? this->SecHeader.FileTableIndex : swap16(this->SecHeader.FileTableIndex));
}

template<typename T>
uint32_t TlmParser<T>::GetContentType(void) const {
    return SANITY_CHECK(this->isHostBigEndian ? this->fileHeader.ContentType : swap32(this->fileHeader.ContentType));
}

template<typename T>
uint32_t TlmParser<T>::GetSubType(void) const {
    return SANITY_CHECK(this->isHostBigEndian ? this->fileHeader.SubType : swap32(this->fileHeader.SubType));
}

template<typename T>
uint32_t TlmParser<T>::GetHeaderLength(void) const {
    return SANITY_CHECK(this->isHostBigEndian ? this->fileHeader.Length : swap32(this->fileHeader.Length));
}

template<typename T>
uint32_t TlmParser<T>::GetSpacecraftID(void) const {
    return SANITY_CHECK(this->isHostBigEndian ? this->fileHeader.SpacecraftID : swap32(this->fileHeader.SpacecraftID));
}

template<typename T>
uint32_t TlmParser<T>::GetProcessorID(void) const {
    return SANITY_CHECK(this->isHostBigEndian ? this->fileHeader.ProcessorID : swap32(this->fileHeader.ProcessorID));
}

template<typename T>
uint32_t TlmParser<T>::GetApplicationID(void) const {
    return SANITY_CHECK(this->isHostBigEndian ? this->fileHeader.ApplicationID : swap32(this->fileHeader.ApplicationID));
}


template<typename T>
bool TlmParser<T>::ValidateFileHeader(void) {

    uint32_t magicValidation;

    magicValidation = this->fileHeader.SpacecraftID;
    magicValidation = this->isHostBigEndian ? magicValidation : swap32(magicValidation);
    if (magicValidation != mimanSpaceCraftID) {
        LogWarning("Error: SCID validation failed (expected %X, got %X)\n", mimanSpaceCraftID, magicValidation);
        if (swap32(magicValidation) == mimanSpaceCraftID) {
            const char* tryThisOne = this->isHostBigEndian ? "SetHostEndianToLittleEndian" : "SetHostEndianToBigEndian";
            LogWarning("The host byte order may be incorretly set. Try %s\n", tryThisOne);
        }
        this->status.SetStatus(ParserStatus::TLM_MISMATCH_SPACECRAFT_ID);
        return false;
    }

    magicValidation = this->fileHeader.ContentType;
    magicValidation = this->isHostBigEndian ? magicValidation : swap32(magicValidation);
    if (magicValidation != mimanContentType) {
        LogWarning("Error: ContentType validation failed (expected %X, got %X)\n", mimanContentType, magicValidation);
        if (swap32(magicValidation) == mimanContentType) {
            const char* tryThisOne = this->isHostBigEndian ? "SetHostEndianToLittleEndian" : "SetHostEndianToBigEndian";
            LogWarning("The host byte order may be incorretly set. Try %s\n", tryThisOne);
        }
        this->status.SetStatus(ParserStatus::TLM_MISMATCH_CONTENT_TYPE);
        return false;
    }

    this->status.SetStatus(goodStat);
    return true;

}



// template <typename T>
// ParserStatus TlmParser<T>::RegisterPacketList(int numMembers, ps_type types[], size_t sizes[], const char* names[]) {

//     size_t sizeSum, packetSize;

//     if (!types || !sizes || !names) {
//         LogError("Incomplete arguments.\n");
//         return this->lastStatus.SetStatus(ParserStatus::TLM_ERR_NULL);
//     }

//     packetSize = this->GetPacketLength();
//     sizeSum = 0;
//     for (int i = 0; i < numMembers; i++) {
//         sizeSum += sizes[i];
//     }

//     if (sizeSum != packetSize) {
//         LogError("The sum of the member sizes %zu does not match the packet length %zu\n", sizeSum, packetSize);
//         return this->lastStatus.SetStatus(ParserStatus::TLM_ERR_RANGE);
//     }




// }




// template <typename T>
// ParserStatus TlmParser<T>::ListPacketData(int index) {

//     T packet;

//     if (this->GetPacket(packet, index) != goodStat) {
//         return lastStatus;
//     }

    


// }

