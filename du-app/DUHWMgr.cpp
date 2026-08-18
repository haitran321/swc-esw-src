#include <unistd.h>     // for sleep()
#include <cmath>
#include "DUHWMgr.h"
#include "DeviceUtilities.h"
#include "ConfigDataManager.h"
#include "SAPDataManager.h"

DUHWMgr::DUHWMgr() :
_logger(Logger::getInstance()),
_iomHWMgr(IOMHWMgr::getInstance()),
_duDev(NULL),
_brdCtrVal(0),
_armInitReady(0),
DCU_CHECK_VERSION_FLAG(0),
DCU_MAJOR_VERSION(0),
DCU_MINOR_VERSION(0),
SAP_RED_OP_THRESHOLD(90),
SAP_YELLOW_OP_THRESHOLD(1)
{
    emTempStatus = GO;
    emPwr12VStatus = GO;
    emPwr24VStatus = GO;
    emAtbStatus = GO;
    emAlphaDCURolledUpStatus = ROLLED_UP_GREEN;
    emBetaDCURolledUpStatus = ROLLED_UP_GREEN;
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
    _duDev->close();
}

STATUS DUHWMgr::initialize(int MODULE_TYPE_, const char *cmdMgrName_)
{
    STATUS rc = OK;

    MODULE_TYPE = MODULE_TYPE_;

    _alphaDUStatus.overallStatus = ROLLED_UP_GREEN;
    _betaDUStatus.overallStatus = ROLLED_UP_GREEN;
    _tuStatus.overallStatus = ROLLED_UP_GREEN;
    _alphaDCURolledUpStatus = ROLLED_UP_GREEN;
    _betaDCURolledUpStatus = ROLLED_UP_GREEN;
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
    int DCU_NUM_TO_REFRESH_GDS = 199;

    _logger.logInfo("DUHWMgr Initializing");

    cmdMgrName = cmdMgrName_;

    // Get config parameters
    ConfigDataManager &configs = ConfigDataManager::getInstance();

    // Verbose parameters
    rc = rc || configs.get("VERBOSE", _verbose);

    if (MODULE_TYPE == ALPHA)
    {
        rc = rc || configs.get("ALPHA_DCU_SPI_DELAY1", DCU_SPI_DELAY1);
        rc = rc || configs.get("ALPHA_DCU_SPI_DELAY2", DCU_SPI_DELAY2);
        rc = rc || configs.get("ALPHA_DCU_SPI_DELAY3", DCU_SPI_DELAY3);
        rc = rc || configs.get("ALPHA_DCU_SPI_DELAY4", DCU_SPI_DELAY4);
        rc = rc || configs.get("ALPHA_DCU_SCLK_READBACK_DELAY", DCU_SCLK_READBACK_DELAY);
    }
    else    // (MODULE_TYPE == BETA)
    {
        rc = rc || configs.get("BETA_DCU_SPI_DELAY1", DCU_SPI_DELAY1);
        rc = rc || configs.get("BETA_DCU_SPI_DELAY2", DCU_SPI_DELAY2);
        rc = rc || configs.get("BETA_DCU_SPI_DELAY3", DCU_SPI_DELAY3);
        rc = rc || configs.get("BETA_DCU_SPI_DELAY4", DCU_SPI_DELAY4);
        rc = rc || configs.get("BETA_DCU_SCLK_READBACK_DELAY", DCU_SCLK_READBACK_DELAY);
    }

    rc = rc || configs.get("FORCE_TEST_MODE", FORCE_TEST_MODE);
    rc = rc || configs.get("SCAN_LIMIT_CENTER_FREQ_SEL", SCAN_LIMIT_CENTER_FREQ_SEL);
    rc = rc || configs.get("DCU_NUM_TO_REFRESH_GDS", DCU_NUM_TO_REFRESH_GDS);

    // DCU version
    rc = rc || configs.get("DCU_CHECK_VERSION_FLAG", DCU_CHECK_VERSION_FLAG);
    rc = rc || configs.get("DCU_MAJOR_VERSION", DCU_MAJOR_VERSION);
    rc = rc || configs.get("DCU_MINOR_VERSION", DCU_MINOR_VERSION);

    rc = rc || configs.get("USE_STATUS_EMULATOR", USE_STATUS_EMULATOR);
    if (USE_STATUS_EMULATOR == 1)
    {
        emDUStatusReg = 0x80000001;
    }
    else
    {
        emDUStatusReg = 0x0;
    }

    SAPDataManager& saps = SAPDataManager::getInstance();
    // For DCU rolled up staus
    rc = rc || saps.get("SAP_RED_OP_THRESHOLD", SAP_RED_OP_THRESHOLD);
    rc = rc || saps.get("SAP_YELLOW_OP_THRESHOLD", SAP_YELLOW_OP_THRESHOLD);

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

        // Set default SWC mode
        _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_SWC_MODE_CMD_MASK, _brdCtrVal, ONLINE);

        // Set default OLTE mode
        _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_OLTE_MODE_CMD_MASK, _brdCtrVal, ONLINE);

        _duDev->setBrdCtrlReg(_brdCtrVal);
    }

    // Get config, mode and OLTE from HW (system status reg)
    getSysConfigStatus();

    // Set Scan Limit calculation center freq
    if (_verbose)
    {
        printf("Setting Scan Limit center freq to %d\n", SCAN_LIMIT_CENTER_FREQ_SEL);
    }
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
        _dcuFWStatusHistoryCount[ALPHA][dcu] = 0;
        for (int historyIdx = 0; historyIdx < 4; historyIdx++)
        {
            _dcuFWStatusHistory[ALPHA][dcu][historyIdx] = 0;
        }

        _dcuStatus[BETA][dcu].group = BETA;
        _dcuStatus[BETA][dcu].loc = 0;
        _dcuFWStatusHistoryCount[BETA][dcu] = 0;
        for (int historyIdx = 0; historyIdx < 4; historyIdx++)
        {
            _dcuFWStatusHistory[BETA][dcu][historyIdx] = 0;
        }
    }

    if (_verbose)
    {
        printf("getFirmwareVersionReg = 0x%x\n", _duDev->getFWVerReg());
        printf("getBoardStatusReg = 0x%x\n", _duDev->getBrdStatusReg());
        printf("getBoardControlReg = 0x%x\n", _duDev->getBrdCtrlReg());
        printf("Setting SPI_DELAY1 = %d, SPI_DELAY2 = %d, SPI_DELAY3 = %d, SPI_DELAY3 = %d, SCLK_DELAY to %d\n", 
               DCU_SPI_DELAY1, DCU_SPI_DELAY2, DCU_SPI_DELAY3, DCU_SPI_DELAY4, DCU_SCLK_READBACK_DELAY);
    }

    _logger.logDebug("MODULE_TYPE = %d, FW Verison = 0x%x, board status = 0x%x, board control = 0x%x",
                     MODULE_TYPE, _duDev->getFWVerReg(), _duDev->getBrdStatusReg(), _duDev->getBrdCtrlReg());
    _logger.logDebug("Setting SPI_DELAY1 = %d, SPI_DELAY2 = %d, SPI_DELAY3 = %d, SPI_DELAY3 = %d, SCLK_DELAY to %d", 
           DCU_SPI_DELAY1, DCU_SPI_DELAY2, DCU_SPI_DELAY3, DCU_SPI_DELAY4, DCU_SCLK_READBACK_DELAY);


    // Set DCU_SPI_DELAY_COMP
    _duDev->setDCUSPIDelay1Reg(DCU_SPI_DELAY1);
    _duDev->setDCUSPIDelay2Reg(DCU_SPI_DELAY2);
    _duDev->setDCUSPIDelay3Reg(DCU_SPI_DELAY3);
    _duDev->setDCUSPIDelay4Reg(DCU_SPI_DELAY4);

    // Set DCU_SCLK_READBACK_DELAY
    for (int i = 0; i < NUM_DCU-1; i++)
    {
        _duDev->setDCUSCLKReg(i, DCU_SCLK_READBACK_DELAY);
    }

    // Fake FW status for Beta DCUs only
    // TO BE REMOVED
    if ((MODULE_TYPE == BETA) && (USE_STATUS_EMULATOR == 1))
    {
        for (int reg = 0; reg < NUM_DCU; reg++)
        {
            emDCUFWStatus[_rfccType][reg] = 0xffffffff; 
        }
        printf("\n************HARDCODING**********\n");
        emDCUFWStatus[_rfccType][40] = 0x3e021f;  // Reg = 40, DCU = 2
        emDCUFWStatus[_rfccType][48] = 0x3e661f;  // Reg = 48, DCU = 102
        emDCUFWStatus[_rfccType][58] = 0x3e451f;  // Reg = 58, DCU = 69
        emDCUFWStatus[_rfccType][66] = 0x3e501f;  // Reg = 66, DCU = 80
        emDCUFWStatus[_rfccType][76] = 0x3e511f;  // Reg = 76, DCU = 81
    }

    // Add DCU_NUM_TO_REFRESH_GDS first to reset the GDS
    resetDCU.group = _rfccType;
    resetDCU.loc = DCU_NUM_TO_REFRESH_GDS;
    if (_verbose)
    {
        printf("Adding DCU_NUM_TO_REFRESH_GDS = %d, status = %d\n",  resetDCU.loc, DCU_NO_GO);
    }
    addDCUStatusToDeque(resetDCU);

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
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_STEERING_WORD_SRC_MASK, _brdCtrVal, Digital);
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_DCU_CMD_MASK, _brdCtrVal, DCU_CMD_BORESIGHT);
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_STEERING_WORD_VALID_MASK, _brdCtrVal, STEERING_WORD_INVALID);
    _duDev->setBrdCtrlReg(_brdCtrVal);
    toggleSWTrigger();

    // Init DCUs status
    readDCUStatus();
    for (int i = 1; i < NUM_DCU; i++)
    {
        if (_verbose)
        {
            printf("DCU %d Status = 0x%x, loc = %d, twgs status = %d, ts status = %d\n", i, 
                             _dcuStatus[_rfccType][i].fwStatusReg,
                             _dcuStatus[_rfccType][i].loc,
                             _dcuStatus[_rfccType][i].overallStatus,
                             _dcuStatus[_rfccType][i].overallStatus & 0x1);
        }
        _logger.logDebug("DCU %d Status = 0x%x, loc = %d, twgs status = %d, ts status = %d", i, 
                         _dcuStatus[_rfccType][i].fwStatusReg,
                         _dcuStatus[_rfccType][i].loc,
                         _dcuStatus[_rfccType][i].overallStatus,
                         _dcuStatus[_rfccType][i].overallStatus & 0x1);
    }

    if (FORCE_TEST_MODE == TEST)
    {
        _brdCtrVal = DeviceUtilities::updateReg(DU_FORCE_TEST_MODE_MASK, _brdCtrVal, TEST);
    }
    else
    {
        _brdCtrVal = DeviceUtilities::updateReg(DU_FORCE_TEST_MODE_MASK, _brdCtrVal, NORMAL);
    }

    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_STEERING_WORD_SRC_MASK, _brdCtrVal, Analog);
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_DCU_CMD_MASK, _brdCtrVal, DCU_CMD_NONE);
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_STEERING_WORD_VALID_MASK, _brdCtrVal, STEERING_WORD_VALID);

    _duDev->setBrdCtrlReg(_brdCtrVal);

    // Ends initialize DCUs

    if (MODULE_TYPE == DU_ALPHA)
    {
        if (_iomHWMgr.initialize() != OK)
        {
            _logger.logError("Failed to initialize IO Module HW Manager");
        }

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

    if (_verbose)
    {
        printf("getBoardControlReg = 0x%x\n", _duDev->getBrdCtrlReg());
    }
    _logger.logDebug("getBoardControlReg = 0x%x", _duDev->getBrdCtrlReg());

    _logger.logDebug("Successfully initialize DUMHWMgr");

    getRegs(0x0, 0x40);

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

void DUHWMgr::setShutdownBit()
{
    printf("DUHWMgr::setShutdownBit\n");
    _brdCtrVal = DeviceUtilities::updateReg(DU_SHUTDOWN_CMD_MASK, _brdCtrVal, 1);
    _duDev->setBrdCtrlReg(_brdCtrVal);
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

    // Translate system config reg
    _sysConfig = (SWC_CONFIG)(DeviceUtilities::readMask(DU_SYSTEM_CONFIG_MASK, _sysConfigReg));
    _swcMode = (SWC_MODE)(DeviceUtilities::readMask(DU_ATB_MODE_MASK, _sysConfigReg));
    _olteMode = (SWC_MODE)(DeviceUtilities::readMask(DU_OLTE_MODE_MASK, _sysConfigReg));

    // Set sys config in board control reg to match sys config in system status reg
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_SYSTEM_CONFIG_MASK, _brdCtrVal, _sysConfig);
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_SWC_MODE_CMD_MASK, _brdCtrVal, _swcMode);
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_OLTE_MODE_CMD_MASK, _brdCtrVal, _olteMode);
    _duDev->setBrdCtrlReg(_brdCtrVal);

    if (_verbose)
    {
        printf("Reg = 0x%x, _sysConfig = %d, _mode = %d, _olteMode = %d\n", _sysConfigReg, _sysConfig, _swcMode, _olteMode);
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
    if (_verbose)
    {
        printf("%d: 0x%x\n", setCounter, _statusToTwgs);
    }
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

void DUHWMgr::setDCUStatusToTwgs(RFCC_CH group, DCUHealthState health, int dcuNum)
{
    if (_verbose)
    {
        printf("****Set DCU status to TWGS: dcu %d, health = %d\n", dcuNum, health);
    }

    _logger.logDebug("****Set DCU status to TWGS: dcu %d, health = %d", dcuNum, health);

    setDCUGroupStatusBit(group);

    // Converting DCU health from DCUHealthState to HealthState
    setDCUHealthStatusBit((HealthState)(health & 0x1));

    setDCUNumberStatusBit(dcuNum);

    setSwcStatusToTwgs();
}

void DUHWMgr::readSWCStatus(SWC_STATUS_DATA_TYPE dataType)
{
    getSysConfigStatus();

    // Compute swcr overall status
    _swcrOverall = GO;
    if ((_sysConfig == CONFIG_ERR) || 
        (_alphaDCURolledUpStatus == ROLLED_UP_RED) ||
        (_betaDCURolledUpStatus == ROLLED_UP_RED) ||
        (_alphaDUStatus.overallStatus == ROLLED_UP_RED) ||
        (_betaDUStatus.overallStatus == ROLLED_UP_RED) ||
        (_tempStatus == NO_GO) || 
//      (_pwr12VStatus == NO_GO) ||
//      (_pwr24VStatus == NO_GO) ||
        (_atbStatus == NO_GO))
    {
        _swcrOverall = NO_GO;
    }

    // Update status to twgs reg
    getSwcStatusToTwgs();

    setOverallStatusBit(_swcrOverall);

    if (dataType == DATA_TYPE_CONFIG_STATUS)
    {
        setDataTypeBit(DATA_TYPE_CONFIG_STATUS);
        setConfigBit(_sysConfig);

        getIOModuleStatus();
        setTempStatusBit(_tempStatus);
        set12VPwrStatusBit(_pwr12VStatus);
        set24VPwrStatusBit(_pwr24VStatus);
        setATBStatusBit(_atbStatus);
    }
    else if (dataType == DATA_TYPE_DCU_ROLLED_UP_STATUS)
    {
        computeDCURolledUpStatus();
        setDataTypeBit(DATA_TYPE_DCU_ROLLED_UP_STATUS);
        setAlphaDCURolledUpStatusBit(_alphaDCURolledUpStatus);
        setBetaDCURolledUpStatusBit(_betaDCURolledUpStatus);
    }
    else    // dataType == DATA_TYPE_DU_STATUS
    {
        setDataTypeBit(DATA_TYPE_DU_STATUS);
        setAlphaOverallStatusBit(_alphaDUStatus.overallStatus);
        setBetaOverallStatusBit(_betaDUStatus.overallStatus);
    }

    setSwcStatusToTwgs();
}

SWCOverallStatusDataType DUHWMgr::getSWCStatus()
{
    SWCOverallStatusDataType status;

    status.swcStatus = _swcrOverall;
    status.swcConfig = _sysConfig;
    status.swcMode = _swcMode;
    status.olteMode = _olteMode;
    status.swcAlphaDUStatus = _alphaDUStatus.overallStatus;
    status.swcBetaDUStatus = _betaDUStatus.overallStatus;
    status.swcAlphaDCURolledUpStatus = _alphaDCURolledUpStatus;
    status.swcBetaDCURolledUpStatus = _betaDCURolledUpStatus;
    status.swcTempStatus = _tempStatus;
    status.swc12VPwrStatus = _pwr12VStatus;
    status.swc24VPwrStatus = _pwr24VStatus;
    status.swcATBStatus = _atbStatus;
    status.testUnitHWStatus = _tuStatus.overallStatus;

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

void DUHWMgr::readDCUStatus(bool sendCurrentDCUList)
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

        // Check DCU location
        if ((status.loc > 0) && (status.loc < 153) && (status.locStatus == GO))
        {
            processDCUStatus(status);
        }
    }

    // Compare to two DCU lists to find out what is missing from the master list
    lookForMissingDCUAfterInit();

    // Send current DCU list.  This is to refresh the GDS DCU status

    if (_verbose)
    {
        printf("%s: sendCurrentDCUList = %d, size = %d\n", cmdMgrName, sendCurrentDCUList, _currentDCUList.size());
    }
    _logger.logDebug("sendCurrentDCUList = %d, size = %d", sendCurrentDCUList, _currentDCUList.size());
    if (sendCurrentDCUList)
    {
        // Add DCU_NUM_TO_REFRESH_GDS first to reset the GDS
        if (_verbose)
        {
            printf("%s: Adding DCU_NUM_TO_REFRESH_GDS = %d, status = %d\n",  cmdMgrName, resetDCU.loc, DCU_NO_GO);
        }
        _logger.logDebug("Adding DCU_NUM_TO_REFRESH_GDS = %d, status = %d", resetDCU.loc, DCU_NO_GO);
        addDCUStatusToDeque(resetDCU);

        for (auto it = _currentDCUList.begin(); it != _currentDCUList.end(); ++it) 
        {
            // Add to DCU send queue
            if (_verbose)
            {
                printf("%s: DCU # = %d, status = %d\n", cmdMgrName, *it, _dcuStatus[_rfccType][*it].overallStatus);
            }
            _logger.logDebug("DCU # = %d, status = %d", *it, _dcuStatus[_rfccType][*it].overallStatus);
            addDCUStatusToDeque(_dcuStatus[_rfccType][*it]);
        }
        // Compare to two DCU lists to find out what is missing from the master list
        lookForMissingDCUAfterInit();
    }

    return;
}

