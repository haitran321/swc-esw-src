#include <stdio.h>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "TimerDevice.h"

TimerDevice::TimerDevice(timespec initTimeoutVal, timespec timeoutVal) :
    Device(),
    _initTimeoutVal(initTimeoutVal),
    _timeoutVal(timeoutVal)
{
}

STATUS TimerDevice::open()
{
    // Create a socket.

    if ((_fd = timerfd_create(CLOCK_REALTIME, 0)) == ERROR)
    {
        printf("Error creating timer");
        return ERROR;
    }

    char dummyBuf[8];
    struct itimerspec spec =
    {
        _initTimeoutVal,
        _timeoutVal
    };
    timerfd_settime(_fd, 0, &spec, NULL);
    return OK;
}

void TimerDevice::read()
{
    uint64_t timeout;
    ::read(_fd, &timeout, sizeof(uint64_t));
}



