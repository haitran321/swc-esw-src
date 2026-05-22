#include <stdio.h>
#include <sstream>
#include <string.h>

#include "DUACmdMgr.h"
#include "SWCOverallStatusRptMsg.h"
#include "DCUDetailedStatusRptMsg.h"
#include "SWCDetailedStatusRptMsg.h"
#include "ConfigDataManager.h"
#include "SAPDataManager.h"

/**
 * Constructor
 */
DUACmdMgr::DUACmdMgr() :
    DUCmdMgrBase(DU_ALPHA),
    _localHWStatus(NULL),
    localStatusCounter(0)
{
}

/**
 * Destructor
 */
DUACmdMgr::~DUACmdMgr()
{
    delete _localHWStatus;
    _localHWStatus = NULL;
}

/** 
 * Creates, opens and registers events
 * Call the base class start()
 */
STATUS DUACmdMgr::start()
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

    // Setup Logger
    _logger.initialize();
    _logger.logInfo("DUACmdMgr Initializing");

    if (rc == ERROR)
    {
        _logger.logInfo("ERROR: reading config file");
        return (ERROR);
    }

    _logger.logInfo("MODULE_TYPE = %d", _moduleType);

    if (initializeCommonCommandDevices(ALPHA_IP_ADDRESS, TEST_SERVER_IP_ADDRESS,
                                       FROM_TEST_SERVER_PORT, TO_TEST_SERVER_PORT,
                                       FROM_STATUS_EMULATOR_PORT, STATUS_TIMER_INTERVAL_SECONDS) != OK)
    {
        return ERROR;
    }

    // From HW devices - Inbound for DU Alpha
    stringstream devName;
    devName.clear();
    devName << "UDP Server ";
    devName << ALPHA_IP_ADDRESS << ":" << LOCAL_HW_STATUS_PORT;

    _localHWStatus = new UDPNetworkDevice(NetworkServer, ALPHA_IP_ADDRESS, LOCAL_HW_STATUS_PORT, false);
    _localHWStatus->setName(devName.str());

    if (_localHWStatus->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _localHWStatus->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_localHWStatus, READ_EVENT, 1, static_cast<EventFunc>(&DUACmdMgr::processLocalHWStatusMsg)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _localHWStatus->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _localHWStatus device");
    printf("Successfully created _localHWStatus device\n");

    // Initialize HW Manager
    _duHWMgr.initialize(_moduleType);

    // Read Alpha DU status
    _duHWMgr.processDUAStatus(_duHWMgr.readDUStatus());

    if (initializeDUCommonDevices() != OK)
    {
        return ERROR;
    }

    // Notify Test Server
    _logger.logInfo("%s sending InitCompleteAck Test Server msg 1", getCommandMgrName());
    sendAckToTestServer(InitCompleteAck);

    EventProcessor::start();

    return OK;
}

void DUACmdMgr::handlePendingDcuStatusAfterScanLimit()
{
    int dequeSize = _duHWMgr.getDCUStatusDequeSize();
    printf("DCU status deque size = %d\n", dequeSize);
    _logger.logDebug("DCU status deque size = %d", dequeSize);
    if (dequeSize > 0)
    {
        DCUStatus status = _duHWMgr.getDCUStatusFromDeque();
        _duHWMgr.setDCUStatusToTwgs(status.group, status.overallStatus, status.loc);
    }
}

void DUACmdMgr::handleConfigInterruptRefresh()
{
    _duHWMgr.readSWCStatus(DATA_TYPE_CONFIG_STATUS);
}

