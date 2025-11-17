/**
* $Id: DUACmdMgr.cpp 6638 2011-03-10 23:07:51Z ste38548 $ 
*/
#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <sys/reboot.h>
#include "DUACmdMgr.h"
#include "ShutdownCmdMsg.h"
#include "SteeringCmdMsg.h"
#include "StatusRequestCmdMsg.h"
#include "SWCOverallStatusRptMsg.h"
#include "DCUDetailedStatusRptMsg.h"
#include "SWCDetailedStatusRptMsg.h"
#include "SWCAckRptMsg.h"
#include "ConfigDataManager.h"
#include "DeviceFactory.h"
#include "ScanLimitCheck.h"
#include "EndianUtils.h"
#include "DeviceUtilities.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//#define PRINT_DEBUG

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
    _udpFromDevPC(NULL)
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

    delete _udpFromDevPC;
    _udpFromDevPC = NULL;
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

    // For Dev PC  - TO BE REMOVED
    string DEV_PC_IP_ADDRESS;
    int FROM_DEV_PC_PORT;
    int TO_DEV_PC_PORT;
    rc = rc || configs.get("DEV_PC_IP_ADDRESS", DEV_PC_IP_ADDRESS);
    rc = rc || configs.get("FROM_DEV_PC_PORT", FROM_DEV_PC_PORT);
    rc = rc || configs.get("TO_DEV_PC_PORT", TO_DEV_PC_PORT);

    // Configuration parameters
    rc = rc || configs.get("FORCE_TEST_MODE", FORCE_TEST_MODE);
    rc = rc || configs.get("STEERING_WORD_SRC", STEERING_WORD_SRC);

    // Status parameters
    rc = rc || configs.get("STATUS_TIMER_INTERVAL_SECONDS", STATUS_TIMER_INTERVAL_SECONDS);

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

    // Incoming from Dev PC device
    devName.clear();
    devName << "UDP Server From Dev PC";
    devName << ALPHA_IP_ADDRESS << ":" << FROM_DEV_PC_PORT;

    _udpFromDevPC = new UDPNetworkDevice(NetworkServer, ALPHA_IP_ADDRESS, FROM_DEV_PC_PORT, false);
    _udpFromDevPC->setName(devName.str());

    if (_udpFromDevPC->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _udpFromDevPC->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_udpFromDevPC, READ_EVENT, 1, static_cast<EventFunc>(&DUACmdMgr::processDEVPCMsg)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _udpFromDevPC->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _udpFromDevPC device");
    printf("Successfully created _udpFromDevPC device\n");

    // Initialize HW Manager
    _duHWMgr.initialize(MODULE_TYPE);

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

    EventProcessor::start();

    return OK;
}

void DUACmdMgr::processSLInterrupt()
{
//  eInterruptProcessing.reset();
//  eInterruptProcessing.start();

    uint64_t startTimeNSec = ts.GetNanoSecondsSinceMidnight();

#ifdef PRINT_DEBUG
    printf("In processSLInterrupt()\n");
#endif

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
    if (STEERING_WORD_SRC == ARM)
    {
        lastAlpha = armAlpha;
        lastBeta = armBeta;
    }
    else
    {
        lastAlpha = atbAlpha;
        lastBeta = atbBeta;
    }

    // Set DCU status to send to TWGS
    // DCU status is sent to TWGS per action
    // Check DCU status queue to see if there are status to send
    int queueSize = _duHWMgr.getDCUStatusQueueSize();
    printf("DCU status queue size = %d\n", queueSize);
    _logger.logDebug("DCU status queue size = %d", queueSize);
    if (queueSize > 0)
    {
        DCUStatusParamsType status = _duHWMgr.getDCUStatusFromQueue();
        _duHWMgr.setDCUStatusToTwgs(status.group, status.dcuStatus.overallStatus, status.number);
    }

//  eInterruptProcessing.stop();
//  printf("SW Scan Limit Check took %f\n", eInterruptProcessing.secs());

    uint64_t stopTimeNSec = ts.GetNanoSecondsSinceMidnight();
//  printf("SW Scan Limit Check took %ld\n", stopTimeNSec - startTimeNSec);

}

