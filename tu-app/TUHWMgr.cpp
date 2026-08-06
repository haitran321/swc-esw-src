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

    // Verbose parameters
    rc = rc || configs.get("VERBOSE", _verbose);

    rc = rc || configs.get("RLTD_PULSE_DURATION", RLTD_PULSE_DURATION);
    rc = rc || configs.get("RLCP_PULSE_DURATION", RLCP_PULSE_DURATION);
    rc = rc || configs.get("RLSC_PULSE_DURATION", RLSC_PULSE_DURATION);
    rc = rc || configs.get("STEERING_WORD_PULSE_DURATION", STEERING_WORD_PULSE_DURATION);
    rc = rc || configs.get("RLTD_PRE_TRIGGER_TIME", RLTD_PRE_TRIGGER_TIME);

    // For testing.  To be removed
    rc = rc || configs.get("USE_STATUS_EMULATOR", USE_STATUS_EMULATOR);
    if (USE_STATUS_EMULATOR == 1)
    {
        emTUStatusReg = 0x80000001;
    }
    else
    {
        emTUStatusReg = 0x0;
    }

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

    if (_verbose)
    {
        printf("getFirmwareVersionReg = 0x%x\n", _tuDev->getFWVerReg());
        printf("getBoardStatusReg = 0x%x\n", _tuDev->getBrdStatusReg());
        printf("getBoardControlReg = 0x%x\n", _tuDev->getBrdCtrlReg());
    }

    _logger.logDebug("MODULE_TYPE = %d, FW Verison = 0x%x, board status = 0x%x, board control = 0x%x",
                     MODULE_TYPE, _tuDev->getFWVerReg(), _tuDev->getBrdStatusReg(), _tuDev->getBrdCtrlReg());
    
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

    if (_verbose)
    {
        printf("getBoardControlReg = 0x%x\n", _tuDev->getBrdCtrlReg());
    }
    _logger.logDebug("getBoardControlReg = 0x%x", _tuDev->getBrdCtrlReg());

    _logger.logDebug("Successfully initialize TUMHWMgr");

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

