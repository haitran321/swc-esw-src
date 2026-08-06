#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#include "TUDevice.h"

static TURegType *regs = NULL;

TUDevice::TUDevice(unsigned int offset) :
Device("/dev/mem"),
_offset(offset)
{
}

STATUS TUDevice::mmap()
{
    _apbBusAddr = ::mmap(NULL, TU_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, _offset);

    printf("Successfully mmap _apbBusAddr = %p\n", _apbBusAddr);

    regs = (TURegType *)_apbBusAddr;

    return OK;
}

int TUDevice::readReg(int offset)
{
   return (*((unsigned *)(_apbBusAddr + offset)));
}

void TUDevice::writeReg(int offset, int data)
{
    *((unsigned *)(_apbBusAddr + offset)) = data;
}

int TUDevice::getFWVerReg()
{
    return (regs->fwVer);
}

int TUDevice::getBrdStatusReg()
{
    return (regs->brdStatus);
}

void TUDevice::setBrdCtrlReg(int val)
{
    regs->brdCtrl = val;
}

int TUDevice::getBrdCtrlReg()
{
    return (regs->brdCtrl);
}

void TUDevice::setArmKSineReg(RFCC_CH ch, int val)
{
    regs->armKSine[ch] = val;
}

int TUDevice::getArmKSineReg(RFCC_CH ch)
{
    return (regs->armKSine[ch]);
}

int TUDevice::getSLStatusReg()
{
    return (regs->slResult);
}

int TUDevice::getSysConfigStatusReg()
{
    return (regs->sysConfigStatus);
}

void TUDevice::setARMInitStatusReg(int val)
{
    regs->armInitStatus = val;
}

int TUDevice::getARMInitStatusReg()
{
    return (regs->armInitStatus);
}

int TUDevice::getSWCStatusReg()
{
    return (regs->swcStatus);
}

int TUDevice::getSWCRStatusReg()
{
    return (regs->swcrStatus);
}

void TUDevice::setRLTDPulseDurReg(int val)
{
    regs->rltdPulseDur = val;
}

int TUDevice::getRLTDPulseDurReg()
{
    return (regs->rltdPulseDur);
}

void TUDevice::setRLCPPulseDurReg(int val)
{
    regs->rlcpPulseDur = val;
}

int TUDevice::getRLCPPulseDurReg()
{
    return (regs->rlcpPulseDur);
}

void TUDevice::setRLSCPulseDurReg(int val)
{
    regs->rlscPulseDur = val;
}

int TUDevice::getRLSCPulseDurReg()
{
    return (regs->rlscPulseDur);
}

void TUDevice::setSteeringWordPulseDurReg(int val)
{
    regs->steeringWordPulseDur = val;
}

int TUDevice::getSteeringWordPulseDurReg()
{
    return (regs->steeringWordPulseDur);
}

void TUDevice::setRLTDPeriodReg(int val)
{
    regs->rltdPeriod = val;
}

int TUDevice::getRLTDPeriodReg()
{
    return (regs->rltdPeriod);
}

void TUDevice::setNumRLTDPerCycleReg(int val)
{
    regs->numRLTDPerCycle = val;
}

int TUDevice::getNumRLTDPerCycleReg()
{
    return (regs->numRLTDPerCycle);
}

void TUDevice::setRLTDPreTimeReg(int val)
{
    regs->rltdPreTime = val;
}

int TUDevice::getRLTDPreTimeReg()
{
    return (regs->rltdPreTime);
}

void TUDevice::setNumIncReg(int val)
{
    regs->numInc = val;
}

int TUDevice::getNumIncReg()
{
    return (regs->numInc);
}

void TUDevice::setAlphaIncReg(int val)
{
    regs->alphaInc = val;
}

int TUDevice::getAlphaIncReg()
{
    return (regs->alphaInc);
}

void TUDevice::setBetaIncReg(int val)
{
    regs->betaInc = val;
}

int TUDevice::getBetaIncReg()
{
    return (regs->betaInc);
}

void TUDevice::setCycleResetTimeReg(int val)
{
    regs->cycleResetTime = val;
}

int TUDevice::getCycleResetTimeReg()
{
    return (regs->cycleResetTime);
}

void TUDevice::setNumCycleReg(int val)
{
    regs->numCycle = val;
}

int TUDevice::getNumCycleReg()
{
    return (regs->numCycle);
}

int TUDevice::getFPGADieTempReg()
{
    return (regs->fpgaDieTemp);
}

void TUDevice::setDiagInfoReg(int val)
{
    regs->diagInfo = val;
}

int TUDevice::getDiagInfoReg()
{
    return (regs->diagInfo);
}

void TUDevice::getRegs(int startReg, int endReg)
{
    int numRegs = (endReg - startReg)/4 + 1;
    printf("Display Regs: start = 0x%x, end = 0x%x, num = %d\n", startReg, endReg, numRegs);
    for (int i = 0; i < numRegs; i++)
    {
        printf("0x%x: 0x%x\n", startReg + (i*4), readReg(startReg + (i*4)));
    }
}
