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

        int getArmKSine(RFCC_CH ch);
        void setArmKSine(RFCC_CH ch, int val);

        int getFWScanLimitCheckStatus();

        void toggleRLTDSignal();
        void setRLCPSignal(CmdOnOff flag);
        void setRLSCSignal(CmdOnOff flag);

        DUTUStatusType readTUStatus();

    protected:
        /**
         * Sends pertinent state data for this event processor to standard output.
        */
        //virtual void printInfo();

    private:

        /* Private constructor */
        TUHWMgr();

        Logger &_logger;

        TUDevice* _tuDev;

        int _brdCtrVal;

        int _diagRegVal;

        /* Common config parameters */
        int MODULE_TYPE;
};


#endif      // TUHWMgr_H
