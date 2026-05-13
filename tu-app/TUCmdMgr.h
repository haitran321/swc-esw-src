/**
* $Id: TUCmdMgr.h 6399 2010-12-28 16:29:28Z tra18693 
*/
#ifndef TUCmdMgr_H
#define TUCmdMgr_H

#include "EventProcessor.h"
#include "UDPNetworkDevice.h"
#include "SWCMsgTypes.h"
#include "TUHWMgr.h"
#include "UIODevice.h"
#include "TimerDevice.h"
#include "Logger.h"
#include "ElapsedTimer.h"
#include "Timestamp.h"

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

        void processIncomingMsg();

        TUHWMgr &_tuHWMgr;

        UIODevice* _uioDevSL;
        void processSLInterrupt();
        UIODevice* _uioDevWLSP;
        void processWLSPInterrupt();
        TimerDevice* _timerDevStatus;
        void processStatusTimer();
        Timestamp ts;
        void sendTUStatusToDUA(DUTUStatusMsg status);

        UDPNetworkDevice* _udpFromStatusEmu;
        void processStatusEmuMsg();
};


#endif      // TUCmdMgr_H
