#ifndef UIODevice_H
#define UIODevice_H

#include "Device.h"

class UIODevice : public Device
{
    public:
        UIODevice(unsigned int offset, unsigned int uioNum);

        STATUS mmap();

        STATUS clearInterrupt();

    private:

        unsigned int _offset;

        void * _baseAddr;

};


#endif // LEDDevice_H
