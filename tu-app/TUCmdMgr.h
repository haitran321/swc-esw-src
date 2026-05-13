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
#include "TimerDevice.h"
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

    private:

        UDPNetworkDevice* _localHWStatus;

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

        virtual const char *getCommandMgrName() const;
        virtual void handleSteeringCommand(const SteeringCmdDataType& params);
        virtual void handleStatusRequest(const StatusRequestCmdDataType& params);
        virtual void handleStatusEmulatorMessage(int msgId, const int *status, int numData);
};


#endif      // TUCmdMgr_H
