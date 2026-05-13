#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <modbus.h>
#include "DUACmdMgr.h"
#include "ShutdownCmdMsg.h"
#include "SteeringCmdMsg.h"
#include "StatusRequestCmdMsg.h"
#include "SWCOverallStatusRptMsg.h"
#include "DCUDetailedStatusRptMsg.h"
#include "SWCDetailedStatusRptMsg.h"
#include "SWCProcessedSteeringWordRptMsg.h"
#include "SWCAckRptMsg.h"  
#include "ConfigDataManager.h"
#include "SAPDataManager.h"
#include "DeviceFactory.h"
#include "ScanLimitCheck.h"
#include "EndianUtils.h"
#include "DeviceUtilities.h"

/**
 * Constructor
 */
DUACmdMgr::DUACmdMgr() :
    MODULE_TYPE(DU_ALPHA),
    _logger(Logger::getInstance()),
    _fromTestServer(NULL),
    _toTestServer(NULL),
    _localHWStatus(NULL),
    _duHWMgr(DUHWMgr::getInstance()),
    _uioDevSL(NULL),
    _uioDevConfig(NULL),
    _timerDevStatus(NULL),
    _udpFromStatusEmu(NULL),
    REFRESH_DCU_STATUS_ON_GDS_INTERVAL(6)
{
}

/**
 * Destructor
 */
DUACmdMgr::~DUACmdMgr()
{
    delete _fromTestServer;
    _fromTestServer = NULL;

    delete _toTestServer;
    _toTestServer = NULL;

    delete _localHWStatus;
    _localHWStatus = NULL;

    delete _uioDevSL;
    _uioDevSL = NULL;

    delete _uioDevConfig;
    _uioDevConfig = NULL;

    delete _timerDevStatus;
    _timerDevStatus = NULL;

    delete _udpFromStatusEmu;
    _udpFromStatusEmu = NULL;
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
    int SEND_ALL_DCU_STATUS;

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
    _logger.logInfo("DUACmdMgr Initializing");

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
    devName << ALPHA_IP_ADDRESS << ":" << FROM_TEST_SERVER_PORT;

    _fromTestServer = new UDPNetworkDevice(NetworkServer, ALPHA_IP_ADDRESS, FROM_TEST_SERVER_PORT, false);
    _fromTestServer->setName(devName.str());

    if (_fromTestServer->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _fromTestServer->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_fromTestServer, READ_EVENT, 1, static_cast<EventFunc>(&DUACmdMgr::processTestServerMsg)) != OK)
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

    // From HW devices - Inbound for DU Alpha
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

    // Incoming from status emulator device
    devName.clear();
    devName << "UDP Server From Status Emulator";
    devName << ALPHA_IP_ADDRESS << ":" << FROM_STATUS_EMULATOR_PORT;

    _udpFromStatusEmu = new UDPNetworkDevice(NetworkServer, ALPHA_IP_ADDRESS, FROM_STATUS_EMULATOR_PORT, false);
    _udpFromStatusEmu->setName(devName.str());

    if (_udpFromStatusEmu->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _udpFromStatusEmu->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_udpFromStatusEmu, READ_EVENT, 1, static_cast<EventFunc>(&DUACmdMgr::processStatusEmuMsg)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _udpFromStatusEmu->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _udpFromStatusEmu device");
    printf("Successfully created _udpFromStatusEmu device\n");

    // Initialize HW Manager
    _duHWMgr.initialize(MODULE_TYPE);

    // Read Alpha DU status
    _duHWMgr.processDUAStatus(_duHWMgr.readDUStatus());

    // Open UIO device for Scan Limit HW Interrupt
    _uioDevSL = new UIODevice(AXI_INT_121_OFFSET, 0);

    if (_uioDevSL->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _uioDevSL->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_uioDevSL, READ_EVENT, 1, static_cast<EventFunc>(&DUACmdMgr::processSLInterrupt)) != OK)
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
    if (addEvent(*_uioDevConfig, READ_EVENT, 1, static_cast<EventFunc>(&DUACmdMgr::processConfigInterrupt)) != OK)
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
    if (addEvent(*_timerDevStatus, READ_EVENT, 1, static_cast<EventFunc>(&DUACmdMgr::processStatusTimer)) != OK)
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

