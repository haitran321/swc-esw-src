#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "LEDDevice.h"

typedef struct 
{
    int reg1;   
    int reg2;  
    int reg3;   
    int reg4;   
    int reg5;  
    int spare[4];
    int reg6;
}LEDRegType;

static LEDRegType *regs = NULL;

LEDDevice::LEDDevice(unsigned int offset) :
Device("/dev/mem"),
_offset(offset)
{
}

STATUS LEDDevice::mmap()
{
	_apbBusAddr = ::mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, _offset);

    printf("Successfully mmap _apbBusAddr = %p\n", _apbBusAddr);

    regs = (LEDRegType *)_apbBusAddr;

    return OK;
}

int LEDDevice::read(int reg)
{
    printf("In ChannelSimDevice::read()\n");

    if (reg == 1)
    {
        #ifdef ZCU104
        printf("Data at reg 1 = 0x%x\n", regs->reg1);
        #else
        printf("Data at reg 6 = 0x%x\n", regs->reg6);
        #endif
        // printf("Data at reg 1 = 0x%x\n", regs->reg1);
    }

    if (reg == 5)
    {
        printf("Data at reg 5 = 0x%x\n", regs->reg5);
    }
    return OK;
}

STATUS LEDDevice::write(int reg, int data)
{
    #ifdef ZCU104
    regs->reg1 = data;
    #else
    regs->reg6 = data;
    #endif
    // regs->reg1 = data;
    return OK;
}
