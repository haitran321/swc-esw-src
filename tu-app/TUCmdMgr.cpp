/**
* $Id: TUCmdMgr.cpp 6638 2011-03-10 23:07:51Z ste38548 $ 
*/
#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <sys/reboot.h>
#include "TUCmdMgr.h"
#include "WriteRegCmdMsg.h"
#include "ShutdownCmdMsg.h"
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
TUCmdMgr::TUCmdMgr() :
    _logger(Logger::getInstance()),
    _udpFromRIMS(NULL),
    _udpWarmRestart(NULL),
    _udpFromTWGS(NULL),
    _tuHWMgr(TUHWMgr::getInstance()),
//  _uio1Dev(NULL),
    _statusRptToTWGS(NULL)
{
}

/**
 * Destructor
 */
TUCmdMgr::~TUCmdMgr()
{
    // Delete all devices
    delete _udpFromRIMS;
    _udpFromRIMS = NULL;

    delete _udpFromTWGS;
    _udpFromTWGS = NULL;

    delete _udpWarmRestart;
    _udpWarmRestart = NULL;

    delete _statusRptToTWGS;
    _statusRptToTWGS = NULL;
}

/** 
 * Creates, opens and registers events
 * Call the base class start()
 */
STATUS TUCmdMgr::start()
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
    _logger.logInfo("TUCmdMgr Initializing");

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
    if (addEvent(*_udpFromRIMS, READ_EVENT, 1, static_cast<EventFunc>(&TUCmdMgr::processIncomingMsg)) != OK)
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
    if (addEvent(*_udpWarmRestart, READ_EVENT, 1, static_cast<EventFunc>(&TUCmdMgr::processWarmRestartMsg)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _udpWarmRestart->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _udpWarmRestart device");
    printf("Successfully created _udpWarmRestart device\n");

    // Initialize rf generator
    _tuHWMgr.initialize();

//  // Open UIO device
//  _uio1Dev = new UIODevice(AXI_INT_OFFSET, 0);
//
//  if (_uio1Dev->open() != OK)
//  {
//      _logger.logInfo("ERROR openning dev %s", _uio1Dev->getName().c_str());
//      return ERROR;
//  }
//  if (addEvent(*_uio1Dev, READ_EVENT, 1, static_cast<EventFunc>(&TUCmdMgr::processInterrupt)) != OK)
//  {
//      _logger.logInfo("ERROR adding event to dev %s", _uio1Dev->getName().c_str());
//      return ERROR;
//  }
//  _logger.logInfo("Successfully created _uio1Dev device");
//
//  // Map UIO address
//  _uio1Dev->mmap();

    // printEventList();

    EventProcessor::start();

    return OK;
}

//void TUCmdMgr::processInterrupt()
//{
//    eInterruptProcessing.start();
//
//#ifdef PRINT_DEBUG
//    printf("In processInterrupt()\n");
//#endif
//
//    size_t bytesRead = 0;
//    int pending = 0;
//
//    _uio1Dev->read((char *)&pending, sizeof(int), bytesRead);
//    printf("Reading interrupt, number of interrupt = %d\n", pending);
//    _uio1Dev->clearInterrupt();
//
//    eInterruptProcessing.stop();
//}

void TUCmdMgr::processIncomingMsg()
{
    // printf("In processIncomingMsg()\n");
    size_t bytesRead = 0;

    CommandMessage *msg = new CommandMessage();

    // Read UDP data
    if (_udpFromRIMS->read(msg->getBuf(), MAX_MSG_SIZE, bytesRead) != OK)
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
//    case RFG_CMD_MSG_ID:
//        {
//            if ((MODULE_TYPE == TX) || (MODULE_TYPE == WG1) || (MODULE_TYPE == WG2))
//            {
//                GenRFSignalCmdMsg *cloneGenRFSignalMsg = new GenRFSignalCmdMsg(msg->getBuf(), msg->getBufSize());
//                cloneGenRFSignalMsg->byteSwapToLocal();
//                int numActions = cloneGenRFSignalMsg->getDataSize() / sizeof(RFGenCmdType);
//
//                static int TxCmdCounter = 0;
//                TxCmdCounter++;
//                if ((TxCmdCounter % 100) == 0)
//                {
//                    printf("RFG_CMD_MSG_ID: pbpId = %d, TxCmdCounter = %d\n", cloneGenRFSignalMsg->getPBPId(), TxCmdCounter);
//                }
//
//                _logger.logInfo("In RFG_CMD_MSG_ID pbpId = %d", cloneGenRFSignalMsg->getPBPId());
//
//                RFGenCmdType *params = reinterpret_cast<RFGenCmdType *>(cloneGenRFSignalMsg->getDataBufPos());
//
//                for (int i = 0; i < numActions; i++)
//                {
//                    TU_CHANNEL chId = params[i].channelId;
//
//                    long freq = (params[i].action.signal.freqMHz * 1000000) + params[i].action.signal.freqHz;
//                    unsigned int ftw = int(float(freq) * TR_FTW_Conversion_sec);
//
////                  int pwUsec = (int)((params[i].action.stopTime - params[i].action.startTime) / 200.0);
//                    unsigned int startUsec = params[i].action.startTime / 1000;
//                    unsigned int stopUsec = params[i].action.stopTime / 1000;
//                    unsigned int pwUsec = (unsigned int)(stopUsec - startUsec);
//
//                    unsigned int rtw = (unsigned int)(float(params[i].action.signal.lfmRamp) / pwUsec * TR_RTW_Conversion);
//
//                    _logger.logDebug("++++++++++++++++++++++++");
//                    _logger.logDebug("TX Action number %d: PBP ID = %d, Receive ID = %d, channelId = %d", i, cloneGenRFSignalMsg->getPBPId(), params[i].recvId, chId);
//                    _logger.logDebug("TX startTime = %d (%d usec), stopTime = %d (%d usec), pw = %d usec",
//                                     params[i].action.startTime, startUsec,
//                                     params[i].action.stopTime, stopUsec,
//                                     pwUsec);
//                    _logger.logDebug("TX amplitude = %d, phaseOffset = %u, freq_Hz = %d (Hz), freq_MHz = %d (MHz), freq = %ld (Hz), ftw = %d, lfmRamp = %d (Hz), rtw = %d, phaseCode = %d",
//                                     params[i].action.signal.amplitude, params[i].action.signal.phaseOffset,
//                                     params[i].action.signal.freqHz, params[i].action.signal.freqMHz, freq, ftw,
//                                     params[i].action.signal.lfmRamp, rtw,
//                                     params[i].action.signal.phaseCode);
//
//                    printf("++++++++++++++++++++++++\n");
//                    printf("TX Action number %d: PBP ID = %d, Receive ID = %d, channelId = %d\n", i, cloneGenRFSignalMsg->getPBPId(), params[i].recvId, chId);
//                    printf("TX startTime = %d (%d usec), stopTime = %d (%d usec), pw = %d usec\n",
//                                     params[i].action.startTime, startUsec,
//                                     params[i].action.stopTime, stopUsec,
//                                     pwUsec);
//                    printf("TX amplitude = %d, phaseOffset = %u, freq = %d (Hz), freq_MHz = %d (MHz),  freq = %ld (Hz), ftw = %d, lfmRamp = %d (Hz), rtw = %d, phaseCode = %d\n",
//                                     params[i].action.signal.amplitude, params[i].action.signal.phaseOffset,
//                                     params[i].action.signal.freqHz, params[i].action.signal.freqMHz, freq, ftw,
//                                     params[i].action.signal.lfmRamp, rtw,
//                                     params[i].action.signal.phaseCode);
//
//                    // Generate hw instructions
//                    if (RFG_INIT_TESTING == 1)
//                    {
////                      _rfGenHWMgr.addInitAction(chId, params[i].action.signal);
//                    }
//                    else
//                    {
//                        // Check action before adding
//                        // Verify no overlapping
//                        bool overlapped = false;
//
//                        if (params[i].action.startTime < lastRFGActionStopTime[chId])
//                        {
//                            overlapped = true;
//                            _logger.logInfo("ERROR: startTime (%d) < lastTXActionStopTime (%d)", params[i].action.startTime, lastRFGActionStopTime[chId]);
//                        }
//
//                        if (params[i].action.stopTime < params[i].action.startTime)
//                        {
//                            overlapped = true;
//                            _logger.logInfo("ERROR: stopTime (%d) < startTime (%d)", params[i].action.stopTime, params[i].action.startTime);
//                        }
//
//                        if (!overlapped)
//                        {
//                            lastRFGActionStopTime[params[i].channelId] = params[i].action.stopTime;
//                            params[i].action.signal.lfmRamp = rtw;
//                            _tuHWMgr.addAction(chId, params[i].action, ftw);
//                        }
//                    }
//
//                }
//
//                // Init setup
//                if (RFG_INIT_TESTING == 1)
//                {
////                  _tuHWMgr.initTest();
//                }
//                else
//                {
//                    // Program actions to FW
//                    _tuHWMgr.programActions();
//
//                    // For WG1 and WG2, expecting 1 message with all actions for the PBP.
//                    // TWGS will be sending the message at DeltaP, so need to do everything
//                    // normally do at DeltaP here and do nothing at DeltaP
//                    if ((MODULE_TYPE == WG1) || (MODULE_TYPE == WG2) || (INTERNAL_TRIGGER == 1))
//                    {
//                        toggleLoadCmdFlag();
//                    }
//                }
//            }
//
//            break;
//        }
        // printf("Successfully write from readUdpData\n");
    }
}

void TUCmdMgr::processWarmRestartMsg()
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




