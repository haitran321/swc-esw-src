#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <sys/reboot.h>
#include "DUBCmdMgr.h"
#include "ShutdownCmdMsg.h"
#include "SteeringCmdMsg.h"
#include "StatusRequestCmdMsg.h"
#include "SWCAckRptMsg.h"
#include "SWCProcessedSteeringWordRptMsg.h"
#include "ConfigDataManager.h"
#include "SAPDataManager.h"
#include "DeviceFactory.h"
#include "EndianUtils.h"
#include "ScanLimitCheck.h"
#include "DeviceUtilities.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/**
 * Constructor
 */
DUBCmdMgr::DUBCmdMgr() :
    MODULE_TYPE(DU_BETA),
    _logger(Logger::getInstance()),
    _fromTestServer(NULL),
    _toTestServer(NULL),
    _localHWStatus(NULL),
    _udpFromStatusEmu(NULL),
    _duHWMgr(DUHWMgr::getInstance()),
    _uioDevSL(NULL),
    _uioDevConfig(NULL),
    _timerDevStatus(NULL),
    REFRESH_DCU_STATUS_ON_GDS_INTERVAL(6)
{
}

/**
 * Destructor
 */
DUBCmdMgr::~DUBCmdMgr()
{
    delete _fromTestServer;
    _fromTestServer = NULL;

    delete _toTestServer;
    _toTestServer = NULL;

    delete _localHWStatus;
    _localHWStatus = NULL;

    delete _udpFromStatusEmu;
    _udpFromStatusEmu = NULL;

    delete _uioDevSL;
    _uioDevSL = NULL;

    delete _uioDevConfig;
    _uioDevConfig = NULL;

    delete _timerDevStatus;
    _timerDevStatus = NULL;
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

    SAPDataManager& saps = SAPDataManager::getInstance();

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
    _logger.logInfo("DUBCmdMgr Initializing");

    if (rc == ERROR)
    {
        _logger.logInfo("ERROR: reading config file");
        return (ERROR);
    }

    _logger.logInfo("MODULE_TYPE = %d", MODULE_TYPE);

    stringstream devName;

    // From Test Server device for commands
    devName.clear();
    devName << "UDP Server ";
    devName << BETA_IP_ADDRESS << ":" << FROM_TEST_SERVER_PORT;

    _fromTestServer = new UDPNetworkDevice(NetworkServer, BETA_IP_ADDRESS, FROM_TEST_SERVER_PORT, false);
    _fromTestServer->setName(devName.str());

    if (_fromTestServer->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _fromTestServer->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_fromTestServer, READ_EVENT, 1, static_cast<EventFunc>(&DUBCmdMgr::processTestServerMsg)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _fromTestServer->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _fromTestServer device");
    printf("Successfully created _fromTestServer device\n");

    devName.clear();
    devName << "UDP Client For Test Server";
    devName << TEST_SERVER_IP_ADDRESS << ":" << TO_TEST_SERVER_PORT;
    _toTestServer = new UDPNetworkDevice(NetworkClient, TEST_SERVER_IP_ADDRESS, TO_TEST_SERVER_PORT, false);
    _toTestServer->setName(devName.str());

    if (_toTestServer->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _toTestServer->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _toTestServer device");
    printf("Successfully created _toTestServer device\n");

    // From HW devices - Outbound for DU Beta
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

    // Incoming from status emulator device
    devName.clear();
    devName << "UDP Server From Status Emulator";
    devName << BETA_IP_ADDRESS << ":" << FROM_STATUS_EMULATOR_PORT;

    _udpFromStatusEmu = new UDPNetworkDevice(NetworkServer, BETA_IP_ADDRESS, FROM_STATUS_EMULATOR_PORT, false);
    _udpFromStatusEmu->setName(devName.str());

    if (_udpFromStatusEmu->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _udpFromStatusEmu->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_udpFromStatusEmu, READ_EVENT, 1, static_cast<EventFunc>(&DUBCmdMgr::processStatusEmuMsg)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _udpFromStatusEmu->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _udpFromStatusEmu device");
    printf("Successfully created _udpFromStatusEmu device\n");

    // Initialize HW Manager
    _duHWMgr.initialize(MODULE_TYPE);

    // Read Beta DU status and send to Alpha DU
    DUTUStatusMsg bduStatus;
    bduStatus.msgID = BETA_DU_STATUS;
    bduStatus.dutuStatus = _duHWMgr.readDUStatus();
    sendDUBStatusToDUA(bduStatus);
    _logger.logInfo("Sent DUB status to DUA");
    usleep(1*1000);   // Sleep 1 msecs

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
            usleep(1*1000);   // Sleep 1 msecs
        }
    }
    _logger.logInfo("Sent DUB DCUs status to DUA");

    // Open UIO device for Scan Limit HW Interrupt
    _uioDevSL = new UIODevice(AXI_INT_121_OFFSET, 0);

    if (_uioDevSL->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _uioDevSL->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_uioDevSL, READ_EVENT, 1, static_cast<EventFunc>(&DUBCmdMgr::processSLInterrupt)) != OK)
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
    _uioDevConfig = new UIODevice(AXI_INT_122_OFFSET, 1);

    if (_uioDevConfig->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _uioDevConfig->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_uioDevConfig, READ_EVENT, 1, static_cast<EventFunc>(&DUBCmdMgr::processConfigInterrupt)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _uioDevConfig->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _uioDevConfig device");
    printf("Successfully created _uioDevConfig device\n");

    // Map UIO address
    _uioDevConfig->mmap();
    _uioDevConfig->clearInterrupt();

    // Status Timer
    timespec init = { STATUS_TIMER_INTERVAL_SECONDS, 0 };
    timespec timeout = { STATUS_TIMER_INTERVAL_SECONDS, 0 };
    _timerDevStatus = new TimerDevice(init, timeout);

    if (_timerDevStatus->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _timerDevStatus->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_timerDevStatus, READ_EVENT, 1, static_cast<EventFunc>(&DUBCmdMgr::processStatusTimer)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _timerDevStatus->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _timerDevStatus device");
    printf("Successfully created _timerDevStatus device\n");

    // Notify Test Server
    sendAckToTestServer(InitCompleteAck);

    EventProcessor::start();

    return OK;
}

void DUBCmdMgr::processSLInterrupt()
{
    size_t bytesRead = 0;
    int pending = 0;

    _uioDevSL->read((char *)&pending, sizeof(int), bytesRead);
    printf("Reading scan limit interrupt, number of interrupt = %d\n", pending);
    _logger.logDebug("Reading scan limit interrupt, number of interrupt = %d", pending);
    _uioDevSL->clearInterrupt();

    int armAlpha = _duHWMgr.getArmKSine(ALPHA);
    // 10-bit signed 2-complement integer
    if ((armAlpha & 0x200) != 0)
    {
       armAlpha |= 0xfffffc00;
    }

    int armBeta = _duHWMgr.getArmKSine(BETA);
    // 10-bit signed 2-complement integer
    if ((armBeta & 0x200) != 0)
    {
       armBeta |= 0xfffffc00;
    }

    int atbAlpha = _duHWMgr.getAtbKSine(ALPHA);
    // 10-bit signed 2-complement integer
    if ((atbAlpha & 0x200) != 0)
    {
       atbAlpha |= 0xfffffc00;
    }

    int atbBeta = _duHWMgr.getAtbKSine(BETA);
    // 10-bit signed 2-complement integer
    if ((atbBeta & 0x200) != 0)
    {
       atbBeta |= 0xfffffc00;
    }

    printf("atbAlpha = %d, atbBeta = %d, armAlpha = %d, armBeta = %d\n", atbAlpha, atbBeta, armAlpha, armBeta);
    _logger.logDebug("atbAlpha = %d, atbBeta = %d, armAlpha = %d, armBeta = %d", atbAlpha, atbBeta, armAlpha, armBeta);

    int armSWSLResult = runSWScanLimitCheck(float(atbAlpha), float(armBeta));
    int atbSWSLResult = runSWScanLimitCheck(float(atbAlpha), float(atbBeta));
    int fwSLResult = _duHWMgr.getFWScanLimitCheckStatus();

    printf("fwSLResult = 0x%x(%d), atbSWSLResult = %d, armSWSLResult = %d\n", fwSLResult, fwSLResult & 0x1, atbSWSLResult, armSWSLResult);
    _logger.logDebug("fwSLResult = 0x%x(%d), atbSWSLResult = %d, armSWSLResult = %d", fwSLResult, fwSLResult & 0x1, atbSWSLResult, armSWSLResult);

    // Set last K Sine processed
    lastProcessedAlpha = atbAlpha;
    lastProcessedBeta = atbBeta;

    // Get last SPI transfer status
    if (_duHWMgr.getOverallSPIStatus() == FAILED)
    {
        printf("Last SPI transfer status has no failures.\n");
        _logger.logDebug("Last SPI transfer status has no failures.");
    }
    else
    {
        printf("Last SPI transfer status has failures.\n");
        _logger.logDebug("Last SPI transfer status has failures.");

        // Determine what DCU reported CRC failures

    }

    // Check DCU status queue to see if there are status to send
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
            usleep(1*1000);   // Sleep 1 msecs
        }
    }

    // Send last processed steering word to Test Server
    if (sendProcessedSW)
    {
        SWCProcessedSteeringWordRptMsg swRptMsg;
        swRptMsg.setModuleType(DU_BETA);
        swRptMsg.setProcessedKSine(lastProcessedBeta);
        swRptMsg.buildMsg();
        int msgSize = swRptMsg.getBufSize();
        swRptMsg.headerByteSwapToNetwork();
        
        _toTestServer->write(swRptMsg.getBuf(), msgSize);
    }
}

