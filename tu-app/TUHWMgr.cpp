#include <unistd.h>     // for sleep()
#include "TUHWMgr.h"
#include "DeviceUtilities.h"
#include "ConfigDataManager.h"

TUHWMgr::TUHWMgr() :
_logger(Logger::getInstance()),
_tuDev(NULL),
brdCtrVal(-1)
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

    printf("getFirmwareVersionReg = 0x%x\n", _tuDev->getFWVerReg());
    printf("getBoardStatusReg = 0x%x\n", _tuDev->getBrdStatusReg());
    printf("getBoardControlReg = 0x%x\n", _tuDev->getBrdCtrlReg());
    
    brdCtrVal = DeviceUtilities::readMask(TU_BRD_CTRL_MASK, _tuDev->getBrdCtrlReg());

    // Set Test mode
    // brdCtrVal = DeviceUtilities::updateReg(TU_MODE_MASK, brdCtrVal, TEST);
    // brdCtrVal = DeviceUtilities::updateReg(TU_STEERING_WORD_SRC_MASK, brdCtrVal, ARM);
    _tuDev->setBrdCtrlReg(brdCtrVal);

//  getRegs(0x8, 0x8);
//  _logger.logInfo("Board control reg = 0x%x", getBoardControl());

    return OK;
}

void TUHWMgr::getRegs(int startReg, int endReg)
{
    _tuDev->getRegs(startReg, endReg);
}

void TUHWMgr::setReg(int offset, int data)
{
    _tuDev->writeReg(offset, data);
}

int TUHWMgr::getArmKSine(RFCC_CH ch)
{
    return(_tuDev->getArmKSineReg(ch));
}

void TUHWMgr::setArmKSine(RFCC_CH ch, int val)
{
    _tuDev->setArmKSineReg(ch, val);
}

int TUHWMgr::getAtbKSine(RFCC_CH ch)
{
    return(_tuDev->getAtbKSineReg(ch));
}

void TUHWMgr::setAtbKSine(RFCC_CH ch, int val)
{
    _tuDev->setAtbKSineReg(ch, val);
}

int TUHWMgr::getFWScanLimitCheckStatus()
{
    return (_tuDev->getSLStatusReg());
}

void TUHWMgr::runFWScanLimitCheck()
{
    brdCtrVal = DeviceUtilities::updateReg(TU_NEW_STEERING_WORD_MASK, brdCtrVal, 1);
    _tuDev->setBrdCtrlReg(brdCtrVal);
    brdCtrVal = DeviceUtilities::updateReg(TU_NEW_STEERING_WORD_MASK, brdCtrVal, 0);
    _tuDev->setBrdCtrlReg(brdCtrVal);
}

