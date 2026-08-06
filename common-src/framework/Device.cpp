#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "Device.h"

Device::Device() :
    _fd(NO_FD),
    _name("")
{
}

Device::Device(const char *name) :
    _fd(NO_FD),
    _name(name)
{
}

Device::Device(const string &name) :
    _fd(NO_FD),
    _name(name)
{
}

int Device::open()
{
   // Open device.

   if ((_fd = ::open(_name.c_str(), O_RDWR, 0)) == ERROR)
   {
      std::cout << "Device::open: open failed for " << _name.c_str() << std::endl;
      return(ERROR);
   }

   // Device successfully opened.

   return(OK);
}

int Device::open(const char *name)
{
   return(open(name, O_RDWR, 0));
}

int Device::open(const char *name, int flags, int mode)
{
   // Open device.

   if ((_fd = ::open(name, flags, mode)) == ERROR)
   {
      _name = "";
      return(ERROR);
   }

   // Device successfully opened.

   _name = name;
   return(OK);
}

int Device::write(const void *buffer, size_t nbytes, size_t &bytesWritten)
{
   // Data buffer pointer in write system call is "char *", need to
   // convert const void * to char *. This hides the non-constantness
   // from the users of this class.

   char *charBufPtr = static_cast<char *>(const_cast<void *>(buffer));

   // Write data to device.

   if ((bytesWritten = ::write(_fd, charBufPtr, nbytes)) != nbytes)
   {
      return(ERROR);
   }

   // Data was successfully written to device.

   return(OK);
}

int Device::write(const void *buffer, size_t nbytes)
{
   size_t bytesWritten;
   return(write(buffer, nbytes, bytesWritten));
}

int Device::read(char *buffer, size_t maxbytes, size_t &bytesRead)
{
   // Read data from device.
   int retStatus = ERROR;

   retStatus = ::read(_fd, buffer, maxbytes);
   if (retStatus == ERROR)
   {
      bytesRead = 0;
      return(ERROR);
   }
   bytesRead = static_cast<size_t>(retStatus);
   
   // Data was successfully read from device.

   return(OK);
}

int Device::control(int function, int params)
{
   return(::ioctl(_fd, function, params));
}

int Device::getStatus(int function, void *results)
{
   return(::ioctl(_fd, function, reinterpret_cast<int *>(results)));
}

int Device::control(int function)
{
   return(::ioctl(_fd, function, 0));
}

int Device::close()
{
   int status = OK;

   // Close device.

   if (isOpen())
   {
      status = ::close(_fd);
      _fd = NO_FD;
   }
   return(status);
}

Device::~Device()
{
   try
   {
      close();
   }
   catch(...)
   {
   }
}

int Device::getStatus(int &deviceStatus)
{
   // Default implemenation.

   if (_fd != NO_FD)
   {
      deviceStatus = 0;
      return(OK);
   }
   else
   {
      return(ERROR);
   }
}

