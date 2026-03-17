#include <unistd.h>     // for sleep()
#include "TUHWMgr.h"
#include "DeviceUtilities.h"
#include "ConfigDataManager.h"

TUHWMgr::TUHWMgr() :
_logger(Logger::getInstance()),
_tuDev(NULL),
_brdCtrVal(0),
_armInitReady(0)
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

    /* Config parameters */
    int RLTD_PULSE_DURATION;
    int RLCP_PULSE_DURATION;
    int RLSC_PULSE_DURATION;
    int STEERING_WORD_PULSE_DURATION;
    int RLTD_PRE_TRIGGER_TIME;

    _logger.logInfo("TUHWMgr Initializing");

    // Get config parameters
    ConfigDataManager &configs = ConfigDataManager::getInstance();
    rc = rc || configs.get("RLTD_PULSE_DURATION", RLTD_PULSE_DURATION);
    rc = rc || configs.get("RLCP_PULSE_DURATION", RLCP_PULSE_DURATION);
    rc = rc || configs.get("RLSC_PULSE_DURATION", RLSC_PULSE_DURATION);
    rc = rc || configs.get("STEERING_WORD_PULSE_DURATION", STEERING_WORD_PULSE_DURATION);
    rc = rc || configs.get("RLTD_PRE_TRIGGER_TIME", RLTD_PRE_TRIGGER_TIME);

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

    // Set Arm Init Reg to indicate the OS is ready
    _armInitReady = DeviceUtilities::updateReg(TU_OS_INIT_STATUS_MASK, _armInitReady, READY);
    _tuDev->setARMInitStatusReg(_armInitReady);

    printf("getFirmwareVersionReg = 0x%x\n", _tuDev->getFWVerReg());
    printf("getBoardStatusReg = 0x%x\n", _tuDev->getBrdStatusReg());
    printf("getBoardControlReg = 0x%x\n", _tuDev->getBrdCtrlReg());
    
    _brdCtrVal = DeviceUtilities::readMask(TU_BRD_CTRL_MASK, _tuDev->getBrdCtrlReg());

    // Set Test mode
    _tuDev->setBrdCtrlReg(_brdCtrVal);

    // Set pulse durations
    _tuDev->setRLTDPulseDurReg(RLTD_PULSE_DURATION);
    _tuDev->setRLCPPulseDurReg(RLCP_PULSE_DURATION);
    _tuDev->setRLSCPulseDurReg(RLSC_PULSE_DURATION);
    _tuDev->setSteeringWordPulseDurReg(STEERING_WORD_PULSE_DURATION);
    _tuDev->setRLTDPreTimeReg(RLTD_PRE_TRIGGER_TIME);

    // Set ARM Init Reg to indicate the app is ready
    _armInitReady = DeviceUtilities::updateReg(TU_APP_INIT_STATUS_MASK, _armInitReady, READY);
    _tuDev->setARMInitStatusReg(_armInitReady);

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
    _brdCtrVal = DeviceUtilities::updateReg(TU_SW_TRIGGER_MASK, _brdCtrVal, 1);
    _tuDev->setBrdCtrlReg(_brdCtrVal);
    _brdCtrVal = DeviceUtilities::updateReg(TU_SW_TRIGGER_MASK, _brdCtrVal, 0);
    _tuDev->setBrdCtrlReg(_brdCtrVal);
}

void TUHWMgr::setRLCPSignal(CmdOnOff flag)
{
    _brdCtrVal = DeviceUtilities::updateReg(TU_RLCP_CMD_MASK, _brdCtrVal, flag);
    _tuDev->setBrdCtrlReg(_brdCtrVal);
}

void TUHWMgr::setRLSCSignal(CmdOnOff flag)
{
    _brdCtrVal = DeviceUtilities::updateReg(TU_RLSC_CMD_MASK, _brdCtrVal, flag);
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

