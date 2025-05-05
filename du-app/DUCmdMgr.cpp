/**
* $Id: DUCmdMgr.cpp 6638 2011-03-10 23:07:51Z ste38548 $ 
*/
#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <cmath>
#include <sys/reboot.h>
#include "DUCmdMgr.h"
#include "WriteRegCmdMsg.h"
#include "ShutdownCmdMsg.h"
#include "SteeringCmdMsg.h"
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
    _udpFromRIMS(NULL),
    _udpWarmRestart(NULL),
    _udpFromTWGS(NULL),
    _duHWMgr(DUHWMgr::getInstance()),
    _uio1Dev(NULL),
    _timerDev(NULL),
    _statusRptToTWGS(NULL)
{
}

/**
 * Destructor
 */
DUCmdMgr::~DUCmdMgr()
{
    // Delete all devices
    delete _udpFromRIMS;
    _udpFromRIMS = NULL;

    delete _udpFromTWGS;
    _udpFromTWGS = NULL;

    delete _udpWarmRestart;
    _udpWarmRestart = NULL;

    delete _uio1Dev;
    _uio1Dev = NULL;

    delete _timerDev;
    _timerDev = NULL;

    delete _statusRptToTWGS;
    _statusRptToTWGS = NULL;
}

/** 
 * Creates, opens and registers events
 * Call the base class start()
 */
STATUS DUCmdMgr::start()
{
    STATUS rc = OK;

    string SOC_IP_ADDRESS;
    int FROM_RIMS_PORT;
    int FROM_TWGS_PORT;
    int WARM_RESTART_PORT;
    string DEV_PC_IP_ADDRESS;
    int DEV_INCOMING_PORT;
    string TWGS_STATUS_IP_ADDRESS;
    int STATUS_TO_TWGS_PORT;

    printf("\nLoading Config file\n");
    if (ConfigDataManager::getInstance().load() != OK)
    {
        printf("Error loading Config file\n");
    }

    ConfigDataManager& configs = ConfigDataManager::getInstance();

    rc = rc || configs.get("SOC_IP_ADDRESS", SOC_IP_ADDRESS);
    rc = rc || configs.get("FROM_RIMS_PORT", FROM_RIMS_PORT);
    rc = rc || configs.get("FROM_TWGS_PORT", FROM_TWGS_PORT);
    rc = rc || configs.get("WARM_RESTART_PORT", WARM_RESTART_PORT);
    rc = rc || configs.get("DEV_PC_IP_ADDRESS", DEV_PC_IP_ADDRESS);
    rc = rc || configs.get("DEV_INCOMING_PORT", DEV_INCOMING_PORT);
    rc = rc || configs.get("TWGS_STATUS_IP_ADDRESS", TWGS_STATUS_IP_ADDRESS);
    rc = rc || configs.get("STATUS_TO_TWGS_PORT", STATUS_TO_TWGS_PORT);
    rc = rc || configs.get("MODULE_TYPE", MODULE_TYPE);

    rc = rc || configs.get("TEST_STATUS", TEST_STATUS);

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
    devName << SOC_IP_ADDRESS << ":" << FROM_RIMS_PORT;

    _udpFromRIMS = new UDPNetworkDevice(NetworkServer, SOC_IP_ADDRESS, FROM_RIMS_PORT, false);
    _udpFromRIMS->setName(devName.str());

    if (_udpFromRIMS->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _udpFromRIMS->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_udpFromRIMS, READ_EVENT, 1, static_cast<EventFunc>(&DUCmdMgr::processIncomingMsg)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _udpFromRIMS->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _udpFromRIMS device");
    printf("Successfully created _udpFromRIMS device\n");

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

    // Initialize rf generator
    _duHWMgr.initialize();

    // Open UIO device
    _uio1Dev = new UIODevice(AXI_INT_OFFSET, 0);

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

    // printEventList();

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
//  printf("Reading interrupt, number of interrupt = %d\n", pending);
    _uio1Dev->clearInterrupt();

    // Get FW SL check status
//  printf("FW SL status = 0x%x\n", _duHWMgr.getFWScanLimitCheckStatus());
    int alpha = _duHWMgr.getArmKSine(ALPHA);
    int beta = _duHWMgr.getArmKSine(BETA);

    int swSLResult = runSWScanLimitCheck(float(alpha), float(beta));
    int fwSLResult = _duHWMgr.getFWScanLimitCheckStatus();

    printf("%d,%d,0x%x,%d,%d\n", alpha, beta, fwSLResult, fwSLResult & 0x1, swSLResult);

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
    // printf("In processIncomingMsg()\n");
    size_t bytesRead = 0;

    RIMSCommandMessage *msg = new RIMSCommandMessage();

    // Read UDP data
    if (_udpFromRIMS->read(msg->getBuf(), MAX_RIMS_MSG_SIZE, bytesRead) != OK)
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

    _logger.logInfo("Processing incoming RIMS messages: msgId = %d", msg->getMsgId());
//  printf("Processing incoming RIMS messages: msgId = %d\n", msg->getMsgId());

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
            if ((SteeringCmdCounter % 100) == 0)
            {
                printf("RFG_CMD_MSG_ID: pbpId = %d, SteeringCmdCounter = %d\n", cloneSteeringCmdMsg->getPBPId(), SteeringCmdCounter);
            }

            _logger.logInfo("In RFG_CMD_MSG_ID pbpId = %d", cloneSteeringCmdMsg->getPBPId());

            SteeringCmdDataType *params = reinterpret_cast<SteeringCmdDataType *>(cloneSteeringCmdMsg->getDataBufPos());

//          printf("Alpha = %d, Beta = %d\n", params->alpha, params->beta);

            // Set KSine Regs
            _duHWMgr.setArmKSine(ALPHA, params->alpha);
            _duHWMgr.setArmKSine(BETA, params->beta);

//          _duHWMgr.getRegs(0xC, 0x10);

            // Toggle the Scan Limit check 
            _duHWMgr.runFWScanLimitCheck();

            break;
        }
        // printf("Successfully write from readUdpData\n");
    }
}

