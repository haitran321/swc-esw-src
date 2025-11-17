/**
* $Id: DUBCmdMgr.h 6399 2010-12-28 16:29:28Z tra18693 
*/
#ifndef DUBCmdMgr_H
#define DUBCmdMgr_H

#include "EventProcessor.h"
#include "UDPNetworkDevice.h"
#include "SWCMsgTypes.h"
#include "DUHWMgr.h"
#include "UIODevice.h"
#include "TimerDevice.h"
#include "Logger.h"
#include "ElapsedTimer.h"
#include "Timestamp.h"

class DUBCmdMgr : public EventProcessor
{
    public:
        /**
         * Constructor
         * @param name - Event processor name
         */
        DUBCmdMgr();

        /**
         * Destructor
         */
        ~DUBCmdMgr();

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

        /* Common config parameters */
        int FORCE_TEST_MODE;
        int STEERING_WORD_SRC;

        DUHWMgr &_duHWMgr;

        UIODevice* _uioDevSL;
        void processSLInterrupt();
        UIODevice* _uioDevConfig;
        void processConfigInterrupt();
        TimerDevice* _timerDevStatus;
        void processStatusTimer();
        void sendDCUStatusToDUA(BetaDCUStatusParamsType status);
        ElapsedTimer eInterruptProcessing;
        Timestamp ts;

        int lastAlpha;
        int lastBeta;
};


#endif      // DUBCmdMgr_H