void DUACmdMgr::handlePreDcuStatusTimer()
{
    // Alternate status between custom components and COTS
    static int statusCounter = 0;
    if (statusCounter == 0)
    {
        _duHWMgr.readSWCStatus(DATA_TYPE_CUSTOM_STATUS);
        _logger.logDebug("DATA_TYPE_CUSTOM_STATUS, calcStatus bits: 0x%x", _duHWMgr.getSwcStatusToTwgs());
        printf("DATA_TYPE_CUSTOM_STATUS, calcStatus bits: 0x%x\n", _duHWMgr.getSwcStatusToTwgs());
    }
    else if (statusCounter == 1)
    {
        _duHWMgr.readSWCStatus(DATA_TYPE_IO_MODULE_STATUS);
        _logger.logDebug("DATA_TYPE_IO_MODULE_STATUS, calcStatus bits: 0x%x", _duHWMgr.getSwcStatusToTwgs());
        printf("DATA_TYPE_IO_MODULE_STATUS, calcStatus bits: 0x%x\n", _duHWMgr.getSwcStatusToTwgs());
    }
    else
    {
        _duHWMgr.readSWCStatus(DATA_TYPE_CONFIG_STATUS);
        _logger.logDebug("DATA_TYPE_CONFIG_STATUS, calcStatus bits: 0x%x", _duHWMgr.getSwcStatusToTwgs());
        printf("DATA_TYPE_CONFIG_STATUS, calcStatus bits: 0x%x\n", _duHWMgr.getSwcStatusToTwgs());
    }
    statusCounter++;
    if (statusCounter >= 3)
    {
        statusCounter = 0;
    }

    // Read Alpha DU status
    _duHWMgr.processDUAStatus(_duHWMgr.readDUStatus());
}

const char *DUACmdMgr::getCommandMgrName() const
{
    return "DUA";
}

