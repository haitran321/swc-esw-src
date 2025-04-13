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

STATUS DUHWMgr::setArmKSine(RFCC_CH ch, int val)
{
    return(_duDev->setArmKSineReg(ch, val));
}

int DUHWMgr::getArmKSine(RFCC_CH ch)
{
    return(_duDev->getArmKSineReg(ch));
}

STATUS DUHWMgr::setAtbKSine(RFCC_CH ch, int val)
{
    return(_duDev->setAtbKSineReg(ch, val));
}

int DUHWMgr::getAtbKSine(RFCC_CH ch)
{
    return(_duDev->getAtbKSineReg(ch));
}

int DUHWMgr::getFWScanLimitCheckStatus()
{
    return (_duDev->getSLStatusReg());
}

void DUHWMgr::runFWScanLimitCheck()
{
    boardControlValue = DeviceUtilities::updateReg(DU_TRIGGER_SL_TEST_MASK, boardControlValue, 1);
    setBoardControl(boardControlValue);
    boardControlValue = DeviceUtilities::updateReg(DU_TRIGGER_SL_TEST_MASK, boardControlValue, 0);
    setBoardControl(boardControlValue);
}

