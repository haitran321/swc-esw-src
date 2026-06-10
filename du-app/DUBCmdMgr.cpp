#include <stdio.h>
#include <sstream>
#include <unistd.h>

#include "DUBCmdMgr.h"
#include "ConfigDataManager.h"
#include "SAPDataManager.h"

/**
 * Constructor
 */
DUBCmdMgr::DUBCmdMgr() :
    DUCmdMgrBase(DU_BETA),
    _localHWStatus(NULL)
{
}

/**
 * Destructor
 */
DUBCmdMgr::~DUBCmdMgr()
{
    delete _localHWStatus;
    _localHWStatus = NULL;
}

/** 
 * Creates, opens and registers events
 * Call the base class start()
 */
STATUS DUBCmdMgr::start()
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
    rc = rc || configs.get("REFRESH_DCU_STATUS_ON_GDS_INTERVAL", REFRESH_DCU_STATUS_ON_GDS_INTERVAL);

    // Verbose parameters
    rc = rc || configs.get("VERBOSE", _verbose);

    // Setup Logger
    _logger.initialize();
    _logger.logInfo("DUBCmdMgr Initializing");

    if (rc == ERROR)
    {
        _logger.logInfo("ERROR: reading config file");
        return (ERROR);
    }

    _logger.logInfo("MODULE_TYPE = %d", _moduleType);

    if (initializeCommonCommandDevices(BETA_IP_ADDRESS, TEST_SERVER_IP_ADDRESS,
                                       FROM_TEST_SERVER_PORT, TO_TEST_SERVER_PORT,
                                       FROM_STATUS_EMULATOR_PORT, STATUS_TIMER_INTERVAL_SECONDS) != OK)
    {
        return ERROR;
    }

    // From HW devices - Outbound for DU Beta
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
    _duHWMgr.initialize(_moduleType);

    // Read Beta DU status and send to Alpha DU
    DUTUStatusMsg bduStatus;
    bduStatus.msgID = BETA_DU_STATUS;
    bduStatus.dutuStatus = _duHWMgr.readDUStatus();
    sendDUBStatusToDUA(bduStatus);
    _logger.logInfo("Sent DUB status to DUA");
    usleep(1 * 1000);   // Sleep 1 msecs

    // Read and send Beta DCUs status to Alpha DU
    for (int dcu = 0; dcu < NUM_DCU; dcu++)
    {
        int dequeSize = _duHWMgr.getDCUStatusDequeSize();
        if (dequeSize > 0)
        {
            DCUStatus dcuStatus = _duHWMgr.getDCUStatusFromDeque();
            BetaDCUStatusMsg status;
            status.msgID = BETA_DCU_STATUS;
            status.betaDCUStatus = dcuStatus;
            sendDCUStatusToDUA(status);
            usleep(1 * 1000);   // Sleep 1 msecs
        }
    }
    _logger.logInfo("Sent DUB DCUs status to DUA");

    if (initializeDUCommonDevices() != OK)
    {
        return ERROR;
    }

    EventProcessor::start();

    return OK;
}

void DUBCmdMgr::handlePendingDcuStatusAfterScanLimit()
{
    for (int dcu = 0; dcu < NUM_DCU; dcu++)
    {
        int dequeSize = _duHWMgr.getDCUStatusDequeSize();
        if (dequeSize > 0)
        {
            DCUStatus dcuStatus = _duHWMgr.getDCUStatusFromDeque();
            BetaDCUStatusMsg status;
            status.msgID = BETA_DCU_STATUS;
            status.betaDCUStatus = dcuStatus;
            sendDCUStatusToDUA(status);
//          usleep(1 * 1000);   // Sleep 1 msecs
        }
    }
}

void DUBCmdMgr::handlePreDcuStatusTimer()
{
    // Read Beta DU status and send to Alpha DU
    DUTUStatusMsg bduStatus;
    bduStatus.msgID = BETA_DU_STATUS;
    bduStatus.dutuStatus = _duHWMgr.readDUStatus();
    sendDUBStatusToDUA(bduStatus);
    usleep(1 * 1000);   // Sleep 1 msecs
}

const char *DUBCmdMgr::getCommandMgrName() const
{
    return "DUB";
}

void DUBCmdMgr::handlePostDcuStatusTimer(int dequeSize)
{
    // Send DCU status to Alpha DU
    for (int i = 0; i < dequeSize; i++)
    {
        if (_duHWMgr.getDCUStatusDequeSize() > 0)
        {
            DCUStatus dcuStatus = _duHWMgr.getDCUStatusFromDeque();
            BetaDCUStatusMsg status;
            status.msgID = BETA_DCU_STATUS;
            status.betaDCUStatus = dcuStatus;
            sendDCUStatusToDUA(status);
            usleep(1 * 1000);   // Sleep 1 msecs
        }
    }
}

void DUBCmdMgr::handleDUStatusRequest(const StatusRequestCmdDataType& params)
{
    printf("ERROR: Invalid status request of %d\n", params.requestType);
    _logger.logError("ERROR: Invalid status request of %d", params.requestType);
}

void DUBCmdMgr::sendDCUStatusToDUA(BetaDCUStatusMsg status)
{
    _localHWStatus->write(&status, sizeof(BetaDCUStatusMsg));
}

void DUBCmdMgr::sendDUBStatusToDUA(DUTUStatusMsg status)
{
    _localHWStatus->write(&status, sizeof(DUTUStatusMsg));
}

int DUBCmdMgr::getProcessedKSineForReport() const
{
    return lastProcessedBeta;
}

#define EMU_MSG_ID_SWCR_STATUS 1
#define EMU_MSG_ID_DUA_STATUS 2
#define EMU_MSG_ID_DUB_STATUS 3
#define EMU_MSG_ID_TU_STATUS 4
#define EMU_MSG_ID_DCU_STATUS 5

void DUBCmdMgr::handleStatusEmulatorMessage(int msg_id, const int *status, int numData)
{
    (void)numData;

    if (msg_id == EMU_MSG_ID_DUB_STATUS)
    {
        _duHWMgr.processDUEmulatorStatus(status[1]);
    }
    else if (msg_id == EMU_MSG_ID_DCU_STATUS)
    {
        _duHWMgr.processDCUEmulatorStatus(status[1], status[2]);
    }
    else
    {
        printf("Error: this SWCR Status Emulator is not being processed by this component\n");
        _logger.logDebug("Error: this SWCR Status Emulator is not being processed by this component");
    }
}
