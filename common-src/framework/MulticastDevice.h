/**
 * $Id: MulticastDevice.h 5100 2009-12-04 20:54:54Z nei18232 $
 *
 * A <code>MulticastDevice</code> is a <code>Device</code>
 * that forwards write calls to zero or more <code>Device</code>s.
 */
#ifndef MulticastDevice_H
#define MulticastDevice_H

#include <list>
#include "Device.h"

using namespace std;

class MulticastDevice : public Device
{
public:

    /**
     * Constructor
     */
    explicit MulticastDevice(const string& name);

    /**
     * Destructor
     */
    virtual ~MulticastDevice();

    /**
     * Add the specified device to the group of devices
     * to be multicast to.<p/>
     * This class takes ownership of the device and is
     * responsible for deleting it.
     */
    void add(Device* device);

    /**
     * Opens all of the participant devices
     *
     * @return Status of operation
     */
    virtual STATUS open();

    /**
     * Writes data to all of the participant devices
     *
     * @param buffer Data buffer
     * @param nbytes Number of bytes to write
     * @param bytesWritten Number of bytes written
     * @return Status of operation
     */
    virtual STATUS write(const void *buffer, size_t nbytes, size_t &bytesWritten);

    /**
     * Read is not supported on MulticastDevices
     *
     * @return ERROR
     */
    virtual STATUS read(char *buffer, size_t maxbytes, size_t &bytesRead);

    /**
     * Closes a Multicast device and all of the participant devices
     *
     * @return Status of operation
     */
    virtual STATUS close();

protected:

    /** collection of devices to multicast to */
    list<Device*> devices;
};

#endif // MulticastDevice_H
