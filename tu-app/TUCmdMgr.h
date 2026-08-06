/**
* $Id: TUCmdMgr.h 6399 2010-12-28 16:29:28Z tra18693 
*/
#ifndef TUCmdMgr_H
#define TUCmdMgr_H

#include "CmdMgrBase.h"
#include "UDPNetworkDevice.h"
#include "SWCMsgTypes.h"
#include "TUHWMgr.h"
#include "UIODevice.h"
#include "Logger.h"
#include "ElapsedTimer.h"
#include "Timestamp.h"

class TUCmdMgr : public CmdMgrBase
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
        virtual void processStatusTimer();

    private:

        UDPNetworkDevice* _localHWStatus;
        UDPNetworkDevice* _localHWCommand;

        void processIncomingMsg();

        TUHWMgr &_tuHWMgr;

        UIODevice* _uioDevSL;
        void processSLInterrupt();
        UIODevice* _uioDevWLSP;
        void processWLSPInterrupt();
        Timestamp ts;
        void sendTUStatusToDUA(DUTUStatusMsg status);
        void processLocalHWCommandMsg();

        virtual const char *getCommandMgrName() const;
        virtual void handleSteeringCommand(const SteeringCmdDataType& params);
        virtual void handleStatusRequest(const StatusRequestCmdDataType& params);
        virtual void handleStressTestCommand(const StressTestCmdDataType& params);
        virtual void handleRepollDCUCommand();
        virtual void handleStatusEmulatorMessage(int msgId, const int *status, int numData);
        virtual void setShutdownBit();

        ConfigModeCmdMsg _configModeCmdMsg[4];
        int configModeCmdCounter;
};


#endif      // TUCmdMgr_H
