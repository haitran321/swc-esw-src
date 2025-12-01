/**
* $Id: DUBCmdMgr.cpp 6638 2011-03-10 23:07:51Z ste38548 $ 
*/
#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <sys/reboot.h>
#include "DUBCmdMgr.h"
#include "ShutdownCmdMsg.h"
#include "SteeringCmdMsg.h"
#include "SWCAckRptMsg.h"
#include "ConfigDataManager.h"
#include "DeviceFactory.h"
#include "EndianUtils.h"
#include "ScanLimitCheck.h"
#include "DeviceUtilities.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//#define PRINT_DEBUG

/**
 * Constructor
 */
DUBCmdMgr::DUBCmdMgr() :
    MODULE_TYPE(DU_BETA),
    _logger(Logger::getInstance()),
    _fromTestServer(NULL),
    _toTestServer(NULL),
    _localHWStatus(NULL),
    _udpFromDevPC(NULL),
    _duHWMgr(DUHWMgr::getInstance()),
    _uioDevSL(NULL),
    _uioDevConfig(NULL),
    _timerDevStatus(NULL)
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

    delete _udpFromDevPC;
    _udpFromDevPC = NULL;

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

    // Status parameters
    rc = rc || configs.get("STATUS_TIMER_INTERVAL_SECONDS", STATUS_TIMER_INTERVAL_SECONDS);

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

    // Incoming from Dev PC device
    devName.clear();
    devName << "UDP Server From Dev PC";
    devName << BETA_IP_ADDRESS << ":" << FROM_DEV_PC_PORT;

    _udpFromDevPC = new UDPNetworkDevice(NetworkServer, BETA_IP_ADDRESS, FROM_DEV_PC_PORT, false);
    _udpFromDevPC->setName(devName.str());

    if (_udpFromDevPC->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _udpFromDevPC->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_udpFromDevPC, READ_EVENT, 1, static_cast<EventFunc>(&DUBCmdMgr::processDEVPCMsg)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _udpFromDevPC->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _udpFromDevPC device");
    printf("Successfully created _udpFromDevPC device\n");

    // Initialize HW Manager
    _duHWMgr.initialize(MODULE_TYPE);

    // Read Beta DU status and send to Alpha DU
    DUTUStatusMsg bduStatus;
    bduStatus.msgID = BETA_DU_STATUS;
    bduStatus.dutuStatus = _duHWMgr.readDUStatus();
    sendDUBStatusToDUA(bduStatus);
    usleep(1*1000);   // Sleep 1 msecs

    // Read and send Beta DCUs status to Alpha DU
    for (int dcu = 0; dcu < NUM_DCU; dcu++)
    {
        int queueSize = _duHWMgr.getDCUStatusQueueSize();
        if (queueSize > 0)
        {
            DCUStatusParamsType dcuStatus = _duHWMgr.getDCUStatusFromQueue();
            BetaDCUStatusMsg status;
            status.msgID = BETA_DCU_STATUS;
            status.betaDCUStatus = dcuStatus;
            sendDCUStatusToDUA(status);
            usleep(1*1000);   // Sleep 1 msecs
        }
    }

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

    EventProcessor::start();

    return OK;
}

void DUBCmdMgr::processSLInterrupt()
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
//  if (TEST_MODE_STEERING_WORD_SRC == TEST_MODE_DU)
//  {
//      lastAlpha = armAlpha;
//      lastBeta = armBeta;
//  }
//  else
//  {
//      lastAlpha = atbAlpha;
//      lastBeta = atbBeta;
//  }

    // Check DCU status queue to see if there are status to send
    for (int dcu = 0; dcu < NUM_DCU; dcu++)
    {
        int queueSize = _duHWMgr.getDCUStatusQueueSize();
        if (queueSize > 0)
        {
            DCUStatusParamsType dcuStatus = _duHWMgr.getDCUStatusFromQueue();
            BetaDCUStatusMsg status;
            status.msgID = BETA_DCU_STATUS;
            status.betaDCUStatus = dcuStatus;
            sendDCUStatusToDUA(status);
            usleep(1*1000);   // Sleep 1 msecs
        }
    }

//  eInterruptProcessing.stop();
//  printf("SW Scan Limit Check took %f\n", eInterruptProcessing.secs());

    uint64_t stopTimeNSec = ts.GetNanoSecondsSinceMidnight();
//  printf("SW Scan Limit Check took %ld\n", stopTimeNSec - startTimeNSec);

}

void DUBCmdMgr::processConfigInterrupt()
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
}

