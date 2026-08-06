/**
* $Id: DUHWMgr.h 6399 2010-12-28 16:29:28Z tra18693 $
* This class is extended from the Event Processor class.
*/
#ifndef DUHWMgr_H
#define DUHWMgr_H

#include <deque>
#include <vector>
#include <algorithm>
#include <iostream>
#include "Uncopyable.h"
#include "DUDevice.h"
#include "IOMHWMgr.h"
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

        int getBrdStatus();

        int getArmKSine(RFCC_CH ch);
        void setArmKSine(RFCC_CH ch, int val);
        int getAtbKSine(RFCC_CH ch);
        void setAtbKSine(RFCC_CH ch, int val);
        int getFWScanLimitCheckStatus();

        // Board Control Reg
        void setShutdownBit();
        void toggleSWTrigger();
        void setSysConfig(SWC_CONFIG config);
        void setTestSrcInTestMode(TestSource testSrc);
        void sendDCUCmdInTestMode(DCU_CMD_ENUM cmd);
        void sendSteeringWordValidFlagInTestMode(DU_STEERING_WORD_VALID_FLAG_ENUM flag);

        // SWCR Status to TWGS Reg
        int getSwcStatusToTwgs();
        void setSwcStatusToTwgs();

        int getSysConfigStatus();

        void readSWCStatus(SWC_STATUS_DATA_TYPE dataType);
        void readDCUStatus(bool sendCurrentDCUList = false);
        DCUStatus readDCUFWStatus(int reg);
        STATUS validateDCUFWStatus(DCUStatus status);

        void setDCUStatusToTwgs(RFCC_CH group, DCUHealthState health, int dcuNum);

        SWCOverallStatusDataType getSWCStatus();

        void processDCUStatus(DCUStatus status);

        void processBetaDCUStatus(DCUStatus status);

        DCUStatus getDCUStatusFromSW(RFCC_CH type, int dcuNum);

        DCUStatus getDCUStatusFromDeque();

        int getDCUStatusDequeSize();

        void addDCUStatusToDeque(DCUStatus status);
        void checkForExistingDCUStatusInDeque(std::deque<DCUStatusWithID>& dq_, const int& dcuID_); 

        void lookForMissingDCUAfterInit();

        DUTUStatusType readDUStatus();
        void processDUAStatus(DUTUStatusType status);
        void processDUBStatus(DUTUStatusType status);
        void processTUStatus(DUTUStatusType status);
        void getIOModuleStatus();
        void computeDCURolledUpStatus();

        int getOverallSPIStatus();

        DUTUStatusType getDUAStatus();
        DUTUStatusType getDUBStatus();
        DUTUStatusType getTUStatus();
        HealthState getTempStatus();
        HealthState get12VPSStatus();
        HealthState get24VPSStatus();
        HealthState getSWCOverallStatus();
        SWC_CONFIG getSWCConfigStatus();
        SWC_MODE getSWCModeStatus();
        SWC_MODE getOLTEModeStatus();
        int getFPGADieTemp();
        void clearDCUData();


        void processSWCREmulatorStatus(SWC_CONFIG sysConfig_,
                           SWC_MODE mode_,
                           SWC_MODE olte_,
                           HealthState tempStatus_,
                           HealthState pwr12VStatus_,
                           HealthState pwr24VStatus_,
                           HealthState atbStatus_,
                           RolledUpStatus alphaDCURolledUpStatus_,
                           RolledUpStatus betaDCURolledUpStatus_);

        void processDUEmulatorStatus(int statusReg);

        void processDCUEmulatorStatus(int dcuNum, int dcuFWStatus);

    protected:
        /**
         * Sends pertinent state data for this event processor to standard output.
        */
        //virtual void printInfo();

    private:

        /* Private constructor */
        DUHWMgr();

        int MODULE_TYPE;
        int _verbose;

        Logger &_logger;

        DUDevice* _duDev;

        IOMHWMgr &_iomHWMgr;

        RFCC_CH _rfccType;
        string cmdMgrName;
        int _brdCtrVal;
        int _diagRegVal;
        int _sysConfigReg;
        int _statusToTwgs;
        int _armInitReady;
        HealthState _swcrOverall;
        SWC_CONFIG _sysConfig;
        SWC_MODE _swcMode;
        SWC_MODE _olteMode;
        DUTUStatusType _alphaDUStatus;
        DUTUStatusType _betaDUStatus;
        DUTUStatusType _tuStatus;
        RolledUpStatus _alphaDCURolledUpStatus;
        RolledUpStatus _betaDCURolledUpStatus;
        HealthState _tempStatus;
        HealthState _pwr12VStatus;
        HealthState _pwr24VStatus;
        HealthState _atbStatus;
        RFCC_CH _dcuGroup;
        int _dcuNum;
        int DCU_CHECK_VERSION_FLAG;
        int DCU_MAJOR_VERSION;
        int DCU_MINOR_VERSION;
        int SAP_RED_OP_THRESHOLD;
        int SAP_YELLOW_OP_THRESHOLD;
        DCUStatus resetDCU;

        std::deque<DCUStatusWithID> _dcuSendDeque;
        // This store the dcu data at index based on the dcu number from the FW
        DCUStatus _dcuStatus[NUM_RFCC_CH][NUM_DCU];
        bool _dcuLocOccupied[NUM_DCU];

        std::vector<int> _initialDCUList;
        std::vector<int> _currentDCUList;

        void setOverallStatusBit(int val);
        void setDataTypeBit(SWC_STATUS_DATA_TYPE val);
        void setConfigBit(int val);
        void setAlphaOverallStatusBit(int val);
        void setBetaOverallStatusBit(int val);
        void setAlphaDCURolledUpStatusBit(int val);
        void setBetaDCURolledUpStatusBit(int val);
        void setTempStatusBit(int val);
        void set12VPwrStatusBit(int val);
        void set24VPwrStatusBit(int val);
        void setATBStatusBit(int val);
        void setDCUGroupStatusBit(RFCC_CH val);
        void setDCUHealthStatusBit(HealthState val);
        void setDCUNumberStatusBit(int val);
        int getMajorityFWStatusFromHistory(int reg, int currentFWStatus);

        int USE_STATUS_EMULATOR;
        int emDCUFWStatus[NUM_RFCC_CH][NUM_DCU];
        int _dcuFWStatusHistory[NUM_RFCC_CH][NUM_DCU][4];
        int _dcuFWStatusHistoryCount[NUM_RFCC_CH][NUM_DCU];
        int emDUStatusReg;
        HealthState emTempStatus;
        HealthState emPwr12VStatus;
        HealthState emPwr24VStatus;
        HealthState emAtbStatus;
        RolledUpStatus emAlphaDCURolledUpStatus;
        RolledUpStatus emBetaDCURolledUpStatus;
};


#endif      // DUHWMgr_H
