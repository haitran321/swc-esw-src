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

        void clearAllActions();

        STATUS addAction(DU_CHANNEL chId, DUActionType action, unsigned int ftw);

        void programActions();

        int getNumActions(DU_CHANNEL ch);

        void toggleLoadCmdFlag();

        void toggleSWInternalTriggerFlag();

        void setTriggerMode(DU_TRIGGER_ENUM mode);

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

        int _numActions[NUM_DU_CHANNELS];

        DUFWActionType *_actions[NUM_DU_CHANNELS];

        /* Common config parameters */
        int MODULE_TYPE;
        int SW_TRIGGER;
        int INIT_TESTING;
        int PBP_MODE_USING_INIT_REGS;
        int INTERNAL_TRIGGER;
        int CHIRP_DIR;

        // WG specific config parameters
        int WG1_COMBINER_ENABLE;
        int WG1_60MHZ_INPUT;

};


#endif      // DUHWMgr_H