void DUBCmdMgr::processConfigInterrupt()
{
    size_t bytesRead = 0;
    int pending = 0;

    _uioDevConfig->read((char *)&pending, sizeof(int), bytesRead);
    printf("Reading config changed interrupt, number of interrupt = %d\n", pending);
    _logger.logDebug("Reading config changed interrupt, number of interrupt = %d", pending);
    _uioDevConfig->clearInterrupt();

    // Get mode
    if (_duHWMgr.getSWCModeStatus() == ONLINE)
    {
        // Reset test source back 0 (TU = 0)
        // Should already be 0 but just to be safe 
        _duHWMgr.setTestSrcInTestMode(Analog);
    }

    // Does CmdMgr need to set anything based on config?  
    // Do it here
}

void DUBCmdMgr::processStatusTimer()
{
    static int timerCounter = 0;

    timerCounter++;

    _logger.logDebug("In processTimer: timerCounter = %d", timerCounter);

    // Read Beta DU status and send to Alpha DU
    DUTUStatusMsg bduStatus;
    bduStatus.msgID = BETA_DU_STATUS;
    bduStatus.dutuStatus = _duHWMgr.readDUStatus();
    sendDUBStatusToDUA(bduStatus);
    usleep(1*1000);   // Sleep 1 msecs

    printf("Read all DCUs status to update local queue.\n");
    _logger.logDebug("Read all DCUs status to update local queue.");
    if (timerCounter % REFRESH_DCU_STATUS_ON_GDS_INTERVAL == 0)
    {
        // Read DCU status and send current DCU status list
        _duHWMgr.readDCUStatus(true);
    }
    else
    {
        // Only read dCU status
        _duHWMgr.readDCUStatus();
    }

    int dequeSize = _duHWMgr.getDCUStatusDequeSize();
    printf("DCU status deque size = %d\n", dequeSize);
    _logger.logDebug("DCU status deque size = %d", dequeSize);

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
            usleep(1*1000);   // Sleep 1 msecs
        }
    }

    // Reset timer
    _timerDevStatus->read();
}

