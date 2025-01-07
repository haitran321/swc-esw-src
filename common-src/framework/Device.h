/**
* $Id: Device.h 5100 2009-12-04 20:54:54Z nei18232 $
*
* Class Device is the base class providing common functionality 
* to all classes accessing VxWorks I/O devices. It provides an 
* interface to the VxWorks I/O system. 
*
*/
#ifndef Device_H
#define Device_H

#include <errno.h>
#include <string>

#include "DefineUtils.h"

using namespace std;

const int NO_FD = -1;

class Device
{
public:

   /**
   * Constructor
   *
   */
   Device();

   /**
   * Constructor
   *
   * @param name Device name
   */
   explicit Device(const string &name);

   /**
   * Constructor
   *
   * @param name Device name
   */
   explicit Device(const char *name);

   /**
   * Destructor
   *
   */
   virtual ~Device();

   /**
   * Method open opens a VxWorks I/O device.
   *
   * @param name Device name
   * @param flags Open flags
   * @param mode Open mode
   * @return Status of operation
   */
   int open(const char *name, int flags, int mode);
   /**
   * Method open opens a VxWorks I/O device.
   *
   * @param name Device name
   * @return Status of operation
   */
   int open(const char *name);

   /**
   * Method open opens a VxWorks I/O device.
   *
   * @return Status of operation
   */
   virtual int open();

   /**
   * Method write writes data to a VxWorks I/O device.
   *
   * @param buffer Data buffer
   * @param nbytes Number of bytes to write
   * @return Status of operation
   */
   virtual int write(const void *buffer, size_t nbytes);

   /**
   * Method write writes data to a VxWorks I/O device.
   *
   * @param buffer Data buffer
   * @param nbytes Number of bytes to write
   * @param bytesWritten Number of bytes written
   * @return Status of operation
   */
   virtual int write(const void *buffer, size_t nbytes, size_t &bytesWritten);

   /**
   * Method read reads data from a VxWorks I/O device.
   *
   * @param buffer Data buffer
   * @param maxbytes Maximum bytes to read
   * @param bytesRead Number of bytes read
   * @return Status of operation
   */
   virtual int read(char *buffer, size_t maxbytes, size_t &bytesRead);

   /**
   * Method control controls a VxWorks I/O device.
   *
   * @param function Control function code
   * @param params Control parameters
   * @return Status of operation
   */
   int control(int function, int params);

   /**
   * Method control controls a VxWorks I/O device.
   *
   * @param function Control function code
   * @return Status of operation
   */
   int control(int function);

   /**
   * Method getStatus obtains device status from a VxWorks I/O 
   * device. 
   *
   * @param function Status function
   * @param results Status results
   * @return Status of operation
   */
   int getStatus(int function, void *results);

   /**
   * Method getStatus obtains device status. It is intended to be 
   * overriden by a derived Device class. 
   *
   * @param deviceStatus Device status
   * @return Status of operation
   */
   virtual int getStatus(int &deviceStatus);

   /**
   * Method close closes a VxWorks I/O device.
   *
   * @return Status of operation
   */
   virtual int close();

   /**
   * Method getFd returns the descriptor for the VxWorks I/O 
   * device. 
   *
   * @return Device descriptor
   */
   inline int getFd() const;

   /**
   * Method isStatusUpdate returns flag indicating if device has 
   * device status available. 
   *
   * @return Flag indicating if device status is available
   */
   inline bool isStatusUpdate() const;

   /**
   * Method getName returns device name.
   *
   * @return Device name
   */
   inline const string &getName();

   /**
   * Method setName sets the device name.
   *
   * @param name Device name
   */
   inline void setName(string name);

   /**
   * Method isOpen returns flag indicating if device is currently 
   * open. 
   *
   * @return Flag indicating if device is currently open
   */
   inline bool isOpen() const;//#incl_udpOutude <usrLib.h>


protected:

   /** Device descriptor */

   int _fd;

   /** Device name */

   string _name;

};

int Device::getFd() const
{
   return(_fd);
}

bool Device::isOpen() const
{
   return(_fd != NO_FD);
}

const string &Device::getName()
{
   return(_name);
}

void Device::setName(string name)
{
   _name = name;
}

bool Device::isStatusUpdate() const
{
   //return(errnoGet() == EIO);
   return OK;
}

#endif

