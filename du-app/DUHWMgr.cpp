#include <unistd.h>     // for sleep()
#include <cmath>
#include "DUHWMgr.h"
#include "DeviceUtilities.h"
#include "ConfigDataManager.h"

DUHWMgr::DUHWMgr() :
_logger(Logger::getInstance()),
_duDev(NULL),
_brdCtrVal(0),
_armInitReady(0)
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

STATUS DUHWMgr::initialize(int MODULE_TYPE_)
{
    STATUS rc = OK;

    MODULE_TYPE = MODULE_TYPE_;

    _alphaDUStatus.overallStatus = GO;
    _betaDUStatus.overallStatus = GO;
    _tuStatus.overallStatus = GO;
    _alphaDCURolledUpStatus = DCU_ROLLED_UP_GREEN;
    _betaDCURolledUpStatus = DCU_ROLLED_UP_GREEN;
    _tempStatus = GO;
    _pwr12VStatus = GO;
    _pwr24VStatus = GO;
    _atbStatus = GO;

    /* Common config parameters */;
    int FORCE_TEST_MODE;
    int DCU_CABLE_DELAY_COMP;
    int DCU_SCLK_READBACK_DELAY;

    _logger.logInfo("DUHWMgr Initializing");

    // Get config parameters
    ConfigDataManager &configs = ConfigDataManager::getInstance();
    rc = rc || configs.get("FORCE_TEST_MODE", FORCE_TEST_MODE);
    rc = rc || configs.get("DCU_CABLE_DELAY_COMP", DCU_CABLE_DELAY_COMP);
    rc = rc || configs.get("DCU_SCLK_READBACK_DELAY", DCU_SCLK_READBACK_DELAY);

    // For testing.  To be removed
    rc = rc || configs.get("USE_STATUS_EMULATOR", USE_STATUS_EMULATOR);

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

    // Set Arm Init Reg to indicate the OS is ready
    _armInitReady = DeviceUtilities::updateReg(DU_OS_INIT_STATUS_MASK, _armInitReady, READY);
    _duDev->setARMInitStatusReg(_armInitReady);

    // Reset brdCtl to default
    _brdCtrVal = 0;
    _duDev->setBrdCtrlReg(_brdCtrVal);

    // Set RFCC Type
    _rfccType = ALPHA;
    if (MODULE_TYPE == BETA)
    {
        _rfccType = BETA;
    }

    // Set Unit Type in FW
    setUnitType(_rfccType);

    // Initialize DCU group status
    for (int dcu = 0; dcu < NUM_DCU; dcu++)
    {
        _dcuStatus[ALPHA][dcu].group = ALPHA;
        _dcuStatus[BETA][dcu].group = BETA;
    }


    printf("getFirmwareVersionReg = 0x%x\n", _duDev->getFWVerReg());
    printf("getBoardStatusReg = 0x%x\n", _duDev->getBrdStatusReg());
    printf("getBoardControlReg = 0x%x\n", _duDev->getBrdCtrlReg());
  
    // Set Force Test mode
    if (FORCE_TEST_MODE == TEST)
    {
        _brdCtrVal = DeviceUtilities::updateReg(DU_FORCE_TEST_MODE_MASK, _brdCtrVal, TEST);

        // Set default config, mode, test enable
        // TO DO:  Remove when the ATB is providing these param
        _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_SYSTEM_CONFIG_MASK, _brdCtrVal, SWCR);
    }

    _duDev->setBrdCtrlReg(_brdCtrVal);

    printf("Setting DCU_CABLE_DELAY_COMP = %d, DCU_SCLK_READBACK_DELAY to %d\n", 
           DCU_CABLE_DELAY_COMP, DCU_SCLK_READBACK_DELAY);

    // Set DCU_CABLE_DELAY_COMP
    _duDev->setCableDelayCompReg(DCU_CABLE_DELAY_COMP);
    //setReg(0x28, 1);

    // Set DCU_SCLK_READBACK_DELAY
    for (int i = 0; i < NUM_DCU-1; i++)
    {
        _duDev->setDCUSCLKReg(i, DCU_SCLK_READBACK_DELAY);
    }

    // Fake FW status
    // TO BE REMOVED
    if (USE_STATUS_EMULATOR)
    {
        for (int i = 0; i < NUM_DCU; i++)
        {
//          fakeDCUFWStatus[_rfccType][i] = 0xb86401;
            fakeDCUFWStatus[_rfccType][i] = 0x3c6415; 
            // Alternate between Red and Green
            fakeDCUFWStatus[_rfccType][i] = DeviceUtilities::updateReg(DCU_BIT_OVERALL_STATUS_MASK, fakeDCUFWStatus[_rfccType][i], i%2);
        }
    }

    // Init DCUs status
    readDCUStatus();
    printf("DCU 100 Status = 0x%x, loc = %d\n", _dcuStatus[_rfccType][100].fwStatusReg, _dcuStatus[_rfccType][100].dcuStatus.fwLoc);
    printf("DCU 101 Status = 0x%x, loc = %d\n", _dcuStatus[_rfccType][101].fwStatusReg, _dcuStatus[_rfccType][101].dcuStatus.fwLoc);

    if (MODULE_TYPE == DU_ALPHA)
    {
        // Set default SWC status to TWGS
        _statusToTwgs = 0x0;
        setSwcStatusToTwgs();

        // Init SWC status
        // Send Config Status at start up
        readSWCStatus(DATA_TYPE_CONFIG_STATUS);
    }

    // Set ARM Init Reg to indicate the app is ready
    _armInitReady = DeviceUtilities::updateReg(DU_APP_INIT_STATUS_MASK, _armInitReady, READY);
    _duDev->setARMInitStatusReg(_armInitReady);

    getRegs(0x0, 0x2C);
    printf("getBoardControlReg = 0x%x\n", _duDev->getBrdCtrlReg());

    getRegs(0x380, 0x380);
    getRegs(0x120, 0x120);

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

