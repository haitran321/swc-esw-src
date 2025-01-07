#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "DevMemDevice.h"

typedef struct 
{
    int reg1;   
    int reg2;  
    int reg3;   
    int reg4;   
    int reg5;  
    int reg6;
}MemRegType;

static MemRegType *regs = NULL;

DevMemDevice::DevMemDevice(unsigned long long offset) :
Device("/dev/mem"),
_offset(offset)
{
}

STATUS DevMemDevice::mmap()
{
    _baseAddr = ::mmap(NULL, DR_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, _offset);

//  printf("Successfully mmap _baseAddr = %p\n", _baseAddr);

    regs = (MemRegType *)_baseAddr;

    return OK;
}

int DevMemDevice::read(int offset)
{
//  printf("Data at offset 0x%x = 0x%x\n", offset, *((unsigned *)(_baseAddr + offset)));
//  printf("%d\n", *((unsigned *)(_baseAddr + offset)));
//  printf("Data at offset 0x%x = 0x%x\n", offset, *((short *)(_baseAddr + offset)));
    printf("%d\n", *((short *)(_baseAddr + offset)));
    return OK;
}

STATUS DevMemDevice::write(int offset, int data)
{
    *((unsigned *)(_baseAddr + offset)) = data;
    return OK;
}


