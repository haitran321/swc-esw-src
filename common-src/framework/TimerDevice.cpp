#include <stdio.h>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstdint>

#include "TimerDevice.h"

TimerDevice::TimerDevice(timespec initTimeoutVal, timespec timeoutVal) :
    Device(),
    _initTimeoutVal(initTimeoutVal),
    _timeoutVal(timeoutVal)
{
}

STATUS TimerDevice::open()
{
    // Create the timer file descriptor and arm it with the configured interval.

    if ((_fd = timerfd_create(CLOCK_REALTIME, 0)) == ERROR)
    {
        printf("Error creating timer");
        return ERROR;
    }

    struct itimerspec spec =
    {
        _initTimeoutVal,
        _timeoutVal
    };

    if (timerfd_settime(_fd, 0, &spec, NULL) == ERROR)
    {
        printf("Error setting timer");
        close();
        return ERROR;
    }

    return OK;
}

void TimerDevice::read()
{
    uint64_t timeout;
    ::read(_fd, &timeout, sizeof(uint64_t));
}

STATUS TimerDevice::setTime(timespec initTimeoutVal, timespec timeoutVal)
{
    _initTimeoutVal = initTimeoutVal;
    _timeoutVal = timeoutVal;

    struct itimerspec spec =
    {
        _initTimeoutVal,
        _timeoutVal
    };

    if (timerfd_settime(_fd, 0, &spec, NULL) == ERROR)
    {
        printf("Error setting timer");
        return ERROR;
    }

    return OK;
}