int DUHWMgr::getBrdStatus()
{
    return (_duDev->getBrdStatusReg());
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
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_SW_TRIGGER_MASK, _brdCtrVal, 1);
    _duDev->setBrdCtrlReg(_brdCtrVal);
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_SW_TRIGGER_MASK, _brdCtrVal, 0);
    _duDev->setBrdCtrlReg(_brdCtrVal);
}

void DUHWMgr::setUnitType(RFCC_CH unitType)
{
    _brdCtrVal = DeviceUtilities::updateReg(DU_UNIT_TYPE_MASK, _brdCtrVal, unitType);
    _duDev->setBrdCtrlReg(_brdCtrVal);
}

void DUHWMgr::setTestSrcInTestMode(TestSource testSrc)
{
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_STEERING_WORD_SRC_MASK, _brdCtrVal, testSrc);
    _duDev->setBrdCtrlReg(_brdCtrVal);
}

void DUHWMgr::setSystemConfigInTestMode(SWC_CONFIG config)
{
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_SYSTEM_CONFIG_MASK, _brdCtrVal, config);
    _duDev->setBrdCtrlReg(_brdCtrVal);
}

void DUHWMgr::sendSteeringWordValidFlagInTestMode(DU_STEERING_WORD_VALID_FLAG_ENUM flag)
{   
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_STEERING_WORD_VALID_MASK, _brdCtrVal, flag);
    _duDev->setBrdCtrlReg(_brdCtrVal);
}

void DUHWMgr::sendDCUCmdInTestMode(DCU_CMD_ENUM cmd)
{   
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_DCU_CMD_MASK, _brdCtrVal, cmd);
    _duDev->setBrdCtrlReg(_brdCtrVal);
}

int DUHWMgr::getSysConfigStatus()
{
    _sysConfigReg = _duDev->getSysConfigStatusReg();
    return _sysConfigReg;
}

int DUHWMgr::getSwcStatusToTwgs()
{
    _statusToTwgs = _duDev->getSwcStatusToTwgsReg();
    return _statusToTwgs;
}

void DUHWMgr::setSwcStatusToTwgs()
{
    static int setCounter = 0;
    setCounter++;
    printf("%d: 0x%x\n", setCounter, _statusToTwgs);
    _duDev->setSwcStatusToTwgsReg(_statusToTwgs);
}

void DUHWMgr::setOverallStatusBit(int val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_OVERALL_STATUS_MASK, _statusToTwgs, val);
}

void DUHWMgr::setDataTypeBit(SWC_STATUS_DATA_TYPE val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_DATA_TYPE_MASK, _statusToTwgs, val);
}

void DUHWMgr::setConfigBit(int val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_CONFIG_STATUS_MASK, _statusToTwgs, val);
}

void DUHWMgr::setModeBit(int val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_MODE_STATUS_MASK, _statusToTwgs, val);
}

