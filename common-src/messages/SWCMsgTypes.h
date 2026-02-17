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

/** Test Server message identifiers */

typedef enum
{
    STEERING_CMD_MSG_ID             = 1,
    SHUTDOWN_CMD_MSG_ID             = 2,
    STATUS_REQUEST_CMD_MSG_ID       = 3,
    SWC_OVERALL_STATUS_RPT_MSG_ID   = 11,
    SWC_DETAILED_STATUS_RPT_MSG_ID  = 12,
    DCU_DETAILED_STATUS_RPT_MSG_ID  = 13,
    SWC_ACK_RPT_MSG_ID              = 14,
} MessageId;

/** Test Server message header */

typedef struct
{
    MessageId msgId;
    int msgLen;
    UTCTimeType time;
    int recNum;
    int recLen;
} MsgHeaderType;

/** Maximum text field size in Test Server messages */

#define MAX_TEXT_FIELD_SIZE 512

/** Alert Report message data */

typedef struct
{
    int alertId;
    int actionId;
    char msg[MAX_TEXT_FIELD_SIZE];
} AlertDataType;

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
    TestSourceTU = 0,       // analog
    TestSourceDU = 1        // digital
} TestSource;

typedef enum
{
    Off = 0,
    On  = 1
} CmdOnOff;

/** Steering message data  */
typedef struct
{
    TestSource testSource;
    int alpha;
    int beta;
    CmdOnOff RLCP;
    CmdOnOff RLSC; 
} SteeringCmdDataType;

typedef enum
{
    SWCOverallStatus        = 1,
    AlphaDCUDetailedStatus  = 2,
    BetaDCUDetailedStatus   = 3,
    SWCDetailedStatus       = 4,
} StatusRequestType;

/** Status Request message data  */
typedef struct
{
    StatusRequestType requestType;
    int dcuNum;
} StatusRequestCmdDataType;

/** Status Report message data  */
typedef struct
{
    HealthState swcStatus;
    SWC_CONFIG  swcConfig;
    SWC_MODE    swcMode;
    HealthState swcAlphaDUStatus;
    HealthState swcBetaDUStatus;
    DCURolledUpStatus swcAlphaDCURolledUpStatus;
    DCURolledUpStatus swcBetaDCURolledUpStatus;
    HealthState swcTempStatus;
    HealthState swc12VPwrStatus;
    HealthState swc24VPwrStatus;
    HealthState swcATBStatus;
    HealthState testUnitHWStatus;
    int lastProcessedAlpha;
    int lastProcessedBeta;
    HealthState alphaDCU[NUM_DCU];
    HealthState betaDCU[NUM_DCU];
} SWCOverallStatusDataType;

typedef struct
{
    int bypassStatus;
    int modeStatus;
    HealthState overallStatus;
    HealthState clockStatus;
    HealthState locValid;
    HealthState spiCommStatus;
    HealthState steeringWordCompare;
    int fwLoc;
    int dcuFWMajorRev;
    int dcuFWMinorRev;
    RFCC_CH dcuType;
    int crcStatus;
} DCUFWStatus;

typedef struct
{
    RFCC_CH group;
    int loc;
    int fwStatusReg;
    DCUFWStatus dcuFWStatus;
} DCUStatus;

typedef enum
{
    ShutdownCmdAck      = 1,
} SWCAckType;

typedef struct
{
    SWCAckType ackType;
} SWCAckDataType;

typedef struct
{
    HealthState overallStatus;
    HealthState readyStatus;
    HealthState highTempAlarm;
    HealthState vccintAlarm;
    HealthState vccauxAlarm;
    HealthState vbramAlarm;
} DUTUStatusType;

typedef struct
{
    HealthState swcStatus;
    DUTUStatusType alphaDUStatus;
    DUTUStatusType betaDUStatus;
    DUTUStatusType tuStatus;
} SWCDetailedStatusDataType;

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

// Internal messages between embedded SW components: DU Alpha, DU Beta, Test Unit
typedef enum 
{
    BETA_DCU_STATUS = 1,
    BETA_DU_STATUS  = 2,
    TU_STATUS       = 3
}InternalMsgID;

typedef struct
{
    InternalMsgID msgID;
    DCUStatus betaDCUStatus;
}BetaDCUStatusMsg;

typedef struct
{
    InternalMsgID msgID;
    DUTUStatusType dutuStatus;
}DUTUStatusMsg;

#endif