void DUACmdMgr::processSLInterrupt()
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

    // Set DCU status to send to TWGS
    // DCU status is sent to TWGS per action
    // Check DCU status queue to see if there are status to send
    int dequeSize = _duHWMgr.getDCUStatusDequeSize();
    printf("DCU status deque size = %d\n", dequeSize);
    _logger.logDebug("DCU status deque size = %d", dequeSize);
    if (dequeSize > 0)
    {
        DCUStatus status = _duHWMgr.getDCUStatusFromDeque();
        _duHWMgr.setDCUStatusToTwgs(status.group, status.overallStatus, status.loc);
    }

    // Send last processed steering word to Test Server
    if (sendProcessedSW)
    {
        SWCProcessedSteeringWordRptMsg swRptMsg;
        swRptMsg.setModuleType(DU_ALPHA);
        swRptMsg.setProcessedKSine(lastProcessedAlpha);
        swRptMsg.buildMsg();
        int msgSize = swRptMsg.getBufSize();
        swRptMsg.headerByteSwapToNetwork();
        
        _toTestServer->write(swRptMsg.getBuf(), msgSize);
    }
}

void DUACmdMgr::processConfigInterrupt()
{
    size_t bytesRead = 0;
    int pending = 0;

    _uioDevConfig->read((char *)&pending, sizeof(int), bytesRead);
    printf("Reading config changed interrupt, number of interrupt = %d\n", pending);
    _logger.logDebug("Reading config changed interrupt, number of interrupt = %d", pending);
    _uioDevConfig->clearInterrupt();

    _duHWMgr.readSWCStatus(DATA_TYPE_CONFIG_STATUS);

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

void DUACmdMgr::processStatusTimer()
{
    static int timerCounter = 0;

    timerCounter++;

    _logger.logDebug("In processTimer: timerCounter = %d", timerCounter);

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

    // Reset timer
    _timerDevStatus->read();
}

void DUACmdMgr::processTestServerMsg()
{
    size_t bytesRead = 0;

    CommandMessage *msg = new CommandMessage();

    // Read UDP data
    if (_fromTestServer->read(msg->getBuf(), MAX_MSG_SIZE, bytesRead) != OK)
    {
        _logger.logDebug("Error reading from _udpIncoming");
        printf("Error reading from _udpIncoming\n");
        return;
    }

    msg->setTotalMsgSize(bytesRead);
    msg->byteSwapHeaderToLocal();

    _logger.logInfo("DUA ProcessTestServerMsg: Processing incoming messages: msgId = %d", msg->getMsgId());
    printf("DUA ProcessTestServerMsg: Processing incoming messages: msgId = %d\n", msg->getMsgId());

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

            _logger.logInfo("===> STEERING_CMD_MSG_ID: SteeringCmdCounter = %d", SteeringCmdCounter);

            SteeringCmdDataType *params = reinterpret_cast<SteeringCmdDataType *>(cloneSteeringCmdMsg->getDataBufPos());

            printf("Test Src = %d, Alpha = %d, Beta = %d, RLCP = %d, RLSC = %d\n", 
                   params->testSource, params->alpha, params->beta, params->RLCP, params->RLSC);
            _logger.logDebug("Test Src = %d, Alpha = %d, Beta = %d, RLCP = %d, RLSC = %d",
                             params->testSource, params->alpha, params->beta, params->RLCP, params->RLSC);

            // Check for Offline Mode from HW or Test Enabled from HW
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

            _logger.logInfo("===> STATUS_REQUEST_CMD_MSG_ID: StatusRequestCmdCounter = %d", StatusRequestCmdCounter);

            StatusRequestCmdDataType *params = reinterpret_cast<StatusRequestCmdDataType *>(cloneStatusRequestCmdMsg->getDataBufPos());

            printf("requestType = %d, dcuNum = %d\n", params->requestType, params->dcuNum);
            _logger.logDebug("requestType = %d, dcuNum = %d", params->requestType, params->dcuNum);

            if (params->requestType == SWCOverallStatus)
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
                _toTestServer->write(swcOverallStatusRptMsg.getBuf(), sizeof(SWCOverallStatusRptMsg));
            }

            else if ((params->requestType == AlphaDCUDetailedStatus) || (params->requestType == BetaDCUDetailedStatus))
            {
                RFCC_CH type;
                if (params->requestType == AlphaDCUDetailedStatus)
                {
                    type = ALPHA;
                    _logger.logInfo("Sending AlphaDCUDetailedStatus Rpt for DCU %d To Test Server", params->dcuNum);
                    printf("Sending AlphaDCUDetailedStatus Rpt for DCU %d To Test Server\n", params->dcuNum);
                }
                else
                {
                    type = BETA;
                    _logger.logInfo("Sending BetaDCUDetailedStatus Rpt for DCU %d To Test Server", params->dcuNum);
                    printf("Sending BetaDCUDetailedStatus Rpt for DCU %d To Test Server\n", params->dcuNum);
                }
                
                DCUDetailedStatusRptMsg dcuDetailedStatusRptMsg;
                dcuDetailedStatusRptMsg.setDCUStatus(_duHWMgr.getDCUStatusFromSW(type, params->dcuNum));
                dcuDetailedStatusRptMsg.buildMsg();
                int msgSize = dcuDetailedStatusRptMsg.getBufSize();
                dcuDetailedStatusRptMsg.headerByteSwapToNetwork();
                _toTestServer->write(dcuDetailedStatusRptMsg.getBuf(), sizeof(DCUDetailedStatusRptMsg));
            }

            else if (params->requestType == SWCDetailedStatus)
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
                _toTestServer->write(swcDetailedStatusRptMsg.getBuf(), sizeof(SWCDetailedStatusRptMsg));
            }

            else if (params->requestType == StartSendingProcessedSteeringWord)
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

