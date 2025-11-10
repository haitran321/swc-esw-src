#include <unistd.h>     // for sleep()
#include <cmath>
#include "DUHWMgr.h"
#include "DeviceUtilities.h"
#include "ConfigDataManager.h"

DUHWMgr::DUHWMgr() :
_logger(Logger::getInstance()),
_duDev(NULL),
_brdCtrVal(-1)
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

    /* Common config parameters */;
    int FORCE_TEST_MODE;
    int STEERING_WORD_SRC;
    int DCU_SCLK_READBACK_DELAY;

    _logger.logInfo("DUHWMgr Initializing");

    // Get config parameters
    ConfigDataManager &configs = ConfigDataManager::getInstance();
    rc = rc || configs.get("FORCE_TEST_MODE", FORCE_TEST_MODE);
    rc = rc || configs.get("STEERING_WORD_SRC", STEERING_WORD_SRC);
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

    // Set RFCC Type
    _rfccType = ALPHA;
    if (MODULE_TYPE == BETA)
    {
        _rfccType = BETA;
    }

//  brdCtrVal = DeviceUtilities::readMask(DU_BRD_CTRL_MASK, _duDev->getBrdCtrlReg());
    // Reset brdCtl to default
    _brdCtrVal = 0;
    _duDev->setBrdCtrlReg(_brdCtrVal);

    printf("getFirmwareVersionReg = 0x%x\n", _duDev->getFWVerReg());
    printf("getBoardStatusReg = 0x%x\n", _duDev->getBrdStatusReg());
    printf("getBoardControlReg = 0x%x\n", _duDev->getBrdCtrlReg());
  
    // Set Force Test mode
    if (FORCE_TEST_MODE == TEST)
    {
        _brdCtrVal = DeviceUtilities::updateReg(DU_FORCE_TEST_MODE_MASK, _brdCtrVal, TEST);
    }
    // Set Steering Word source to ARM
    if (STEERING_WORD_SRC == ARM)
    {
        _brdCtrVal = DeviceUtilities::updateReg(DU_STEERING_WORD_SRC_MASK, _brdCtrVal, ARM);
    }
    _duDev->setBrdCtrlReg(_brdCtrVal);

    // Set DCU_SCLK_READBACK_DELAY
    printf("Setting DCU_SCLK_READBACK_DELAY to %d\n", DCU_SCLK_READBACK_DELAY);
    for (int i = 0; i < NUM_DCU-1; i++)
    {
        _duDev->setDCUSCLKReg(i, DCU_SCLK_READBACK_DELAY);
    }

    // Init DCUs status
    readDCUStatus();
    printf("DCU 100 Status = 0x%x, loc = %d\n", _dcuStatus[_rfccType][100].fwStatusReg, _dcuStatus[_rfccType][100].dcuStatus.fwLoc);

    if (MODULE_TYPE == DU_ALPHA)
    {
        // Set default SWC status to TWGS
        _statusToTwgs = 0x0;
        setSwcStatusToTwgs();

        // Init SWC status
        // Send Config Status at start up
        readSWCStatus(DATA_TYPE_CONFIG_STATUS);
    }

    getRegs(0x0, 0x28);
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
    _brdCtrVal = DeviceUtilities::updateReg(DU_SW_TRIGGER_MASK, _brdCtrVal, 1);
    _duDev->setBrdCtrlReg(_brdCtrVal);
    _brdCtrVal = DeviceUtilities::updateReg(DU_SW_TRIGGER_MASK, _brdCtrVal, 0);
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

