/**
* $Id: DUCmdMgr.cpp 6638 2011-03-10 23:07:51Z ste38548 $ 
*/
#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <cmath>
#include <sys/reboot.h>
#include "DUCmdMgr.h"
#include "ShutdownCmdMsg.h"
#include "SteeringCmdMsg.h"
#include "StatusRequestCmdMsg.h"
#include "SWCStatusRptMsg.h"
#include "ConfigDataManager.h"
#include "DeviceFactory.h"
#include "EndianUtils.h"
#include "DeviceUtilities.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//#define PRINT_DEBUG

/**
 * Constructor
 */
DUCmdMgr::DUCmdMgr() :
    _logger(Logger::getInstance()),
    _udpIncoming(NULL),
    _udpWarmRestart(NULL),
    _duHWMgr(DUHWMgr::getInstance()),
    _uio1Dev(NULL),
    _timerDev(NULL),
    _udpOutToTestServer(NULL),
    _udpFromDevPC(NULL),
    _udpToDevPC(NULL)
{
}

/**
 * Destructor
 */
DUCmdMgr::~DUCmdMgr()
{
    // Delete all devices
    delete _udpIncoming;
    _udpIncoming = NULL;

    delete _udpWarmRestart;
    _udpWarmRestart = NULL;

    delete _uio1Dev;
    _uio1Dev = NULL;

    delete _timerDev;
    _timerDev = NULL;

    delete _udpOutToTestServer;
    _udpOutToTestServer = NULL;

    delete _udpFromDevPC;
    _udpFromDevPC = NULL;

    delete _udpToDevPC;
    _udpToDevPC = NULL;
}

/** 
 * Creates, opens and registers events
 * Call the base class start()
 */
