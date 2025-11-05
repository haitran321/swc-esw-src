/**
* $Id: DUHWMgr.h 6399 2010-12-28 16:29:28Z tra18693 $
* This class is extended from the Event Processor class.
*/
#ifndef DUHWMgr_H
#define DUHWMgr_H

#include <queue>
#include "Uncopyable.h"
#include "DUDevice.h"
#include "Logger.h"
#include "SWCMsgTypes.h"

class DUHWMgr : public Uncopyable
{
    public:

        /**
         * Destructor
         */
        virtual ~DUHWMgr();

        static DUHWMgr &getInstance();

        STATUS initialize(int MODULE_TYPE_);

        void close();

        void getRegs(int startReg, int endReg);
        void setReg(int offset, int data);

        int getArmKSine(RFCC_CH ch);
        void setArmKSine(RFCC_CH ch, int val);
        void toggleSWTrigger();
        int getFWScanLimitCheckStatus();

        int getAtbKSine(RFCC_CH ch);
        void setAtbKSine(RFCC_CH ch, int val);

        int getSwcStatusToTwgs();
        void setSwcStatusToTwgs();

        int getSysConfigStatus();

        void readSWCStatus(SWC_STATUS_DATA_TYPE dataType);
        void readDCUStatus();
        DCUStatusParamsType readDCUFWStatus(int reg);

        void setDCUStatusToTwgs(RFCC_CH group, HealthState health, int dcuNum);

        // To be removed.  This is to allow the emulator to set the status
        void processEmulatorStatus(HealthState swcrOverall_,
                                   SWC_CONFIG sysConfig_,
                                   SWC_MODE mode_,
                                   HealthState alphaDUStatus_,
                                   HealthState betaDUStatus_,
                                   DCURolledUpStatus alphaDCURolledUpStatus_,
                                   DCURolledUpStatus betaDCURolledUpStatus_,
                                   HealthState tempStatus_,
                                   HealthState pwrStatus_,
                                   HealthState atbStatus_,
                                   RFCC_CH dcuGroup_,
                                   HealthState dcuStatus_,
                                   int dcuNum_);

        SWCStatusDataType getSWCStatus();

        void processDCUStatus(DCUStatusParamsType status);

        DCUStatusParamsType getDCUStatusFromSW(RFCC_CH type, int dcuNum);

        DCUStatusParamsType getDCUStatusFromQueue();

        int getDCUStatusQueueSize();

        void addDCUStatusToQueue(DCUStatusParamsType status);

        int runSWScanLimitCheck(float alpha, float beta);

    protected:
        /**
         * Sends pertinent state data for this event processor to standard output.
        */
        //virtual void printInfo();

    private:

        /* Private constructor */
        DUHWMgr();

        int MODULE_TYPE;

        Logger &_logger;

        DUDevice* _duDev;

        RFCC_CH _rfccType;
        int _brdCtrVal;
        int _diagRegVal;
        int _sysConfigReg;
        int _statusToTwgs;
        HealthState _swcrOverall;
        SWC_CONFIG _sysConfig;
        SWC_MODE _mode;
        int _testEnabled;
        HealthState _alphaDUStatus;
        HealthState _betaDUStatus;
        DCURolledUpStatus _alphaDCURolledUpStatus;
        DCURolledUpStatus _betaDCURolledUpStatus;
        HealthState _tempStatus;
        HealthState _pwrStatus;
        HealthState _atbStatus;

        std::queue<DCUStatusParamsType> _dcuSendQueue;
        // This store the dcu data at index based on the dcu number from the FW
        DCUStatusParamsType _dcuStatus[NUM_RFCC_CH][NUM_DCU];

        void setOverallStatusBit(int val);
        void setDataTypeBit(SWC_STATUS_DATA_TYPE val);
        void setConfigBit(int val);
        void setModeBit(int val);
        void setAlphaOverallStatusBit(int val);
        void setBetaOverallStatusBit(int val);
        void setAlphaDCURolledUpStatusBit(int val);
        void setBetaDCURolledUpStatusBit(int val);
        void setTempStatusBit(int val);
        void setPwrSuppliesStatusBit(int val);
        void setATBStatusBit(int val);
        void setDCUGroupStatusBit(RFCC_CH val);
        void setDCUHealthStatusBit(HealthState val);
        void setDCUNumberStatusBit(int val);

        int USE_STATUS_EMULATOR;

};


#endif      // DUHWMgr_H
