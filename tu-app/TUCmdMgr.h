/**
* $Id: TUCmdMgr.h 6399 2010-12-28 16:29:28Z tra18693 $
*/
#ifndef TUCmdMgr_H
#define TUCmdMgr_H

#include "EventProcessor.h"
#include "UDPNetworkDevice.h"
#include "DevMemDevice.h"
#include "WriteRegCmdMsg.h"
#include "LEDDevice.h"
#include "TUHWMgr.h"
#include "UIODevice.h"
#include "Logger.h"
#include "ElapsedTimer.h"

class TUCmdMgr : public EventProcessor
{
    public:
        /**
         * Constructor
         * @param name - Event processor name
         */
        TUCmdMgr();

        /**
         * Destructor
         */
        ~TUCmdMgr();

        virtual STATUS start();

    protected:
        /**
         * Sends pertinent state data for this event processor to standard output.
        */
        //virtual void printInfo();

    private:

        Logger &_logger;

        /**
         * UDP device for receiving Ent Network Msgs
         */
        Device* _udpFromRIMS;
        // UDPNetworkDevice* _udpOut;

        // Warm Restart Device
        UDPNetworkDevice* _udpWarmRestart;
        RIMSHeaderType _rimsHeaderBuf[4];
        int warmRestartBufCounter;
        void processWarmRestartMsg();

        int MODULE_TYPE;

        int TEST_STATUS;

        void processIncomingMsg();

        // Variables for TWGS Timing Triggers
        UDPNetworkDevice* _udpFromTWGS;
        TimingTriggersMsg _triggerMsgBuf[4];
        int triggerBufCounter;

        TUHWMgr &_tuHWMgr;

//      UIODevice* _uio1Dev;
        void processInterrupt();
        ElapsedTimer eInterruptProcessing;

        // For status report
        UDPNetworkDevice* _statusRptToTWGS;

};


#endif      // TUCmdMgr_H