void DUBCmdMgr::processTestServerMsg()
{
    size_t bytesRead = 0;

    CommandMessage *msg = new CommandMessage();

    // Read UDP data
    if (_fromTestServer->read(msg->getBuf(), MAX_MSG_SIZE, bytesRead) != OK)
    {
        _logger.logDebug("error reading from _udpIncoming");
        printf("error reading from _udpIncoming\n");
        return;
    }

    msg->setTotalMsgSize(bytesRead);
    msg->byteSwapHeaderToLocal();

    _logger.logInfo("DUB ProcessTestServerMsg: Processing incoming messages: msgId = %d", msg->getMsgId());
    printf("DUB ProcessTestServerMsg: Processing incoming messages: msgId = %d\n", msg->getMsgId());

    switch (msg->getMsgId())
    {
    case SHUTDOWN_CMD_MSG_ID:
        {
            ShutdownCmdMsg *cloneShutdownMsg = new ShutdownCmdMsg(msg->getBuf(), msg->getBufSize());
            cloneShutdownMsg->byteSwapToLocal();
            _logger.logInfo("In SHUTDOWN_CMD_MSG_ID case: shutdown option = %d", cloneShutdownMsg->getType());

            // Notify Test Server
            sendAckToTestServer(ShutdownCmdAck);

            if (cloneShutdownMsg->getType() == PowerOff)
            {
                printf("****Calling System Shutdown****\n");
                _logger.logInfo("****Calling System Shutdown****");
                sleep(3);
//              reboot(RB_POWER_OFF);
            }
            else
            {
                printf("****Calling System Reboot****\n");
                _logger.logInfo("****Calling System Reboot****");
                sleep(3);
                reboot(RB_AUTOBOOT);
            }
            break;
        }
    case STEERING_CMD_MSG_ID:
        {
            SteeringCmdMsg *cloneSteeringCmdMsg = new SteeringCmdMsg(msg->getBuf(), msg->getBufSize());
            cloneSteeringCmdMsg->byteSwapToLocal();
            int numActions = cloneSteeringCmdMsg->getDataSize() / sizeof(SteeringCmdDataType);

            static int SteeringCmdCounter = 0;
            SteeringCmdCounter++;

            printf("===> STEERING_CMD_MSG_ID: SteeringCmdCounter = %d\n", SteeringCmdCounter);
            _logger.logInfo("===> STEERING_CMD_MSG_ID: SteeringCmdCounter = %d", SteeringCmdCounter);

            SteeringCmdDataType *params = reinterpret_cast<SteeringCmdDataType *>(cloneSteeringCmdMsg->getDataBufPos());

            printf("Test Src = %d, Alpha = %d, Beta = %d, RLCP = %d, RLSC = %d\n", 
                   params->testSource, params->alpha, params->beta, params->RLCP, params->RLSC);
            _logger.logDebug("Test Src = %d, Alpha = %d, Beta = %d, RLCP = %d, RLSC = %d",
                             params->testSource, params->alpha, params->beta, params->RLCP, params->RLSC);

            // Check for Offline Mode from HW or Test Enabled from HW or Force Test Mode from Config File
            // Force Test Mode from Config File set SWCR mode to offline
            if ((_duHWMgr.getSWCModeStatus() == OFFLINE) || (_duHWMgr.getOLTEModeStatus() == OFFLINE) || (FORCE_TEST_MODE == TEST))
            {
                // Set KSine Regs
                if (params->testSource == Digital)
                {
                    // Set test source based on command
                    _duHWMgr.setTestSrcInTestMode(params->testSource);

                    _duHWMgr.setArmKSine(ALPHA, params->alpha);
                    _duHWMgr.setArmKSine(BETA, params->beta);

                    if (params->RLCP == On)
                    {
                        _logger.logDebug("Setting RLCP to On");
                        _duHWMgr.sendSteeringWordValidFlagInTestMode(STEERING_WORD_INVALID);
                        _duHWMgr.sendDCUCmdInTestMode(DCU_CMD_BORESIGHT);
                    }
                    if (params->RLSC == On)
                    {
                        _logger.logDebug("Setting RLSC to On");
                        _duHWMgr.sendSteeringWordValidFlagInTestMode(STEERING_WORD_INVALID);
                        _duHWMgr.sendDCUCmdInTestMode(DCU_CMD_CALIBRATION);
                    }

                    // Toggle SW trigger in place of /RLTD
                    _duHWMgr.toggleSWTrigger();

                    // Reset Steering Word valid flag back to valid for the next command
                    _duHWMgr.sendSteeringWordValidFlagInTestMode(STEERING_WORD_VALID);

                    // Reset test source back 0 (TU = 0)
                    _duHWMgr.setTestSrcInTestMode(Analog);
                }
            }

            break;
        }
    case STATUS_REQUEST_CMD_MSG_ID:
        {
            StatusRequestCmdMsg *cloneStatusRequestCmdMsg = new StatusRequestCmdMsg(msg->getBuf(), msg->getBufSize());
            cloneStatusRequestCmdMsg->byteSwapToLocal();
            int numActions = cloneStatusRequestCmdMsg->getDataSize() / sizeof(StatusRequestCmdDataType);

            static int StatusRequestCmdCounter = 0;
            StatusRequestCmdCounter++;

            printf("===> STATUS_REQUEST_CMD_MSG_ID: StatusRequestCmdCounter = %d\n", StatusRequestCmdCounter);
            _logger.logInfo("===> STATUS_REQUEST_CMD_MSG_ID: StatusRequestCmdCounter = %d", StatusRequestCmdCounter);

            StatusRequestCmdDataType *params = reinterpret_cast<StatusRequestCmdDataType *>(cloneStatusRequestCmdMsg->getDataBufPos());

            printf("requestType = %d, dcuNum = %d\n", params->requestType, params->dcuNum);
            _logger.logDebug("requestType = %d, dcuNum = %d", params->requestType, params->dcuNum);

            if (params->requestType == StartSendingProcessedSteeringWord)
            {
                sendProcessedSW = true;
            }

            else if (params->requestType == StopSendingProcessedSteeringWord)
            {
                sendProcessedSW = false;
            }

            else
            {
                printf("ERROR: Invalid status request of %d\n", params->requestType);
                _logger.logError("ERROR: Invalid status request of %d", params->requestType);
            }

            break;
        }
        // printf("Successfully write from readUdpData\n");
    }
}

