#include <unistd.h>     // for sleep()
#include <cmath>
#include "DUHWMgr.h"
#include "DeviceUtilities.h"
#include "ConfigDataManager.h"

DUHWMgr::DUHWMgr() :
_logger(Logger::getInstance()),
_iomHWMgr(IOMHWMgr::getInstance()),
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

    /* Config parameters */
    int FORCE_TEST_MODE;
    int DCU_SPI_DELAY1;
    int DCU_SPI_DELAY2;
    int DCU_SPI_DELAY3;
    int DCU_SPI_DELAY4;
    int DCU_SCLK_READBACK_DELAY;
    int SCAN_LIMIT_CENTER_FREQ_SEL;

    _logger.logInfo("DUHWMgr Initializing");

    // Get config parameters
    ConfigDataManager &configs = ConfigDataManager::getInstance();
    rc = rc || configs.get("FORCE_TEST_MODE", FORCE_TEST_MODE);
    rc = rc || configs.get("DCU_SPI_DELAY1", DCU_SPI_DELAY1);
    rc = rc || configs.get("DCU_SPI_DELAY2", DCU_SPI_DELAY2);
    rc = rc || configs.get("DCU_SPI_DELAY3", DCU_SPI_DELAY3);
    rc = rc || configs.get("DCU_SPI_DELAY4", DCU_SPI_DELAY4);
    rc = rc || configs.get("DCU_SCLK_READBACK_DELAY", DCU_SCLK_READBACK_DELAY);
    rc = rc || configs.get("SCAN_LIMIT_CENTER_FREQ_SEL", SCAN_LIMIT_CENTER_FREQ_SEL);

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
    _brdCtrVal = DeviceUtilities::updateReg(DU_UNIT_TYPE_MASK, _brdCtrVal, _rfccType);

    // Set Force Test mode
    // By default mode will be in Offline when Force Test is set
    if (FORCE_TEST_MODE == TEST)
    {
        _brdCtrVal = DeviceUtilities::updateReg(DU_FORCE_TEST_MODE_MASK, _brdCtrVal, TEST);

        // Set default config
        _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_SYSTEM_CONFIG_MASK, _brdCtrVal, SWCR);
    }
    else
    {
        // Get config, mode and OLTE from HW (system status reg)
        getSysConfigStatus();
    }

    // Set Scan Limit calculation center freq
    printf("Setting Scan Limit center freq to %d\n", SCAN_LIMIT_CENTER_FREQ_SEL);
    _logger.logDebug("Setting Scan Limit center freq to %d", SCAN_LIMIT_CENTER_FREQ_SEL);
    if (SCAN_LIMIT_CENTER_FREQ_SEL < 0 && SCAN_LIMIT_CENTER_FREQ_SEL > 3)
    {
        // Default to 0
        SCAN_LIMIT_CENTER_FREQ_SEL = 0;
        printf("ERROR: Invalid Scan Limit center freq, default to 442MHz\n");
        _logger.logDebug("ERROR: Invalid Scan Limit center freq, default to 442MHz");
    }
    _brdCtrVal = DeviceUtilities::updateReg(DU_SCAN_LIMIT_CENTER_FREQ_MASK, _brdCtrVal, SCAN_LIMIT_CENTER_FREQ_SEL);

    _duDev->setBrdCtrlReg(_brdCtrVal);

    // Initialize DCU group status
    for (int dcu = 0; dcu < NUM_DCU; dcu++)
    {
        _dcuStatus[ALPHA][dcu].group = ALPHA;
        _dcuStatus[ALPHA][dcu].loc = 0;

        _dcuStatus[BETA][dcu].group = BETA;
        _dcuStatus[BETA][dcu].loc = 0;
    }

    printf("getFirmwareVersionReg = 0x%x\n", _duDev->getFWVerReg());
    printf("getBoardStatusReg = 0x%x\n", _duDev->getBrdStatusReg());
    printf("getBoardControlReg = 0x%x\n", _duDev->getBrdCtrlReg());
    printf("Setting SPI_DELAY1 = %d, SPI_DELAY2 = %d, SPI_DELAY3 = %d, SPI_DELAY3 = %d, SCLK_DELAY to %d\n", 
           DCU_SPI_DELAY1, DCU_SPI_DELAY2, DCU_SPI_DELAY3, DCU_SPI_DELAY4, DCU_SCLK_READBACK_DELAY);

    _logger.logDebug("FW Verison = 0x%x, board status = 0x%x, board control = 0x%x",
                     _duDev->getFWVerReg(), _duDev->getBrdStatusReg(), _duDev->getBrdCtrlReg());
    _logger.logDebug("Setting SPI_DELAY1 = %d, SPI_DELAY2 = %d, SPI_DELAY3 = %d, SPI_DELAY3 = %d, SCLK_DELAY to %d", 
           DCU_SPI_DELAY1, DCU_SPI_DELAY2, DCU_SPI_DELAY3, DCU_SPI_DELAY4, DCU_SCLK_READBACK_DELAY);


    // Set DCU_SPI_DELAY_COMP
    _duDev->setDCUSPIDelay1Reg(DCU_SPI_DELAY1);
    _duDev->setDCUSPIDelay2Reg(DCU_SPI_DELAY2);
    _duDev->setDCUSPIDelay3Reg(DCU_SPI_DELAY3);
    _duDev->setDCUSPIDelay4Reg(DCU_SPI_DELAY4);
    //setReg(0x28, 1);

    // Set DCU_SCLK_READBACK_DELAY
    for (int i = 0; i < NUM_DCU-1; i++)
    {
        _duDev->setDCUSCLKReg(i, DCU_SCLK_READBACK_DELAY);
    }

    // Fake FW status for Beta DCUs only
    // TO BE REMOVED
    if (MODULE_TYPE == BETA)
    {
        for (int reg = 0; reg < NUM_DCU; reg++)
        {
            fakeDCUFWStatus[_rfccType][reg] = 0xffffffff; 
        }
        fakeDCUFWStatus[_rfccType][40] = 0x3e021b;  // Reg = 40, DCU = 2
        fakeDCUFWStatus[_rfccType][48] = 0x3e661b;  // Reg = 48, DCU = 102
        fakeDCUFWStatus[_rfccType][58] = 0x3e451b;  // Reg = 58, DCU = 69
        fakeDCUFWStatus[_rfccType][66] = 0x3e501b;  // Reg = 66, DCU = 80
        fakeDCUFWStatus[_rfccType][76] = 0x3e511b;  // Reg = 76, DCU = 81
    }

    // Perform these steps to initialize the DCUs and get theirs status at Init without processing an steering action
    // Set Force Test
    // Set Digital 
    // Set DCU cmd to boresight
    // Set Steering Word valid bit to invalid
    // Toggle SW trigger
    // Read DCU status
    // Reset Force Test
    // Set Analog

    _brdCtrVal = DeviceUtilities::updateReg(DU_FORCE_TEST_MODE_MASK, _brdCtrVal, TEST);
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_STEERING_WORD_SRC_MASK, _brdCtrVal, TestSourceDU);
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_DCU_CMD_MASK, _brdCtrVal, DCU_CMD_BORESIGHT);
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_STEERING_WORD_VALID_MASK, _brdCtrVal, STEERING_WORD_INVALID);
    _duDev->setBrdCtrlReg(_brdCtrVal);
    toggleSWTrigger();

    // Init DCUs status
    readDCUStatus();
    for (int i = 1; i < NUM_DCU; i++)
    {
        printf("DCU %d Status = 0x%x, loc = %d, status = %d\n", i, _dcuStatus[_rfccType][i].fwStatusReg, 
               _dcuStatus[_rfccType][i].loc,
               _dcuStatus[_rfccType][i].overallStatus);
        _logger.logDebug("DCU %d Status = 0x%x, loc = %d, status = %d\n", i, 
                         _dcuStatus[_rfccType][i].fwStatusReg,
                         _dcuStatus[_rfccType][i].loc,
                         _dcuStatus[_rfccType][i].overallStatus);
    }

    if (FORCE_TEST_MODE == TEST)
    {
        _brdCtrVal = DeviceUtilities::updateReg(DU_FORCE_TEST_MODE_MASK, _brdCtrVal, TEST);
    }
    else
    {
        _brdCtrVal = DeviceUtilities::updateReg(DU_FORCE_TEST_MODE_MASK, _brdCtrVal, NORMAL);
    }

    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_STEERING_WORD_SRC_MASK, _brdCtrVal, TestSourceTU);
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_DCU_CMD_MASK, _brdCtrVal, DCU_CMD_NONE);
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_STEERING_WORD_VALID_MASK, _brdCtrVal, STEERING_WORD_VALID);

    _duDev->setBrdCtrlReg(_brdCtrVal);

    // Ends initialize DCUs

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

    getRegs(0x0, 0x30);
    printf("getBoardControlReg = 0x%x\n", _duDev->getBrdCtrlReg());

    // Initialize IO Module HW Manager for DUA
    if (MODULE_TYPE == DU_ALPHA)
    {
        _iomHWMgr.initialize();
    }

    _logger.logDebug("Successfully initialize DUMHWMgr");

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

