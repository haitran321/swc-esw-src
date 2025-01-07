#include <unistd.h>     // for sleep()
#include "TUHWMgr.h"
#include "DeviceUtilities.h"
#include "ConfigDataManager.h"

TUHWMgr::TUHWMgr() :
_logger(Logger::getInstance()),
_tuDev(NULL),
boardControlValue(-1)
{
}

TUHWMgr::~TUHWMgr()
{
}

TUHWMgr &TUHWMgr::getInstance()
{
    static TUHWMgr object;
    return (object);
}

void TUHWMgr::close()
{
    // 	delete _tuDev;
	// _tuDev = NULL;

    _tuDev->close();
}

STATUS TUHWMgr::initialize()
{
    STATUS rc = OK;

    _logger.logInfo("TUHWMgr Initializing");

    // Get config parameters
    ConfigDataManager &configs = ConfigDataManager::getInstance();
    rc = rc || configs.get("MODULE_TYPE", MODULE_TYPE);

    // Open /dev/mem device
    _tuDev = new TUDevice(APB_BUS_OFFSET);
    if (_tuDev->open() == ERROR)
    {
        return ERROR;
    }
    if (_tuDev->mmap() == ERROR)
    {
        return ERROR;
    }

    printf("After create _tuDev\n");

    printf("getFirmwareVersionReg = 0x%x\n", _tuDev->getFirmwareVersionReg());
    printf("getBoardStatusReg = 0x%x\n", _tuDev->getBoardStatusReg());
    printf("getBoardControlReg = 0x%x\n", _tuDev->getBoardControlReg());

    // Default tx module mode to: 
    // CH1: enable, pbp mode, no pbsk, lfm down chirp, no trigger timeout, external trigger
    // CH2: disable
    // CH3: disable
    // CH4: disable
    // All: use action cmd in PBP mode, external trigger, 
    
    boardControlValue = DeviceUtilities::readMask(TU_BOARD_CONTROL_MASK, getBoardControl());

    // Set trigger mode to external
//  boardControlValue = DeviceUtilities::updateReg(TU_TRIGGER_MODE_MASK, boardControlValue, TU_TRIGGER_EXTERNAL);
//  setBoardControl(boardControlValue);

    getRegs(0x8, 0x8);
    _logger.logInfo("Board control reg = 0x%x", getBoardControl());
}

void TUHWMgr::getRegs(int startReg, int endReg)
{
    _tuDev->getRegs(startReg, endReg);
}

void TUHWMgr::setReg(int offset, int data)
{
    _tuDev->writeReg(offset, data);
}

int TUHWMgr::getBoardControl()
{
    return (_tuDev->getBoardControlReg());
}

STATUS TUHWMgr::setBoardControl(int val)
{
    return(_tuDev->setBoardControlReg(val));
}

int TUHWMgr::getBoardStatus()
{
    return (_tuDev->getBoardStatusReg());
}

void TUHWMgr::clearAllActions()
{
    for (int i = 0; i < NUM_TU_CHANNELS; i++)
    {
        _numActions[i] = 0;
    }
}

STATUS TUHWMgr::addAction(TU_CHANNEL chId, TUActionType action, unsigned int ftw)
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

void TUHWMgr::programActions()
{
    _logger.logInfo("In Non Combined Mode: TU actions - numActions: ch0 = %d, ch1 = %d, ch2 = %d, ch3 = %d",
                    _numActions[TU_CHANNEL_0], _numActions[TU_CHANNEL_1],
                    _numActions[TU_CHANNEL_2], _numActions[TU_CHANNEL_3]);
    _tuDev->setNumActionsReg(TU_CHANNEL_0, _numActions[TU_CHANNEL_0]);
    _tuDev->setNumActionsReg(TU_CHANNEL_1, _numActions[TU_CHANNEL_1]);
    _tuDev->setNumActionsReg(TU_CHANNEL_2, _numActions[TU_CHANNEL_2]);
    _tuDev->setNumActionsReg(TU_CHANNEL_3, _numActions[TU_CHANNEL_3]);
}

int TUHWMgr::getNumActions(TU_CHANNEL ch)
{
    return _numActions[ch];
}

void TUHWMgr::toggleLoadCmdFlag()
{
    boardControlValue = DeviceUtilities::updateReg(TU_LOAD_ACTION_CMD_MASK, boardControlValue, 1);
    setBoardControl(boardControlValue);
    boardControlValue = DeviceUtilities::updateReg(TU_LOAD_ACTION_CMD_MASK, boardControlValue, 0);
    setBoardControl(boardControlValue);
}

void TUHWMgr::toggleSWInternalTriggerFlag()
{
    boardControlValue = DeviceUtilities::updateReg(TU_FORCE_TRIGGER_MASK, boardControlValue, 1);
    setBoardControl(boardControlValue);
    getRegs(0x8, 0x8);
    boardControlValue = DeviceUtilities::updateReg(TU_FORCE_TRIGGER_MASK, boardControlValue, 0);
    setBoardControl(boardControlValue);
    getRegs(0x8, 0x8);
}

void TUHWMgr::setTriggerMode(TU_TRIGGER_ENUM mode)
{
    boardControlValue = DeviceUtilities::updateReg(TU_TRIGGER_MODE_MASK, boardControlValue, mode);
    _logger.logInfo("Setting trigger mode to %d, board control reg = 0x%x", mode, getBoardControl());
    setBoardControl(boardControlValue);
}

