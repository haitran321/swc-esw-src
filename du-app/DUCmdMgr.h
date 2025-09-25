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

        /* Common config parameters */
        int MODULE_TYPE;
        int FORCE_TEST_MODE;
        int STEERING_WORD_SRC;

        void processIncomingMsg();

        DUHWMgr &_duHWMgr;

        UIODevice* _uio1Dev;
        TimerDevice* _timerDev;
        void processInterrupt();
        void processTimer();
        ElapsedTimer eInterruptProcessing;
        Timestamp ts;

        // For status report
        UDPNetworkDevice* _udpOutToTestServer;

        int runSWScanLimitCheck(float alpha, float beta);

        DCUStatusParamsType alphaDCU[NUM_DCU];
        DCUStatusParamsType betaDCU[NUM_DCU];
        void processDCUStatus();

        UDPNetworkDevice* _udpFromDevPC;
        UDPNetworkDevice* _udpToDevPC;
        void processDEVPCMsg();

        int lastAlpha;
        int lastBeta;


};


#endif      // DUCmdMgr_H