STATUS DUCmdMgr::start()
{
    STATUS rc = OK;

    string SOC_IP_ADDRESS;
    int INCOMING_PORT;
    int WARM_RESTART_PORT;
    string TEST_SERVER_IP_ADDRESS;
    int TO_TEST_SERVER_PORT;

    printf("\nLoading Config file\n");
    if (ConfigDataManager::getInstance().load() != OK)
    {
        printf("Error loading Config file\n");
    }

    ConfigDataManager& configs = ConfigDataManager::getInstance();

    rc = rc || configs.get("SOC_IP_ADDRESS", SOC_IP_ADDRESS);
    rc = rc || configs.get("INCOMING_PORT", INCOMING_PORT);
    rc = rc || configs.get("WARM_RESTART_PORT", WARM_RESTART_PORT);
    rc = rc || configs.get("MODULE_TYPE", MODULE_TYPE);

    rc = rc || configs.get("TEST_SERVER_IP_ADDRESS", TEST_SERVER_IP_ADDRESS);
    rc = rc || configs.get("TO_TEST_SERVER_PORT", TO_TEST_SERVER_PORT);

    string DEV_PC_IP_ADDRESS;
    int FROM_DEV_PC_PORT;
    int TO_DEV_PC_PORT;
    rc = rc || configs.get("DEV_PC_IP_ADDRESS", DEV_PC_IP_ADDRESS);
    rc = rc || configs.get("FROM_DEV_PC_PORT", FROM_DEV_PC_PORT);
    rc = rc || configs.get("TO_DEV_PC_PORT", TO_DEV_PC_PORT);

    rc = rc || configs.get("FORCE_TEST_MODE", FORCE_TEST_MODE);
    rc = rc || configs.get("STEERING_WORD_SRC", STEERING_WORD_SRC);

    // Setup Logger
    _logger.initialize();
    _logger.logInfo("DUCmdMgr Initializing");

    if (rc == ERROR)
    {
        _logger.logInfo("ERROR: reading config file");
        return (ERROR);
    }

    _logger.logInfo("MODULE_TYPE = %d", MODULE_TYPE);

    // From RIMS device for commands
    stringstream devName;
    devName << "UDP Server ";
    devName << SOC_IP_ADDRESS << ":" << INCOMING_PORT;

    _udpIncoming = new UDPNetworkDevice(NetworkServer, SOC_IP_ADDRESS, INCOMING_PORT, false);
    _udpIncoming->setName(devName.str());

    if (_udpIncoming->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _udpIncoming->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_udpIncoming, READ_EVENT, 1, static_cast<EventFunc>(&DUCmdMgr::processIncomingMsg)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _udpIncoming->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _udpIncoming device");
    printf("Successfully created _udpIncoming device\n");

    // Warm Restart device
    stringstream warmRestartDevName;
    warmRestartDevName << "UDP Server For Warm Restart";
    warmRestartDevName << SOC_IP_ADDRESS << ":" << WARM_RESTART_PORT;
    _udpWarmRestart = new UDPNetworkDevice(NetworkServer, SOC_IP_ADDRESS, WARM_RESTART_PORT, false);
    _udpWarmRestart->setName(warmRestartDevName.str());

    if (_udpWarmRestart->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _udpWarmRestart->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_udpWarmRestart, READ_EVENT, 1, static_cast<EventFunc>(&DUCmdMgr::processWarmRestartMsg)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _udpWarmRestart->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _udpWarmRestart device");
    printf("Successfully created _udpWarmRestart device\n");

    stringstream toTSDevName;
    toTSDevName << "UDP Client For Test Server";
    toTSDevName << TEST_SERVER_IP_ADDRESS << ":" << TO_TEST_SERVER_PORT;
    _udpOutToTestServer = new UDPNetworkDevice(NetworkClient, TEST_SERVER_IP_ADDRESS, TO_TEST_SERVER_PORT, false);
    _udpOutToTestServer->setName(toTSDevName.str());

    if (_udpOutToTestServer->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _udpOutToTestServer->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _udpOutToTestServer device");
    printf("Successfully created _udpOutToTestServer device\n");

    // Initialize rf generator
    _duHWMgr.initialize();

    // Open UIO device
    _uio1Dev = new UIODevice(AXI_INT_121_OFFSET, 0);

    if (_uio1Dev->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _uio1Dev->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_uio1Dev, READ_EVENT, 1, static_cast<EventFunc>(&DUCmdMgr::processInterrupt)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _uio1Dev->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _uio1Dev device");

    // Map UIO address
    _uio1Dev->mmap();

    // Timer testing
    timespec init = { 0, 0 };
    timespec timeout = { 0, 0 };
    _timerDev = new TimerDevice(init, timeout);

    if (_timerDev->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _timerDev->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_timerDev, READ_EVENT, 1, static_cast<EventFunc>(&DUCmdMgr::processTimer)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _timerDev->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _timerDev device");

    // Incoming from Dev PC device
    stringstream incomingDevPCName;
    incomingDevPCName << "UDP Server From Dev PC";
    incomingDevPCName << SOC_IP_ADDRESS << ":" << FROM_DEV_PC_PORT;

    _udpFromDevPC = new UDPNetworkDevice(NetworkServer, SOC_IP_ADDRESS, FROM_DEV_PC_PORT, false);
    _udpFromDevPC->setName(incomingDevPCName.str());

    if (_udpFromDevPC->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _udpFromDevPC->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_udpFromDevPC, READ_EVENT, 1, static_cast<EventFunc>(&DUCmdMgr::processDEVPCMsg)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _udpFromDevPC->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _udpFromDevPC device");
    printf("Successfully created _udpFromDevPC device\n");

    // To Dev PC device
    stringstream toDevPCName;
    toDevPCName << "UDP Client To Dev PC";
    toDevPCName << DEV_PC_IP_ADDRESS << ":" << _udpToDevPC;
    _udpToDevPC = new UDPNetworkDevice(NetworkClient, DEV_PC_IP_ADDRESS, TO_DEV_PC_PORT, false);
    _udpToDevPC->setName(toDevPCName.str());

    if (_udpToDevPC->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _udpToDevPC->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _udpToDevPC device");
    printf("Successfully created _udpToDevPC device\n");
    EventProcessor::start();

    return OK;
}

void DUCmdMgr::processInterrupt()
{
//  eInterruptProcessing.reset();
//  eInterruptProcessing.start();

    uint64_t startTimeNSec = ts.GetNanoSecondsSinceMidnight();

#ifdef PRINT_DEBUG
    printf("In processInterrupt()\n");
#endif

    size_t bytesRead = 0;
    int pending = 0;

    _uio1Dev->read((char *)&pending, sizeof(int), bytesRead);
    printf("Reading interrupt, number of interrupt = %d\n", pending);
    _uio1Dev->clearInterrupt();

    // Get FW SL check status
//  printf("FW SL status = 0x%x\n", _duHWMgr.getFWScanLimitCheckStatus());

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

    int armSWSLResult = runSWScanLimitCheck(float(atbAlpha), float(armBeta));
    int atbSWSLResult = runSWScanLimitCheck(float(atbAlpha), float(atbBeta));
    int fwSLResult = _duHWMgr.getFWScanLimitCheckStatus();

    printf("fwSLResult = 0x%x(%d), atbSWSLResult = %d, armSWSLResult = %d\n", fwSLResult, fwSLResult & 0x1, atbSWSLResult, armSWSLResult);

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

    // Send to Dev PC for display
    int devPCData[8];
    devPCData[0] = 1; // for Ksine
    devPCData[1] = atbAlpha;
    devPCData[2] = atbBeta;
    devPCData[3] = armAlpha;
    devPCData[4] = armBeta;
    devPCData[5] = fwSLResult;
    devPCData[6] = atbSWSLResult;
    devPCData[7] = armSWSLResult;
    _udpToDevPC->write(&devPCData[0], sizeof(int)*8);

//  eInterruptProcessing.stop();
//  printf("SW Scan Limit Check took %f\n", eInterruptProcessing.secs());

    uint64_t stopTimeNSec = ts.GetNanoSecondsSinceMidnight();
//  printf("SW Scan Limit Check took %ld\n", stopTimeNSec - startTimeNSec);

}

void DUCmdMgr::processTimer()
{
    static int timerCounter = 0;

    timerCounter++;

//  if (timerCounter % 100 == 0)
//  {
        printf("In processTimer: timerCounter = %d\n", timerCounter);
//  }

    // Set Diag bit to generate interrupt

//  _duHWMgr.toggleInterruptBit();

    _timerDev->read();
}

void DUCmdMgr::processIncomingMsg()
{
    printf("In processIncomingMsg()\n");
    size_t bytesRead = 0;

    CommandMessage *msg = new CommandMessage();

    // Read UDP data
    if (_udpIncoming->read(msg->getBuf(), MAX_MSG_SIZE, bytesRead) != OK)
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

            printf("requestType = %d, Beta = %d\n", params->requestType, params->dcuNum);

            if (params->requestType == SWCDetailedStatus)
            {
                _logger.logInfo("Sending SWCDetailedStatus Rpt To Test Server");
                printf("Sending SWCDetailedStatus Rpt To Test Server\n");
                SWCStatusRptMsg swcStatusRptMsg;
                swcStatusRptMsg.setSWCStatus(Go);
                swcStatusRptMsg.setSWCConfig(SWCR);
                swcStatusRptMsg.setSWCMode(TEST_ENABLE);
                swcStatusRptMsg.setLastAlpha(lastAlpha);
                swcStatusRptMsg.setLastBeta(lastBeta);
                swcStatusRptMsg.buildMsg();
                int msgSize = swcStatusRptMsg.getBufSize();
                swcStatusRptMsg.headerByteSwapToNetwork();
                _udpOutToTestServer->write(swcStatusRptMsg.getBuf(), sizeof(SWCStatusRptMsg));
            }

            if (params->requestType == AlphaDCUDetailedStatus)
            {
                _logger.logInfo("Sending AlphaDCUDetailedStatus Rpt To Test Server");
                printf("Sending AlphaDCUDetailedStatus Rpt for DCU %d To Test Server\n", params->dcuNum);
            }

            if (params->requestType == BetaDCUDetailedStatus)
            {
                _logger.logInfo("Sending BetaDCUDetailedStatus Rpt To Test Server");
                printf("Sending BetaDCUDetailedStatus Rpt for DCU %d To Test Server\n", params->dcuNum);
            }

            break;
        }
        // printf("Successfully write from readUdpData\n");
    }
}

void DUCmdMgr::processWarmRestartMsg()
{
    size_t bytesRead = 0;

    MsgHeaderType *msgHeaderPtr;
    msgHeaderPtr = &_msgHeaderBuf[warmRestartBufCounter];
    warmRestartBufCounter++;

    if (warmRestartBufCounter >= 4)
    {
        warmRestartBufCounter = 0;
    }

    // Read UDP data
    if (_udpWarmRestart->read((char *)msgHeaderPtr, sizeof(MsgHeaderType), bytesRead) != OK)
    {
        printf("error reading from _udpWarmRestart\n");
        return;
    }
    else
    {
        // printf("Successfully read %d bytes\n", (int)bytesRead);
    }

    // Convert to Little Endian
    msgHeaderPtr->msgId = (MessageId)fromNetworkInt(msgHeaderPtr->msgId);

    _logger.logInfo("Processing incoming Warm Restart messages");

    if (msgHeaderPtr->msgId == 2003)
    {
        printf("****Calling System Reboot****\n");
        _logger.logInfo("****Calling System Reboot****");
        sleep(3);
        reboot(RB_AUTOBOOT);
    }
}

#define GAMMA 1.207234
#define CENTER_FREQ 442.0
#define K 533.597428    // (GAMMA*CENTER_FREQ)
#define UV_THRESHOLD 0.8703556
#define W_THRESHOLD 0.333807
#define EL_THRESHOLD 0.01658

int DUCmdMgr::runSWScanLimitCheck(float alpha, float beta)
{
#ifdef PRINT_DEBUG
    printf("****************************************************************\n");
    printf("UV_THRESHOLD = %f, W_THRESHOLD = %f, EL_THRESHOLD = %f\n", UV_THRESHOLD, W_THRESHOLD, EL_THRESHOLD);
#endif

    float u = -beta/K;
    float v = alpha/K;
    float w = sqrt(1 - u*u - v*v);
    float sinEl = v*cos(0.785398) + w*sin(0.785398);   // el_b is boresight elevation at 45 degree in radians

#ifdef PRINT_DEBUG
    printf("alpha = %f, beta = %f, K = %f, u = %f, v = %f, w = %f, sinEl = %f\n", alpha, beta, K, u, v, w, sinEl);
#endif

    // Check against thresholds
    bool slResult = PASSED;
    bool failed = false;
    bool elFailed = false;

    // Check w for NAN
    if (std::isnan(w)) 
    {
        failed = true;
    }
    // Elevation test
    else if (sinEl < EL_THRESHOLD)
    {
        elFailed = true;
    }
    // u, v, w tests
    else if ((abs(u) > UV_THRESHOLD) || (abs(v) > UV_THRESHOLD) || (w < W_THRESHOLD))
    {
        // One of u, v, or w has failed, but elevation passed.  Recompute
        // rounded values of u, v, and w, then check them again.
        float signBeta = (beta > 0.0) ? 1.0 : ((beta < 0.0) ? -1.0 : 0.0);
        float signAlpha = (alpha > 0.0) ? 1.0 : ((alpha < 0.0) ? -1.0 : 0.0);
        u = -(beta - signBeta/2.0)/K;
        v = -(alpha - signAlpha/2.0)/K;
        w = sqrt(1 - u*u - v*v);

        // Re-test u, v and w
        if ((abs(u) > UV_THRESHOLD) || (abs(v) > UV_THRESHOLD) || (w < W_THRESHOLD))
        {
            // Even the rounded values fail.
            failed = true;
        }
    }

    // Set overall test result
    if (failed || elFailed)
    {
        slResult = FAILED;
#ifdef PRINT_DEBUG
        printf("slResult = %d, failed = %d, elFailed = %d\n", slResult, failed, elFailed);
#endif
    }

    return slResult;
}

void DUCmdMgr::processDCUStatus()
{
    // For now just sent the register
    for (int i = 0; i < NUM_DCU; i++)
    {
//      alphaDCU[i].statusReg = _duDev->getDCUStatusReg();
    }
}

void DUCmdMgr::processDEVPCMsg()
{
    size_t bytesRead = 0;

    int status[12];

//  self.wcStatusRpt = pack(">llllllllllll", error, scan_limit, swcr_overall, config, mode, \
//                                       alpha_status, beta_status, temp_status, pwr_status, \
//                                       dcu_type, dcu_num, dcu_status)

    // Read UDP data
    if (_udpFromDevPC->read((char *)&status[0], sizeof(int)*12, bytesRead) != OK)
    {
        printf("error reading from _udpFromDevPC\n");
        return;
    }
    else
    {
        printf("processDEVPCMsg Successfully read %d bytes\n", (int)bytesRead);
    }

    printf("SWC status to TWGS before: 0x%x\n", _duHWMgr.getSwcStatusToTwgs());

    // Set status reg based on what received from the emulator
    _duHWMgr.setOverallStatusBit(swcr_overall);

}






