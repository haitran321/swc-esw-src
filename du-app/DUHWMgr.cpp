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

    /* Common config parameters */
    int MODULE_TYPE;
    int FORCE_TEST_MODE;
    int STEERING_WORD_SRC;
    int DCU_SCLK_READBACK_DELAY;

    _logger.logInfo("DUHWMgr Initializing");

    // Get config parameters
    ConfigDataManager &configs = ConfigDataManager::getInstance();
    rc = rc || configs.get("MODULE_TYPE", MODULE_TYPE);
    rc = rc || configs.get("FORCE_TEST_MODE", FORCE_TEST_MODE);
    rc = rc || configs.get("STEERING_WORD_SRC", STEERING_WORD_SRC);
    rc = rc || configs.get("DCU_SCLK_READBACK_DELAY", DCU_SCLK_READBACK_DELAY);

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

//  brdCtrVal = DeviceUtilities::readMask(DU_BRD_CTRL_MASK, _duDev->getBrdCtrlReg());
    // Reset brdCtl to default
    brdCtrVal = 0;
    _duDev->setBrdCtrlReg(brdCtrVal);

    printf("getFirmwareVersionReg = 0x%x\n", _duDev->getFWVerReg());
    printf("getBoardStatusReg = 0x%x\n", _duDev->getBrdStatusReg());
    printf("getBoardControlReg = 0x%x\n", _duDev->getBrdCtrlReg());
  
    // Set Force Test mode
    if (FORCE_TEST_MODE == TEST)
    {
        brdCtrVal = DeviceUtilities::updateReg(DU_FORCE_TEST_MODE_MASK, brdCtrVal, TEST);
    }
    // Set Steering Word source to ARM
    if (STEERING_WORD_SRC == ARM)
    {
        brdCtrVal = DeviceUtilities::updateReg(DU_STEERING_WORD_SRC_MASK, brdCtrVal, ARM);
    }
    _duDev->setBrdCtrlReg(brdCtrVal);

    // Set DCU_SCLK_READBACK_DELAY
    printf("Setting DCU_SCLK_READBACK_DELAY to %d\n", DCU_SCLK_READBACK_DELAY);
    for (int i = 0; i < NUM_DCU; i++)
    {
        _duDev->setDCUSCLKReg(i, DCU_SCLK_READBACK_DELAY);
    }

    // Set default SWC status to TWGS
    statusToTwgs = getSwcStatusToTwgs();

    getRegs(0x0, 0x24);
    printf("getBoardControlReg = 0x%x\n", _duDev->getBrdCtrlReg());

    setReg(0x408, 12);
    getRegs(0x408, 0x408);
    getRegs(0x1A8, 0x1AC);

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

void DUHWMgr::toggleSWTrigger()
{
    brdCtrVal = DeviceUtilities::updateReg(DU_SW_TRIGGER_MASK, brdCtrVal, 1);
    _duDev->setBrdCtrlReg(brdCtrVal);
    brdCtrVal = DeviceUtilities::updateReg(DU_SW_TRIGGER_MASK, brdCtrVal, 0);
    _duDev->setBrdCtrlReg(brdCtrVal);
}

int DUHWMgr::getSysConfigStatus()
{
    sysConfig = _duDev->getSysConfigStatusReg();

    return sysConfig;
}

int DUHWMgr::getSwcStatusToTwgs()
{
    int status = _duDev->getSwcStatusToTwgsReg();
    return (status)
}

void DUHWMgr::setOverallStatusBit(int val)
{
    statusToTwgs = DeviceUtilities::updateReg(DU_OVERALL_STATUS_MASK, statusToTwgs, val);
}

void DUHWMgr::setConfigBit(int val)
{
    statusToTwgs = DeviceUtilities::updateReg(DU_CONFIG_STATUS_MASK, statusToTwgs, val);
}

void DUHWMgr::setModeBit(int val)
{
    statusToTwgs = DeviceUtilities::updateReg(DU_MODE_STATUS_MASK, statusToTwgs, val);
}

void DUHWMgr::setAlphaOverallStatusBit(int val)
{
    statusToTwgs = DeviceUtilities::updateReg(DU_ALPHA_OVERALL_STATUS_MASK, statusToTwgs, val);
}

void DUHWMgr::setBetaOverallStatusBit(int val)
{
    statusToTwgs = DeviceUtilities::updateReg(DU_BETA_OVERALL_STATUS_MASK, statusToTwgs, val);
}

// TODO: Change to Temp status Mask
void DUHWMgr::setTempStatusBit(int val)
{
    statusToTwgs = DeviceUtilities::updateReg(DU_ALPHA_PS_STATUS_MASK, statusToTwgs, val);
}

// TODO: Change to Pwr Supply status Mask
void DUHWMgr::setPwrSuppliesStatusBit(int val)
{
    statusToTwgs = DeviceUtilities::updateReg(DU_BETA_PS_STATUS_MASK, statusToTwgs, val);
}

void DUHWMgr::setDCUGroupStatusBit(int val)
{
    statusToTwgs = DeviceUtilities::updateReg(DU_DCU_GROUP_STATUS_MASK, statusToTwgs, val);
}

void DUHWMgr::setDCUNumberStatusBit(int val)
{
    statusToTwgs = DeviceUtilities::updateReg(DU_DCU_NUMBER_STATUS_MASK, statusToTwgs, val);
}

void DUHWMgr::setDCUHealthStatusBit(int val)
{
    statusToTwgs = DeviceUtilities::updateReg(DU_DCU_HEALTH_STATUS_MASK, statusToTwgs, val);
}
