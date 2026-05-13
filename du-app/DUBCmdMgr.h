/**
* $Id: DUBCmdMgr.h 6399 2010-12-28 16:29:28Z tra18693 
*/
#ifndef DUBCmdMgr_H
#define DUBCmdMgr_H

#include "DUCmdMgrBase.h"
#include "UDPNetworkDevice.h"

class DUBCmdMgr : public DUCmdMgrBase
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

        UDPNetworkDevice* _localHWStatus;
        void sendDCUStatusToDUA(BetaDCUStatusMsg status);
        void sendDUBStatusToDUA(DUTUStatusMsg status);

        virtual const char *getCommandMgrName() const;
        virtual void handleDUStatusRequest(const StatusRequestCmdDataType& params);
        virtual void handlePendingDcuStatusAfterScanLimit();
        virtual void handlePreDcuStatusTimer();
        virtual void handlePostDcuStatusTimer(int dequeSize);
        virtual int getProcessedKSineForReport() const;
        virtual void handleStatusEmulatorMessage(int msgId, const int *status, int numData);
};


#endif      // DUBCmdMgr_H