void DUACmdMgr::handleDUStatusRequest(const StatusRequestCmdDataType& params)
{
    if (params.requestType == SWCOverallStatus)
    {
        _logger.logInfo("Sending SWCDetailedStatus Rpt To Test Server");
        SWCOverallStatusDataType swcStatus = _duHWMgr.getSWCStatus();
        SWCOverallStatusRptMsg swcOverallStatusRptMsg;
        swcOverallStatusRptMsg.setSWCStatus(swcStatus.swcStatus);
        swcOverallStatusRptMsg.setSWCConfig(swcStatus.swcConfig);
        swcOverallStatusRptMsg.setSWCMode(swcStatus.swcMode);
        swcOverallStatusRptMsg.setOLTEMode(swcStatus.olteMode);
        swcOverallStatusRptMsg.setAlphaDUStatus(swcStatus.swcAlphaDUStatus);
        swcOverallStatusRptMsg.setBetaDUStatus(swcStatus.swcBetaDUStatus);
        swcOverallStatusRptMsg.setAlphaDCURolledUpStatus(swcStatus.swcAlphaDCURolledUpStatus);
        swcOverallStatusRptMsg.setBetaDCURolledUpStatus(swcStatus.swcBetaDCURolledUpStatus);
        swcOverallStatusRptMsg.setTempStatus(swcStatus.swcTempStatus);
        swcOverallStatusRptMsg.set12VPwrStatus(swcStatus.swc12VPwrStatus);
        swcOverallStatusRptMsg.set24VPwrStatus(swcStatus.swc24VPwrStatus);
        swcOverallStatusRptMsg.setATBStatus(swcStatus.swcATBStatus);
        swcOverallStatusRptMsg.setTUHWStatus(swcStatus.testUnitHWStatus);
        for (int dcu = 0; dcu < NUM_DCU; dcu++)
        {
            swcOverallStatusRptMsg.setAlphaDCUStatus(dcu, swcStatus.alphaDCU[dcu]);
        }

        for (int dcu = 0; dcu < NUM_DCU; dcu++)
        {
            swcOverallStatusRptMsg.setBetaDCUStatus(dcu, swcStatus.betaDCU[dcu]);
        }
        swcOverallStatusRptMsg.setLastProcessedAlpha(lastProcessedAlpha);
        swcOverallStatusRptMsg.setLastProcessedBeta(lastProcessedBeta);
        swcOverallStatusRptMsg.buildMsg();
        int msgSize = swcOverallStatusRptMsg.getBufSize();
        swcOverallStatusRptMsg.headerByteSwapToNetwork();
        _toTestServer->write(swcOverallStatusRptMsg.getBuf(), msgSize);
    }
    else if ((params.requestType == AlphaDCUDetailedStatus) || (params.requestType == BetaDCUDetailedStatus))
    {
        RFCC_CH type;
        if (params.requestType == AlphaDCUDetailedStatus)
        {
            type = ALPHA;
            _logger.logInfo("Sending AlphaDCUDetailedStatus Rpt for DCU %d To Test Server", params.dcuNum);
            printf("Sending AlphaDCUDetailedStatus Rpt for DCU %d To Test Server\n", params.dcuNum);
        }
        else
        {
            type = BETA;
            _logger.logInfo("Sending BetaDCUDetailedStatus Rpt for DCU %d To Test Server", params.dcuNum);
            printf("Sending BetaDCUDetailedStatus Rpt for DCU %d To Test Server\n", params.dcuNum);
        }

        DCUDetailedStatusRptMsg dcuDetailedStatusRptMsg;
        dcuDetailedStatusRptMsg.setDCUStatus(_duHWMgr.getDCUStatusFromSW(type, params.dcuNum));
        dcuDetailedStatusRptMsg.buildMsg();
        int msgSize = dcuDetailedStatusRptMsg.getBufSize();
        dcuDetailedStatusRptMsg.headerByteSwapToNetwork();
        _toTestServer->write(dcuDetailedStatusRptMsg.getBuf(), msgSize);
    }
    else if (params.requestType == SWCDetailedStatus)
    {
        _logger.logDebug("Received SWCDetailedStatus request");

        SWCDetailedStatusDataType swcDetailedStatus;

        swcDetailedStatus.swcStatus = _duHWMgr.getSWCOverallStatus();

        swcDetailedStatus.alphaDUStatus = _duHWMgr.getDUAStatus();
        _logger.logDebug("DUA: %d, %d, %d, %d, %d, %d, %d",
               swcDetailedStatus.alphaDUStatus.overallStatus,
               swcDetailedStatus.alphaDUStatus.readyStatus,
               swcDetailedStatus.alphaDUStatus.highTempAlarm,
               swcDetailedStatus.alphaDUStatus.overTempAlarm,
               swcDetailedStatus.alphaDUStatus.vccintAlarm,
               swcDetailedStatus.alphaDUStatus.vccauxAlarm,
               swcDetailedStatus.alphaDUStatus.vbramAlarm);
        swcDetailedStatus.betaDUStatus = _duHWMgr.getDUBStatus();
        _logger.logDebug("DUB: %d, %d, %d, %d, %d, %d, %d",
               swcDetailedStatus.betaDUStatus.overallStatus,
               swcDetailedStatus.betaDUStatus.readyStatus,
               swcDetailedStatus.betaDUStatus.highTempAlarm,
               swcDetailedStatus.betaDUStatus.overTempAlarm,
               swcDetailedStatus.betaDUStatus.vccintAlarm,
               swcDetailedStatus.betaDUStatus.vccauxAlarm,
               swcDetailedStatus.betaDUStatus.vbramAlarm);
        swcDetailedStatus.tuStatus = _duHWMgr.getTUStatus();

        SWCDetailedStatusRptMsg swcDetailedStatusRptMsg;
        swcDetailedStatusRptMsg.setSWCDetailedStatus(swcDetailedStatus);
        swcDetailedStatusRptMsg.buildMsg();
        int msgSize = swcDetailedStatusRptMsg.getBufSize();
        swcDetailedStatusRptMsg.headerByteSwapToNetwork();
        _toTestServer->write(swcDetailedStatusRptMsg.getBuf(), msgSize);
    }
    else
    {
        printf("ERROR: Invalid status request of %d\n", params.requestType);
        _logger.logError("ERROR: Invalid status request of %d", params.requestType);
    }
}