void DUHWMgr::setSysConfig(SWC_CONFIG config)
{
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_SYSTEM_CONFIG_MASK, _brdCtrVal, config);
    _duDev->setBrdCtrlReg(_brdCtrVal);
}

void DUHWMgr::setTestSrcInTestMode(TestSource testSrc)
{
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_STEERING_WORD_SRC_MASK, _brdCtrVal, testSrc);
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

    _sysConfig = (SWC_CONFIG)(DeviceUtilities::readMask(DU_SYSTEM_CONFIG_MASK, _sysConfigReg));

    // Set sys config in board control reg to match sys config in system status reg
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_SYSTEM_CONFIG_MASK, _brdCtrVal, _sysConfig);

    if (!USE_STATUS_EMULATOR)
    {
        // Translate system config reg
        _mode = (SWC_MODE)(DeviceUtilities::readMask(DU_MODE_MASK, _sysConfigReg));
        _testEnabled = DeviceUtilities::readMask(DU_OFFLINE_TEST_ENABLED_MASK, _sysConfigReg);
    }

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
    _logger.logDebug("SetSwcStatusToTwgs %d: 0x%x", setCounter, _statusToTwgs);
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
    printf("****SET DCU STATUS TO TWGS: dcu %d, health = %d\n", dcuNum, health);
    setDCUGroupStatusBit(group);
    setDCUHealthStatusBit(health);
    setDCUNumberStatusBit(dcuNum);

    setSwcStatusToTwgs();
}