void DUBCmdMgr::processStatusTimer()
{
    static int timerCounter = 0;

    timerCounter++;

//  if (timerCounter % 100 == 0)
//  {
        printf("In processTimer: timerCounter = %d\n", timerCounter);
//  }

    // Read Beta DU status and send to Alpha DU
    DUTUStatusMsg bduStatus;
    bduStatus.msgID = BETA_DU_STATUS;
    bduStatus.dutuStatus = _duHWMgr.readDUStatus();
    sendDUBStatusToDUA(bduStatus);
    usleep(1*1000);   // Sleep 1 msecs

    printf("Read all DCUs status to update local queue.\n");
    _logger.logDebug("Read all DCUs status to update local queue.");
    _duHWMgr.readDCUStatus();

    int queueSize = _duHWMgr.getDCUStatusQueueSize();
    printf("DCU status queue size = %d\n", queueSize);
    _logger.logDebug("DCU status queue size = %d", queueSize);

    // TO BE REMOVED WHEN RUNNING ON ACTUAL DU HW THAT HAS INTERRUPT
    for (int dcu = 0; dcu < NUM_DCU; dcu++)
    {
        int queueSize = _duHWMgr.getDCUStatusQueueSize();
        if (queueSize > 0)
        {
            DCUStatusParamsType dcuStatus = _duHWMgr.getDCUStatusFromQueue();
            BetaDCUStatusMsg status;
            status.msgID = BETA_DCU_STATUS;
            status.betaDCUStatus = dcuStatus;
            sendDCUStatusToDUA(status);
            usleep(1*1000);   // Sleep 1 msecs
        }
    }

    _timerDevStatus->read();
}

void DUBCmdMgr::processTestServerMsg()
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
            printf("SWCAckRptMsg msgSize = %d\n", msgSize);
            shutdownAckRptMsg.headerByteSwapToNetwork();
            _toTestServer->write(shutdownAckRptMsg.getBuf(), sizeof(SWCAckRptMsg));

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

            printf("Test Src = %d, Alpha = %d, Beta = %d, RLCP = %d, RLSC = %d\n", 
                   params->testSource, params->alpha, params->beta, params->RLCP, params->RLSC);
            _logger.logDebug("Test Src = %d, Alpha = %d, Beta = %d, RLCP = %d, RLSC = %d",
                             params->testSource, params->alpha, params->beta, params->RLCP, params->RLSC);

            // Check for Offline Mode from HW or Test Enabled from HW or Force Test Mode from Config File
            if ((_duHWMgr.getSWCModeStatus() == TEST_ENABLE) || (_duHWMgr.getTestEnabledStatus() == 1) || (FORCE_TEST_MODE == TEST))
            {
                // Set test source based on command
                _duHWMgr.setTestSrcInTestMode(params->testSource);

                // Set KSine Regs
                if (params->testSource == TestSourceDU)
                {
                    _duHWMgr.setArmKSine(ALPHA, params->alpha);
                    _duHWMgr.setArmKSine(BETA, params->beta);

                    if (params->RLCP == On)
                    {
                        _duHWMgr.sendSteeringWordValidFlagInTestMode(STEERING_WORD_INVALID);
                        _duHWMgr.sendDCUCmdInTestMode(DCU_CMD_BORESIGHT);
                    }
                    if (params->RLSC == On)
                    {
                        _duHWMgr.sendSteeringWordValidFlagInTestMode(STEERING_WORD_INVALID);
                        _duHWMgr.sendDCUCmdInTestMode(DCU_CMD_CALIBRATION);
                    }

                    // Toggle SW trigger in place of /RLTD
                    _duHWMgr.toggleSWTrigger();

                    // Reset Steering Word valid flag back to valid for the next command
                    _duHWMgr.sendSteeringWordValidFlagInTestMode(STEERING_WORD_VALID);
                }
            }

            break;
        }
        // printf("Successfully write from readUdpData\n");
    }
}

void DUBCmdMgr::sendDCUStatusToDUA(BetaDCUStatusMsg status)
{
    _localHWStatus->write(&status, sizeof(BetaDCUStatusMsg));
}

void DUBCmdMgr::sendDUBStatusToDUA(DUTUStatusMsg status)
{
    _localHWStatus->write(&status, sizeof(DUTUStatusMsg));
}

void DUBCmdMgr::processDEVPCMsg()
{
    size_t bytesRead = 0;

    int status[15];

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
                                   HealthState(status[11]),
                                   RFCC_CH(status[12]),
                                   HealthState(status[13]),
                                   status[14]);

}

