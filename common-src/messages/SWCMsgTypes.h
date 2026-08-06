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
    STEERING_CMD_MSG_ID                 = 1,
    SHUTDOWN_CMD_MSG_ID                 = 2,
    STATUS_REQUEST_CMD_MSG_ID           = 3,
    STRESS_TEST_CMD_MSG_ID              = 4,
    REPOLL_DCU_CMD_MSG_ID               = 5,
    SWC_OVERALL_STATUS_RPT_MSG_ID       = 11,
    SWC_DETAILED_STATUS_RPT_MSG_ID      = 12,
    DCU_DETAILED_STATUS_RPT_MSG_ID      = 13,
    SWC_ACK_RPT_MSG_ID                  = 14,
    SWC_PROCESSED_SW_RPT_MSG_ID         = 15,
    SYSTEM_STATUS_TO_TWGS_RPT_MSG_ID    = 16,
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
    Analog = 0,       
    Digital = 1       
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
    SWCOverallStatus                    = 1,
    AlphaDCUDetailedStatus              = 2,
    BetaDCUDetailedStatus               = 3,
    SWCDetailedStatus                   = 4,
    StartSendingProcessedSteeringWord   = 5,
    StopSendingProcessedSteeringWord    = 6,
    SystemStatusSentToTWGS              = 7
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
    SWC_MODE    olteMode;
    RolledUpStatus swcAlphaDUStatus;
    RolledUpStatus swcBetaDUStatus;
    RolledUpStatus swcAlphaDCURolledUpStatus;
    RolledUpStatus swcBetaDCURolledUpStatus;
    HealthState swcTempStatus;
    HealthState swc12VPwrStatus;
    HealthState swc24VPwrStatus;
    HealthState swcATBStatus;
    RolledUpStatus testUnitHWStatus;
    int lastProcessedAlpha;
    int lastProcessedBeta;
    DCUHealthState alphaDCU[NUM_DCU];
    DCUHealthState betaDCU[NUM_DCU];
} SWCOverallStatusDataType;

/** Status Report message data  */
typedef struct
{
    MODULE_TYPE moduleType;
    int processedKSine;
} SWCProcessedSteerWordDataType;

/** Status Report message data  */
typedef struct
{
    int swcStatus;
    int swcrStatus;
} StatusSentToTWGSDataType;

typedef struct
{
    RFCC_CH group;
    int loc;
    int fwStatusReg;
    HealthState locStatus;
    int dcuFWMajorRev;
    int dcuFWMinorRev;
    int bypassStatus;
    int modeStatus;
    DCUHealthState overallStatus;
    HealthState clockStatus;
    HealthState spiCommStatus;
    HealthState crcStatus;
    HealthState steeringWordCompare;
} DCUStatus;

struct DCUStatusWithID
{
    int dcuID;
    DCUStatus dcuStatus;

    // Overload the equality operator for convenience with some algorithms
    bool operator==(const DCUStatusWithID& dcu) const {
        return dcuID == dcu.dcuID;
    }
};

typedef enum
{
    ShutdownCmdAck      = 1,
    InitCompleteAck     = 2,
} SWCAckType;

typedef struct
{
    int modType;
    SWCAckType ackType;
} SWCAckDataType;

typedef struct
{
    RolledUpStatus overallStatus;      
    HealthState readyStatus;
    HealthState highTempAlarm;
    HealthState overTempAlarm;
    HealthState vccintAlarm;
    HealthState vccauxAlarm;
    HealthState vbramAlarm;
    int dieTemp;
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

typedef struct
{
    SWC_CONFIG  swcConfig;
    SWC_MODE    swcMode;
    SWC_MODE    olteMode;
} ConfigModeCmdType;

// Internal messages between embedded SW components: DU Alpha, DU Beta, Test Unit
typedef enum 
{
    BETA_DCU_STATUS     = 1,
    BETA_DU_STATUS      = 2,
    TU_STATUS           = 3,
    CONFIG_MODE_REQUEST = 4,
    CONFIG_MODE_CMD     = 5, 
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

typedef struct
{
    InternalMsgID msgID;
}ConfigModeRequestMsg;

typedef struct
{
    InternalMsgID msgID;
    ConfigModeCmdType configMode;
}ConfigModeCmdMsg;

typedef struct
{
    int numCycle;
    int numTest;
    int numInc;
    int cycleResetTime;
    int spacingUsec;
    int alpha;
    int alphaInc;
    int beta;
    int betaInc;
} StressTestCmdDataType;

#endif
