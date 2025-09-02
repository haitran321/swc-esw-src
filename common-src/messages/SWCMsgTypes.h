#ifndef SWCMsgTypes_H
#define SWCMsgTypes_H

#include "IRIGTypes.h"
#include "DefineUtils.h"

/** Concatenate two parts into a single symbol. */
#define CONCAT(prefix, suffix) prefix ## suffix

/** Concatenate two parts into a single symbol after expanding macro references. */
#define XCONCAT(prefix, suffix) CONCAT(prefix, suffix)

/** Create a unique identifier with a given prefix. */
#define UNIQUE(prefix) XCONCAT(prefix, __LINE__)

/** Insert padding into a structure. */
#define PADDING(bytes) unsigned char UNIQUE(padding)[bytes]

/** CSPU message identifiers */

typedef enum
{
    STEERING_CMD_MSG_ID             = 1,
    SHUTDOWN_CMD_MSG_ID             = 2,
    STATUS_REQUEST_CMD_MSG_ID       = 3,
    LRU_STATUS_RPT_MSG_ID           = 11,
    DCU_DETAILED_STATUS_RPT_MSG_ID  = 12,
    CONFIG_MODE_STATUS_RPT_MSG_ID   = 13,

} MessageId;

/** RIMS message header */

typedef struct
{
    MessageId msgId;
    int pbpId;
    int msgLen;
    UTCTimeType time;
    int recNum;
    int recLen;
    int recvId;
} MsgHeaderType;

/** Maximum text field size in RIMS messages */

#define MAX_TEXT_FIELD_SIZE 512

/** Alert Report message data */

typedef struct
{
    int alertId;
    int actionId;
    char msg[MAX_TEXT_FIELD_SIZE];
} AlertDataType;

typedef enum
{
    Go = 1,
    No_Go
} HealthState;

/** Shutdown Command message data  */
typedef enum
{
    RestartApp = 1,
    Reboot,
    PowerOff
} ShutdownOption;

typedef struct
{
    ShutdownOption type;
} ShutdownCmdDataType;

typedef enum
{
    Analog = 1,
    Digital
} TestOption;

/** Steering message data  */
typedef struct
{
    TestOption testOption;
    int alpha;
    int beta;
} SteeringCmdDataType;

typedef enum
{
    SWCRDetailedStatus      = 1,
    AlphaDCUDetailedStatus  = 2,
    BetaDCUDetailedStatus   = 3,
} StatusRequestType;

/** Status Request message data  */
typedef struct
{
    StatusRequestType requestType;
    int dcuNum;
} StatusRequestCmdDataType;

/** LRU Status Report message data  */
typedef struct
{
    LRUOption lruOption;
    HealthState health;
} LRUStatusRptDataType;

///** Config Mode Status Reporrt message data  */
//typedef struct
//{
//    LRUOption lruOption;
//    HealthState health;
//} LRUStatusRptDataType;

/** SW Exception Report message data */

static const unsigned int MAX_TASK_NAME = 60;

typedef struct
{
    int boardId;
    int cpuId;
    char taskName[MAX_TASK_NAME];
    char text[MAX_TEXT_FIELD_SIZE];
} SWExceptionDataType;


/** Common alert definitions */

namespace CommonAlerts
{
// This cannot be a named enumeration type because it is extended by
// the individual subsystems to add subsystem specific alerts.

enum
{
    InvalidCommandMsgHeader = 1,
    InvalidCommandMsg,
    InvalidRecordLength,
    InvalidCommandData,
    InitFailed,
    SAPFileError,
    VMEChassisStatusChange,
    SBCStatusChange,
    IRIGStatusChange,
    EntNetCmdError,
    RadarNetCmdError,
    EntNetRptError,
    RadarNetRptError,
    ShutdownError
};

typedef struct
{
    int reg;
    int val;
} RegCmdDataType;

}

#endif
