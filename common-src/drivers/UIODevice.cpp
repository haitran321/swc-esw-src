#include <stdio.h>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "UIODevice.h"

// TO DO:  set these to correct value
typedef struct 
{
    int reg1;   
    int reg2;  
    int reg3;   
    int reg4;   
    int reg5;  
    int reg6;
}UIORegType;

static UIORegType *regs = NULL;

UIODevice::UIODevice(unsigned int offset, unsigned int uioNum) :
Device(),
_offset(offset)
{
    stringstream ss("");
    ss << "/dev/uio";
    ss << uioNum;
    setName(ss.str());
}

STATUS UIODevice::mmap()
{
    _baseAddr = ::mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);

    regs = (UIORegType *)_baseAddr;

    return OK;
}

STATUS UIODevice::clearInterrupt()
{
    int reenable = 1;
    write((void *)&reenable, sizeof(int));
    return OK;
}