int TUHWMgr::getBrdCtrl()
{
    return (_tuDev->getBrdCtrlReg());
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

void TUHWMgr::setShutdownBit()
{
    printf("TUHWMgr::setShutdownBit\n");
    _brdCtrVal = DeviceUtilities::updateReg(TU_SHUTDOWN_CMD_MASK, _brdCtrVal, 1);
    _tuDev->setBrdCtrlReg(_brdCtrVal);
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

void TUHWMgr::setConfig(SWC_CONFIG config)
{
    _brdCtrVal = DeviceUtilities::updateReg(TU_SYSTEM_CONFIG_MASK, _brdCtrVal, config);
    _tuDev->setBrdCtrlReg(_brdCtrVal);
}

void TUHWMgr::setMode(SWC_MODE mode)
{
    _brdCtrVal = DeviceUtilities::updateReg(TU_SYSTEM_MODE_MASK, _brdCtrVal, mode);
    _tuDev->setBrdCtrlReg(_brdCtrVal);
}

void TUHWMgr::setOLTE(SWC_MODE olte)
{
    _brdCtrVal = DeviceUtilities::updateReg(TU_SYSTEM_OLTE_MASK, _brdCtrVal, olte);
    _tuDev->setBrdCtrlReg(_brdCtrVal);
}

int TUHWMgr::getSWCStatus()
{
    return(_tuDev->getSWCStatusReg());
}

int TUHWMgr::getSWCRStatus()
{
    return(_tuDev->getSWCRStatusReg());
}

int TUHWMgr::getRLTDPeriod()
{
    return(_tuDev->getRLTDPeriodReg());
}

void TUHWMgr::setRLTDPeriod(int val)
{
    // Converting usec to 100MHz clk count
    _tuDev->setRLTDPeriodReg(val*100);
}

int TUHWMgr::getNumRLTDPerCycle()
{
    return(_tuDev->getNumRLTDPerCycleReg());
}

void TUHWMgr::setNumRLTDPerCycle(int val)
{
    _tuDev->setNumRLTDPerCycleReg(val);
}

int TUHWMgr::getNumInc()
{
    return(_tuDev->getNumIncReg());
}

void TUHWMgr::setNumInc(int val)
{
    _tuDev->setNumIncReg(val);
}

int TUHWMgr::getAlphaInc()
{
    return(_tuDev->getAlphaIncReg());
}

void TUHWMgr::setAlphaInc(int val)
{
    _tuDev->setAlphaIncReg(val);
}

int TUHWMgr::getBetaInc()
{
    return(_tuDev->getBetaIncReg());
}

void TUHWMgr::setBetaInc(int val)
{
    _tuDev->setBetaIncReg(val);
}

int TUHWMgr::getCycleResetTime()
{
    return(_tuDev->getCycleResetTimeReg());
}

void TUHWMgr::setCycleResetTime(int val)
{
    // Converting usec to 100MHz clk count
    _tuDev->setCycleResetTimeReg(val*100);
}

int TUHWMgr::getNumCycle()
{
    return(_tuDev->getNumCycleReg());
}

void TUHWMgr::setNumCycle(int val)
{
    _tuDev->setNumCycleReg(val);
}

DUTUStatusType TUHWMgr::readTUStatus()
{
    _tuDev->setARMInitStatusReg(_armInitReady);

    int status = _tuDev->getBrdStatusReg();

    _logger.logDebug("TU status 0x%x", status);
    if (_verbose)
    {
        printf("TU status 0x%x\n", status);
    }

    if (USE_STATUS_EMULATOR == 1)
    {
        status = emTUStatusReg;

        _logger.logDebug("USE_STATUS_EMULATOR: TU Status for 0x%x module", status);
        if (_verbose)
        {
            printf("USE_STATUS_EMULATOR: TU Status for 0x%x module\n", status);
        }
    }

    DUTUStatusType tuStatus;
    HealthState fwOverallStatus = NO_GO;
    tuStatus.overallStatus = ROLLED_UP_GREEN;

    fwOverallStatus = (HealthState)(DeviceUtilities::readMask(TU_BIT_RESULT_MASK, status));
    if (fwOverallStatus == NO_GO)
    {
        tuStatus.overallStatus = ROLLED_UP_RED;
    }

    tuStatus.readyStatus = (HealthState)(DeviceUtilities::readMask(TU_READY_STATUS_MASK, status));
    if ((tuStatus.readyStatus == NO_GO) && (tuStatus.overallStatus == ROLLED_UP_GREEN))
    {
        tuStatus.overallStatus = ROLLED_UP_YELLOW;
    }

    tuStatus.highTempAlarm = (HealthState)(~(DeviceUtilities::readMask(TU_HIGH_TEMP_ALARM_MASK, status)) & 0x1);
    if ((tuStatus.highTempAlarm == NO_GO) && (tuStatus.overallStatus == ROLLED_UP_GREEN))
    {
        tuStatus.overallStatus = ROLLED_UP_YELLOW;
    }

    tuStatus.overTempAlarm = (HealthState)(~(DeviceUtilities::readMask(TU_OVER_TEMP_ALARM_MASK, status)) & 0x1);
    if (tuStatus.overTempAlarm == NO_GO)
    {
        tuStatus.overallStatus = ROLLED_UP_RED;
    }

    tuStatus.vccintAlarm = (HealthState)(~(DeviceUtilities::readMask(TU_VCC_INT_ALARM_MASK, status)) & 0x1);
    tuStatus.vccauxAlarm = (HealthState)(~(DeviceUtilities::readMask(TU_VCC_AUX_ALARM_MASK, status)) & 0x1);
    tuStatus.vbramAlarm = (HealthState)(~(DeviceUtilities::readMask(TU_VBRAM_ALARM_MASK, status)) & 0x1);

    tuStatus.dieTemp = getFPGADieTemp();

    return (tuStatus);
}

int TUHWMgr::getFPGADieTemp()
{
    int adcCounts = _tuDev->getFPGADieTempReg();
    float temp = (0.007771515 * float(adcCounts)) - 280.2308787;
    return(int(temp + 0.5));
}

void TUHWMgr::processTUEmulatorStatus(int statusReg)
{
    if (_verbose)
    {
        printf("From SWCR Status Emulator: setting TU status to 0x%x\n", statusReg);
    }
    _logger.logDebug("From SWCR Status Emulator: setting TU status to 0x%x", statusReg);

    emTUStatusReg = statusReg;
}

