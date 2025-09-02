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

        Logger &_logger;

        /**
         * UDP device for receiving Ent Network Msgs
         */
        Device* _udpFromRIMS;

        // Warm Restart Device
        UDPNetworkDevice* _udpWarmRestart;
        MsgHeaderType _msgHeaderBuf[4];
        int warmRestartBufCounter;
        void processWarmRestartMsg();

        int MODULE_TYPE;

        int TEST_STATUS;

        void processIncomingMsg();

        TUHWMgr &_tuHWMgr;

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


#endif      // TUCmdMgr_H