void DUHWMgr::setPwrSuppliesStatusBit(int val)
{
    _statusToTwgs = DeviceUtilities::updateReg(DU_PS_STATUS_MASK, _statusToTwgs, val);
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
    if (!USE_STATUS_EMULATOR)
    {
        getSysConfigStatus();

        // Translate system config reg
        _sysConfig = (SWC_CONFIG)(DeviceUtilities::readMask(DU_SYSTEM_CONFIG_MASK, _sysConfigReg));
        _mode = (SWC_MODE)(DeviceUtilities::readMask(DU_MODE_MASK, _sysConfigReg));
        _testEnabled = DeviceUtilities::readMask(DU_OFFLINE_TEST_ENABLED_MASK, _sysConfigReg);

        // TO DO: How do we get these status
        _alphaDUStatus = GO;
        _betaDUStatus = GO;
        _alphaDCURolledUpStatus = DCU_ROLLED_UP_GREEN;
        _betaDCURolledUpStatus = DCU_ROLLED_UP_GREEN;
        _tempStatus = GO;
        _pwrStatus = GO;
        _atbStatus = GO;

        // Compute swcr overall status
        _swcrOverall = GO;
        if ((_alphaDUStatus == NO_GO) || (_betaDUStatus == NO_GO) || (_tempStatus == NO_GO) || (_pwrStatus == NO_GO))
        {
            // Should DCU status be included in the SWCR overall rolled up?
            _swcrOverall = NO_GO;
        }
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
        setAlphaOverallStatusBit(_alphaDUStatus);
        setBetaOverallStatusBit(_betaDUStatus);
        setAlphaDCURolledUpStatusBit(_alphaDCURolledUpStatus);
        setBetaDCURolledUpStatusBit(_betaDCURolledUpStatus);
    }
    else    // dataType == DATA_TYPE_IO_MODULE_STATUS
    {
        setDataTypeBit(DATA_TYPE_IO_MODULE_STATUS);
        setTempStatusBit(_tempStatus);
        setPwrSuppliesStatusBit(_pwrStatus);
        setATBStatusBit(_atbStatus);
    }

//  setDataTypeBit(DATA_TYPE_IO_MODULE_STATUS);
//  setConfigBit(_sysConfig);
//  setModeBit(_mode);
//  setAlphaOverallStatusBit(_alphaDUStatus);
//  setBetaOverallStatusBit(_betaDUStatus);
//  setAlphaDCURolledUpStatusBit(_alphaDCURolledUpStatus);
//  setBetaDCURolledUpStatusBit(_betaDCURolledUpStatus);
//  setTempStatusBit(_tempStatus);
//  setPwrSuppliesStatusBit(_pwrStatus);
//  setATBStatusBit(_atbStatus);
//
//  setDCUStatusToTwgs(_dcuGroup, _dcuStatus[_dcuGroup][_dcuNum].dcuStatus.overallStatus, _dcuNum);

    setSwcStatusToTwgs();
}