void DUHWMgr::setAlphaOverallStatusBit(int val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_ALPHA_OVERALL_STATUS_MASK, _statusToTwgs, val);
}

void DUHWMgr::setBetaOverallStatusBit(int val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_BETA_OVERALL_STATUS_MASK, _statusToTwgs, val);
}

void DUHWMgr::setAlphaDCURolledUpStatusBit(int val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_ALPHA_DCU_ROLLED_UP_STATUS_MASK, _statusToTwgs, val);
}

void DUHWMgr::setBetaDCURolledUpStatusBit(int val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_BETA_DCU_ROLLED_UP_STATUS_MASK, _statusToTwgs, val);
}

void DUHWMgr::setTempStatusBit(int val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_TEMP_STATUS_MASK, _statusToTwgs, val);
}

void DUHWMgr::set12VPwrStatusBit(int val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_12V_PWR_STATUS_MASK, _statusToTwgs, val);
}

void DUHWMgr::set24VPwrStatusBit(int val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_24V_PWR_STATUS_MASK, _statusToTwgs, val);
}

void DUHWMgr::setATBStatusBit(int val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_ATB_STATUS_MASK, _statusToTwgs, val);
}

void DUHWMgr::setDCUGroupStatusBit(RFCC_CH val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_DCU_GROUP_STATUS_MASK, _statusToTwgs, val);
}

void DUHWMgr::setDCUHealthStatusBit(HealthState val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_DCU_HEALTH_STATUS_MASK, _statusToTwgs, val);
}

void DUHWMgr::setDCUNumberStatusBit(int val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_DCU_NUMBER_STATUS_MASK, _statusToTwgs, val);
}

void DUHWMgr::setDCUStatusToTwgs(RFCC_CH group, HealthState health, int dcuNum)
{
    setDCUGroupStatusBit(group);
    setDCUHealthStatusBit(health);
    setDCUNumberStatusBit(dcuNum);

    setSwcStatusToTwgs();
}

void DUHWMgr::readSWCStatus(SWC_STATUS_DATA_TYPE dataType)
{
    getSysConfigStatus();
    _sysConfig = (SWC_CONFIG)(DeviceUtilities::readMask(DU_SYSTEM_CONFIG_MASK, _sysConfigReg));

    if (!USE_STATUS_EMULATOR)
    {
        // Translate system config reg
        _mode = (SWC_MODE)(DeviceUtilities::readMask(DU_MODE_MASK, _sysConfigReg));
        _testEnabled = DeviceUtilities::readMask(DU_OFFLINE_TEST_ENABLED_MASK, _sysConfigReg);
    }

    processIOModuleStatus();
    computeDCURolledUpStatus();

    // Compute swcr overall status
    _swcrOverall = GO;
    if ((_alphaDUStatus.overallStatus == NO_GO) || 
        (_betaDUStatus.overallStatus == NO_GO) || 
        (_tempStatus == NO_GO) || 
        (_pwr12VStatus == NO_GO) ||
        (_pwr24VStatus == NO_GO))
    {
        // Should DCU status be included in the SWCR overall rolled up?
        _swcrOverall = NO_GO;
    }

    // Update status to twgs reg
    getSwcStatusToTwgs();

    setOverallStatusBit(_swcrOverall);

    if (dataType == DATA_TYPE_CONFIG_STATUS)
    {
        setDataTypeBit(DATA_TYPE_CONFIG_STATUS);
        setConfigBit(_sysConfig);
        setModeBit(_mode);
    }
    else if (dataType == DATA_TYPE_CUSTOM_STATUS)
    {
        setDataTypeBit(DATA_TYPE_CUSTOM_STATUS);
        setAlphaOverallStatusBit(_alphaDUStatus.overallStatus);
        setBetaOverallStatusBit(_betaDUStatus.overallStatus);
        setAlphaDCURolledUpStatusBit(_alphaDCURolledUpStatus);
        setBetaDCURolledUpStatusBit(_betaDCURolledUpStatus);
    }
    else    // dataType == DATA_TYPE_IO_MODULE_STATUS
    {
        setDataTypeBit(DATA_TYPE_IO_MODULE_STATUS);
        setTempStatusBit(_tempStatus);
        set12VPwrStatusBit(_pwr12VStatus);
        set24VPwrStatusBit(_pwr24VStatus);
        setATBStatusBit(_atbStatus);
    }

    setSwcStatusToTwgs();
}

