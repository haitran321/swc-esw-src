#include <stdio.h>
#include <sstream>
#include <unistd.h>

#include "TUCmdMgr.h"
#include "ConfigDataManager.h"
#include "SAPDataManager.h"
#include "ScanLimitCheck.h"

/**
 * Constructor
 */
TUCmdMgr::TUCmdMgr() :
    CmdMgrBase(TU),
    _localHWStatus(NULL),
    _tuHWMgr(TUHWMgr::getInstance()),
    _uioDevSL(NULL),
    _uioDevWLSP(NULL),
    _timerDevStatus(NULL)
{
}

/**
 * Destructor
 */
TUCmdMgr::~TUCmdMgr()
{
    delete _localHWStatus;
    _localHWStatus = NULL;

    delete _uioDevSL;
    _uioDevSL = NULL;

    delete _uioDevWLSP;
    _uioDevWLSP = NULL;

    delete _timerDevStatus;
    _timerDevStatus = NULL;
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
    int TO_TEST_SERVER_PORT;
    int STATUS_TIMER_INTERVAL_SECONDS;

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

    // For Status Emulator
    int FROM_STATUS_EMULATOR_PORT;
    rc = rc || configs.get("FROM_STATUS_EMULATOR_PORT", FROM_STATUS_EMULATOR_PORT);

    // Configuration parameters
    rc = rc || configs.get("FORCE_TEST_MODE", FORCE_TEST_MODE);

    // Status parameters
    rc = rc || configs.get("STATUS_TIMER_INTERVAL_SECONDS", STATUS_TIMER_INTERVAL_SECONDS);

    // Setup Logger
    _logger.initialize();
    _logger.logInfo("TUCmdMgr Initializing");

    if (rc == ERROR)
    {
        _logger.logInfo("ERROR: reading config file");
        return (ERROR);
    }

    _logger.logInfo("MODULE_TYPE = %d", _moduleType);

    if (initializeCommonCommandDevices(TEST_UNIT_IP_ADDRESS, TEST_SERVER_IP_ADDRESS,
                                       FROM_TEST_SERVER_PORT, TO_TEST_SERVER_PORT,
                                       FROM_STATUS_EMULATOR_PORT, STATUS_TIMER_INTERVAL_SECONDS) != OK)
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

    // Initialize HW Manager
    _tuHWMgr.initialize();

    // Read TU status and send to Alpha DU
    DUTUStatusMsg tuStatus;
    tuStatus.msgID = TU_STATUS;
    tuStatus.dutuStatus = _tuHWMgr.readTUStatus();
    sendTUStatusToDUA(tuStatus);
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

    // Status Timer
    timespec init = { STATUS_TIMER_INTERVAL_SECONDS, 0 };
    timespec timeout = { STATUS_TIMER_INTERVAL_SECONDS, 0 };
    _timerDevStatus = new TimerDevice(init, timeout);

    if (_timerDevStatus->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _timerDevStatus->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_timerDevStatus, READ_EVENT, 1, static_cast<EventFunc>(&TUCmdMgr::processStatusTimer)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _timerDevStatus->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _timerDevStatus device");
    printf("Successfully created _timerDevStatus device\n");

    EventProcessor::start();

    return OK;
}

void TUCmdMgr::processSLInterrupt()
{
    size_t bytesRead = 0;
    int pending = 0;

    _uioDevSL->read((char *)&pending, sizeof(int), bytesRead);
    printf("Reading scan limit interrupt, number of interrupt = %d\n", pending);
    _logger.logDebug("Reading scan limit interrupt, number of interrupt = %d", pending);
    _uioDevSL->clearInterrupt();

    // Get FW SL check status
    int alpha = _tuHWMgr.getArmKSine(ALPHA);
    int beta = _tuHWMgr.getArmKSine(BETA);

    printf("alpha = %d, beta = %d\n", alpha, beta);
    _logger.logDebug("alpha = %d, beta = %d", alpha, beta);

    int swSLResult = runSWScanLimitCheck(float(alpha), float(beta));
    int fwSLResult = _tuHWMgr.getFWScanLimitCheckStatus();

    printf("fwSLResult = 0x%x(%d), swSLResult = %d\n", fwSLResult, fwSLResult & 0x1, swSLResult);
    _logger.logDebug("fwSLResult = 0x%x(%d), swSLResult = %d", fwSLResult, fwSLResult & 0x1, swSLResult);
}

void TUCmdMgr::processWLSPInterrupt()
{
    size_t bytesRead = 0;
    int pending = 0;

    _uioDevWLSP->read((char *)&pending, sizeof(int), bytesRead);
    printf("Reading WLSP interrupt, number of interrupt = %d\n", pending);
    _logger.logDebug("Reading WLSP changed interrupt, number of interrupt = %d", pending);
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
        _tuHWMgr.setArmKSine(ALPHA, params.alpha);
        _tuHWMgr.setArmKSine(BETA, params.beta);

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
    printf("ERROR: Invalid status request of %d\n", params.requestType);
    _logger.logError("ERROR: Invalid status request of %d", params.requestType);
}

void TUCmdMgr::sendTUStatusToDUA(DUTUStatusMsg status)
{
    _localHWStatus->write(&status, sizeof(DUTUStatusMsg));
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

