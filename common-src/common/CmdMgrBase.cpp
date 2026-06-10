#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <sys/reboot.h>

#include "CmdMgrBase.h"
#include "CommandMessage.h"
#include "ShutdownCmdMsg.h"
#include "SteeringCmdMsg.h"
#include "StatusRequestCmdMsg.h"
#include "StressTestCmdMsg.h"
#include "SWCAckRptMsg.h"

#include "Timestamp.h"

CmdMgrBase::CmdMgrBase(MODULE_TYPE moduleType) :
    _logger(Logger::getInstance()),
    _moduleType(moduleType),
    _fromTestServer(NULL),
    _toTestServer(NULL),
    _udpFromStatusEmu(NULL),
    FORCE_TEST_MODE(0),
    _timerDevStatus(NULL),
    _statusTimerCounter(0)
{
}

CmdMgrBase::~CmdMgrBase()
{
    delete _fromTestServer;
    _fromTestServer = NULL;

    delete _toTestServer;
    _toTestServer = NULL;

    delete _udpFromStatusEmu;
    _udpFromStatusEmu = NULL;

    delete _timerDevStatus;
    _timerDevStatus = NULL;
}

STATUS CmdMgrBase::initializeCommonCommandDevices(const std::string& bindIp,
                                                  const std::string& testServerIp,
                                                  int fromTestServerPort,
                                                  int toTestServerPort,
                                                  int fromStatusEmulatorPort,
                                                  int statusTimerIntervalSeconds)
{
    std::stringstream devName;

    devName << "UDP Server " << bindIp << ":" << fromTestServerPort;
    _fromTestServer = new UDPNetworkDevice(NetworkServer, bindIp, fromTestServerPort, false);
    _fromTestServer->setName(devName.str());

    if (_fromTestServer->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _fromTestServer->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_fromTestServer, READ_EVENT, 1, static_cast<EventFunc>(&CmdMgrBase::processTestServerMsg)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _fromTestServer->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _fromTestServer device");
    printf("Successfully created _fromTestServer device\n");

    devName.str("");
    devName.clear();
    devName << "UDP Client For Test Server " << testServerIp << ":" << toTestServerPort;
    _toTestServer = new UDPNetworkDevice(NetworkClient, testServerIp, toTestServerPort, false);
    _toTestServer->setName(devName.str());

    if (_toTestServer->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _toTestServer->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _toTestServer device");
    printf("Successfully created _toTestServer device\n");

    devName.str("");
    devName.clear();
    devName << "UDP Server From Status Emulator" << bindIp << ":" << fromStatusEmulatorPort;
    _udpFromStatusEmu = new UDPNetworkDevice(NetworkServer, bindIp, fromStatusEmulatorPort, false);
    _udpFromStatusEmu->setName(devName.str());

    if (_udpFromStatusEmu->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _udpFromStatusEmu->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_udpFromStatusEmu, READ_EVENT, 1, static_cast<EventFunc>(&CmdMgrBase::processStatusEmuMsg)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _udpFromStatusEmu->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _udpFromStatusEmu device");
    printf("Successfully created _udpFromStatusEmu device\n");

    if (statusTimerIntervalSeconds <= 0)
    {
        _logger.logError("Invalid STATUS_TIMER_INTERVAL_SECONDS value: %d", statusTimerIntervalSeconds);
        return ERROR;
    }

    timespec init = { statusTimerIntervalSeconds, 0 };
    timespec timeout = { statusTimerIntervalSeconds, 0 };
    _timerDevStatus = new TimerDevice(init, timeout);

    if (_timerDevStatus->open() != OK)
    {
        _logger.logInfo("ERROR openning dev %s", _timerDevStatus->getName().c_str());
        return ERROR;
    }
    if (addEvent(*_timerDevStatus, READ_EVENT, 1, static_cast<EventFunc>(&CmdMgrBase::processStatusTimer)) != OK)
    {
        _logger.logInfo("ERROR adding event to dev %s", _timerDevStatus->getName().c_str());
        return ERROR;
    }
    _logger.logInfo("Successfully created _timerDevStatus device");
    printf("Successfully created _timerDevStatus device\n");

    return OK;
}

void CmdMgrBase::sendAckToTestServer(SWCAckType ackType)
{
    SWCAckRptMsg ackRptMsg;
    ackRptMsg.setModuleType(_moduleType);
    ackRptMsg.setAckType(ackType);
    ackRptMsg.buildMsg();
    int msgSize = ackRptMsg.getBufSize();
    ackRptMsg.headerByteSwapToNetwork();

    _toTestServer->write(ackRptMsg.getBuf(), msgSize);
}

void CmdMgrBase::processTestServerMsg()
{
    size_t bytesRead = 0;
    CommandMessage msg;

    if (_fromTestServer->read(msg.getBuf(), MAX_MSG_SIZE, bytesRead) != OK)
    {
        _logger.logDebug("Error reading from _fromTestServer");
        printf("Error reading from _fromTestServer\n");
        return;
    }

    if (bytesRead < sizeof(MsgHeaderType))
    {
        _logger.logError("%s received undersized command packet: %d", getCommandMgrName(), static_cast<int>(bytesRead));
        if (_verbose)
        {
            printf("%s received undersized command packet: %d\n", getCommandMgrName(), static_cast<int>(bytesRead));
        }
        return;
    }

    msg.setTotalMsgSize(bytesRead);
    msg.byteSwapHeaderToLocal();

    if (!msg.isHeaderValid())
    {
        _logger.logError("%s received invalid command header", getCommandMgrName());
        printf("%s received invalid command header\n", getCommandMgrName());
        return;
    }

    _logger.logInfo("%s ProcessTestServerMsg: Processing incoming messages: msgId = %d",
                    getCommandMgrName(), msg.getMsgId());
    if (_verbose)
    {
        printf("%s ProcessTestServerMsg: Processing incoming messages: msgId = %d\n",
               getCommandMgrName(), msg.getMsgId());
    }

    switch (msg.getMsgId())
    {
        case SHUTDOWN_CMD_MSG_ID:
        {
            ShutdownCmdMsg shutdownMsg(msg.getBuf(), msg.getBufSize());
            shutdownMsg.byteSwapToLocal();
            if (shutdownMsg.validateData() != OK)
            {
                _logger.logError("%s received invalid shutdown command", getCommandMgrName());
                return;
            }

            _logger.logInfo("In SHUTDOWN_CMD_MSG_ID case: shutdown option = %d", shutdownMsg.getType());
            handleShutdownCommand(shutdownMsg.getType());
            break;
        }

        case STEERING_CMD_MSG_ID:
        {
            SteeringCmdMsg steeringCmdMsg(msg.getBuf(), msg.getBufSize());
            steeringCmdMsg.byteSwapToLocal();
            if (steeringCmdMsg.validateData() != OK)
            {
                _logger.logError("%s received invalid steering command", getCommandMgrName());
                return;
            }

            SteeringCmdDataType *params = reinterpret_cast<SteeringCmdDataType *>(steeringCmdMsg.getDataBufPos());
            if (_verbose)
            {
                printf("Test Src = %d, Alpha = %d, Beta = %d, RLCP = %d, RLSC = %d\n",
                       params->testSource, params->alpha, params->beta, params->RLCP, params->RLSC);
            }
            _logger.logDebug("Test Src = %d, Alpha = %d, Beta = %d, RLCP = %d, RLSC = %d",
                             params->testSource, params->alpha, params->beta, params->RLCP, params->RLSC);
            handleSteeringCommand(*params);
            break;
        }

        case STATUS_REQUEST_CMD_MSG_ID:
        {
            StatusRequestCmdMsg statusRequestCmdMsg(msg.getBuf(), msg.getBufSize());
            statusRequestCmdMsg.byteSwapToLocal();
            if (statusRequestCmdMsg.validateData() != OK)
            {
                _logger.logError("%s received invalid status request command", getCommandMgrName());
                return;
            }

            StatusRequestCmdDataType *params =
                reinterpret_cast<StatusRequestCmdDataType *>(statusRequestCmdMsg.getDataBufPos());
            if (_verbose)
            {
                printf("requestType = %d, dcuNum = %d\n", params->requestType, params->dcuNum);
            }
            _logger.logDebug("requestType = %d, dcuNum = %d", params->requestType, params->dcuNum);
            handleStatusRequest(*params);
            break;
        }

        case STRESS_TEST_CMD_MSG_ID:
        {
            StressTestCmdMsg stressTestCmdMsg(msg.getBuf(), msg.getBufSize());
            stressTestCmdMsg.byteSwapToLocal();
            if (stressTestCmdMsg.validateData() != OK)
            {
                _logger.logError("%s received invalid stess test command", getCommandMgrName());
                return;
            }

            StressTestCmdDataType *params =
                reinterpret_cast<StressTestCmdDataType *>(stressTestCmdMsg.getDataBufPos());
            if (_verbose)
            {
                printf("numTest = %d, numAction = %d, spacingUsec = %d, "
                       "alpha = %d, alphaInc = %d, "
                       "beta = %d, betaInc = %d\n",
                       params->numTest, params->numInc, params->spacingUsec,
                       params->alpha, params->alphaInc,
                       params->beta, params->betaInc);
            }
//          _logger.logDebug("requestType = %d, dcuNum = %d", params->requestType, params->dcuNum);
            handleStressTestCommand(*params);
            break;
        }


        default:
            printf("ERROR: Invalid command msgId %d\n", msg.getMsgId());
            _logger.logError("ERROR: Invalid command msgId %d", msg.getMsgId());
            break;
    }
}

void CmdMgrBase::processStatusEmuMsg()
{
    size_t bytesRead = 0;
    const int numData = 14;
    int status[numData];

    if (_udpFromStatusEmu->read(reinterpret_cast<char *>(&status[0]), sizeof(int) * numData, bytesRead) != OK)
    {
        printf("Error reading from _udpFromStatusEmu\n");
        _logger.logDebug("Error reading from _udpFromStatusEmu");
        return;
    }

    int msgId = status[0];
    if (_verbose)
    {
        printf("Received data from SWCR Status Emulator: msg id = %d\n", msgId);
    }
    _logger.logDebug("Received data from SWCR Status Emulator: msg id = %d", msgId);

    handleStatusEmulatorMessage(msgId, status, numData);
}

void CmdMgrBase::handleShutdownCommand(ShutdownOption shutdownType)
{
    sendAckToTestServer(ShutdownCmdAck);

    if (shutdownType == PowerOff)
    {
        printf("****Calling System Shutdown****\n");
        _logger.logInfo("****Calling System Shutdown****");
        sleep(3);
    }
    else
    {
        printf("****Calling System Reboot****\n");
        _logger.logInfo("****Calling System Reboot****");
        sleep(3);
        reboot(RB_AUTOBOOT);
    }
}

void CmdMgrBase::processStatusTimer()
{
    _statusTimerCounter++;
    _logger.logDebug("%s In processStatusTimer: timerCounter = %d", getCommandMgrName(), _statusTimerCounter);

    if (_statusTimerCounter == 1)
    {
        printf("%s sending InitCompleteAck Test Server\n", getCommandMgrName());
        _logger.logInfo("%s sending InitCompleteAck Test Server", getCommandMgrName());
        sendAckToTestServer(InitCompleteAck);
    }
}
