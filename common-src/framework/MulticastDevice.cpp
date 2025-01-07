/**
 * $Id: MulticastDevice.cpp 5100 2009-12-04 20:54:54Z nei18232 $
 */
#include "MulticastDevice.h"

MulticastDevice::MulticastDevice(const string& name) :
Device(name)
{
}

STATUS MulticastDevice::open()
{
    STATUS status = OK;
    list<Device*>::iterator iter;

    // open each device
    for (iter = devices.begin(); iter != devices.end(); ++iter)
    {
        status |= (*iter)->open();
    }

    return status;
}

void MulticastDevice::add(Device* device)
{
    devices.push_back(device);
}

STATUS MulticastDevice::write(const void *buffer, size_t nbytes, size_t &bytesWritten)
{
    STATUS status = OK;
    list<Device*>::iterator iter;

    // write to each device
    for (iter = devices.begin(); iter != devices.end(); ++iter)
    {
        status |= (*iter)->write(buffer, nbytes, bytesWritten);
    }

    return status;
}

STATUS MulticastDevice::read(char *buffer, size_t maxbytes, size_t &bytesRead)
{
    // no concept of reading from each device
//    errnoSet(EINVAL);
    return ERROR;
}

STATUS MulticastDevice::close()
{
    STATUS status = OK;
    list<Device*>::iterator iter;

    // close each device
    for (iter = devices.begin(); iter != devices.end(); ++iter)
    {
        status |= (*iter)->close();
    }

    return status;
}

MulticastDevice::~MulticastDevice()
{
    try
    {
        close();
    }
    catch (...)
    {
    }

    list<Device*>::iterator iter;

    // delete each device
    for (iter = devices.begin(); iter != devices.end(); ++iter)
    {
        delete *iter;
    }
}
