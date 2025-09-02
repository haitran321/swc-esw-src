/**
* $Id: TUHWMgr.h 6399 2010-12-28 16:29:28Z tra18693 $
* This class is extended from the Event Processor class.
*/
#ifndef TUHWMgr_H
#define TUHWMgr_H

#include "Uncopyable.h"
#include "TUDevice.h"
#include "Logger.h"

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

        int getArmKSine(RFCC_CH ch);
        void setArmKSine(RFCC_CH ch, int val);
        void runFWScanLimitCheck();
        int getFWScanLimitCheckStatus();

        int getAtbKSine(RFCC_CH ch);
        void setAtbKSine(RFCC_CH ch, int val);

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

        int brdCtrVal;

        int diagRegVal;

        /* Common config parameters */
        int MODULE_TYPE;
};


#endif      // TUHWMgr_H
