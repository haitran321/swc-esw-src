#ifndef CmdMgrBase_H
#define CmdMgrBase_H

#include <string>

#include "EventProcessor.h"
#include "Logger.h"
#include "SWCMsgTypes.h"
#include "UDPNetworkDevice.h"
#include "TimerDevice.h"

#include "Timestamp.h"

class CmdMgrBase : public EventProcessor
{
    public:
        virtual ~CmdMgrBase();

    protected:
        explicit CmdMgrBase(MODULE_TYPE moduleType);

        STATUS initializeCommonCommandDevices(const std::string& bindIp,
                                              const std::string& testServerIp,
                                              int fromTestServerPort,
                                              int toTestServerPort,
                                              int fromStatusEmulatorPort,
                                              int statusTimerIntervalSeconds);

        void sendAckToTestServer(SWCAckType ackType);

        virtual const char *getCommandMgrName() const = 0;
        virtual void handleSteeringCommand(const SteeringCmdDataType& params) = 0;
        virtual void handleStatusRequest(const StatusRequestCmdDataType& params) = 0;
        virtual void handleStatusEmulatorMessage(int msgId, const int *status, int numData) = 0;
        virtual void processStatusTimer();

        Logger &_logger;
        MODULE_TYPE _moduleType;
        UDPNetworkDevice *_fromTestServer;
        UDPNetworkDevice *_toTestServer;
        UDPNetworkDevice *_udpFromStatusEmu;
        TimerDevice *_timerDevStatus;
        int FORCE_TEST_MODE;
        int _statusTimerCounter;

    private:
        void processTestServerMsg();
        void processStatusEmuMsg();
        void handleShutdownCommand(ShutdownOption shutdownType);

        Timestamp ts;

};

#endif