SWCStatusDataType DUHWMgr::getSWCStatus()
{
    SWCStatusDataType status;

    status.swcStatus = _swcrOverall;
    status.swcConfig = _sysConfig;
    status.swcMode = _mode;
    status.swcAlphaDUStatus = _alphaDUStatus;
    status.swcBetaDUStatus = _betaDUStatus;
    status.swcAlphaDCURolledUpStatus = _alphaDCURolledUpStatus;
    status.swcBetaDCURolledUpStatus = _betaDCURolledUpStatus;
    status.swcTempStatus = _tempStatus;
    status.swcPwrSuppliesStatus = _pwrStatus;
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

    // Use fake dcu status except for dcu at index 40, real dcu at index 40 has dcu number 100
    // So set index 99 dcu number 41
    if ((reg != 40) && (reg != 99) && (MODULE_TYPE == DU_ALPHA))
    {
        fwStatus = 0xb86401;
        fwStatus = DeviceUtilities::updateReg(DCU_LOCATION_STATUS_MASK, fwStatus, reg+1);
    }
    if ((reg == 99) && (MODULE_TYPE == DU_ALPHA))
    {
        fwStatus = 0xb86401;
        fwStatus = DeviceUtilities::updateReg(DCU_LOCATION_STATUS_MASK, fwStatus, 41);
    }
    if (MODULE_TYPE == DU_BETA)
    {
        fwStatus = 0xb86401;
        fwStatus = DeviceUtilities::updateReg(DCU_LOCATION_STATUS_MASK, fwStatus, reg+1);
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

#define GAMMA 1.207234
#define CENTER_FREQ 442.0
#define K 533.597428    // (GAMMA*CENTER_FREQ)
#define UV_THRESHOLD 0.8703556
#define W_THRESHOLD 0.333807
#define EL_THRESHOLD 0.01658

int DUHWMgr::runSWScanLimitCheck(float alpha, float beta)
{
#ifdef PRINT_DEBUG
    printf("****************************************************************\n");
    printf("UV_THRESHOLD = %f, W_THRESHOLD = %f, EL_THRESHOLD = %f\n", UV_THRESHOLD, W_THRESHOLD, EL_THRESHOLD);
#endif

    float u = -beta/K;
    float v = alpha/K;
    float w = sqrt(1 - u*u - v*v);
    float sinEl = v*cos(0.785398) + w*sin(0.785398);   // el_b is boresight elevation at 45 degree in radians

#ifdef PRINT_DEBUG
    printf("alpha = %f, beta = %f, K = %f, u = %f, v = %f, w = %f, sinEl = %f\n", alpha, beta, K, u, v, w, sinEl);
#endif

    // Check against thresholds
    bool slResult = PASSED;
    bool failed = false;
    bool elFailed = false;

    // Check w for NAN
    if (std::isnan(w)) 
    {
        failed = true;
    }
    // Elevation test
    else if (sinEl < EL_THRESHOLD)
    {
        elFailed = true;
    }
    // u, v, w tests
    else if ((abs(u) > UV_THRESHOLD) || (abs(v) > UV_THRESHOLD) || (w < W_THRESHOLD))
    {
        // One of u, v, or w has failed, but elevation passed.  Recompute
        // rounded values of u, v, and w, then check them again.
        float signBeta = (beta > 0.0) ? 1.0 : ((beta < 0.0) ? -1.0 : 0.0);
        float signAlpha = (alpha > 0.0) ? 1.0 : ((alpha < 0.0) ? -1.0 : 0.0);
        u = -(beta - signBeta/2.0)/K;
        v = -(alpha - signAlpha/2.0)/K;
        w = sqrt(1 - u*u - v*v);

        // Re-test u, v and w
        if ((abs(u) > UV_THRESHOLD) || (abs(v) > UV_THRESHOLD) || (w < W_THRESHOLD))
        {
            // Even the rounded values fail.
            failed = true;
        }
    }

    // Set overall test result
    if (failed || elFailed)
    {
        slResult = FAILED;
#ifdef PRINT_DEBUG
        printf("slResult = %d, failed = %d, elFailed = %d\n", slResult, failed, elFailed);
#endif
    }

    return slResult;
}

void DUHWMgr::processEmulatorStatus(HealthState swcrOverall_,
                                   SWC_CONFIG sysConfig_,
                                   SWC_MODE mode_,
                                   HealthState alphaDUStatus_,
                                   HealthState betaDUStatus_,
                                   DCURolledUpStatus alphaDCURolledUpStatus_,
                                   DCURolledUpStatus betaDCURolledUpStatus_,
                                   HealthState tempStatus_,
                                   HealthState pwrStatus_,
                                   HealthState atbStatus_,
                                   RFCC_CH dcuGroup_,
                                   HealthState dcuStatus_,
                                   int dcuNum_)
{
    _swcrOverall = swcrOverall_;
    _sysConfig = sysConfig_;
    _mode = mode_;
    printf("From Emulator: overall = %d, config = %d, mode = %d\n", _swcrOverall, _sysConfig, _mode);
    _alphaDUStatus = alphaDUStatus_;
    _betaDUStatus = betaDUStatus_;
    _alphaDCURolledUpStatus = alphaDCURolledUpStatus_;
    _betaDCURolledUpStatus = betaDCURolledUpStatus_;
    _tempStatus = tempStatus_;
    _pwrStatus = pwrStatus_;
    _atbStatus = atbStatus_;
    printf("_atbStatus = %d\n", _atbStatus);
    _dcuGroup = dcuGroup_;
    _dcuNum = dcuNum_;
    printf("From Emulator: setting rfcc %d dcu %d to %d\n", dcuGroup_, dcuNum_, dcuStatus_);
    _dcuStatus[dcuGroup_][dcuNum_].dcuStatus.overallStatus = dcuStatus_;
}
