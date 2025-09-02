#include <unistd.h>     // for sleep()
#include "DUHWMgr.h"
#include "DeviceUtilities.h"
#include "ConfigDataManager.h"

DUHWMgr::DUHWMgr() :
_logger(Logger::getInstance()),
_duDev(NULL),
brdCtrVal(-1)
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

    printf("getFirmwareVersionReg = 0x%x\n", _duDev->getFWVerReg());
    printf("getBoardStatusReg = 0x%x\n", _duDev->getBrdStatusReg());
    printf("getBoardControlReg = 0x%x\n", _duDev->getBrdCtrlReg());
    
    brdCtrVal = DeviceUtilities::readMask(DU_BRD_CTRL_MASK, _duDev->getBrdCtrlReg());

    // Set Test mode
    brdCtrVal = DeviceUtilities::updateReg(DU_FORCE_TEST_MODE_MASK, brdCtrVal, TEST);
    brdCtrVal = DeviceUtilities::updateReg(DU_STEERING_WORD_SRC_MASK, brdCtrVal, ARM);
    _duDev->setBrdCtrlReg(brdCtrVal);

    getRegs(0x0, 0x24);
    printf("getBoardControlReg = 0x%x\n", _duDev->getBrdCtrlReg());

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

int DUHWMgr::getArmKSine(RFCC_CH ch)
{
    return(_duDev->getArmKSineReg(ch));
}

void DUHWMgr::setArmKSine(RFCC_CH ch, int val)
{
    _duDev->setArmKSineReg(ch, val);
}

int DUHWMgr::getAtbKSine(RFCC_CH ch)
{
    return(_duDev->getAtbKSineReg(ch));
}

void DUHWMgr::setAtbKSine(RFCC_CH ch, int val)
{
    _duDev->setAtbKSineReg(ch, val);
}

int DUHWMgr::getFWScanLimitCheckStatus()
{
    return (_duDev->getSLStatusReg());
}
void DUHWMgr::runFWScanLimitCheck()
{
    brdCtrVal = DeviceUtilities::updateReg(DU_NEW_STEERING_WORD_MASK, brdCtrVal, 1);
    _duDev->setBrdCtrlReg(brdCtrVal);
    brdCtrVal = DeviceUtilities::updateReg(DU_NEW_STEERING_WORD_MASK, brdCtrVal, 0);
    _duDev->setBrdCtrlReg(brdCtrVal);
}

