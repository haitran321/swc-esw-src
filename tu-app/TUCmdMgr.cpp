/**
* $Id: TUCmdMgr.cpp 6638 2011-03-10 23:07:51Z ste38548 $ 
*/
#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <cmath>
#include <sys/reboot.h>
#include "TUCmdMgr.h"
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
TUCmdMgr::TUCmdMgr() :
    MODULE_TYPE(TU),
    _logger(Logger::getInstance()),
    _fromTestServer(NULL),
    _toTestServer(NULL),
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
    delete _fromTestServer;
    _fromTestServer = NULL;

    delete _toTestServer;
    _toTestServer = NULL;

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
    _logger.logInfo("TUCmdMgr Initializing");

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
    devName << TEST_UNIT_IP_ADDRESS << ":" << FROM_TEST_SERVER_PORT;

    _fromTestServer = new UDPNetworkDevice(NetworkServer, TEST_UNIT_IP_ADDRESS, FROM_TEST_SERVER_PORT, false);
    _fromTestServer->setName(devName.str());

    if (_fromTestServer->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _fromTestServer->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_fromTestServer, READ_EVENT, 1, static_cast<EventFunc>(&TUCmdMgr::processTestServerMsg)) != OK)
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

    // From HW devices - Outbound for TU
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
    usleep(1*1000);   // Sleep 1 msecs

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

    EventProcessor::start();

    return OK;
}

void TUCmdMgr::processSLInterrupt()
{
//  eInterruptProcessing.reset();
//  eInterruptProcessing.start();

    uint64_t startTimeNSec = ts.GetNanoSecondsSinceMidnight();

#ifdef PRINT_DEBUG
    printf("In processInterrupt()\n");
#endif

    size_t bytesRead = 0;
    int pending = 0;

    _uioDevSL->read((char *)&pending, sizeof(int), bytesRead);
    printf("Reading scan limit interrupt, number of interrupt = %d\n", pending);
    _logger.logDebug("Reading scan limit interrupt, number of interrupt = %d", pending);
    _uioDevSL->clearInterrupt();

    // Get FW SL check status
//  printf("FW SL status = 0x%x\n", _tuHWMgr.getFWScanLimitCheckStatus());
    int alpha = _tuHWMgr.getArmKSine(ALPHA);
    int beta = _tuHWMgr.getArmKSine(BETA);

    int swSLResult = runSWScanLimitCheck(float(alpha), float(beta));
    int fwSLResult = _tuHWMgr.getFWScanLimitCheckStatus();

    printf("%d,%d,0x%x,%d,%d\n", alpha, beta, fwSLResult, fwSLResult & 0x1, swSLResult);

//  eInterruptProcessing.stop();
//  printf("SW Scan Limit Check took %f\n", eInterruptProcessing.secs());

    uint64_t stopTimeNSec = ts.GetNanoSecondsSinceMidnight();
//  printf("SW Scan Limit Check took %ld\n", stopTimeNSec - startTimeNSec);

}

void TUCmdMgr::processWLSPInterrupt()
{
#ifdef PRINT_DEBUG
    printf("In processWLSPInterrupt()\n");
#endif

    size_t bytesRead = 0;
    int pending = 0;

    _uioDevWLSP->read((char *)&pending, sizeof(int), bytesRead);
    printf("Reading WLSP interrupt, number of interrupt = %d\n", pending);
    _logger.logDebug("Reading WLSP changed interrupt, number of interrupt = %d", pending);
    _uioDevWLSP->clearInterrupt();
}

void TUCmdMgr::processStatusTimer()
{
    static int timerCounter = 0;

    timerCounter++;

//  if (timerCounter % 100 == 0)
//  {
        printf("In processTimer: timerCounter = %d\n", timerCounter);
//  }

    // Read TU status and send to Alpha DU
    DUTUStatusMsg tuStatus;
    tuStatus.msgID = TU_STATUS;
    tuStatus.dutuStatus = _tuHWMgr.readTUStatus();
    sendTUStatusToDUA(tuStatus);
    usleep(1*1000);   // Sleep 1 msecs

	_timerDevStatus->read();
}

void TUCmdMgr::processTestServerMsg()
{
    // printf("In processIncomingMsg()\n");
    size_t bytesRead = 0;

    CommandMessage *msg = new CommandMessage();

    // Read UDP data
    if (_fromTestServer->read(msg->getBuf(), MAX_MSG_SIZE, bytesRead) != OK)
    {
        printf("error reading from _udpFromRIMS\n");
        return;
    }
    else
    {
//      printf("Successfully read %d bytes\n", (int)bytesRead);
    }

    msg->setTotalMsgSize(bytesRead);
    msg->byteSwapHeaderToLocal();

    _logger.logInfo("Processing incoming messages: msgId = %d", msg->getMsgId());
//  printf("Processing incoming messages: msgId = %d\n", msg->getMsgId());

    switch (msg->getMsgId())
    {
    case SHUTDOWN_CMD_MSG_ID:
        {
            _logger.logInfo("In SHUTDOWN_CMD_MSG_ID case");
            ShutdownCmdMsg *cloneShutdownMsg = new ShutdownCmdMsg(msg->getBuf(), msg->getBufSize());
            cloneShutdownMsg->byteSwapToLocal();
            printf("msg id = %d, option = %d\n", cloneShutdownMsg->getMsgId(), cloneShutdownMsg->getType());

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

            // Set KSine Regs
            if (params->testSource == TestSourceTU)
            {
                _tuHWMgr.setArmKSine(ALPHA, params->alpha);
                _tuHWMgr.setArmKSine(BETA, params->beta);

                if (params->RLCP == On)
                {
                    _tuHWMgr.setRLCPSignal(On);
                }
                if (params->RLSC == On)
                {
                    _tuHWMgr.setRLSCSignal(On);
                }

                // Toggle RLTD signal to start steering words processing 
                _tuHWMgr.toggleRLTDSignal();

                // Reset RLCP and RLSC back to off
                _tuHWMgr.setRLCPSignal(Off);
                _tuHWMgr.setRLSCSignal(Off);
            }

            break;
        }
        // printf("Successfully write from readUdpData\n");
    }
}

void TUCmdMgr::sendTUStatusToDUA(DUTUStatusMsg status)
{
    _localHWStatus->write(&status, sizeof(DUTUStatusMsg));
}




