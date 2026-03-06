#ifndef IOMHWMgr_H
#define IOMHWMgr_H

#include <queue>
#include "Uncopyable.h"
#include "DUDevice.h"
#include "Logger.h"
#include "SWCMsgTypes.h"
#include <modbus.h>

typedef struct
{
    int ch1; // bad bit
    int ch2; // bad bit
    int atbIOMStatus;  //ch 3
    int ch4;
    int tempIOMStatus;  // ch 5
    int ch6;
    int ps12IOMStatus;  // ch 7
    int ch8;
    int ps24IOMStatus;  //  ch 9
    int ch10;
    int ch11;
    int ch12;
    int ch13;
    int ch14;
    int ch15;
    int ch16;
} IOMStatusDataType;

class IOMHWMgr : public Uncopyable
{
    public:

        /**
         * Destructor
         */
        virtual ~IOMHWMgr();

        static IOMHWMgr &getInstance();

        STATUS initialize();

        IOMStatusDataType readStatus();

        void close();

    protected:
        /**
         * Sends pertinent state data for this event processor to standard output.
        */
        //virtual void printInfo();

    private:

        /* Private constructor */
        IOMHWMgr();

        Logger &_logger;

        modbus_t *_modbusCtx;
};


#endif      // IOMHWMgr_H