SWCOverallStatusDataType DUHWMgr::getSWCStatus()
{
    SWCOverallStatusDataType status;

    status.swcStatus = _swcrOverall;
    status.swcConfig = _sysConfig;
    status.swcMode = _mode;
    status.swcAlphaDUStatus = _alphaDUStatus.overallStatus;
    status.swcBetaDUStatus = _betaDUStatus.overallStatus;
    status.swcAlphaDCURolledUpStatus = _alphaDCURolledUpStatus;
    status.swcBetaDCURolledUpStatus = _betaDCURolledUpStatus;
    status.swcTempStatus = _tempStatus;
    status.swc12VPwrStatus = _pwr12VStatus;
    status.swc24VPwrStatus = _pwr24VStatus;
    status.swcATBStatus = _atbStatus;
    status.testUnitHWStatus = GO;

    for (int dcu = 0; dcu < NUM_DCU; dcu++)
    {
        status.alphaDCU[dcu] = _dcuStatus[ALPHA][dcu].dcuStatus.overallStatus;
    }

    for (int dcu = 0; dcu < NUM_DCU; dcu++)
    {
        status.betaDCU[dcu] = _dcuStatus[BETA][dcu].dcuStatus.overallStatus;
    }
   
    return status;
}

void DUHWMgr::readDCUStatus()
{
    DCUStatusParamsType status;

    for (int reg = 0; reg < NUM_DCU-1; reg++)
    {
//      printf("Reg: %d: ", reg);

        // Read DCU status registers
        status = readDCUFWStatus(reg);
        if (status.number > 0)
        {
            processDCUStatus(status);
        }
    }
//  printf("Queue Size = %d\n", _dcuSendQueue.size());

    return;
}

// reg is 0 to 151
// dcuNum is from 1 to 152 - These are the actual DCU number
DCUStatusParamsType DUHWMgr::readDCUFWStatus(int reg)
{
    DCUStatusParamsType status;
    status.number = -1;

    int fwStatus = _duDev->getDCUStatusReg(reg);

    if (USE_STATUS_EMULATOR)
    {
        // Use fake dcu status except for dcu at index 40, real dcu at index 40 has dcu number 100
        // So set index 99 dcu number 41
        if ((reg != 40) && (reg != 99) && (reg != 44) && (reg != 100) && (MODULE_TYPE == DU_ALPHA))
        {
            fwStatus = fakeDCUFWStatus[_rfccType][reg+1];
            fwStatus = DeviceUtilities::updateReg(DCU_LOCATION_STATUS_MASK, fwStatus, reg+1);
        }
        if ((reg == 99) && (MODULE_TYPE == DU_ALPHA))
        {
            fwStatus = fakeDCUFWStatus[_rfccType][reg+1];
            fwStatus = DeviceUtilities::updateReg(DCU_LOCATION_STATUS_MASK, fwStatus, 41);
        }
        if ((reg == 100) && (MODULE_TYPE == DU_ALPHA))
        {
            fwStatus = fakeDCUFWStatus[_rfccType][reg+1];
            fwStatus = DeviceUtilities::updateReg(DCU_LOCATION_STATUS_MASK, fwStatus, 45);
        }
        if (MODULE_TYPE == DU_BETA)
        {
            fwStatus = fakeDCUFWStatus[_rfccType][reg+1];
            fwStatus = DeviceUtilities::updateReg(DCU_LOCATION_STATUS_MASK, fwStatus, reg+1);
        }
    }

    // Get location number from FW
    HealthState locValid = (HealthState)(DeviceUtilities::readMask(DCU_BIT_LOC_VALID_STATUS_MASK, fwStatus));
    int dcuNum = DeviceUtilities::readMask(DCU_LOCATION_STATUS_MASK, fwStatus);

    if (dcuNum < 1 || dcuNum > 152)
    {
        printf("ERROR: invalid dcuNum of %d for reg %d with fw value 0x%x\n", dcuNum, reg, fwStatus);
    }
    else
    {
//      printf("fwStatus = 0x%x, dcuNum = %d\n", fwStatus, dcuNum);

        // Check for duplicate dcuNum
        // For init only
    //  if (_dcuStatus[_rfccType][dcuNum].number != 0)
    //  {
    //  }

        status.group = _rfccType;
        status.number = dcuNum;
        status.fwStatusReg = fwStatus;
        status.dcuStatus.bypassStatus =
            DeviceUtilities::readMask(DCU_BYPASS_STATUS_MASK, fwStatus);
        status.dcuStatus.modeStatus =
            DeviceUtilities::readMask(DCU_MODE_STATUS_MASK, fwStatus);
        status.dcuStatus.overallStatus =
            (HealthState)(DeviceUtilities::readMask(DCU_BIT_OVERALL_STATUS_MASK, fwStatus));
        status.dcuStatus.clockStatus =
            (HealthState)(DeviceUtilities::readMask(DCU_BIT_CLK_STATUS_MASK, fwStatus));
        status.dcuStatus.locValid = locValid;
        status.dcuStatus.spiCommStatus =
            (HealthState)(DeviceUtilities::readMask(DCU_BIT_SPI_STATUS_MASK, fwStatus));
        status.dcuStatus.steeringWordCompare =
            (HealthState)(DeviceUtilities::readMask(DCU_BIT_COMPARE_STATUS_MASK, fwStatus));
        status.dcuStatus.fwLoc = dcuNum;
        status.dcuStatus.crcStatus =
            DeviceUtilities::readMask(DCU_CRC_STATUS_MASK, fwStatus);
    }

    return status;
}