void DUBCmdMgr::sendAckToTestServer(SWCAckType ackType)
{
    SWCAckRptMsg ackRptMsg;
    ackRptMsg.setModuleType(MODULE_TYPE);
    ackRptMsg.setAckType(ackType);
    ackRptMsg.buildMsg();
    int msgSize = ackRptMsg.getBufSize();
    ackRptMsg.headerByteSwapToNetwork();
    
    _toTestServer->write(ackRptMsg.getBuf(), msgSize);
}

void DUBCmdMgr::sendDCUStatusToDUA(BetaDCUStatusMsg status)
{
    _localHWStatus->write(&status, sizeof(BetaDCUStatusMsg));
}

void DUBCmdMgr::sendDUBStatusToDUA(DUTUStatusMsg status)
{
    _localHWStatus->write(&status, sizeof(DUTUStatusMsg));
}

#define EMU_MSG_ID_SWCR_STATUS 1
#define EMU_MSG_ID_DUA_STATUS 2
#define EMU_MSG_ID_DUB_STATUS 3
#define EMU_MSG_ID_TU_STATUS 4
#define EMU_MSG_ID_DCU_STATUS 5

void DUBCmdMgr::processStatusEmuMsg()
{
    size_t bytesRead = 0;

    int num_data = 14;
    int status[num_data];

    // Read UDP data
    if (_udpFromStatusEmu->read((char *)&status[0], sizeof(int)*num_data, bytesRead) != OK)
    {
        printf("Error reading from _udpFromStatusEmu\n");
        _logger.logDebug("Error reading from _udpFromStatusEmu");
        return;
    }

    int msg_id = status[0];
    printf("Received data from SWCR Status Emulator: msg id = %d\n", msg_id);
    _logger.logDebug("Received data from SWCR Status Emulator: msg id = %d", msg_id);

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
