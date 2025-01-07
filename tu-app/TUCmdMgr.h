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

        // Tx Module Configs
        int RFG_SW_TRIGGER;
        int RFG_INIT_TESTING;
        int RFG_MIN_START_TIME;
        int lastRFGActionStopTime[NUM_TU_CHANNELS];

        // TX HW Setup variables
        int INTERNAL_TRIGGER;
        int RFG_HW_SETUP_ENABLE;
        int RFG_HW_SETUP_CH;
        int RFG_HW_SETUP_START;
        int RFG_HW_SETUP_STOP;
        int RFG_HW_SETUP_FREQUENCY;
        int RFG_HW_SETUP_AMPLITUDE;
        int RFG_HW_SETUP_LFM;
        int RFG_HW_SETUP_PHASE;
        int RFG_HW_SETUP_PC;
        int RFG_DDS_FREQ_MHZ;

        float TR_FTW_Conversion_usec;
        float TR_FTW_Conversion_sec;
        float TR_RTW_Conversion;

        void processIncomingMsg();

        // Variables for TWGS Timing Triggers
        UDPNetworkDevice* _udpFromTWGS;
        TimingTriggersMsg _triggerMsgBuf[4];
        int triggerBufCounter;

        TUHWMgr &_tuHWMgr;

        UIODevice* _uio1Dev;
        void processInterrupt();
        ElapsedTimer eInterruptProcessing;

        // For status report
        UDPNetworkDevice* _statusRptToTWGS;

};


#endif      // TUCmdMgr_H