void DUHWMgr::readSWCStatus(SWC_STATUS_DATA_TYPE dataType)
{
    getSysConfigStatus();

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
        computeDCURolledUpStatus();
        setDataTypeBit(DATA_TYPE_CUSTOM_STATUS);
        setAlphaOverallStatusBit(_alphaDUStatus.overallStatus);
        setBetaOverallStatusBit(_betaDUStatus.overallStatus);
        setAlphaDCURolledUpStatusBit(_alphaDCURolledUpStatus);
        setBetaDCURolledUpStatusBit(_betaDCURolledUpStatus);
    }
    else    // dataType == DATA_TYPE_IO_MODULE_STATUS
    {
        getIOModuleStatus();
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
        status.alphaDCU[dcu] = _dcuStatus[ALPHA][dcu].overallStatus;
    }

    for (int dcu = 0; dcu < NUM_DCU; dcu++)
    {
        status.betaDCU[dcu] = _dcuStatus[BETA][dcu].overallStatus;
    }
   
    return status;
}

void DUHWMgr::readDCUStatus()
{
    DCUStatus status;

    // Set all DCU location occupied to false
    for (int loc = 0; loc < NUM_DCU; loc++)
    {
        _dcuLocOccupied[loc] = false;
    }

    // Clear the currentDCUList
    _currentDCUList.clear();

    for (int reg = 0; reg < NUM_DCU-1; reg++)
    {
        // Read DCU status registers
        status = readDCUFWStatus(reg);

//      printf("****Reg = %d: last read = %d, now = %d\n", reg, _dcuStatus[_rfccType][reg].loc, status.loc);

        if ((status.loc > 0) && (status.loc < 153))
        {
            processDCUStatus(status);
        }
        else    // Bad DCU location
        {
//          printf("****BAD Reg = %d: last read = %d, now = %d\n", reg, _dcuStatus[_rfccType][reg].loc, status.loc);

            // Check to see if there was a good location for this DCU from the last read
//          if (_dcuStatus[_rfccType][reg].loc > 0)
//          {
//              _dcuStatus[_rfccType][reg].overallStatus = NO_GO;
//
//              // Add to DCU send queue
//              addDCUStatusToDeque(_dcuStatus[_rfccType][reg]);

                // Now set location number for this DCU to 0 to be prepared for when it is good again
//              _dcuStatus[_rfccType][reg].loc = 0;
//          }
        }
//      printf("Reg = %d, loc = %d, fw = 0x%x\n", reg, _dcuStatus[_rfccType][status.loc].loc, _dcuStatus[_rfccType][status.loc].fwStatusReg);
    }

    // Compare to two DCU lists to find out what is missing from the master list
    lookForMissingDCUAfterInit();

    return;
}