// reg is 0 to 151
// dcuNum is from 1 to 152 - These are the actual DCU number
int DUHWMgr::getMajorityFWStatusFromHistory(int reg, int currentFWStatus)
{
    if ((reg < 0) || (reg >= NUM_DCU))
    {
        return currentFWStatus;
    }

    int samples[5];
    int historyCount = _dcuFWStatusHistoryCount[_rfccType][reg];

    for (int i = 0; i < 4; i++)
    {
        if (i < historyCount)
        {
            samples[i] = _dcuFWStatusHistory[_rfccType][reg][i];
        }
        else
        {
            // During startup, use the current read to fill unused slots.
            samples[i] = currentFWStatus;
        }
    }
    samples[4] = currentFWStatus;

    int selectedFWStatus = samples[4];
    int maxMatchCount = 0;
    for (int i = 0; i < 5; i++)
    {
        int matchCount = 0;
        for (int j = 0; j < 5; j++)
        {
            if (samples[i] == samples[j])
            {
                matchCount++;
            }
        }

        if (matchCount > maxMatchCount)
        {
            maxMatchCount = matchCount;
            selectedFWStatus = samples[i];
        }
        else if ((matchCount == maxMatchCount) && (samples[i] == currentFWStatus))
        {
            // On ties, prefer the most recent FW value read.
            selectedFWStatus = samples[i];
        }
    }

    // Keep the last 4 FW status reads for the next majority vote.
    if (historyCount < 4)
    {
        _dcuFWStatusHistory[_rfccType][reg][historyCount] = currentFWStatus;
        _dcuFWStatusHistoryCount[_rfccType][reg] = historyCount + 1;
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            _dcuFWStatusHistory[_rfccType][reg][i] = _dcuFWStatusHistory[_rfccType][reg][i + 1];
        }
        _dcuFWStatusHistory[_rfccType][reg][3] = currentFWStatus;
    }

    return selectedFWStatus;
}

