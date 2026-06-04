/*******************************************************************************************
 * @file tlm_parser.h
 * 
 * @author Han-Gyeol Ryu (morimn21@gmail.com)
 * 
 * @brief core Flight System DS (Data Storage) app file parser.
 * 
 * @version 2.0
 * 
 * @date 2022-07-01
 * 
 * @copyright Copyright (c) 2021 Astrodynamics & Control Lab. Yonsei Univ.
 * 
 ******************************************************************************************/
#pragma once


#include <stdint.h>
#include <string>
#include "packet/header.h"
#include "utils/ParserStatus.h"


template <typename T>
class TlmParser {

public:

    TlmParser(): status(ParserStatus(ParserStatus::TLM_NOT_LOADED)), packetData(NULL), isHostBigEndian(false), ignoreHeaderValidation(false) {};
    ~TlmParser();

    ParserStatus    LoadFromFile(std::string filename);

    /* Packet data extraction. */
    ParserStatus    GetPacket(T& packet, int index);
    ParserStatus    GetFileHeader(CFE_FS_Header_t& header);
    ParserStatus    GetSecHeader(DS_FileHeader_t& header);
    int             GetPacketCount(void) const;
    size_t          GetPacketLength(void) const;
    size_t          GetTotalBytes(void) const;
    size_t          GetDataBytes(void) const;
    ParserStatus    GetLastStatus(void) const;
    uint32_t        GetPacketTimeStamp(int index) const;
    uint16_t        GetPacketTimeStampSubSeconds(int index) const;

    /* Parser options. */
    bool IsValidTlm(void) const;
    bool IsValidTlmVerbose(const char* caller) const;
    void IgnoreHeaderValidation(void);
    void SetHostEndianToBigEndian(void);
    void SetHostEndianToLittleEndian(void);

    // ParserStatus RegisterPacketList(int numMembers, ps_type types[], size_t sizes[], const char* names[]);
    // ParserStatus ListPacketData(int index);
    // void ListPacketDataToStream(std::ofstream stream, int index);

    void HexDumpFileHeader(void) const;
    void HexDumpFileHeader(std::ostream stream) const;
    void HexDumpSecHeader(void) const;
    void HexDumpSecHeader(std::ostream stream) const;
    void HexDumpData(uint32_t length, uint32_t offset) const;
    void HexDumpData(std::ostream stream, uint32_t length, uint32_t offset) const;

    /* File header attributes */
    uint32_t GetCreateSeconds(void) const;
    uint32_t GetCreateSubseconds(void) const;
    double GetCreateMilliseconds(void) const;
    uint32_t GetCloseSeconds(void) const;
    uint32_t GetCloseSubseconds(void) const;
    double GetCloseMilliseconds(void) const;

    uint16_t GetFileTableIndex(void) const;
    const char* GetFileName(void) const;

    uint32_t GetContentType(void) const;
    uint32_t GetSubType(void) const;
    uint32_t GetHeaderLength(void) const;
    uint32_t GetSpacecraftID(void) const;
    uint32_t GetProcessorID(void) const;
    uint32_t GetApplicationID(void) const;


private:

    ParserStatus status;
    ParserStatus lastStatus;

    size_t totalBytes;
    size_t dataBytes;
    size_t packetSize;
    int packetCount;

    CFE_FS_Header_t fileHeader;
    DS_FileHeader_t secHeader;
    T*  packetData;

    bool isHostBigEndian;
    bool ignoreHeaderValidation;

    bool ValidateFileHeader(void);

};

#include "tlm_parser.tpp"