void DUHWMgr::processDCUStatus(DCUStatusParamsType status)
{
    int rfccType = status.group;
    int dcuNum = status.number;

    // Add DCU to DCU Send Queue
    if ((dcuNum > 0) && (dcuNum < 153))
    {
        // Check for dcuNum = 0 in the SW array.  This indicates data from initialization
        if (_dcuStatus[rfccType][dcuNum].number == 0)
        {
            // Update SW status
            _dcuStatus[rfccType][dcuNum] = status;

            // Add to DCU send queue
            _dcuSendQueue.push(status);
        }
        else    // Not data from initialization
        {
            // Determine if there are changes in the data to add to send queue
            // Checking if overallStatus has been changed 
            // TO DO:  Anything else from the DCU status that we need to check???
            if (_dcuStatus[rfccType][dcuNum].dcuStatus.overallStatus != status.dcuStatus.overallStatus)
            {
                // Overal Status has been changed

                // Update SW status
                _dcuStatus[rfccType][dcuNum] = status;

                // Add to DCU send queue
                _dcuSendQueue.push(status);

                printf("Added to send queue rfccType = %d, dcuNum = %d\n", rfccType, dcuNum);
            }
            else
            {
                // printf("No status changes for rfccType = %d, dcuNum = %d\n", rfccType, dcuNum);
            }
        }
    }
}

DCUStatusParamsType DUHWMgr::getDCUStatusFromSW(RFCC_CH type, int dcuNum)
{
    return (_dcuStatus[type][dcuNum]);
}

DCUStatusParamsType DUHWMgr::getDCUStatusFromQueue()
{
    // Get the front element
    DCUStatusParamsType status = _dcuSendQueue.front();

    // Remove the front element
    _dcuSendQueue.pop();

    return (status);
}

int DUHWMgr::getDCUStatusQueueSize()
{
    return (_dcuSendQueue.size());
}

void DUHWMgr::addDCUStatusToQueue(DCUStatusParamsType status)
{
    _dcuSendQueue.push(status);
}

DUTUStatusType DUHWMgr::readDUStatus()
{
    int status = getBrdStatus();

    _logger.logDebug("DU %d Status for 0x%x module", MODULE_TYPE, status);
    printf("DU %d Status for 0x%x module\n", MODULE_TYPE, status);

    DUTUStatusType duStatus;
    duStatus.overallStatus = (HealthState)(DeviceUtilities::readMask(DU_BIT_RESULT_MASK, status));
    duStatus.readyStatus = (HealthState)(DeviceUtilities::readMask(DU_READY_STATUS_MASK, status));
    duStatus.highTempAlarm = (HealthState)(DeviceUtilities::readMask(DU_HIGH_TEMP_ALARM_MASK, status));
    duStatus.vccintAlarm = (HealthState)(DeviceUtilities::readMask(DU_VCC_INT_ALARM_MASK, status));
    duStatus.vccauxAlarm = (HealthState)(DeviceUtilities::readMask(DU_VCC_AUX_ALARM_MASK, status));
    duStatus.vbramAlarm = (HealthState)(DeviceUtilities::readMask(DU_VBRAM_ALARM_MASK, status));

    return (duStatus);
}