DCUStatus DUHWMgr::readDCUFWStatus(int reg)
{
    DCUStatus status;
    status.loc = -1;

    int fwStatus = _duDev->getDCUStatusReg(reg);

    // For Beta DCUs, use fake status for now
    // Fake status can be updated using the Python Status Emulator
    if ((MODULE_TYPE == BETA)  && (USE_STATUS_EMULATOR == 1))
    {
        fwStatus = emDCUFWStatus[_rfccType][reg]; 
    }

    fwStatus = getMajorityFWStatusFromHistory(reg, fwStatus);

    status.group = _rfccType;
    status.fwStatusReg = fwStatus;

    // Get location and version number from FW
    status.locStatus = (HealthState)(DeviceUtilities::readMask(DCU_BIT_LOC_VALID_STATUS_MASK, fwStatus));
    status.loc = DeviceUtilities::readMask(DCU_LOCATION_STATUS_MASK, fwStatus);
    status.dcuFWMajorRev = DeviceUtilities::readMask(DCU_FW_MAJOR_REV_MASK, fwStatus);
    status.dcuFWMinorRev = DeviceUtilities::readMask(DCU_FW_MINOR_REV_MASK, fwStatus);

    _logger.logDebug("Data from FW for reg %d: 0x%x, loc = %d", reg, fwStatus, status.loc);

    // Validate FW status before using it
    if (validateDCUFWStatus(status) == OK)
    {
        status.bypassStatus =
            DeviceUtilities::readMask(DCU_BYPASS_STATUS_MASK, fwStatus);
        status.modeStatus =
            DeviceUtilities::readMask(DCU_MODE_STATUS_MASK, fwStatus);
        status.overallStatus =
            (DCUHealthState)(DeviceUtilities::readMask(DCU_BIT_OVERALL_STATUS_MASK, fwStatus) + 0x2);
        status.clockStatus =
            (HealthState)(DeviceUtilities::readMask(DCU_BIT_CLK_STATUS_MASK, fwStatus));
        status.spiCommStatus =
            (HealthState)(DeviceUtilities::readMask(DCU_BIT_SPI_STATUS_MASK, fwStatus));
        status.steeringWordCompare =
            (HealthState)(DeviceUtilities::readMask(DCU_BIT_COMPARE_STATUS_MASK, fwStatus));
        status.crcStatus =
            (HealthState)DeviceUtilities::readMask(DCU_CRC_STATUS_MASK, fwStatus);   
    }
    else
    {
        // Set DCU overall status to NO_GO
        status.overallStatus = (DCUHealthState)(0x2);
//      printf("ERROR: invalid loc %d for reg %d with fw value 0x%x\n", status.loc, reg, fwStatus);
//      _logger.logDebug("ERROR: invalid loc %d for reg %d with fw value 0x%x", status.loc, reg, fwStatus);
    }

    return status;
}

