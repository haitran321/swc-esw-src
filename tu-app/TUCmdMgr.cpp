#include <stdio.h>
#include <sstream>
#include <unistd.h>

#include "TUCmdMgr.h"
#include "ConfigDataManager.h"
#include "SAPDataManager.h"
#include "ScanLimitCheck.h"
#include "StatusSentToTWGSRptMsg.h"

/**
 * Constructor
 */
TUCmdMgr::TUCmdMgr() :
    CmdMgrBase(TU),
    _localHWStatus(NULL),
    _localHWCommand(NULL),
    _tuHWMgr(TUHWMgr::getInstance()),
    _uioDevSL(NULL),
    _uioDevWLSP(NULL)
{
}

/**
 * Destructor
 */
TUCmdMgr::~TUCmdMgr()
{
    delete _localHWStatus;
    _localHWStatus = NULL;

    delete _localHWCommand;
    _localHWCommand = NULL;

    delete _uioDevSL;
    _uioDevSL = NULL;

    delete _uioDevWLSP;
    _uioDevWLSP = NULL;
}

/** 
 * Creates, opens and registers events
 * Call the base class start()
 */
STATUS TUCmdMgr::start()
{
    STATUS rc = OK;

    string TEST_SERVER_IP_ADDRESS;
    string ALPHA_IP_ADDRESS;
    string BETA_IP_ADDRESS;
    string TEST_UNIT_IP_ADDRESS;
    int FROM_TEST_SERVER_PORT;
    int WARM_RESTART_PORT;
    int LOCAL_HW_STATUS_PORT;
    int LOCAL_HW_COMMAND_PORT;
    int TO_TEST_SERVER_PORT;
    int SAP_STATUS_TIMER_INTERVAL_SECONDS;

    printf("\nLoading Config file\n");
    if (ConfigDataManager::getInstance().load() != OK)
    {
        printf("Error loading Config file\n");
    }
    ConfigDataManager& configs = ConfigDataManager::getInstance();

    printf("\nLoading SAP file\n");
    if (SAPDataManager::getInstance().load() != OK)
    {
        printf("Error loading Config file\n");
    }
    SAPDataManager& saps = SAPDataManager::getInstance();

    // Verbose parameters
    rc = rc || configs.get("VERBOSE", _verbose);

    // Get IP addresses
    rc = rc || configs.get("TEST_SERVER_IP_ADDRESS", TEST_SERVER_IP_ADDRESS);
    rc = rc || configs.get("ALPHA_IP_ADDRESS", ALPHA_IP_ADDRESS);
    rc = rc || configs.get("BETA_IP_ADDRESS", BETA_IP_ADDRESS);
    rc = rc || configs.get("TEST_UNIT_IP_ADDRESS", TEST_UNIT_IP_ADDRESS);

    // Get port number to/from Test Server
    rc = rc || configs.get("FROM_TEST_SERVER_PORT", FROM_TEST_SERVER_PORT);
    rc = rc || configs.get("TO_TEST_SERVER_PORT", TO_TEST_SERVER_PORT);
    rc = rc || configs.get("WARM_RESTART_PORT", WARM_RESTART_PORT);

    // Get port number to/from Local HW devices
    rc = rc || configs.get("LOCAL_HW_STATUS_PORT", LOCAL_HW_STATUS_PORT);
    rc = rc || configs.get("LOCAL_HW_COMMAND_PORT", LOCAL_HW_COMMAND_PORT);

    // For Status Emulator
    int FROM_STATUS_EMULATOR_PORT;
    rc = rc || configs.get("FROM_STATUS_EMULATOR_PORT", FROM_STATUS_EMULATOR_PORT);

    // Configuration parameters
    rc = rc || configs.get("FORCE_TEST_MODE", FORCE_TEST_MODE);

    // Status parameters
    rc = rc || saps.get("SAP_STATUS_TIMER_INTERVAL_SECONDS", SAP_STATUS_TIMER_INTERVAL_SECONDS);

    // Setup Logger
    _logger.initialize(getCommandMgrName());
    _logger.logInfo("TUCmdMgr Initializing");

    if (rc == ERROR)
    {
        _logger.logInfo("ERROR: reading config file");
        return (ERROR);
    }

    _logger.logInfo("MODULE_TYPE = %d", _moduleType);

    if (initializeCommonCommandDevices(TEST_UNIT_IP_ADDRESS, TEST_SERVER_IP_ADDRESS,
                                       FROM_TEST_SERVER_PORT, TO_TEST_SERVER_PORT,
                                       FROM_STATUS_EMULATOR_PORT, SAP_STATUS_TIMER_INTERVAL_SECONDS) != OK)
    {
        return ERROR;
    }

    // From HW devices - Outbound for TU
    stringstream devName;
    devName.clear();
    devName << "UDP Client ";
    devName << ALPHA_IP_ADDRESS << ":" << LOCAL_HW_STATUS_PORT;
    _localHWStatus = new UDPNetworkDevice(NetworkClient, ALPHA_IP_ADDRESS, LOCAL_HW_STATUS_PORT, false);
    _localHWStatus->setName(devName.str());

    if (_localHWStatus->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _localHWStatus->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _localHWStatus device");
    printf("Successfully created _localHWStatus device\n");

    // From HW devices - Inbound for TU
    devName.str("");
    devName.clear();
    devName << "UDP Server ";
    devName << TEST_UNIT_IP_ADDRESS << ":" << LOCAL_HW_COMMAND_PORT;

    _localHWCommand = new UDPNetworkDevice(NetworkServer, TEST_UNIT_IP_ADDRESS, LOCAL_HW_COMMAND_PORT, false);
    _localHWCommand->setName(devName.str());

    if (_localHWCommand->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _localHWCommand->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_localHWCommand, READ_EVENT, 1, static_cast<EventFunc>(&TUCmdMgr::processLocalHWCommandMsg)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _localHWCommand->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _localHWCommand device");
    printf("Successfully created _localHWCommand device\n");

    // Initialize HW Manager
    _tuHWMgr.initialize(getCommandMgrName());

    // Read TU status and send to Alpha DU
    DUTUStatusMsg tuStatus;
    tuStatus.msgID = TU_STATUS;
    tuStatus.dutuStatus = _tuHWMgr.readTUStatus();
    sendTUStatusToDUA(tuStatus);
    usleep(1 * 1000);   // Sleep 1 msecs

    // Request config/mode
    ConfigModeRequestMsg request;
    request.msgID = CONFIG_MODE_REQUEST;
    _localHWStatus->write(&request, sizeof(ConfigModeRequestMsg));
    usleep(1 * 1000);   // Sleep 1 msecs

    // Open UIO device for Scan Limit HW Interrupt
    _uioDevSL = new UIODevice(AXI_INT_121_OFFSET, 0);

    if (_uioDevSL->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _uioDevSL->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_uioDevSL, READ_EVENT, 1, static_cast<EventFunc>(&TUCmdMgr::processSLInterrupt)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _uioDevSL->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _uioDevSL device");
    printf("Successfully created _uioDevSL device\n");

    // Map UIO address
    _uioDevSL->mmap();
    _uioDevSL->clearInterrupt();

    // Open UIO device for HW Config Changed Interrupt
    _uioDevWLSP = new UIODevice(AXI_INT_122_OFFSET, 1);

    if (_uioDevWLSP->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _uioDevWLSP->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_uioDevWLSP, READ_EVENT, 1, static_cast<EventFunc>(&TUCmdMgr::processWLSPInterrupt)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _uioDevWLSP->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _uioDevWLSP device");
    printf("Successfully created _uioDevWLSP device\n");

    // Map UIO address
    _uioDevWLSP->mmap();
    _uioDevWLSP->clearInterrupt();

    EventProcessor::start();

    return OK;
}

void TUCmdMgr::setShutdownBit()
{
    printf("TUCmdMgr::setShutdownBit\n");
    _tuHWMgr.setShutdownBit();
}   

void TUCmdMgr::processSLInterrupt()
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

    // Get FW SL check status
    int alpha = _tuHWMgr.getArmKSine(ALPHA);
    int beta = _tuHWMgr.getArmKSine(BETA);

    if (_verbose)
    {
        printf("alpha = %d, beta = %d\n", alpha, beta);
    }
    _logger.logDebug("alpha = %d, beta = %d", alpha, beta);

    int swSLResult = runSWScanLimitCheck(float(alpha), float(beta));
    int fwSLResult = _tuHWMgr.getFWScanLimitCheckStatus();

    if (_verbose)
    {
        printf("fwSLResult = 0x%x(%d), swSLResult = %d\n", fwSLResult, fwSLResult & 0x1, swSLResult);
    }
    _logger.logDebug("fwSLResult = 0x%x(%d), swSLResult = %d", fwSLResult, fwSLResult & 0x1, swSLResult);
}

void TUCmdMgr::processWLSPInterrupt()
{
    size_t bytesRead = 0;
    int pending = 0;

    _uioDevWLSP->read((char *)&pending, sizeof(int), bytesRead);
    if (_verbose)
    {
        printf("Reading WLSP interrupt, number of interrupt = %d\n", pending);
    
        _logger.logDebug("reading WLSP changed interrupt, number of interrupt = %d", pending);
    }
    _uioDevWLSP->clearInterrupt();
}

void TUCmdMgr::processStatusTimer()
{
    CmdMgrBase::processStatusTimer();

    // Read TU status and send to Alpha DU
    DUTUStatusMsg tuStatus;
    tuStatus.msgID = TU_STATUS;
    tuStatus.dutuStatus = _tuHWMgr.readTUStatus();
    sendTUStatusToDUA(tuStatus);
    usleep(1 * 1000);   // Sleep 1 msecs

    _timerDevStatus->read();
}

const char *TUCmdMgr::getCommandMgrName() const
{
    return "TU";
}

void TUCmdMgr::handleSteeringCommand(const SteeringCmdDataType& params)
{
    static int SteeringCmdCounter = 0;
    SteeringCmdCounter++;

    _logger.logInfo("===> STEERING_CMD_MSG_ID: SteeringCmdCounter = %d", SteeringCmdCounter);
    
    // Set KSine Regs
    if (params.testSource == Analog)
    {
        if (_verbose)
        {
            printf("===> STEERING_CMD_MSG_ID: SteeringCmdCounter = %d\n", SteeringCmdCounter);
        }
        _tuHWMgr.setArmKSine(ALPHA, params.alpha);
        _tuHWMgr.setArmKSine(BETA, params.beta);

        // Setup TU for 1 action
        _tuHWMgr.setNumCycle(1);
        _tuHWMgr.setNumRLTDPerCycle(1);
        _tuHWMgr.setNumInc(0);
        _tuHWMgr.setCycleResetTime(0);
        _tuHWMgr.setRLTDPeriod(0);
        _tuHWMgr.setAlphaInc(0);
        _tuHWMgr.setBetaInc(0);

        if (params.RLCP == On)
        {
            _tuHWMgr.setRLCPSignal(On);
        }
        if (params.RLSC == On)
        {
            _tuHWMgr.setRLSCSignal(On);
        }

        // Toggle RLTD signal to start steering words processing
        _tuHWMgr.toggleRLTDSignal();

        // Reset RLCP and RLSC back to off
        _tuHWMgr.setRLCPSignal(Off);
        _tuHWMgr.setRLSCSignal(Off);
    }
}

void TUCmdMgr::handleStatusRequest(const StatusRequestCmdDataType& params)
{
    if (params.requestType == SystemStatusSentToTWGS)
    {
        int swcStatus = _tuHWMgr.getSWCStatus();
        int swcrStatus = _tuHWMgr.getSWCRStatus();

        _logger.logDebug("SWC Status 0x%x, SWCR Status 0x%x", swcStatus, swcrStatus);
        if (_verbose)
        {
            printf("SWC Status 0x%x, SWCR Status 0x%x\n", swcStatus, swcrStatus);
        }

        StatusSentToTWGSRptMsg rptMsg;
        rptMsg.setSWCStatus(swcStatus);
        rptMsg.setSWCRStatus(swcrStatus);
        rptMsg.buildMsg();
        int msgSize = rptMsg.getBufSize();
        rptMsg.headerByteSwapToNetwork();

        _toTestServer->write(rptMsg.getBuf(), msgSize);
    }
}

void TUCmdMgr::handleStressTestCommand(const StressTestCmdDataType& params)
{
    static int StressTestCmdCounter = 0;
    StressTestCmdCounter++;

    _logger.logInfo("===> STRESS_TEST_CMD_MSG_ID: StressTestCmdCounter = %d", StressTestCmdCounter);
    _logger.logDebug("Stress Test: num cycle = %d, numTest = %d, numInc = %d, reset time = %d, spacing = %d, alpha = %d, alpha inc = %d, beta = %d, beta inc = %d",
           params.numCycle, params.numTest, params.numInc, params.cycleResetTime, params.spacingUsec, params.alpha, params.alphaInc, params.beta, params.betaInc);
    if (_verbose)
    {
        printf("Stress Test: num cycle = %d, numTest = %d, numInc = %d, reset time = %d, spacing = %d, alpha = %d, alpha inc = %d, beta = %d, beta inc = %d\n",
           params.numCycle, params.numTest, params.numInc, params.cycleResetTime, params.spacingUsec, params.alpha, params.alphaInc, params.beta, params.betaInc);
    }

    _tuHWMgr.setArmKSine(ALPHA, params.alpha);
    _tuHWMgr.setArmKSine(BETA, params.beta);

    // Setup TU for 1 action
    _tuHWMgr.setNumCycle(params.numCycle);
    _tuHWMgr.setNumRLTDPerCycle(params.numTest);
    _tuHWMgr.setNumInc(params.numInc);
    _tuHWMgr.setCycleResetTime(params.cycleResetTime);
    _tuHWMgr.setRLTDPeriod(params.spacingUsec);
    _tuHWMgr.setAlphaInc(params.alphaInc);
    _tuHWMgr.setBetaInc(params.betaInc);

    // Toggle RLTD signal to start steering words processing
    _tuHWMgr.toggleRLTDSignal();
}

void TUCmdMgr::handleRepollDCUCommand()
{
    printf("ERROR: Not processing repoll dcu command\n");
    _logger.logError("ERROR: Not processing repoll dcu command");
}

void TUCmdMgr::sendTUStatusToDUA(DUTUStatusMsg status)
{
    _localHWStatus->write(&status, sizeof(DUTUStatusMsg));
}

void TUCmdMgr::processLocalHWCommandMsg()
{
    size_t bytesRead = 0;
    const int localCommandBufferCount = sizeof(_configModeCmdMsg) / sizeof(_configModeCmdMsg[0]);
    ConfigModeCmdMsg *configModeCmdMsg = &_configModeCmdMsg[configModeCmdCounter];
    configModeCmdCounter++;

    if (configModeCmdCounter >= localCommandBufferCount)
    {
        configModeCmdCounter = 0;
    }

    // Read UDP data
    if (_localHWCommand->read((char *)configModeCmdMsg, sizeof(ConfigModeCmdMsg), bytesRead) != OK)
    {
        printf("Error reading from _localHWCommand\n");
        _logger.logDebug("Error reading from _localHWCommand");
        return;
    }

    // Get message id
    if (configModeCmdMsg->msgID == CONFIG_MODE_CMD)
    {
        if (_verbose)
        {
            printf("Received configModeCmdMsg: config %d mode %d oltd = %d\n", configModeCmdMsg->configMode.swcConfig,
                   configModeCmdMsg->configMode.swcMode, configModeCmdMsg->configMode.olteMode);
        }
        _logger.logDebug("Received configModeCmdMsg: config %d mode %d oltd = %d", configModeCmdMsg->configMode.swcConfig,
                         configModeCmdMsg->configMode.swcMode, configModeCmdMsg->configMode.olteMode);
        _tuHWMgr.setConfig(configModeCmdMsg->configMode.swcConfig);
        _tuHWMgr.setMode(configModeCmdMsg->configMode.swcMode);
        _tuHWMgr.setOLTE(configModeCmdMsg->configMode.olteMode);
    }
}

#define EMU_MSG_ID_SWCR_STATUS 1
#define EMU_MSG_ID_DUA_STATUS 2
#define EMU_MSG_ID_DUB_STATUS 3
#define EMU_MSG_ID_TU_STATUS 4
#define EMU_MSG_ID_DCU_STATUS 5

void TUCmdMgr::handleStatusEmulatorMessage(int msg_id, const int *status, int numData)
{
    (void)numData;

    if (msg_id == EMU_MSG_ID_TU_STATUS)
    {
        _tuHWMgr.processTUEmulatorStatus(status[1]);
    }
    else
    {
        printf("Error: this SWCR Status Emulator is not being processed by this component\n");
        _logger.logDebug("Error: this SWCR Status Emulator is not being processed by this component");
    }
}