// _logger.logInfo("Received Status Request");
//        //      printf("Received Status Request\n");
//if ((MODULE_TYPE == WG1) || (MODULE_TYPE == WG2))
//{
//    _logger.logInfo("Sending Status Rpt To TU");
//    // Send status report to TU
//    WGStatusRptMsg wgStatusRptMsg;
//    wgStatusRptMsg.msgID = (InternalMsgID)toNetworkInt(STATUS_RPT);
//    wgStatusRptMsg.status = toNetworkInt(_duHWMgr.getBoardStatus());
//
//    // For testing to be removed
//    wgStatusRptMsg.status = toNetworkInt(TEST_STATUS);
//
//    _statusRptToTWGS->write(&wgStatusRptMsg, sizeof(wgStatusRptMsg));
//}

void DUCmdMgr::processWarmRestartMsg()
{
    size_t bytesRead = 0;

    RIMSHeaderType *rimsHeaderPtr;
    rimsHeaderPtr = &_rimsHeaderBuf[warmRestartBufCounter];
    warmRestartBufCounter++;

    if (warmRestartBufCounter >= 4)
    {
        warmRestartBufCounter = 0;
    }

    // Read UDP data
    if (_udpWarmRestart->read((char *)rimsHeaderPtr, sizeof(RIMSHeaderType), bytesRead) != OK)
    {
        printf("error reading from _udpWarmRestart\n");
        return;
    }
    else
    {
        // printf("Successfully read %d bytes\n", (int)bytesRead);
    }

    // Convert to Little Endian
    rimsHeaderPtr->msgId = (RIMSMessageId)fromNetworkInt(rimsHeaderPtr->msgId);

    _logger.logInfo("Processing incoming Warm Restart messages");

    if (rimsHeaderPtr->msgId == 2003)
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