STATUS DUHWMgr::validateDCUFWStatus(DCUStatus status)
{
    STATUS rc = OK;

    if (status.fwStatusReg == 0xffffff)
    {
        rc = ERROR;
    }
    else
    {

        // Check location valid bit
        if (status.locStatus == NO_GO)
        {
            _logger.logDebug("Failed validateDCUFWStatus: DCU location valid bit is set to %d", status.locStatus);
            rc = ERROR;
        }

        // Check location number
        if ((status.loc < 0) || (status.loc > 153))
        {
            _logger.logDebug("Failed validateDCUFWStatus: DCU location number is %d", status.loc);
            rc = ERROR;
        }

        // Check correct RFCC group
        if (status.group != _rfccType)
        {
            _logger.logDebug("Incorrect rfcc group: expected %d, received %d", _rfccType, status.group);
            rc = ERROR;
        }

        // Check FW version
        if (DCU_CHECK_VERSION_FLAG == 1)
        {
            if ((status.dcuFWMajorRev != DCU_MAJOR_VERSION) || (status.dcuFWMinorRev != DCU_MINOR_VERSION))
            {
                _logger.logDebug("Failed validateDCUFWStatus: DCU version major = %d, minor = %d", status.dcuFWMajorRev, status.dcuFWMinorRev);
                rc = ERROR;
            }
        }
    }

    // If failing default status.locStatus to NO GO
    if (rc == ERROR)
    {
        status.locStatus = NO_GO;
    }

    return rc;
}

