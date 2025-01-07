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

        int getBoardControl();

        STATUS setBoardControl(int val);

        int getBoardStatus();

        void clearAllActions();

        STATUS addAction(TU_CHANNEL chId, TUActionType action, unsigned int ftw);

        void setCWRegs(TU_CHANNEL channel, TUCWSignalType signal);

        void programActions();

        int getNumActions(TU_CHANNEL ch);

        void toggleLoadCmdFlag();

        void toggleSWInternalTriggerFlag();

        void setTriggerMode(TU_TRIGGER_ENUM mode);

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

        int boardControlValue;

        int _numActions[NUM_TU_CHANNELS];

        TUFWActionType *_actions[NUM_TU_CHANNELS];

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


#endif      // TUHWMgr_H
