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
        void sendAckToTestServer(SWCAckType ackType);

        UDPNetworkDevice* _localHWStatus;

        /* Common config parameters */
        int FORCE_TEST_MODE;

        DUHWMgr &_duHWMgr;

        UIODevice* _uioDevSL;
        void processSLInterrupt();
        UIODevice* _uioDevConfig;
        void processConfigInterrupt();
        TimerDevice* _timerDevStatus;
        void processStatusTimer();
        void sendDCUStatusToDUA(BetaDCUStatusMsg status);
        void sendDUBStatusToDUA(DUTUStatusMsg status);

        bool sendProcessedSW;
        int lastProcessedAlpha;
        int lastProcessedBeta;

        UDPNetworkDevice* _udpFromStatusEmu;
        void processStatusEmuMsg();

        int REFRESH_DCU_STATUS_ON_GDS_INTERVAL;

};


#endif      // DUBCmdMgr_H
