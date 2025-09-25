/**
* $Id: DUHWMgr.h 6399 2010-12-28 16:29:28Z tra18693 $
* This class is extended from the Event Processor class.
*/
#ifndef DUHWMgr_H
#define DUHWMgr_H

#include "Uncopyable.h"
#include "DUDevice.h"
#include "Logger.h"

class DUHWMgr : public Uncopyable
{
    public:

        /**
         * Destructor
         */
        virtual ~DUHWMgr();

        static DUHWMgr &getInstance();

        STATUS initialize();

        void close();

        void getRegs(int startReg, int endReg);
        void setReg(int offset, int data);

        int getArmKSine(RFCC_CH ch);
        void setArmKSine(RFCC_CH ch, int val);
        void toggleSWTrigger();
        int getFWScanLimitCheckStatus();

        int getAtbKSine(RFCC_CH ch);
        void setAtbKSine(RFCC_CH ch, int val);

        int sysConfig;
        int statusToTwgs;
        int getSysConfigStatus();
        int getSwcStatusToTwgs();
        void setOverallStatusBit(int val);
        void setConfigBit(int val);
        void setModeBit(int val);
        void setAlphaOverallStatusBit(int val);
        void setBetaOverallStatusBit(int val);
        void setTempStatusBit(int val);
        void setPwrSuppliesStatusBit(int val);
        void setDCUGroupStatusBit(int val);
        void setDCUNumberStatusBit(int val);
        void setDCUHealthStatusBit(int val);

    protected:
        /**
         * Sends pertinent state data for this event processor to standard output.
        */
        //virtual void printInfo();

    private:

        /* Private constructor */
        DUHWMgr();

        Logger &_logger;

        DUDevice* _duDev;

        int brdCtrVal;

        int diagRegVal;

};


#endif      // DUHWMgr_H