// reg is 0 to 151
// dcuNum is from 1 to 152 - These are the actual DCU number
DCUStatus DUHWMgr::readDCUFWStatus(int reg)
{
    DCUStatus status;
    status.loc = -1;

    int fwStatus = _duDev->getDCUStatusReg(reg);

    // For Beta DCUs, use fake status for now
    // Fake status can be updated using the Python Status Emulator
    if (MODULE_TYPE == BETA)
    {
        fwStatus = fakeDCUFWStatus[_rfccType][reg]; 
    }

    status.group = _rfccType;
    status.fwStatusReg = fwStatus;

    // Get location number from FW
    status.locStatus = (HealthState)(DeviceUtilities::readMask(DCU_BIT_LOC_VALID_STATUS_MASK, fwStatus));
    status.loc = DeviceUtilities::readMask(DCU_LOCATION_STATUS_MASK, fwStatus);

    if ((status.loc > 0) && (status.loc < 153))
    {
        status.bypassStatus =
            DeviceUtilities::readMask(DCU_BYPASS_STATUS_MASK, fwStatus);
        status.modeStatus =
            DeviceUtilities::readMask(DCU_MODE_STATUS_MASK, fwStatus);
        status.overallStatus =
            (HealthState)(DeviceUtilities::readMask(DCU_BIT_OVERALL_STATUS_MASK, fwStatus));
        status.clockStatus =
            (HealthState)(DeviceUtilities::readMask(DCU_BIT_CLK_STATUS_MASK, fwStatus));
        status.spiCommStatus =
            (HealthState)(DeviceUtilities::readMask(DCU_BIT_SPI_STATUS_MASK, fwStatus));
        status.steeringWordCompare =
            (HealthState)(DeviceUtilities::readMask(DCU_BIT_COMPARE_STATUS_MASK, fwStatus));
        status.dcuFWMajorRev =
            DeviceUtilities::readMask(DCU_FW_MAJOR_REV_MASK, fwStatus);
        status.dcuFWMinorRev =
            DeviceUtilities::readMask(DCU_FW_MINOR_REV_MASK, fwStatus);
//      status.dcuType =
//          (RFCC_CH)(DeviceUtilities::readMask(DCU_TYPE_STATUS_MASK, fwStatus));
        status.crcStatus =
            (HealthState)DeviceUtilities::readMask(DCU_CRC_STATUS_MASK, fwStatus);   
    }
    else
    {
//      printf("ERROR: invalid loc %d for reg %d with fw value 0x%x\n", status.loc, reg, fwStatus);
    }

    return status;
}