void DUACmdMgr::sendAckToTestServer(SWCAckType ackType)
{
    SWCAckRptMsg ackRptMsg;
    ackRptMsg.setModuleType(MODULE_TYPE);
    ackRptMsg.setAckType(ackType);
    ackRptMsg.buildMsg();
    int msgSize = ackRptMsg.getBufSize();
    ackRptMsg.headerByteSwapToNetwork();
    
    _toTestServer->write(ackRptMsg.getBuf(), msgSize);
}

void DUACmdMgr::processLocalHWStatusMsg()
{
    size_t bytesRead = 0;

    BetaDCUStatusMsg *localStatus;
    localStatus = &_localStatus[localStatusCounter];
    localStatusCounter++;

    if (localStatusCounter >= 4)
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
    else
    {
        // Get message id
        if (localStatus->msgID == BETA_DCU_STATUS)  
        {
            printf("Received %d bytes: Group %d DCU %d\n", bytesRead, localStatus->betaDCUStatus.group, localStatus->betaDCUStatus.loc);
            _logger.logDebug("Received %d bytes: Group %d DCU %d", bytesRead, localStatus->betaDCUStatus.group, localStatus->betaDCUStatus.loc);
            // Update local SW status and add to send queue
            _duHWMgr.processBetaDCUStatus(localStatus->betaDCUStatus);
        }
        if (localStatus->msgID == BETA_DU_STATUS)
        {
            DUTUStatusMsg betaStatus;
            memcpy((DUTUStatusMsg *)&betaStatus, (DUTUStatusMsg *)localStatus, sizeof(DUTUStatusMsg));
            printf("Received %d bytes for Beta DU status: overall %d, ready %d\n", bytesRead, 
                             betaStatus.dutuStatus.overallStatus, betaStatus.dutuStatus.readyStatus);
            _logger.logDebug("Received  %d bytes for Beta DU status: overall %d, ready %d", bytesRead, 
                             betaStatus.dutuStatus.overallStatus, betaStatus.dutuStatus.readyStatus);
            // Update local SW status and add to send queue
            _duHWMgr.processDUBStatus(betaStatus.dutuStatus);
        }
        if (localStatus->msgID == TU_STATUS)
        {
            DUTUStatusMsg tuStatus;
            memcpy((DUTUStatusMsg *)&tuStatus, (DUTUStatusMsg *)localStatus, sizeof(DUTUStatusMsg));
            printf("Received %d bytes for TU status: overall %d, ready %d\n", bytesRead, 
                             tuStatus.dutuStatus.overallStatus, tuStatus.dutuStatus.readyStatus);
            _logger.logDebug("Received  %d bytes for TU status: overall %d, ready %d", bytesRead, 
                             tuStatus.dutuStatus.overallStatus, tuStatus.dutuStatus.readyStatus);
            // Update local SW status and add to send queue
            _duHWMgr.processTUStatus(tuStatus.dutuStatus);
        }
    }
}

#define EMU_MSG_ID_SWCR_STATUS 1
#define EMU_MSG_ID_DUA_STATUS 2
#define EMU_MSG_ID_DUB_STATUS 3
#define EMU_MSG_ID_TU_STATUS 4
#define EMU_MSG_ID_DCU_STATUS 5

void DUACmdMgr::processStatusEmuMsg()
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