void DUHWMgr::processDCUStatus(DCUStatus status)
{
    int rfccType = status.group;
    int loc = status.loc;

    if (_verbose)
    {
        printf("Processing DCU loc = %d, fwStatusReg = 0x%x\n", loc, status.fwStatusReg);
    }
    _logger.logDebug("Processing loc = %d, fwStatusReg = 0x%x", loc, status.fwStatusReg);

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

            _logger.logDebug("Adding init status for rfccType = %d, loc = %d, status = %d", 
                             rfccType, loc, _dcuStatus[rfccType][loc].overallStatus);

            // Add DCU to both lists
            _initialDCUList.insert(_initialDCUList.begin(), loc);
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
                _dcuStatus[rfccType][loc].overallStatus = DCU_NO_GO;

                // Add to DCU send queue
                _dcuStatus[rfccType][loc].locStatus = NO_GO;
                addDCUStatusToDeque(_dcuStatus[rfccType][loc]);
                
                if (_verbose)
                {
                    printf("Duplicate location rfccType = %d, loc = %d\n", rfccType, loc);
                }
                _logger.logDebug("Duplicate location rfccType = %d, loc = %d", rfccType, loc);
            }
            else
            {
                _dcuLocOccupied[loc] = true;

                // Determine if there are changes in the data to add to send queue to TWGS
                // Checking if overallStatus has been changed 
                if (_dcuStatus[rfccType][loc].overallStatus != status.overallStatus)
                {
                    // Add to DCU send queue
                    addDCUStatusToDeque(_dcuStatus[rfccType][loc]);
                }

                _logger.logDebug("Status for rfccType = %d, loc = %d, current fw status = 0x%x, new fw status = 0x%x", 
                                 rfccType, loc, _dcuStatus[rfccType][loc].fwStatusReg, status.fwStatusReg);

                // Update SW status
                _dcuStatus[rfccType][loc] = status;
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
    
    if (status.group < NUM_RFCC_CH && status.loc == resetDCU.loc)
    {
        if (_verbose)
        {
            printf("Adding DCU_NUM_TO_REFRESH_GDS = %d, status = %d\n",  status.loc, DCU_NO_GO);
        }

        // Add to DCU send queue
        addDCUStatusToDeque(status);
    }
}