void DUHWMgr::processDCUStatus(DCUStatus status)
{
    int rfccType = status.group;
    int loc = status.loc;

    printf("In processDCUStatus loc = %d, fwStatusReg = 0x%x\n", loc, status.fwStatusReg);
    _logger.logDebug("In processDCUStatus loc = %d, fwStatusReg = 0x%x", loc, status.fwStatusReg);

    // Add DCU to DCU Send Queue
    if ((loc > 0) && (loc < 153))
    {
        // Check for dcuNum = 0 in the SW array.  This indicates data from initialization
        if (_dcuStatus[rfccType][loc].loc == 0)
        {
            _dcuLocOccupied[loc] = true;

            // Update status
            _dcuStatus[rfccType][loc] = status;
            _dcuStatus[rfccType][loc].locStatus = GO;

            // Add DCU to both lists
            _masterDCUList.insert(_masterDCUList.begin(), loc);
            _currentDCUList.insert(_currentDCUList.begin(), loc);

            // Add to DCU send queue
            addDCUStatusToDeque(_dcuStatus[rfccType][loc]);
        }
        else    // Not data from initialization
        {
            _currentDCUList.insert(_currentDCUList.begin(), loc);

            // Check if this location has been occupied
            if (_dcuLocOccupied[loc] == true)
            {
                // Update overall status
                _dcuStatus[rfccType][loc].overallStatus = NO_GO;

                // Add to DCU send queue
                _dcuStatus[rfccType][loc].locStatus = NO_GO;
                addDCUStatusToDeque(_dcuStatus[rfccType][loc]);
                
                printf("DUPLICATE rfccType = %d, loc = %d\n", rfccType, loc);
                _logger.logDebug("DUPLICATE rfccType = %d, loc = %d\n", rfccType, loc);
            }
            else
            {
                _dcuLocOccupied[loc] = true;

                // Determine if there are changes in the data to add to send queue
                // Checking if overallStatus has been changed 
                // TO DO:  Anything else from the DCU status that we need to check???
                if (_dcuStatus[rfccType][loc].overallStatus != status.overallStatus)
                {
                    // Overal Status has been changed

                    // Update SW status
                    _dcuStatus[rfccType][loc] = status;

                    // Add to DCU send queue
                    addDCUStatusToDeque(_dcuStatus[rfccType][loc]);
                }
                else
                {
                    // printf("No status changes for rfccType = %d, dcuNum = %d\n", rfccType, dcuNum);
                }
            }
        }
    }
}

void DUHWMgr::processBetaDCUStatus(DCUStatus status)
{
    if (status.group < NUM_RFCC_CH && status.loc < NUM_DCU)
    {
        // Update SW Beta DCU status
        _dcuStatus[status.group][status.loc] = status;

        // Add to DCU send queue
        addDCUStatusToDeque(status);
    }
}

DCUStatus DUHWMgr::getDCUStatusFromSW(RFCC_CH type, int dcuNum)
{
    return (_dcuStatus[type][dcuNum]);
}

DCUStatus DUHWMgr::getDCUStatusFromDeque()
{
    // Get the front element
    DCUStatusWithID status = _dcuSendDeque.front();

    // Remove the front element
    _dcuSendDeque.pop_front();

    // Only return the status
    return (status.dcuStatus);
}

int DUHWMgr::getDCUStatusDequeSize()
{
    return (_dcuSendDeque.size());
}

void DUHWMgr::addDCUStatusToDeque(DCUStatus status)
{
    // Check if there already is an existing DCU ID in the deque
    // and remove before 
    DCUStatusWithID dcuStatusWithID;
    dcuStatusWithID.dcuID = status.loc + (1000 * status.group);
    dcuStatusWithID.dcuStatus = status;

    // Check and remove 
    checkForExistingDCUStatusInDeque(_dcuSendDeque, dcuStatusWithID.dcuID);
    _dcuSendDeque.push_back(dcuStatusWithID);

    printf("Adding DCU to send queue group = %d, loc = %d, status = %d, locStatus = %d\n", 
           status.group, status.loc, status.overallStatus, status.locStatus);
    _logger.logDebug("Adding DCU to send queue group = %d, loc = %d, status = %d, locStatus = %d", 
                     status.group, status.loc, status.overallStatus, status.locStatus);
}