void DUACmdMgr::processConfigInterrupt()
{
#ifdef PRINT_DEBUG
    printf("In processConfigInterrupt()\n");
#endif

    size_t bytesRead = 0;
    int pending = 0;

    _uioDevConfig->read((char *)&pending, sizeof(int), bytesRead);
    printf("Reading config changed interrupt, number of interrupt = %d\n", pending);
    _logger.logDebug("Reading config changed interrupt, number of interrupt = %d", pending);
    _uioDevConfig->clearInterrupt();

    _duHWMgr.readSWCStatus(DATA_TYPE_CONFIG_STATUS);
}

void DUACmdMgr::processStatusTimer()
{
    static int timerCounter = 0;

    timerCounter++;

//  if (timerCounter % 100 == 0)
//  {
        printf("In processTimer: timerCounter = %d\n", timerCounter);
//  }

    // Alternalte status between custom components and COTS
    static int statusCounter = 0;
    if (statusCounter == 0)
    {
        _duHWMgr.readSWCStatus(DATA_TYPE_CUSTOM_STATUS);
        printf("DATA_TYPE_CUSTOM_STATUS, calcStatus bits: 0x%x\n", _duHWMgr.getSwcStatusToTwgs());
    }
    else if (statusCounter == 1)
    {
        _duHWMgr.readSWCStatus(DATA_TYPE_IO_MODULE_STATUS);
        printf("DATA_TYPE_IO_MODULE_STATUS, calcStatus bits: 0x%x\n", _duHWMgr.getSwcStatusToTwgs());
    }
    else
    {
        _duHWMgr.readSWCStatus(DATA_TYPE_CONFIG_STATUS);
        printf("DATA_TYPE_CONFIG_STATUS, calcStatus bits: 0x%x\n", _duHWMgr.getSwcStatusToTwgs());
    }
    statusCounter++;
    if (statusCounter >= 3)
    {
        statusCounter = 0;
    }

    printf("Read all DCUs status to update local queue.\n");
    _logger.logDebug("Read all DCUs status to update local queue.");
    _duHWMgr.readDCUStatus();

    // For testing to be removed
    int queueSize = _duHWMgr.getDCUStatusQueueSize();
    printf("DCU status queue size = %d\n", queueSize);
    _logger.logDebug("DCU status queue size = %d", queueSize);
    if (queueSize > 0)
    {
        DCUStatusParamsType status = _duHWMgr.getDCUStatusFromQueue();
        _duHWMgr.setDCUStatusToTwgs(status.group, status.dcuStatus.overallStatus, status.number);
    }
    // End For testing

    _timerDevStatus->read();
}

