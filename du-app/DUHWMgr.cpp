#include <unistd.h>     // for sleep()
#include "DUHWMgr.h"
#include "DeviceUtilities.h"
#include "ConfigDataManager.h"

DUHWMgr::DUHWMgr() :
_logger(Logger::getInstance()),
_duDev(NULL),
boardControlValue(-1)
{
}

DUHWMgr::~DUHWMgr()
{
}

DUHWMgr &DUHWMgr::getInstance()
{
    static DUHWMgr object;
    return (object);
}

void DUHWMgr::close()
{
    // 	delete _duDev;
	// _duDev = NULL;

    _duDev->close();
}

STATUS DUHWMgr::initialize()
{
    STATUS rc = OK;

    _logger.logInfo("DUHWMgr Initializing");

    // Get config parameters
    ConfigDataManager &configs = ConfigDataManager::getInstance();
    rc = rc || configs.get("MODULE_TYPE", MODULE_TYPE);

    // Open /dev/mem device
    _duDev = new DUDevice(APB_BUS_OFFSET);
    if (_duDev->open() == ERROR)
    {
        return ERROR;
    }
    if (_duDev->mmap() == ERROR)
    {
        return ERROR;
    }

    printf("After create _duDev\n");

    printf("getFirmwareVersionReg = 0x%x\n", _duDev->getFirmwareVersionReg());
    printf("getBoardStatusReg = 0x%x\n", _duDev->getBoardStatusReg());
    printf("getBoardControlReg = 0x%x\n", _duDev->getBoardControlReg());
    
    boardControlValue = DeviceUtilities::readMask(DU_BOARD_CONTROL_MASK, getBoardControl());

    // Set trigger mode to external
//  boardControlValue = DeviceUtilities::updateReg(DU_TRIGGER_MODE_MASK, boardControlValue, DU_TRIGGER_EXTERNAL);
//  setBoardControl(boardControlValue);

//  getRegs(0x8, 0x8);
//  _logger.logInfo("Board control reg = 0x%x", getBoardControl());

    return OK;
}

void DUHWMgr::getRegs(int startReg, int endReg)
{
    _duDev->getRegs(startReg, endReg);
}

void DUHWMgr::setReg(int offset, int data)
{
    _duDev->writeReg(offset, data);
}

int DUHWMgr::getBoardControl()
{
    return (_duDev->getBoardControlReg());
}

STATUS DUHWMgr::setBoardControl(int val)
{
    return(_duDev->setBoardControlReg(val));
}

int DUHWMgr::getBoardStatus()
{
    return (_duDev->getBoardStatusReg());
}

int DUHWMgr::getDiagInfo()
{
    return (_duDev->getDiagInfoReg());
}

STATUS DUHWMgr::setDiagInfo(int val)
{
    return(_duDev->setDiagInfoReg(val));
}

void DUHWMgr::clearAllActions()
{
    for (int i = 0; i < NUM_DU_CHANNELS; i++)
    {
        _numActions[i] = 0;
    }
}

STATUS DUHWMgr::addAction(DU_CHANNEL chId, DUActionType action, unsigned int ftw)
{
    STATUS rc = OK;

    if (_numActions[chId] < MAX_ACTION - 1)
    {
        // TODO: Need to validate the actions

        // Add action
        (_actions[chId] + _numActions[chId])->startTime = (unsigned int)(action.startTime / 5);
        (_actions[chId] + _numActions[chId])->stopTime = (unsigned int)(action.stopTime / 5);
        (_actions[chId]+_numActions[chId])->signal.amplitude = action.signal.amplitude;					
        (_actions[chId]+_numActions[chId])->signal.phaseOffset = action.signal.phaseOffset;
        (_actions[chId]+_numActions[chId])->signal.freq = ftw;
        (_actions[chId]+_numActions[chId])->signal.lfmRamp = action.signal.lfmRamp;
        (_actions[chId]+_numActions[chId])->signal.phaseCode = action.signal.phaseCode;

        // Increment number of action
        _numActions[chId] += 1;

//      getRegs(0xF0, 0x120);
    }
    else
    {
        rc = ERROR;
    }

    return rc;
}

// In non-combined mode:
// CH0 --> DAC0
// CH1 --> DAC1
// CH2 --> DAC2
// CH3 --> DAC3

// In combined mode:
// CH0 + CH1 --> DAC0
// CH2 + CH3 --> DAC1
// CH4 + CH5 --> DAC2
// (CH6 + CH7) + (CH8 + CH9) --> DAC3

void DUHWMgr::programActions()
{
    _logger.logInfo("In Non Combined Mode: DU actions - numActions: ch0 = %d, ch1 = %d, ch2 = %d, ch3 = %d",
                    _numActions[DU_CHANNEL_0], _numActions[DU_CHANNEL_1],
                    _numActions[DU_CHANNEL_2], _numActions[DU_CHANNEL_3]);
    _duDev->setNumActionsReg(DU_CHANNEL_0, _numActions[DU_CHANNEL_0]);
    _duDev->setNumActionsReg(DU_CHANNEL_1, _numActions[DU_CHANNEL_1]);
    _duDev->setNumActionsReg(DU_CHANNEL_2, _numActions[DU_CHANNEL_2]);
    _duDev->setNumActionsReg(DU_CHANNEL_3, _numActions[DU_CHANNEL_3]);
}

int DUHWMgr::getNumActions(DU_CHANNEL ch)
{
    return _numActions[ch];
}

void DUHWMgr::toggleLoadCmdFlag()
{
    boardControlValue = DeviceUtilities::updateReg(DU_LOAD_ACTION_CMD_MASK, boardControlValue, 1);
    setBoardControl(boardControlValue);
    boardControlValue = DeviceUtilities::updateReg(DU_LOAD_ACTION_CMD_MASK, boardControlValue, 0);
    setBoardControl(boardControlValue);
}

void DUHWMgr::toggleSWInternalTriggerFlag()
{
    boardControlValue = DeviceUtilities::updateReg(DU_FORCE_TRIGGER_MASK, boardControlValue, 1);
    setBoardControl(boardControlValue);
    getRegs(0x8, 0x8);
    boardControlValue = DeviceUtilities::updateReg(DU_FORCE_TRIGGER_MASK, boardControlValue, 0);
    setBoardControl(boardControlValue);
    getRegs(0x8, 0x8);
}

void DUHWMgr::setTriggerMode(DU_TRIGGER_ENUM mode)
{
    boardControlValue = DeviceUtilities::updateReg(DU_TRIGGER_MODE_MASK, boardControlValue, mode);
    _logger.logInfo("Setting trigger mode to %d, board control reg = 0x%x", mode, getBoardControl());
    setBoardControl(boardControlValue);
}

void DUHWMgr::toggleInterruptBit()
{
    diagRegValue = DeviceUtilities::updateReg(0x1, diagRegValue, 0);
    setDiagInfo(diagRegValue);
    diagRegValue = DeviceUtilities::updateReg(0x1, diagRegValue, 1);
    setDiagInfo(diagRegValue);
}