void DUHWMgr::checkForExistingDCUStatusInDeque(std::deque<DCUStatusWithID>& dq_, const int& dcuID_) 
{
    // std::remove_if moves all elements that satisfy the predicate to the end
    // and returns an iterator to the new logical end of the deque.
    auto new_end = std::remove_if(dq_.begin(), dq_.end(), [dcuID_](const DCUStatusWithID& dcu){
        return dcu.dcuID == dcuID_;
    });

    // Erase the range of elements from the new logical end to the physical end.
    dq_.erase(new_end, dq_.end());
}

void DUHWMgr::lookForMissingDCUAfterInit() 
{
    std::vector<int> difference;

    // 1. Sort both vectors
    std::sort(_masterDCUList.begin(), _masterDCUList.end());
    std::sort(_currentDCUList.begin(), _currentDCUList.end());

    // 2. Find the set difference (elements in _masterDCUList but not in _currentDCUList)
    std::set_difference(
        _masterDCUList.begin(), _masterDCUList.end(),
        _currentDCUList.begin(), _currentDCUList.end(),
        std::back_inserter(difference) // Output to the 'difference' vector
    );

    // 3. Send the difference
    for (int loc : difference) 
    {
        printf("Diff between init and now for loc %d\n", loc);
        _dcuStatus[_rfccType][loc].overallStatus = NO_GO;
        _dcuStatus[_rfccType][loc].locStatus = NO_GO;

        // Add to DCU send queue
        addDCUStatusToDeque(_dcuStatus[_rfccType][loc]);
    }
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
//  // TO BE REMOVED
//  if (USE_STATUS_EMULATOR)
//  {
//      status.overallStatus = _alphaDUStatus.overallStatus;
//  }

    _alphaDUStatus = status;
}

void DUHWMgr::processDUBStatus(DUTUStatusType status)
{
//  // TO BE REMOVED
//  if (USE_STATUS_EMULATOR)
//  {
//      status.overallStatus = _betaDUStatus.overallStatus;
//  }

    _betaDUStatus = status;
}

void DUHWMgr::processTUStatus(DUTUStatusType status)
{
    _tuStatus = status;
}

void DUHWMgr::getIOModuleStatus()
{
    IOMStatusDataType iomStatus = _iomHWMgr.readStatus();
    _atbStatus = (HealthState)iomStatus.atbIOMStatus;
    _pwr12VStatus = (HealthState)iomStatus.ps12IOMStatus;
    _pwr24VStatus = (HealthState)iomStatus.ps24IOMStatus;
    _tempStatus = (HealthState)iomStatus.tempIOMStatus;

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
    return(DeviceUtilities::readMask(DU_SPI_HEALTH_LAST_CMD_MASK, getSysConfigStatus()));
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
    if (dcuGroup_ < NUM_RFCC_CH && dcuNum_ < NUM_DCU)  
    {
        _dcuGroup = dcuGroup_;
        _dcuNum = dcuNum_;
        printf("From Emulator: setting rfcc %d dcu %d to %d\n", dcuGroup_, dcuNum_, dcuStatus_);
        fakeDCUFWStatus[_dcuGroup][_dcuNum] = (HealthState)(DeviceUtilities::updateReg(DCU_BIT_OVERALL_STATUS_MASK, 0xb86401, dcuStatus_));
    }
}

void DUHWMgr::processDCUEmulatorStatus(RFCC_CH dcuGroup, int dcuNum, int dcuFWStatus)
{
    printf("From DCU Emulator: setting rfcc %d dcu %d fw status 0x%x \n", dcuGroup, dcuNum, dcuFWStatus);
    fakeDCUFWStatus[dcuGroup][dcuNum] = dcuFWStatus;
}
