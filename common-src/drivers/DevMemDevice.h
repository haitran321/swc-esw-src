#ifndef DevMemDevice_H
#define DevMemDevice_H

#include "Device.h"

class DevMemDevice : public Device
{
    public:
        DevMemDevice(unsigned long long offset);

        STATUS mmap();

        virtual int read(int offset);

        virtual STATUS write(int offset, int data);

    private:

        unsigned long long _offset;

        void * _baseAddr;

};


#endif // DevMemDevice_H
