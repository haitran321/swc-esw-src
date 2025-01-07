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

TUFWActionType * TUDevice::getCh0StartingAddress()
{
    return (TUFWActionType *)(_apbBusAddr + TU_CH0_STARTING_ADDR_OFFSET);
}

TUFWActionType * TUDevice::getCh1StartingAddress()
{
    return (TUFWActionType *)(_apbBusAddr + TU_CH1_STARTING_ADDR_OFFSET);
}

TUFWActionType * TUDevice::getCh2StartingAddress()
{
    return (TUFWActionType *)(_apbBusAddr + TU_CH2_STARTING_ADDR_OFFSET);
}

TUFWActionType * TUDevice::getCh3StartingAddress()
{
    return (TUFWActionType *)(_apbBusAddr + TU_CH3_STARTING_ADDR_OFFSET);
}

TUFWActionType * TUDevice::getCh4StartingAddress()
{
    return (TUFWActionType *)(_apbBusAddr + TU_CH4_STARTING_ADDR_OFFSET);
}

TUFWActionType * TUDevice::getCh5StartingAddress()
{
    return (TUFWActionType *)(_apbBusAddr + TU_CH5_STARTING_ADDR_OFFSET);
}

TUFWActionType * TUDevice::getCh6StartingAddress()
{
    return (TUFWActionType *)(_apbBusAddr + TU_CH6_STARTING_ADDR_OFFSET);
}

TUFWActionType * TUDevice::getCh7StartingAddress()
{
    return (TUFWActionType *)(_apbBusAddr + TU_CH7_STARTING_ADDR_OFFSET);
}

TUFWActionType * TUDevice::getCh8StartingAddress()
{
    return (TUFWActionType *)(_apbBusAddr + TU_CH8_STARTING_ADDR_OFFSET);
}

TUFWActionType * TUDevice::getCh9StartingAddress()
{
    return (TUFWActionType *)(_apbBusAddr + TU_CH9_STARTING_ADDR_OFFSET);
}

int TUDevice::readReg(int offset)
{
   return (*((unsigned *)(_apbBusAddr + offset)));
}

void TUDevice::writeReg(int offset, int data)
{
    *((unsigned *)(_apbBusAddr + offset)) = data;
}

int TUDevice::getFirmwareVersionReg()
{
    return (regs->firmwareVersion);
}

int TUDevice::getBoardStatusReg()
{
    return (regs->boardStatus);
}

STATUS TUDevice::setBoardControlReg(int val)
{
    STATUS rc = OK;

    regs->boardControl = val;

    return rc;
}

STATUS TUDevice::getBoardControlReg()
{
    return (regs->boardControl);
}

STATUS TUDevice::setCWRegs(TU_CHANNEL channel, TUCWSignalType signal)
{
    STATUS rc = OK;

    regs->cwSignals[channel].amplitude = signal.amplitude;
    regs->cwSignals[channel].freq = signal.freq;

    return rc;    
}

TUCWSignalType TUDevice::getCWRegs(TU_CHANNEL channel)
{
    TUCWSignalType action;

    action.amplitude = regs->cwSignals[channel].amplitude;
    action.freq = regs->cwSignals[channel].freq;
    
    return action;    
}

// STATUS TUDevice::setDMAControllerReg(int val)
// {
//     STATUS rc = OK;

//     regs->dmaControlleReg = val;

//     return rc;
// }

// void TUDevice::readFeedbackRegs(int numRegs)
// {
//     for (int i = 0; i < numRegs; i++)
//     {
//         printf("spare[%d] = 0x%x\n", i, regs->spare3[i]);
//     }
// }

// void TUDevice::readDMAReg()
// {
//     writeReg(0x9C, 0x8);
//     sleep(0.1);
//     writeReg(0x9C, 0x0);
//     printf("Write control reg = 0x%x\n", readReg(0x90));
//     printf("Write status reg = 0x%x\n", readReg(0x24));
//     printf("Read control reg = 0x%x\n", readReg(0x4C));
//     printf("Read status reg = 0x%x\n", readReg(0x28));
// }

STATUS TUDevice::setNumActionsReg(TU_CHANNEL ch, int val)
{
    STATUS rc = OK;

    if (ch == TU_CHANNEL_0)
    {
        regs->ch0NumActions = val;
    }
    else if (ch == TU_CHANNEL_1)
    {
        regs->ch1NumActions = val;
    }
    else if (ch == TU_CHANNEL_2)
    {
        regs->ch2NumActions = val;
    }
    else if (ch == TU_CHANNEL_3)
    {
        regs->ch3NumActions = val;
    }
    else
    {
        printf("ERROR:  invalid TU channel number = %d\n", ch);
        rc = ERROR;
    }
    
    return rc;
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

void TUDevice::setGatedCWEmulatorRegs(int period)
{
//  regs->gatedCWEmOnDur = duration;
    regs->gatedCWEmPeriod = period; 
}
