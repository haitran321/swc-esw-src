#include <stdio.h>

#include "DUCmdMgrBase.h"
#include "DeviceUtilities.h"
#include "ScanLimitCheck.h"

DUCmdMgrBase::DUCmdMgrBase(MODULE_TYPE moduleType) :
    CmdMgrBase(moduleType),
    _duHWMgr(DUHWMgr::getInstance()),
    _uioDevSL(NULL),
    _uioDevConfig(NULL),
    sendProcessedSW(false),
    lastProcessedAlpha(0),
    lastProcessedBeta(0),
    REFRESH_DCU_STATUS_ON_GDS_INTERVAL(6),
    _steeringCmdCounter(0),
    _statusRequestCmdCounter(0)
{
}

DUCmdMgrBase::~DUCmdMgrBase()
{
    delete _uioDevSL;
    _uioDevSL = NULL;

    delete _uioDevConfig;
    _uioDevConfig = NULL;
}

STATUS DUCmdMgrBase::initializeDUCommonDevices()
{
    _uioDevSL = new UIODevice(AXI_INT_121_OFFSET, 0);

    if (_uioDevSL->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _uioDevSL->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_uioDevSL, READ_EVENT, 1, static_cast<EventFunc>(&DUCmdMgrBase::processSLInterrupt)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _uioDevSL->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _uioDevSL device");
    printf("Successfully created _uioDevSL device\n");

    _uioDevSL->mmap();
    _uioDevSL->clearInterrupt();

    _uioDevConfig = new UIODevice(AXI_INT_122_OFFSET, 1);

    if (_uioDevConfig->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _uioDevConfig->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_uioDevConfig, READ_EVENT, 1, static_cast<EventFunc>(&DUCmdMgrBase::processConfigInterrupt)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _uioDevConfig->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _uioDevConfig device");
    printf("Successfully created _uioDevConfig device\n");

    _uioDevConfig->mmap();
    _uioDevConfig->clearInterrupt();

    return OK;
}

void DUCmdMgrBase::setShutdownBit()
{
    printf("DUCmdMgrBase::setShutdownBit\n");
    _duHWMgr.setShutdownBit();
}   

void DUCmdMgrBase::handleSteeringCommand(const SteeringCmdDataType& params)
{
    _steeringCmdCounter++;
    _logger.logInfo("===> STEERING_CMD_MSG_ID: SteeringCmdCounter = %d", _steeringCmdCounter);

    if ((_duHWMgr.getSWCModeStatus() == OFFLINE) || (_duHWMgr.getOLTEModeStatus() == OFFLINE) || (FORCE_TEST_MODE == TEST))
    {
        if (params.testSource == Digital)
        {
            _duHWMgr.setTestSrcInTestMode(params.testSource);

            _duHWMgr.setArmKSine(ALPHA, params.alpha);
            _duHWMgr.setArmKSine(BETA, params.beta);

            if (params.RLCP == On)
            {
                _logger.logDebug("Setting RLCP to On");
                _duHWMgr.sendSteeringWordValidFlagInTestMode(STEERING_WORD_INVALID);
                _duHWMgr.sendDCUCmdInTestMode(DCU_CMD_BORESIGHT);
            }
            if (params.RLSC == On)
            {
                _logger.logDebug("Setting RLSC to On");
                _duHWMgr.sendSteeringWordValidFlagInTestMode(STEERING_WORD_INVALID);
                _duHWMgr.sendDCUCmdInTestMode(DCU_CMD_CALIBRATION);
            }

            _duHWMgr.toggleSWTrigger();

            usleep(1 * 1000);   // Sleep 1 msecs

            _duHWMgr.sendSteeringWordValidFlagInTestMode(STEERING_WORD_VALID);
            _duHWMgr.setTestSrcInTestMode(Analog);
        }
    }
}

void DUCmdMgrBase::handleStatusRequest(const StatusRequestCmdDataType& params)
{
    _statusRequestCmdCounter++;
    _logger.logInfo("===> STATUS_REQUEST_CMD_MSG_ID: StatusRequestCmdCounter = %d", _statusRequestCmdCounter);

    if (params.requestType == StartSendingProcessedSteeringWord)
    {
        sendProcessedSW = true;
        return;
    }

    if (params.requestType == StopSendingProcessedSteeringWord)
    {
        sendProcessedSW = false;
        return;
    }

    handleDUStatusRequest(params);
}

void DUCmdMgrBase::handleStressTestCommand(const StressTestCmdDataType& params)
{
    printf("ERROR: Not processing stress test command\n");
    _logger.logError("ERROR: Not processing stress test command");
}

void DUCmdMgrBase::handleRepollDCUCommand()
{
    _duHWMgr.clearDCUData();
}

void DUCmdMgrBase::processSLInterrupt()
{
    size_t bytesRead = 0;
    int pending = 0;

    _uioDevSL->read((char *)&pending, sizeof(int), bytesRead);
    if (_verbose)
    {
        printf("Reading scan limit interrupt, number of interrupt = %d\n", pending);
    }
    _logger.logDebug("reading scan limit interrupt, number of interrupt = %d", pending);
    _uioDevSL->clearInterrupt();

    int atbAlpha = _duHWMgr.getAtbKSine(ALPHA);
    if ((atbAlpha & 0x200) != 0)
    {
        atbAlpha |= 0xfffffc00;
    }

    int atbBeta = _duHWMgr.getAtbKSine(BETA);
    if ((atbBeta & 0x200) != 0)
    {
        atbBeta |= 0xfffffc00;
    }

    if (_verbose)
    {
        printf("atbAlpha = %d, atbBeta = %d\n", atbAlpha, atbBeta);
    }
    _logger.logDebug("atbAlpha = %d, atbBeta = %d", atbAlpha, atbBeta);

    int atbSWSLResult = runSWScanLimitCheck(float(atbAlpha), float(atbBeta));
    int fwSLResult = _duHWMgr.getFWScanLimitCheckStatus();

    if (_verbose)
    {
        printf("fwSLResult = 0x%x(%d), atbSWSLResult = %d\n",
               fwSLResult, fwSLResult & 0x1, atbSWSLResult);
    }

    _logger.logDebug("fwSLResult = 0x%x(%d), atbSWSLResult = %d",
                     fwSLResult, fwSLResult & 0x1, atbSWSLResult);

    lastProcessedAlpha = atbAlpha;
    lastProcessedBeta = atbBeta;

    if (_duHWMgr.getOverallSPIStatus() == FAILED)
    {
        if (_verbose)
        {
            printf("Last SPI transfer status has no failures.\n");
        }
        _logger.logDebug("Last SPI transfer status has no failures.");
    }
    else
    {
        if (_verbose)
        {
            printf("Last SPI transfer status has failures.\n");
        }
        _logger.logDebug("Last SPI transfer status has failures.");
    }

    handlePendingDcuStatusAfterScanLimit();

    if (sendProcessedSW)
    {
        sendProcessedSteeringWordReport(getProcessedKSineForReport());
    }
}

void DUCmdMgrBase::processConfigInterrupt()
{
    size_t bytesRead = 0;
    int pending = 0;

    _uioDevConfig->read((char *)&pending, sizeof(int), bytesRead);
//  if (_verbose)
//  {
        printf("Reading config changed interrupt, number of interrupt = %d\n", pending);
//  }
    _logger.logDebug("reading config changed interrupt, number of interrupt = %d", pending);
    _uioDevConfig->clearInterrupt();

    handleConfigInterruptRefresh();

    if (_duHWMgr.getSWCModeStatus() == ONLINE)
    {
        _duHWMgr.setTestSrcInTestMode(Analog);
    }
}

void DUCmdMgrBase::processStatusTimer()
{
    CmdMgrBase::processStatusTimer();

    handlePreDcuStatusTimer();

    if (_verbose)
    {
        printf("Read all DCUs status to update local queue.\n");
    }
    _logger.logDebug("Read all DCUs status to update local queue.");
    if (_statusTimerCounter % REFRESH_DCU_STATUS_ON_GDS_INTERVAL == 0)
    {
        _duHWMgr.readDCUStatus(true);
    }
    else
    {
        _duHWMgr.readDCUStatus();
    }

    int dequeSize = _duHWMgr.getDCUStatusDequeSize();
    if (_verbose)
    {
        printf("DCU status deque size = %d\n", dequeSize);
    }
    _logger.logDebug("DCU status deque size = %d", dequeSize);

    handlePostDcuStatusTimer(dequeSize);

    _timerDevStatus->read();
}

void DUCmdMgrBase::sendProcessedSteeringWordReport(int processedKSine)
{
    SWCProcessedSteeringWordRptMsg swRptMsg;
    swRptMsg.setModuleType(static_cast<MODULE_TYPE>(_moduleType));
    swRptMsg.setProcessedKSine(processedKSine);
    swRptMsg.buildMsg();
    int msgSize = swRptMsg.getBufSize();
    swRptMsg.headerByteSwapToNetwork();

    _toTestServer->write(swRptMsg.getBuf(), msgSize);
}

void DUCmdMgrBase::handlePostDcuStatusTimer(int dequeSize)
{
    (void)dequeSize;
}

void DUCmdMgrBase::handleConfigInterruptRefresh()
{
}