int DUACmdMgr::getProcessedKSineForReport() const
{
    return lastProcessedAlpha;
}

void DUACmdMgr::processLocalHWStatusMsg()
{
    size_t bytesRead = 0;
    const int localStatusBufferCount = sizeof(_localStatus) / sizeof(_localStatus[0]);
    BetaDCUStatusMsg *localStatus = &_localStatus[localStatusCounter];
    localStatusCounter++;

    if (localStatusCounter >= localStatusBufferCount)
    {
        localStatusCounter = 0;
    }

    // Read UDP data
    if (_localHWStatus->read((char *)localStatus, sizeof(BetaDCUStatusMsg), bytesRead) != OK)
    {
        printf("Error reading from _localHWStatus\n");
        _logger.logDebug("Error reading from _localHWStatus");
        return;
    }

    // Get message id
    if (localStatus->msgID == BETA_DCU_STATUS)
    {
        printf("Received %d bytes: Group %d DCU %d\n", bytesRead, localStatus->betaDCUStatus.group, localStatus->betaDCUStatus.loc);
        _logger.logDebug("Received %d bytes: Group %d DCU %d", bytesRead, localStatus->betaDCUStatus.group, localStatus->betaDCUStatus.loc);
        // Update local SW status and add to send queue
        _duHWMgr.processBetaDCUStatus(localStatus->betaDCUStatus);
    }
    else if (localStatus->msgID == BETA_DU_STATUS)
    {
        DUTUStatusMsg betaStatus;
        memcpy(&betaStatus, localStatus, sizeof(DUTUStatusMsg));
        printf("Received %d bytes for Beta DU status: overall %d, ready %d\n", bytesRead,
               betaStatus.dutuStatus.overallStatus, betaStatus.dutuStatus.readyStatus);
        _logger.logDebug("Received  %d bytes for Beta DU status: overall %d, ready %d", bytesRead,
                         betaStatus.dutuStatus.overallStatus, betaStatus.dutuStatus.readyStatus);
        // Update local SW status and add to send queue
        _duHWMgr.processDUBStatus(betaStatus.dutuStatus);
    }
    else if (localStatus->msgID == TU_STATUS)
    {
        DUTUStatusMsg tuStatus;
        memcpy(&tuStatus, localStatus, sizeof(DUTUStatusMsg));
        printf("Received %d bytes for TU status: overall %d, ready %d\n", bytesRead,
               tuStatus.dutuStatus.overallStatus, tuStatus.dutuStatus.readyStatus);
        _logger.logDebug("Received  %d bytes for TU status: overall %d, ready %d", bytesRead,
                         tuStatus.dutuStatus.overallStatus, tuStatus.dutuStatus.readyStatus);
        // Update local SW status and add to send queue
        _duHWMgr.processTUStatus(tuStatus.dutuStatus);
    }
}

#define EMU_MSG_ID_SWCR_STATUS 1
#define EMU_MSG_ID_DUA_STATUS 2
#define EMU_MSG_ID_DUB_STATUS 3
#define EMU_MSG_ID_TU_STATUS 4
#define EMU_MSG_ID_DCU_STATUS 5

void DUACmdMgr::handleStatusEmulatorMessage(int msg_id, const int *status, int numData)
{
    (void)numData;

    if (msg_id == EMU_MSG_ID_SWCR_STATUS)
    {
        _duHWMgr.processSWCREmulatorStatus(SWC_CONFIG(status[1]),
                                           SWC_MODE(status[2]),
                                           SWC_MODE(status[3]),
                                           HealthState(status[4]),
                                           HealthState(status[5]),
                                           HealthState(status[6]),
                                           HealthState(status[7]),
                                           DCURolledUpStatus(status[8]),
                                           DCURolledUpStatus(status[9]));
    }
    else if (msg_id == EMU_MSG_ID_DUA_STATUS)
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

