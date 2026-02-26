/**
* $Id: DUACmdMgr.h 6399 2010-12-28 16:29:28Z tra18693 
*/
#ifndef DUACmdMgr_H
#define DUACmdMgr_H

#include "EventProcessor.h"
#include "UDPNetworkDevice.h"
#include "SWCMsgTypes.h"
#include "DUHWMgr.h"
#include "UIODevice.h"
#include "TimerDevice.h"
#include "Logger.h"
#include "ElapsedTimer.h"
#include "Timestamp.h"

class DUACmdMgr : public EventProcessor
{
    public:
        /**
         * Constructor
         * @param name - Event processor name
         */
        DUACmdMgr();

        /**
         * Destructor
         */
        ~DUACmdMgr();

        virtual STATUS start();

    protected:
        /**
         * Sends pertinent state data for this event processor to standard output.
        */
        //virtual void printInfo();

    private:

        int MODULE_TYPE;
        Logger &_logger;

        // Test Server Device
        UDPNetworkDevice* _fromTestServer;
        UDPNetworkDevice* _toTestServer;
        void processTestServerMsg();

        UDPNetworkDevice* _localHWStatus;
        void processLocalHWStatusMsg();

        /* Common config parameters */
        int FORCE_TEST_MODE;

        DUHWMgr &_duHWMgr;

        UIODevice* _uioDevSL;
        void processSLInterrupt();
        UIODevice* _uioDevConfig;
        void processConfigInterrupt();
        TimerDevice* _timerDevStatus;
        void processStatusTimer();
        void calcStatus();
        ElapsedTimer eInterruptProcessing;
        Timestamp ts;

        UDPNetworkDevice* _udpFromDevPC;
        UDPNetworkDevice* _udpToDevPC;
        void processDEVPCMsg();

        bool sendProcessedSW;
        int lastProcessedAlpha;
        int lastProcessedBeta;

        BetaDCUStatusMsg _localStatus[4];
        int localStatusCounter;

};


#endif      // DUACmdMgr_H
