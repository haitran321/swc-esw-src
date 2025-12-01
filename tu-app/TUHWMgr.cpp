#include <unistd.h>     // for sleep()
#include "TUHWMgr.h"
#include "DeviceUtilities.h"
#include "ConfigDataManager.h"

TUHWMgr::TUHWMgr() :
_logger(Logger::getInstance()),
_tuDev(NULL),
_brdCtrVal(-1)
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
    _tuDev->close();
}

STATUS TUHWMgr::initialize()
{
    STATUS rc = OK;

    _logger.logInfo("TUHWMgr Initializing");

    // Get config parameters
    ConfigDataManager &configs = ConfigDataManager::getInstance();
//  rc = rc || configs.get("MODULE_TYPE", MODULE_TYPE);

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
    
    _brdCtrVal = DeviceUtilities::readMask(TU_BRD_CTRL_MASK, _tuDev->getBrdCtrlReg());

    // Set Test mode
    _tuDev->setBrdCtrlReg(_brdCtrVal);

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

int TUHWMgr::getBrdStatus()
{
    return (_tuDev->getBrdStatusReg());
}

int TUHWMgr::getArmKSine(RFCC_CH ch)
{
    return(_tuDev->getArmKSineReg(ch));
}

void TUHWMgr::setArmKSine(RFCC_CH ch, int val)
{
    _tuDev->setArmKSineReg(ch, val);
}

int TUHWMgr::getFWScanLimitCheckStatus()
{
    return (_tuDev->getSLStatusReg());
}

void TUHWMgr::toggleRLTDSignal()
{
    _brdCtrVal = DeviceUtilities::updateReg(TU_SET_RLTD_SIGNAL_MASK, _brdCtrVal, 1);
    _tuDev->setBrdCtrlReg(_brdCtrVal);
    _brdCtrVal = DeviceUtilities::updateReg(TU_SET_RLTD_SIGNAL_MASK, _brdCtrVal, 0);
    _tuDev->setBrdCtrlReg(_brdCtrVal);
}

void TUHWMgr::setRLCPSignal(CmdOnOff flag)
{
    _brdCtrVal = DeviceUtilities::updateReg(TU_SET_RLCP_SIGNAL_MASK, _brdCtrVal, flag);
    _tuDev->setBrdCtrlReg(_brdCtrVal);
}

void TUHWMgr::setRLSCSignal(CmdOnOff flag)
{
    _brdCtrVal = DeviceUtilities::updateReg(TU_SET_RLSC_SIGNAL_MASK, _brdCtrVal, flag);
    _tuDev->setBrdCtrlReg(_brdCtrVal);
}

DUTUStatusType TUHWMgr::readTUStatus()
{
    int status = _tuDev->getBrdStatusReg();

    // TO BE REMOVED
//  status = 0x80000001;
    status = 0x00000018;

    _logger.logDebug("TU %d Status for 0x%x module", MODULE_TYPE, status);
    printf("TU %d Status for 0x%x module\n", MODULE_TYPE, status);

    DUTUStatusType tuStatus;
    tuStatus.overallStatus = (HealthState)(DeviceUtilities::readMask(TU_BIT_RESULT_MASK, status));
    tuStatus.readyStatus = (HealthState)(DeviceUtilities::readMask(TU_READY_STATUS_MASK, status));
    tuStatus.highTempAlarm = (HealthState)(DeviceUtilities::readMask(TU_HIGH_TEMP_ALARM_MASK, status));
    tuStatus.vccintAlarm = (HealthState)(DeviceUtilities::readMask(TU_VCC_INT_ALARM_MASK, status));
    tuStatus.vccauxAlarm = (HealthState)(DeviceUtilities::readMask(TU_VCC_AUX_ALARM_MASK, status));
    tuStatus.vbramAlarm = (HealthState)(DeviceUtilities::readMask(TU_VBRAM_ALARM_MASK, status));

    return (tuStatus);
}

