#ifndef IOMHWMgr_H
#define IOMHWMgr_H

#include <queue>
#include <string>
#include "Uncopyable.h"
#include "DUDevice.h"
#include "Logger.h"
#include "SWCMsgTypes.h"
#include <modbus.h>

typedef struct
{
    int configBit0; 
    int configBit1; 
    int mode; 
    int ps12;
    int ps24;  
    int temp;
    int ch7;  
    int ch8;
    int ch9;  
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

        STATUS createContext();

        STATUS ensureConnected();

        int _verbose;

        Logger &_logger;

        std::string _ioModuleIpAddress;

        int _ioModulePort;

        modbus_t *_modbusCtx;
};


#endif      // IOMHWMgr_H
