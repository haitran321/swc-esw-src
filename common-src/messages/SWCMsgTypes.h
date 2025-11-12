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
    SWC_OVERALL_STATUS_RPT_MSG_ID   = 11,
    SWC_DETAILED_STATUS_RPT_MSG_ID  = 12,
    DCU_DETAILED_STATUS_RPT_MSG_ID  = 13,
    SWC_ACK_RPT_MSG_ID              = 14,
} MessageId;

/** RIMS message header */

typedef struct
{
    MessageId msgId;
    int msgLen;
    UTCTimeType time;
    int recNum;
    int recLen;
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
    SWCOverallStatus       = 1,
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
    HealthState swcPwrSuppliesStatus;
    HealthState swcATBStatus;
    HealthState testUnitHWStatus;
    int lastAlpha;
    int lastBeta;
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
    int crcStatus;
} DCUStatus;

typedef struct
{
    RFCC_CH group;
    int number;
    int fwStatusReg;
    DCUStatus dcuStatus;
} DCUStatusParamsType;

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
    // Alpha
    HealthState alphaOverall;
    HealthState alphaStatus1;
    HealthState alphaStatus2;
    HealthState alphaStatus3;
    HealthState alphaStatus4;
    HealthState alphaStatus5;
    // Beta
    HealthState betaOverall;
    HealthState betaStatus1;
    HealthState betaStatus2;
    HealthState betaStatus3;
    HealthState betaStatus4;
    HealthState betaStatus5;
    // Pwr Supplies
    HealthState psOverall;
    HealthState psStatus1;
    HealthState psStatus2;
    HealthState psStatus3;
    HealthState psStatus4;
    HealthState psStatus5;
    // Temp
    HealthState tempOverall;
    HealthState tempStatus1;
    HealthState tempStatus2;
    HealthState tempStatus3;
    HealthState tempStatus4;
    HealthState tempStatus5;
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

#endif
