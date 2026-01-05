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

int DUDevice::getSysConfigStatusReg()
{
    return (regs->sysConfigStatus);
}

int DUDevice::getSwcStatusToTwgsReg()
{
    return (regs->swcStatusToTwgs);
}

void DUDevice::setSwcStatusToTwgsReg(int val)
{
    regs->swcStatusToTwgs = val;
}

int DUDevice::getCableDelayCompReg()
{
    return (regs->cableDelayComp);
}

void DUDevice::setCableDelayCompReg(int val)
{
    regs->cableDelayComp = val;
}

int DUDevice::getARMInitStatusReg()
{
    return (regs->armInitStatus);
}

void DUDevice::setARMInitStatusReg(int val)
{
    regs->armInitStatus = val;
}

int DUDevice::getDiagInfoReg()
{
    return (regs->diagInfo);
}

int DUDevice::getDCUStatusReg(int regNum)
{
    return (regs->dcuStatus[regNum]);
}

void DUDevice::setDCUSCLKReg(int dcuNum, int val)
{
    regs->dcuSCLKDelay[dcuNum] = val;
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
