/**
* $Id: DUACmdMgr.h 6399 2010-12-28 16:29:28Z tra18693 
*/
#ifndef DUACmdMgr_H
#define DUACmdMgr_H

#include "DUCmdMgrBase.h"
#include "UDPNetworkDevice.h"

class DUACmdMgr : public DUCmdMgrBase
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

        UDPNetworkDevice* _localHWStatus;
        void processLocalHWStatusMsg();

        virtual const char *getCommandMgrName() const;
        virtual void handleDUStatusRequest(const StatusRequestCmdDataType& params);
        virtual void handlePendingDcuStatusAfterScanLimit();
        virtual void handlePreDcuStatusTimer();
        virtual void handleConfigInterruptRefresh();
        virtual int getProcessedKSineForReport() const;
        virtual void handleStatusEmulatorMessage(int msgId, const int *status, int numData);

        BetaDCUStatusMsg _localStatus[4];
        int localStatusCounter;
};


#endif      // DUACmdMgr_H
