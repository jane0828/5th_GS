/*******************************************************************************************
 * @file example.cpp
 * 
 * @author Han-Gyeol Ryu (morimn21@gmail.com)
 * 
 * @brief For basic parser usage demonstrations.
 * 
 * @version 2.0
 * 
 * @date 2022-07-01
 * 
 * @copyright Copyright (c) 2021 Astrodynamics & Control Lab. Yonsei Univ.
 * 
 ******************************************************************************************/
#include <iostream>
#include <iomanip>
using std::cout;
using std::endl;


/**
 *  Include the two headers below.
 */
#include "tlm_parser.h"
#include "packets.h"


int main(void) {

    hk_packet_t hk;
    uint32_t u32;


    /**
     *  Create a specialized parser.
     */
    TlmParser<HK>* parser = new TlmParser<HK>;
    
    /**
     *  Other telemetry types:
     *      TlmParser<ADCS>* parser = new TlmParser<ADCS>;
     *      TlmParser<SNSR>* parser = new TlmParser<SNSR>;
     *      TlmParser<GPS>* parser = new TlmParser<GPS>;
     */


    /**
     *  Load a TLM file.
     */
    parser->LoadFromFile("hktlm00000000.hk");


    /**
     *  Extract header data.
     */
    u32 = parser->GetSpacecraftID();
    cout << "Spacecraft ID: 0x" << std::hex << u32 << std::dec << endl;


    /**
     *  Extract n-th packet.
     */
    cout << "AX100 BoardTemperature:" << endl;
    for (int i = 0; i < 10; i++) {
        parser->GetPacket(hk, i);
        u32 = parser->GetPacketTimeStamp(i);
        cout << "\ttime: " << u32 << " s" << std::setw(12) << "temp: " << hk.utrx.AX100.BoardTemperature/10. << " degC" << endl;
    }

    cout << "ICEPS BatteryInputVoltage:" << endl;
    for (int i = 10; i < 20; i++) {
        parser->GetPacket(hk, i);
        u32 = parser->GetPacketTimeStamp(i);
        cout << "\ttime: " << u32 << " s" << std::setw(12) << "volt: " << hk.eps.Battery.BatteryInputVoltage << " mV" << endl;
    }



    /**
     *  Invalid operations will display an error message and set lastStatus accordingly.
     */
    parser->GetPacket(hk, 10000);


    /**
     *  Check the status of the last operation to do error-handling stuffs (see the ParserStatus class).
     */
    if (parser->GetLastStatus().IsBad()) {
        /* Error handling here. */
        cout << "Last status description says: " << parser->GetLastStatus().GetDescription() << endl;
    }



    /**
     *  Don't forget to delete after use.
     */
    delete parser;

    return 0;

}