void DUACmdMgr::processTestServerMsg()
{
    printf("In processIncomingMsg()\n");
    size_t bytesRead = 0;

    CommandMessage *msg = new CommandMessage();

    // Read UDP data
    if (_fromTestServer->read(msg->getBuf(), MAX_MSG_SIZE, bytesRead) != OK)
    {
        printf("error reading from _udpIncoming\n");
        return;
    }
    else
    {
        printf("Successfully read %d bytes\n", (int)bytesRead);
    }

    msg->setTotalMsgSize(bytesRead);
    msg->byteSwapHeaderToLocal();

    _logger.logInfo("Processing incoming messages: msgId = %d", msg->getMsgId());
    printf("Processing incoming messages: msgId = %d\n", msg->getMsgId());

    switch (msg->getMsgId())
    {
    case SHUTDOWN_CMD_MSG_ID:
        {
            _logger.logInfo("In SHUTDOWN_CMD_MSG_ID case");
            ShutdownCmdMsg *cloneShutdownMsg = new ShutdownCmdMsg(msg->getBuf(), msg->getBufSize());
            cloneShutdownMsg->byteSwapToLocal();
            printf("msg id = %d, option = %d\n", cloneShutdownMsg->getMsgId(), cloneShutdownMsg->getType());

            SWCAckRptMsg shutdownAckRptMsg;
            shutdownAckRptMsg.setAckType(ShutdownCmdAck);
            shutdownAckRptMsg.buildMsg();
            int msgSize = shutdownAckRptMsg.getBufSize();
            printf("SWCAckRptMsg msgSize = %d, id = %d\n", msgSize, shutdownAckRptMsg.getMsgId());
            shutdownAckRptMsg.headerByteSwapToNetwork();
            
            _toTestServer->write(shutdownAckRptMsg.getBuf(), msgSize);

            if (cloneShutdownMsg->getType() == PowerOff)
            {
                printf("****Calling System Shutdown****\n");
                _logger.logInfo("****Calling System Shutdown****");
                sleep(3);
                reboot(RB_POWER_OFF);
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
//          if ((SteeringCmdCounter % 100) == 0)
//          {
                printf("===> STEERING_CMD_MSG_ID: SteeringCmdCounter = %d\n", SteeringCmdCounter);
//          }

            _logger.logInfo("===> STEERING_CMD_MSG_ID: SteeringCmdCounter = %d", SteeringCmdCounter);

            SteeringCmdDataType *params = reinterpret_cast<SteeringCmdDataType *>(cloneSteeringCmdMsg->getDataBufPos());

            printf("Alpha = %d, Beta = %d\n", params->alpha, params->beta);
            _logger.logDebug("Alpha = %d, Beta = %d", params->alpha, params->beta);

            // Set KSine Regs
            if (STEERING_WORD_SRC == ARM)
            {
                _duHWMgr.setArmKSine(ALPHA, params->alpha);
                _duHWMgr.setArmKSine(BETA, params->beta);
            }

//          _duHWMgr.getRegs(0xC, 0x10);

            // Toggle the Scan Limit check 
            if (FORCE_TEST_MODE == TEST)
            {
                _duHWMgr.toggleSWTrigger();
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
//          if ((StatusRequestCmdCounter % 100) == 0)
//          {
                printf("===> STATUS_REQUEST_CMD_MSG_ID: StatusRequestCmdCounter = %d\n", StatusRequestCmdCounter);
//          }

            _logger.logInfo("===> STATUS_REQUEST_CMD_MSG_ID: StatusRequestCmdCounter = %d", StatusRequestCmdCounter);

            StatusRequestCmdDataType *params = reinterpret_cast<StatusRequestCmdDataType *>(cloneStatusRequestCmdMsg->getDataBufPos());

            printf("requestType = %d, dcuNum = %d\n", params->requestType, params->dcuNum);
            _logger.logDebug("requestType = %d, dcuNum = %d", params->requestType, params->dcuNum);

            if (params->requestType == SWCOverallStatus)
            {
                _logger.logInfo("Sending SWCDetailedStatus Rpt To Test Server");
                printf("Sending SWCDetailedStatus Rpt To Test Server\n");
                SWCOverallStatusDataType swcStatus = _duHWMgr.getSWCStatus();
                SWCOverallStatusRptMsg swcOverallStatusRptMsg;
                swcOverallStatusRptMsg.setSWCStatus(swcStatus.swcStatus);
                swcOverallStatusRptMsg.setSWCConfig(swcStatus.swcConfig);
                swcOverallStatusRptMsg.setSWCMode(swcStatus.swcMode);
                swcOverallStatusRptMsg.setAlphaDUStatus(swcStatus.swcAlphaDUStatus);
                swcOverallStatusRptMsg.setBetaDUStatus(swcStatus.swcBetaDUStatus);
                swcOverallStatusRptMsg.setAlphaDCURolledUpStatus(swcStatus.swcAlphaDCURolledUpStatus);
                swcOverallStatusRptMsg.setBetaDCURolledUpStatus(swcStatus.swcBetaDCURolledUpStatus);
                swcOverallStatusRptMsg.setTempStatus(swcStatus.swcTempStatus);
                swcOverallStatusRptMsg.setPwrSuppliesStatus(swcStatus.swcPwrSuppliesStatus);
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
                swcOverallStatusRptMsg.setLastAlpha(lastAlpha);
                swcOverallStatusRptMsg.setLastBeta(lastBeta);
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
                    _logger.logInfo("Sending AlphaDCUDetailedStatus Rpt To Test Server");
                    printf("Sending AlphaDCUDetailedStatus Rpt for DCU %d To Test Server\n", params->dcuNum);
                }
                else
                {
                    type = BETA;
                    _logger.logInfo("Sending BetaDCUDetailedStatus Rpt To Test Server");
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
                printf("Received AlphaDUDetailedStatus request\n");

                SWCDetailedStatusDataType swcDetailedStatus;
                swcDetailedStatus.alphaOverall = GO;
                swcDetailedStatus.alphaStatus1 = NO_GO;
                swcDetailedStatus.alphaStatus2 = GO;
                swcDetailedStatus.alphaStatus3 = GO;
                swcDetailedStatus.alphaStatus4 = GO;
                swcDetailedStatus.alphaStatus5 = GO;

                swcDetailedStatus.betaOverall = GO;
                swcDetailedStatus.betaStatus1 = GO;
                swcDetailedStatus.betaStatus2 = NO_GO;
                swcDetailedStatus.betaStatus3 = GO;
                swcDetailedStatus.betaStatus4 = GO;
                swcDetailedStatus.betaStatus5 = GO;

                swcDetailedStatus.tuOverall = GO;
                swcDetailedStatus.tuStatus1 = GO;
                swcDetailedStatus.tuStatus2 = GO;
                swcDetailedStatus.tuStatus3 = GO;
                swcDetailedStatus.tuStatus4 = GO;
                swcDetailedStatus.tuStatus5 = NO_GO;

                swcDetailedStatus.psOverall = GO;
                swcDetailedStatus.psStatus1 = GO;
                swcDetailedStatus.psStatus2 = GO;
                swcDetailedStatus.psStatus3 = NO_GO;
                swcDetailedStatus.psStatus4 = GO;
                swcDetailedStatus.psStatus5 = GO;

                swcDetailedStatus.tempOverall = GO;
                swcDetailedStatus.tempStatus1 = GO;
                swcDetailedStatus.tempStatus2 = GO;
                swcDetailedStatus.tempStatus3 = GO;
                swcDetailedStatus.tempStatus4 = NO_GO;
                swcDetailedStatus.tempStatus5 = GO;


                SWCDetailedStatusRptMsg swcDetailedStatusRptMsg;
                swcDetailedStatusRptMsg.setSWCDetailedStatus(swcDetailedStatus);
                swcDetailedStatusRptMsg.buildMsg();
                int msgSize = swcDetailedStatusRptMsg.getBufSize();
                swcDetailedStatusRptMsg.headerByteSwapToNetwork();
                _toTestServer->write(swcDetailedStatusRptMsg.getBuf(), sizeof(SWCDetailedStatusRptMsg));
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

void DUACmdMgr::processLocalHWStatusMsg()
{
    static int counter = 0;
    counter++;
    printf("Received Beta DCU data %d\n", counter);
    size_t bytesRead = 0;

    BetaDCUStatusParamsType *localStatus;
    localStatus = &_localStatus[localStatusCounter];
    localStatusCounter++;

    if (localStatusCounter >= 4)
    {
        localStatusCounter = 0;
    }

    // Read UDP data
    if (_localHWStatus->read((char *)localStatus, sizeof(BetaDCUStatusParamsType), bytesRead) != OK)
    {
        printf("Error reading from _localHWStatus\n");
        return;
    }
    else
    {
        // Get message id
        if (localStatus->msgID == BETA_DCU_STATUS)
        {
            printf("Received % bytes: Group %d DCU %d\n", bytesRead, localStatus->betaDCUStatus.group, localStatus->betaDCUStatus.number);
            // Update local SW status and add to send queue
            _duHWMgr.processDCUStatus(localStatus->betaDCUStatus);
        }
    }
}

void DUACmdMgr::processDEVPCMsg()
{
    size_t bytesRead = 0;

    int status[14];

    // Read UDP data
    if (_udpFromDevPC->read((char *)&status[0], sizeof(int)*14, bytesRead) != OK)
    {
        printf("error reading from _udpFromDevPC\n");
        return;
    }
    else
    {
        printf("processDEVPCMsg Successfully read %d bytes\n", (int)bytesRead);
    }

    _duHWMgr.processEmulatorStatus(HealthState(status[1]),
                                   SWC_CONFIG(status[2]),
                                   SWC_MODE(status[3]),
                                   HealthState(status[4]),
                                   HealthState(status[5]),
                                   DCURolledUpStatus(status[6]),
                                   DCURolledUpStatus(status[7]),
                                   HealthState(status[8]),
                                   HealthState(status[9]),
                                   HealthState(status[10]),
                                   RFCC_CH(status[11]),
                                   HealthState(status[12]),
                                   status[13]);

}






