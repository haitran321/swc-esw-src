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
    STATUS_CMD_MSG_ID               = 3,
    TOD_CMD_MSG_ID                  = 4,
    STATUS_RPT_MSG_ID               = 11,
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

/** Mode values */

typedef enum
{
   Offline = 1,
   Mission,
   TestAssist
} Mode;

/** Operational states */

typedef enum
{
    Live = 1,
    Simulation
} OpState;

#ifndef _CVI_

typedef enum
{
    Green = 1,
    White,
    Yellow,
    Red
} HealthState;

/** Summary Status Report message data */

// typedef struct
// {
//    HealthState overallHealth        :8;
//    Mode mode                        :8;
//    SubsystemState subsystemState    :8;
//    HealthState cornerChannelStatus  :8;
// } SummaryStatusDataType;

/** Mode Command message data */

typedef struct
{
   Mode mode;
   OpState opState;
} ModeCmdDataType;

#endif

/** Shutdown Command message data  */

typedef enum
{
    Reboot = 1,
    PowerOff
} ShutdownOption;

typedef struct
{
    ShutdownOption type;
} ShutdownCmdDataType;

/** Steering message data  */
typedef struct
{
   int alpha;
   int beta;
} SteeringCmdDataType;

/** SW Exception Report message data */

#ifndef _CVI_

// Labwindows/CVI compiler used to compile the PXI code is a C compiler
// only. It does not support using a const as an  array size. This type
// is not used by the PXI code.

static const unsigned int MAX_TASK_NAME = 60;

typedef struct
{
    int boardId;
    int cpuId;
    char taskName[MAX_TASK_NAME];
    char text[MAX_TEXT_FIELD_SIZE];
} SWExceptionDataType;

#endif

/** Common alert definitions */

#ifndef _CVI_

// Labwindows/CVI compiler used to compile the PXI code is a C compiler
// only. It does not support namespaces. This type is not used by the
// PXI code.

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
}

#endif

#endif