DCUStatus DUHWMgr::getDCUStatusFromSW(RFCC_CH type, int dcuNum)
{
    DCUStatus status = {};

    if ((type < ALPHA) || (type >= NUM_RFCC_CH) || (dcuNum <= 0) || (dcuNum >= NUM_DCU))
    {
        status.group = (type >= ALPHA && type < NUM_RFCC_CH) ? type : ALPHA;
        status.loc = dcuNum;
        status.locStatus = NO_GO;
        status.overallStatus = DCU_NO_GO;
        _logger.logError("ERROR: Invalid DCU status request type=%d dcuNum=%d", type, dcuNum);
        return status;
    }

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

    if (_verbose)
    {
        printf("%s: Adding DCU to send queue: group = %d, loc = %d, status = %d, locStatus = %d\n", 
               cmdMgrName, status.group, status.loc, status.overallStatus, status.locStatus);
    }
    _logger.logDebug("Adding DCU to send queue: group = %d, loc = %d, status = %d, locStatus = %d", 
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
    std::sort(_initialDCUList.begin(), _initialDCUList.end());
    std::sort(_currentDCUList.begin(), _currentDCUList.end());

    // 2. Find the set difference (elements in _initialDCUList but not in _currentDCUList)
    std::set_difference(
        _initialDCUList.begin(), _initialDCUList.end(),
        _currentDCUList.begin(), _currentDCUList.end(),
        std::back_inserter(difference) // Output to the 'difference' vector
    );

    // 3. Send the difference
    for (int loc : difference) 
    {
        if (_verbose)
        {
            printf("Diff between init and now for loc %d\n", loc);
        }
        _logger.logDebug("Diff between init and now for loc %d", loc);
        _dcuStatus[_rfccType][loc].overallStatus = DCU_NO_GO;
        _dcuStatus[_rfccType][loc].locStatus = NO_GO;

        // Add to DCU send queue
        addDCUStatusToDeque(_dcuStatus[_rfccType][loc]);
    }
}

DUTUStatusType DUHWMgr::readDUStatus()
{
    _duDev->setARMInitStatusReg(_armInitReady);

    int status = getBrdStatus();

    _logger.logDebug("DU %d status 0x%x", MODULE_TYPE, status);
    if (_verbose)
    {
        printf("DU %d status 0x%x\n", MODULE_TYPE, status);
    }

    if (USE_STATUS_EMULATOR == 1)
    {
        status = emDUStatusReg;

        _logger.logDebug("USE_STATUS_EMULATOR: DU %d Status for 0x%x module", MODULE_TYPE, status);
        if (_verbose)
        {
            printf("USE_STATUS_EMULATOR: DU %d Status for 0x%x module\n", MODULE_TYPE, status);
        }
    }

    DUTUStatusType duStatus;
    HealthState fwOverallStatus = NO_GO;
    duStatus.overallStatus = ROLLED_UP_GREEN;

    fwOverallStatus = (HealthState)(DeviceUtilities::readMask(DU_BIT_RESULT_MASK, status));
    if (fwOverallStatus == NO_GO)
    {
        duStatus.overallStatus = ROLLED_UP_RED;
    }

    duStatus.readyStatus = (HealthState)(DeviceUtilities::readMask(DU_READY_STATUS_MASK, status));
    if ((duStatus.readyStatus == NO_GO) && (duStatus.overallStatus == ROLLED_UP_GREEN))
    {
        duStatus.overallStatus = ROLLED_UP_YELLOW;
    }

    // Need to invert the alarms status: 0 = No alarm, 1 = alarm
    duStatus.highTempAlarm = (HealthState)(~(DeviceUtilities::readMask(DU_HIGH_TEMP_ALARM_MASK, status)) & 0x1);
    if ((duStatus.highTempAlarm == NO_GO) && (duStatus.overallStatus == ROLLED_UP_GREEN))
    {
        duStatus.overallStatus = ROLLED_UP_YELLOW;
    }

    duStatus.overTempAlarm = (HealthState)(~(DeviceUtilities::readMask(DU_OVER_TEMP_ALARM_MASK, status)) & 0x1);
    if (duStatus.overTempAlarm == NO_GO)
    {
        duStatus.overallStatus = ROLLED_UP_RED;
    }

    duStatus.vccintAlarm = (HealthState)(~(DeviceUtilities::readMask(DU_VCC_INT_ALARM_MASK, status)) & 0x1);
    duStatus.vccauxAlarm = (HealthState)(~(DeviceUtilities::readMask(DU_VCC_AUX_ALARM_MASK, status)) & 0x1);
    duStatus.vbramAlarm = (HealthState)(~(DeviceUtilities::readMask(DU_VBRAM_ALARM_MASK, status)) & 0x1);

    duStatus.dieTemp = getFPGADieTemp();

    return (duStatus);
}

void DUHWMgr::processDUAStatus(DUTUStatusType status)
{
    _alphaDUStatus = status;
}

void DUHWMgr::processDUBStatus(DUTUStatusType status)
{
    _betaDUStatus = status;
}

void DUHWMgr::processTUStatus(DUTUStatusType status)
{
    _tuStatus = status;
}

void DUHWMgr::getIOModuleStatus()
{
    IOMStatusDataType iomStatus = _iomHWMgr.readStatus();

    // ATB Status = NO_GO if both config bits are zeros 
    _atbStatus = (HealthState)(iomStatus.configBit0 | iomStatus.configBit1);

    // Need to invert the power supply since they are wired to send 1 for No Go and 0 for Go
    _pwr12VStatus = (HealthState)!iomStatus.ps12;
    _pwr24VStatus = (HealthState)!iomStatus.ps24;

    _tempStatus = (HealthState)iomStatus.temp;

    if (USE_STATUS_EMULATOR == 1)
    {
        _tempStatus = emTempStatus;
        _pwr12VStatus = emPwr12VStatus;
        _pwr24VStatus = emPwr24VStatus;
        _atbStatus = emAtbStatus;
    }
}

void DUHWMgr::computeDCURolledUpStatus()
{
    // Init to Green 
    _alphaDCURolledUpStatus = ROLLED_UP_GREEN;
    _betaDCURolledUpStatus = ROLLED_UP_GREEN;
    
    // Get the number of failed DCU
    int alphaDCUFailedCnt = 0;
    int betaDCUFailedCnt = 0;
    for (int dcu = 1; dcu < NUM_DCU; dcu++)
    {
        if (_dcuStatus[ALPHA][dcu].overallStatus == DCU_NO_GO)
        {
            alphaDCUFailedCnt++;
        }
        if (_dcuStatus[BETA][dcu].overallStatus == DCU_NO_GO)
        {
            betaDCUFailedCnt++;
        }
    }

    // Alpha
    if (alphaDCUFailedCnt >= SAP_YELLOW_OP_THRESHOLD && alphaDCUFailedCnt < SAP_RED_OP_THRESHOLD)
    {
        _alphaDCURolledUpStatus = ROLLED_UP_YELLOW;
    }
    else if (alphaDCUFailedCnt >= SAP_RED_OP_THRESHOLD)
    {
        _alphaDCURolledUpStatus = ROLLED_UP_RED;
    }

    // Beta
    if (betaDCUFailedCnt >= SAP_YELLOW_OP_THRESHOLD && betaDCUFailedCnt < SAP_RED_OP_THRESHOLD)
    {
        _betaDCURolledUpStatus = ROLLED_UP_YELLOW;
    }
    else if (betaDCUFailedCnt >= SAP_RED_OP_THRESHOLD)
    {
        _betaDCURolledUpStatus = ROLLED_UP_RED;
    }

    _logger.logDebug("Alpha DCUs rolled up status: failed DCUs count = %d, rolled up status = %d\n", alphaDCUFailedCnt, _alphaDCURolledUpStatus);
    _logger.logDebug("Beta DCUs rolled up status: failed DCUs count = %d, rolled up status = %d\n", betaDCUFailedCnt, _betaDCURolledUpStatus);
    

//  if (USE_STATUS_EMULATOR == 1)
//  {
//      _alphaDCURolledUpStatus = emAlphaDCURolledUpStatus;
//      _betaDCURolledUpStatus = emBetaDCURolledUpStatus;
//  }
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
    return (_swcMode);
}

SWC_MODE DUHWMgr::getOLTEModeStatus()
{
    return (_olteMode);
}

int DUHWMgr::getFPGADieTemp()
{
    int adcCounts = _duDev->getFPGADieTempReg();
    float temp = (0.007771515 * float(adcCounts)) - 280.2308787;
    return(int(temp + 0.5));
}

void DUHWMgr::clearDCUData()
{
    _logger.logDebug("Clearing DCU data");
    if (_verbose)
    {
        printf("%s: Clearing DCU data", cmdMgrName);
    }

    _dcuSendDeque.clear();

    DCUStatus cleanDCUStatus;

    for (int dcu = 0; dcu < NUM_DCU; dcu++)
    {
        _dcuStatus[ALPHA][dcu] = cleanDCUStatus;
        _dcuStatus[ALPHA][dcu].group = ALPHA;
        _dcuStatus[ALPHA][dcu].loc = 0;
        _dcuFWStatusHistoryCount[ALPHA][dcu] = 0;
        for (int historyIdx = 0; historyIdx < 4; historyIdx++)
        {
            _dcuFWStatusHistory[ALPHA][dcu][historyIdx] = 0;
        }

        _dcuStatus[BETA][dcu] = cleanDCUStatus;
        _dcuStatus[BETA][dcu].group = BETA;
        _dcuStatus[BETA][dcu].loc = 0;
        _dcuFWStatusHistoryCount[BETA][dcu] = 0;
        for (int historyIdx = 0; historyIdx < 4; historyIdx++)
        {
            _dcuFWStatusHistory[BETA][dcu][historyIdx] = 0;
        }

        _dcuLocOccupied[dcu] = true;
    }

    _currentDCUList.clear();
    _initialDCUList.clear();
}

void DUHWMgr::processSWCREmulatorStatus(SWC_CONFIG sysConfig_,
                                   SWC_MODE mode_,
                                   SWC_MODE olte_,
                                   HealthState tempStatus_,
                                   HealthState pwr12VStatus_,
                                   HealthState pwr24VStatus_,
                                   HealthState atbStatus_,
                                   RolledUpStatus alphaDCURolledUpStatus_,
                                   RolledUpStatus betaDCURolledUpStatus_)
{
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_SYSTEM_CONFIG_MASK, _brdCtrVal, sysConfig_);
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_SWC_MODE_CMD_MASK, _brdCtrVal, mode_);
    _brdCtrVal = DeviceUtilities::updateReg(DU_TEST_MODE_OLTE_MODE_CMD_MASK, _brdCtrVal, olte_);
    _duDev->setBrdCtrlReg(_brdCtrVal);

    if (_verbose)
    {
        printf("From Emulator: config = %d, mode = %d, olte = %d\n", sysConfig_, mode_, olte_);
    }
    _logger.logDebug("From Emulator: config = %d, mode = %d, olte = %d", sysConfig_, mode_, olte_);

    emTempStatus = tempStatus_;
    emPwr12VStatus = pwr12VStatus_;
    emPwr24VStatus = pwr24VStatus_;
    emAtbStatus = atbStatus_;
    emAlphaDCURolledUpStatus = alphaDCURolledUpStatus_;
    emBetaDCURolledUpStatus = betaDCURolledUpStatus_;
}

void DUHWMgr::processDUEmulatorStatus(int statusReg)
{
    if (_verbose)
    {
        printf("From SWCR Status Emulator: setting DU status to 0x%x\n", statusReg);
    }
    _logger.logDebug("From SWCR Status Emulator: setting DU status to 0x%x", statusReg);

    emDUStatusReg = statusReg;
}

void DUHWMgr::processDCUEmulatorStatus(int dcuNum, int dcuFWStatus)
{
    if (_verbose)
    {
        printf("From SWCR Status Emulator: setting DCU %d fw status 0x%x\n", dcuNum, dcuFWStatus);
    }
    _logger.logDebug("From SWCR Status Emulator: setting DCU %d fw status 0x%x", dcuNum, dcuFWStatus);
    emDCUFWStatus[_rfccType][dcuNum] = dcuFWStatus;
}
