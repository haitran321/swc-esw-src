#ifndef IOMHWMgr_H
#define IOMHWMgr_H

#include <queue>
#include "Uncopyable.h"
#include "DUDevice.h"
#include "Logger.h"
#include "SWCMsgTypes.h"

class IOMHWMgr : public Uncopyable
{
    public:

        /**
         * Destructor
         */
        virtual ~IOMHWMgr();

        static IOMHWMgr &getInstance();

        STATUS initialize();

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

};


#endif      // IOMHWMgr_H
