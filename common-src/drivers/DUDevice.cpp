#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#include "DUDevice.h"

static DURegType *regs = NULL;

DUDevice::DUDevice(unsigned int offset) :
Device("/dev/mem"),
_offset(offset)
{
}

STATUS DUDevice::mmap()
{
    _apbBusAddr = ::mmap(NULL, DU_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, _offset);

    printf("Successfully mmap _apbBusAddr = %p\n", _apbBusAddr);

    regs = (DURegType *)_apbBusAddr;

    return OK;
}

//DUFWActionType * DUDevice::getCmdStartingAddress()
//{
//    return (DUFWActionType *)(_apbBusAddr + DU_CMD_STARTING_ADDR_OFFSET);
//}

int DUDevice::readReg(int offset)
{
   return (*((unsigned *)(_apbBusAddr + offset)));
}

void DUDevice::writeReg(int offset, int data)
{
    *((unsigned *)(_apbBusAddr + offset)) = data;
}

int DUDevice::getFirmwareVersionReg()
{
    return (regs->firmwareVersion);
}

int DUDevice::getBoardStatusReg()
{
    return (regs->boardStatus);
}

STATUS DUDevice::setBoardControlReg(int val)
{
    STATUS rc = OK;

    regs->boardControl = val;

    return rc;
}

int DUDevice::getBoardControlReg()
{
    return (regs->boardControl);
}

STATUS DUDevice::setDiagInfoReg(int val)
{
    STATUS rc = OK;

    regs->diagInfo = val;

    return rc;
}

int DUDevice::getDiagInfoReg()
{
    return (regs->diagInfo);
}

// STATUS DUDevice::setDMAControllerReg(int val)
// {
//     STATUS rc = OK;

//     regs->dmaControlleReg = val;

//     return rc;
// }

// void DUDevice::readFeedbackRegs(int numRegs)
// {
//     for (int i = 0; i < numRegs; i++)
//     {
//         printf("spare[%d] = 0x%x\n", i, regs->spare3[i]);
//     }
// }

// void DUDevice::readDMAReg()
// {
//     writeReg(0x9C, 0x8);
//     sleep(0.1);
//     writeReg(0x9C, 0x0);
//     printf("Write control reg = 0x%x\n", readReg(0x90));
//     printf("Write status reg = 0x%x\n", readReg(0x24));
//     printf("Read control reg = 0x%x\n", readReg(0x4C));
//     printf("Read status reg = 0x%x\n", readReg(0x28));
// }

STATUS DUDevice::setNumActionsReg(DU_CHANNEL ch, int val)
{
    STATUS rc = OK;

//  if (ch == DU_CHANNEL_0)
//  {
//      regs->ch0NumActions = val;
//  }
//  else if (ch == DU_CHANNEL_1)
//  {
//      regs->ch1NumActions = val;
//  }
//  else if (ch == DU_CHANNEL_2)
//  {
//      regs->ch2NumActions = val;
//  }
//  else if (ch == DU_CHANNEL_3)
//  {
//      regs->ch3NumActions = val;
//  }
//  else
//  {
//      printf("ERROR:  invalid DU channel number = %d\n", ch);
//      rc = ERROR;
//  }
    
    return rc;
}    

void DUDevice::getRegs(int startReg, int endReg)
{
    int numRegs = (endReg - startReg)/4 + 1;
    printf("Display Regs: start = 0x%x, end = 0x%x, num = %d\n", startReg, endReg, numRegs);
    for (int i = 0; i < numRegs; i++)
    {
        printf("0x%x: 0x%x\n", startReg + (i*4), readReg(startReg + (i*4)));
    }
}
