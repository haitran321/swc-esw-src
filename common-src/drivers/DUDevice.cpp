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

int DUDevice::getFWVerReg()
{
    return (regs->fwVer);
}

int DUDevice::getBrdStatusReg()
{
    return (regs->brdStatus);
}

void DUDevice::setBrdCtrlReg(int val)
{
    regs->brdCtrl = val;
}

int DUDevice::getBrdCtrlReg()
{
    return (regs->brdCtrl);
}

void DUDevice::setDiagInfoReg(int val)
{
    regs->diagInfo = val;
}

int DUDevice::getArmKSineReg(RFCC_CH ch)
{
    return (regs->armKSine[ch]);
}

void DUDevice::setArmKSineReg(RFCC_CH ch, int val)
{
    regs->armKSine[ch] = val;
}

int DUDevice::getAtbKSineReg(RFCC_CH ch)
{
    return (regs->atbKSine[ch]);
}

void DUDevice::setAtbKSineReg(RFCC_CH ch, int val)
{
    regs->atbKSine[ch] = val;
}

int DUDevice::getSLStatusReg()
{
    return (regs->slResult);
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

void DUDevice::getRegs(int startReg, int endReg)
{
    int numRegs = (endReg - startReg)/4 + 1;
    printf("Display Regs: start = 0x%x, end = 0x%x, num = %d\n", startReg, endReg, numRegs);
    for (int i = 0; i < numRegs; i++)
    {
        printf("0x%x: 0x%x\n", startReg + (i*4), readReg(startReg + (i*4)));
    }
}
