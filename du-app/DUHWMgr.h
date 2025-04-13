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

        int getBoardControl();
        STATUS setBoardControl(int val);

        int getBoardStatus();

        int getDiagInfo();
        STATUS setDiagInfo(int val);

        STATUS setArmKSine(RFCC_CH ch, int val);
        int getArmKSine(RFCC_CH ch);
        void runFWScanLimitCheck();
        int getFWScanLimitCheckStatus();

        STATUS setAtbKSine(RFCC_CH ch, int val);
        int getAtbKSine(RFCC_CH ch);

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

        int boardControlValue;

        int diagRegValue;

        /* Common config parameters */
        int MODULE_TYPE;
};


#endif      // DUHWMgr_H
