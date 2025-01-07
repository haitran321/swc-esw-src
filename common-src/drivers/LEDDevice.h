#ifndef LEDDevice_H
#define LEDDevice_H

#include "Device.h"

class LEDDevice : public Device
{
    public:
        LEDDevice(unsigned int offset);

        STATUS mmap();

        virtual int read(int reg);

        virtual STATUS write(int reg, int data);

    private:

        unsigned int _offset;

        void * _apbBusAddr;

};


#endif // LEDDevice_H
