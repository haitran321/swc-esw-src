/**
* $Id: DUCmdMgr.cpp 6638 2011-03-10 23:07:51Z ste38548 $ 
*/
#include <stdio.h>
#include <sstream>
#include <unistd.h>
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

#define PRINT_DEBUG

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
    timespec init = { 10, 0 };
    timespec timeout = { 10, 0 };
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
    eInterruptProcessing.start();

#ifdef PRINT_DEBUG
    printf("In processInterrupt()\n");
#endif

    size_t bytesRead = 0;
    int pending = 0;

    _uio1Dev->read((char *)&pending, sizeof(int), bytesRead);
    printf("Reading interrupt, number of interrupt = %d\n", pending);
    _uio1Dev->clearInterrupt();

    eInterruptProcessing.stop();
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

    _duHWMgr.toggleInterruptBit();

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
        printf("Successfully read %d bytes\n", (int)bytesRead);
    }

    msg->setTotalMsgSize(bytesRead);
    msg->byteSwapHeaderToLocal();

    _logger.logInfo("Processing incoming RIMS messages: msgId = %d", msg->getMsgId());
    printf("Processing incoming RIMS messages: msgId = %d\n", msg->getMsgId());

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




