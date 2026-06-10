/**
* $Id: TUHWMgr.h 6399 2010-12-28 16:29:28Z tra18693 $
* This class is extended from the Event Processor class.
*/
#ifndef TUHWMgr_H
#define TUHWMgr_H

#include "Uncopyable.h"
#include "TUDevice.h"
#include "Logger.h"
#include "SWCMsgTypes.h"

class TUHWMgr : public Uncopyable
{
    public:

        /**
         * Destructor
         */
        virtual ~TUHWMgr();

        static TUHWMgr &getInstance();

        STATUS initialize();

        void close();

        void getRegs(int startReg, int endReg);
        void setReg(int offset, int data);

        int getBrdStatus();
        int getBrdCtrl();

        int getArmKSine(RFCC_CH ch);
        void setArmKSine(RFCC_CH ch, int val);

        int getFWScanLimitCheckStatus();

        void toggleRLTDSignal();
        void setRLCPSignal(CmdOnOff flag);
        void setRLSCSignal(CmdOnOff flag);

        void setConfig(SWC_CONFIG config);
        void setMode(SWC_MODE mode);
        void setOLTE(SWC_MODE olte);

        int getRLTDPeriod();
        void setRLTDPeriod(int val);

        int getNumTest();
        void setNumTest(int val);

        int getNumInc();
        void setNumInc(int val);

        int getAlphaInc();
        void setAlphaInc(int val);

        int getBetaInc();
        void setBetaInc(int val);

        DUTUStatusType readTUStatus();

        int getFPGADieTemp();

        void processTUEmulatorStatus(int statusReg);

    protected:
        /**
         * Sends pertinent state data for this event processor to standard output.
        */
        //virtual void printInfo();

    private:

        /* Private constructor */
        TUHWMgr();

        int _verbose;

        Logger &_logger;

        TUDevice* _tuDev;

        int MODULE_TYPE;

        int _brdCtrVal;
        int _diagRegVal;
        int _armInitReady;

        int USE_STATUS_EMULATOR;
        int emTUStatusReg;
};


#endif      // TUHWMgr_H