void DUHWMgr::processDUAStatus(DUTUStatusType status)
{
    // TO BE REMOVED
    if (USE_STATUS_EMULATOR)
    {
        status.overallStatus = _alphaDUStatus.overallStatus;
    }

    _alphaDUStatus = status;
}

void DUHWMgr::processDUBStatus(DUTUStatusType status)
{
    // TO BE REMOVED
    if (USE_STATUS_EMULATOR)
    {
        status.overallStatus = _betaDUStatus.overallStatus;
    }

    _betaDUStatus = status;
}

void DUHWMgr::processTUStatus(DUTUStatusType status)
{
    _tuStatus = status;
}

void DUHWMgr::processIOModuleStatus()
{
    if (!USE_STATUS_EMULATOR)
    {
        _tempStatus = GO;
        _pwr12VStatus = GO;
        _pwr24VStatus = GO;
        _atbStatus = GO;
    }
}

void DUHWMgr::computeDCURolledUpStatus()
{
    // TO BE REMOVED
    if (!USE_STATUS_EMULATOR)
    {
        _alphaDCURolledUpStatus = DCU_ROLLED_UP_GREEN;
        _betaDCURolledUpStatus = DCU_ROLLED_UP_GREEN;
    }
}

int DUHWMgr::getOverallSPIStatus()
{
    return(DeviceUtilities::readMask(DU_SPI_HEALTH_MASK, getSysConfigStatus()));
}

DUTUStatusType DUHWMgr::getDUAStatus()
{
    return (_alphaDUStatus);
}

DUTUStatusType DUHWMgr::getDUBStatus()
{
    return (_betaDUStatus);
}

DUTUStatusType DUHWMgr::getTUStatus()
{
    return (_tuStatus);
}

HealthState DUHWMgr::getTempStatus()
{
    return (_tempStatus);
}

HealthState DUHWMgr::get12VPSStatus()
{
    return (_pwr12VStatus);
}

HealthState DUHWMgr::get24VPSStatus()
{
    return (_pwr24VStatus);
}

HealthState DUHWMgr::getSWCOverallStatus()
{
    return (_swcrOverall);
}

SWC_CONFIG DUHWMgr::getSWCConfigStatus()
{
    return (_sysConfig);
}

SWC_MODE DUHWMgr::getSWCModeStatus()
{
    return (_mode);
}

int DUHWMgr::getTestEnabledStatus()
{
    return (_testEnabled);
}

void DUHWMgr::processEmulatorStatus(HealthState swcrOverall_,
                                   SWC_CONFIG sysConfig_,
                                   SWC_MODE mode_,
                                   HealthState alphaDUStatus_,
                                   HealthState betaDUStatus_,
                                   DCURolledUpStatus alphaDCURolledUpStatus_,
                                   DCURolledUpStatus betaDCURolledUpStatus_,
                                   HealthState tempStatus_,
                                   HealthState pwr12VStatus_,
                                   HealthState pwr24VStatus_,
                                   HealthState atbStatus_,
                                   RFCC_CH dcuGroup_,
                                   HealthState dcuStatus_,
                                   int dcuNum_)
{
    _swcrOverall = swcrOverall_;

//  _sysConfig = sysConfig_;
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_SYSTEM_CONFIG_MASK, _brdCtrVal, sysConfig_);
    _duDev->setBrdCtrlReg(_brdCtrVal);

    _mode = mode_;
    printf("From Emulator: overall = %d, config = %d, mode = %d\n", _swcrOverall, _sysConfig, _mode);
    _alphaDUStatus.overallStatus = alphaDUStatus_;
    _betaDUStatus.overallStatus = betaDUStatus_;
    _alphaDCURolledUpStatus = alphaDCURolledUpStatus_;
    _betaDCURolledUpStatus = betaDCURolledUpStatus_;
    _tempStatus = tempStatus_;
    _pwr12VStatus = pwr12VStatus_;
    _pwr24VStatus = pwr24VStatus_;
    _atbStatus = atbStatus_;
    _dcuGroup = dcuGroup_;
    _dcuNum = dcuNum_;
    printf("From Emulator: setting rfcc %d dcu %d to %d\n", dcuGroup_, dcuNum_, dcuStatus_);
    fakeDCUFWStatus[_dcuGroup][_dcuNum] = (HealthState)(DeviceUtilities::updateReg(DCU_BIT_OVERALL_STATUS_MASK, 0xb86401, dcuStatus_));
}
