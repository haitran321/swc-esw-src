#ifndef DUCmdMgrBase_H
#define DUCmdMgrBase_H

#include "CmdMgrBase.h"
#include "DUHWMgr.h"
#include "SWCProcessedSteeringWordRptMsg.h"
#include "UIODevice.h"

class DUCmdMgrBase : public CmdMgrBase
{
    public:
        virtual ~DUCmdMgrBase();

    protected:
        explicit DUCmdMgrBase(MODULE_TYPE moduleType);

        STATUS initializeDUCommonDevices();

        virtual void handleSteeringCommand(const SteeringCmdDataType& params);
        virtual void handleStatusRequest(const StatusRequestCmdDataType& params);

        DUHWMgr &_duHWMgr;
        UIODevice *_uioDevSL;
        UIODevice *_uioDevConfig;
        bool sendProcessedSW;
        int lastProcessedAlpha;
        int lastProcessedBeta;
        int REFRESH_DCU_STATUS_ON_GDS_INTERVAL;

        virtual void handleDUStatusRequest(const StatusRequestCmdDataType& params) = 0;
        virtual void handlePendingDcuStatusAfterScanLimit() = 0;
        virtual void handlePreDcuStatusTimer() = 0;
        virtual void handlePostDcuStatusTimer(int dequeSize);
        virtual void handleConfigInterruptRefresh();
        virtual int getProcessedKSineForReport() const = 0;
        virtual void processStatusTimer();

    private:
        void processSLInterrupt();
        void processConfigInterrupt();
        
        void sendProcessedSteeringWordReport(int processedKSine);

        int _steeringCmdCounter;
        int _statusRequestCmdCounter;
};

#endif
