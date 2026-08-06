#ifndef TimerDevice_H
#define TimerDevice_H

#include "Device.h"
#include <sys/timerfd.h>

class TimerDevice : public Device
{
    public:
        TimerDevice(timespec initTimeoutVal, timespec timeoutVal);

        virtual STATUS open();

        virtual void read();

        STATUS setTime(timespec initTimeoutVal, timespec timeoutVal);

    private:

        timespec _initTimeoutVal;
        timespec _timeoutVal;

};


#endif // TimerDevice_H
