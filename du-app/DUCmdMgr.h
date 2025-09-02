/**
* $Id: DUCmdMgr.h 6399 2010-12-28 16:29:28Z tra18693 
*/
#ifndef DUCmdMgr_H
#define DUCmdMgr_H

#include "EventProcessor.h"
#include "UDPNetworkDevice.h"
#include "SWCMsgTypes.h"
#include "DUHWMgr.h"
#include "UIODevice.h"
#include "TimerDevice.h"
#include "Logger.h"
#include "ElapsedTimer.h"
#include "Timestamp.h"

class DUCmdMgr : public EventProcessor
{
    public:
        /**
         * Constructor
         * @param name - Event processor name
         */
        DUCmdMgr();

        /**
         * Destructor
         */
        ~DUCmdMgr();

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
        Device* _udpIncoming;

        // Warm Restart Device
        UDPNetworkDevice* _udpWarmRestart;
        MsgHeaderType _msgHeaderBuf[4];
        int warmRestartBufCounter;
        void processWarmRestartMsg();

        int MODULE_TYPE;

        int TEST_STATUS;

        void processIncomingMsg();

        DUHWMgr &_duHWMgr;

        UIODevice* _uio1Dev;
        TimerDevice* _timerDev;
        void processInterrupt();
        void processTimer();
        ElapsedTimer eInterruptProcessing;
        Timestamp ts;

        // For status report
        UDPNetworkDevice* _statusRptToTWGS;

        int runSWScanLimitCheck(float alpha, float beta);

};


#endif      // DUCmdMgr_H
