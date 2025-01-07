/**
* $Id: CSPUMsgTypes.h 8548 2014-01-09 20:09:45Z tra18693 $
*
* Defines types, enums, constants, etc in support of CSPU 
* messages. 
*
*/
#ifndef CSPUMsgTypes_H
#define CSPUMsgTypes_H

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
    // NO_MSG_ID                       = 0,
    // BEAM_STEER_CMD_MSG_ID           = 1,
    BEAM_PROCESS_CMD_MSG_ID         = 2,
    // RHOP_CMD_MSG_ID                 = 3,
    // TEST_TARGET_CMD_MSG_ID          = 4,
    // SIGNAL_INJECTION_CMD_MSG_ID     = 5,
    // SCOPE_TRIGGER_CMD_MSG_ID        = 6,
    // MODE_CMD_MSG_ID                 = 101,
    SHUTDOWN_CMD_MSG_ID             = 102,
    // TBRS_DIAGNOSTICS_CMD_MSG_ID     = 104,
    // TBRS_HW_SETUP_CMD_MSG_ID        = 105,
    // TWGS_DIAGNOSTICS_CMD_MSG_ID     = 106,
    // TWGS_HW_SETUP_CMD_MSG_ID        = 107,
    // SLB_MODE_CMD_MSG_ID				= 108,
    DETECTION_PRT_MSG_ID       = 1001,
    SP_SUMMARY_PRT_MSG_ID       = 1006,
    // TBRS_BASEBAND_START_ID          = 1201,
    // TBRS_BASEBAND_REPORT_ID         = 1202,
    // TBRS_DIAGNOSTICS_RPT_MSG_ID     = 1204,
    // TBRS_INIT_STATUS_RPT_MSG_ID     = 1220,
    // TBRS_DETAILED_STATUS_RPT_MSG_ID = 1221,
    // TBRS_SUMMARY_STATUS_RPT_MSG_ID  = 1222,
    // TBRS_ALERT_RPT_MSG_ID           = 1223,
    // TBRS_SW_EXC_RPT_ID              = 1224,
    // DSPS_PP_GAIN_RPT_MSG_ID         = 1225,
    // RHOP_RPT_MSG_ID                 = 1301,
    // WC_STATUS_RPT_MSG_ID            = 1302,
    // TWGS_DIAGNOSTICS_RPT_MSG_ID     = 1304,
    // SLB_STATUS_RPT_MSG_ID           = 1305,
    // TWGS_INIT_STATUS_RPT_MSG_ID     = 1320,
    // TWGS_DETAILED_STATUS_RPT_MSG_ID = 1321,
    // TWGS_SUMMARY_STATUS_RPT_MSG_ID  = 1322,
    // TWGS_ALERT_RPT_MSG_ID           = 1323,
    // TWGS_SW_EXC_RPT_ID              = 1324,
    // TWGS_BEAM_STEER_CAL_RPT_MSG_ID  = 1325,
    // Y_SWITCH_STATUS_RPT_MSG_ID      = 1326,
    // For Channel Sim
    CS_READ_REG_CMD_MSG_ID          = 2001,
    CS_WRITE_REG_CMD_MSG_ID         = 2002,
    CS_DELAY_CMD_MSG_ID             = 1401,
    RFG_CMD_MSG_ID                  = 1402,
    RX_CMD_MSG_ID                   = 1403,
    RFG_CW_CMD_MSG_ID               = 1404,
    CS_STATUS_RPT_MSG_ID            = 1421,
    RFG_STATUS_RPT_MSG_ID           = 1422,
    RX_STATUS_RPT_MSG_ID            = 1423,

} RIMSMessageId;

/** RIMS message header */

typedef struct
{
    RIMSMessageId msgId;
    int pbpId;
    int msgLen;
    UTCTimeType time;
    int recNum;
    int recLen;
    int recvId;
} RIMSHeaderType;

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

/** LISS states */

typedef enum
{
    LISS_Legacy = 1,
    LISS_CSPU
} LISSState;

/** Operational states */

typedef enum
{
    Live = 1,
    Simulation
} OpState;

/** Simulation interference states */

typedef enum
{
    SimIntOff = 0,
    SimIntOn
} SimInterferenceState;

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
   LISSState lissState;
   OpState opState;
   SimInterferenceState simInterference;
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

/** SLB Mode type values */
typedef enum  
{
    SLB_Off = 0,
    SLB_On,
    SLB_OnExtended
} SLBModeOption;

/** SLB Mode Command message data  */
typedef struct
{
   SLBModeOption type;
} SLBModeCmdDataType;

/**
 * Automated Gain Control Levels
 */
enum AGCLevel
{
    Auto = 0,
    AGC1 = 1,
    AGC2 = 2,
    AGC3 = 3,
    AGC4 = 4,
    STC = 5
};

/**
 * TBRS Report Types
 */
enum TBRSRptType
{
    AllChannelBaseband = 1,
    CrossChannelBaseband = 2,
    MultipulseBaseband = 3,
    PerformanceMonitor = 4
};

/**
 * DSPS Report Types
 */
enum DSPSRptType
{
    Detection = 1,
    PeakPowerTBRS = 2,
    NoiseMeasurement = 3,
    PeakPowerRIMS = 4,
    Multipulse = 5,
    MultipulseLFM = 6,
    Calibration = 7,
    SLBTestDetection = 8
};

/**
 * Multi-pulse Template
 */
enum MultipulseTemplate
{
    SinglePulse = 0, // 1 XMT and 1 RCV
    Search125,       // 2 XMT and 1 RCV
    Track125,        // 2 XMT and 2 RCV
    Track25,         // 10 XMT and 2 RCV
    LFMSearch250,
    LFMSearch128,
    LFMTrack250,
    LFMTrack128
};

/** Pulse Types */

typedef enum
{
    None,
    Usec1_CW,
    Usec10_CW,
    Usec16_1MHz_LFM,
    Usec25_CW,
    Usec32_1MHz_LFM,
    Usec64_1MHz_LFM,
    Usec125_CW,
    Usec128_100kHz_LFM,
    Usec128_1MHz_LFM,
    Usec250_100kHz_LFM,
    Usec250_1MHz_LFM,
    Usec60_CW,
    LastPulseType   // This must be last.
} PulseType;

/** PBP counts of pulse types */

static const int pwCount[LastPulseType] =
{
    0,     // Not used
    60,    // Usec1_CW
    600,   // Usec10_CW,
    960,   // Usec16_1MHz_LFM,
    1500,  // Usec25_CW,
    1920,  // Usec32_1MHz_LFM,
    3840,  // Usec64_1MHz_LFM,
    7500,  // Usec125_CW,
    7680,  // Usec128_100kHz_LFM,
    7680,  // Usec128_1MHz_LFM,
    15000, // Usec250_100kHz_LFM,
    15000, // Usec250_1MHz_LFM
    3600   // Usec60_CW,
};

typedef struct
{
    int rcvID;
    int satelliteID;
    int start;           // in 16.667ns units
    int stop;            // in 16.667ns units
    TBRSRptType tbrsRptType;
    PulseType pulseType;
    int freq;
    AGCLevel agc;
    float powerStart;
    float rangeStart;
    float phaseCorr; // System Computer Phase Correction.  Units of [Radian], [0..2*pi]
    float lohStartPhase; // [Radians], [0..2*pi]
    float lovStartPhase; // [Radians], [0..2*pi]
    DSPSRptType dspsRptType;
    float ampCorr;
    float cfar;
    float pdtBiasFactor;
    int sequenceNum;
    MultipulseTemplate multipulseTemplate;
    bool isSoiPulse;
    int misapMajorFunc;
    int misapMinorFunc;
} BeamProcessCmdDataType;

/**
 * SignalSource
 */
enum SignalSource
{
    PilotPulse = 1,
    TTG = 2
};

/**
 * The definition of the repeating data portion of the message.
 */
typedef struct 
{
    int rcvID;
    int start;        // in 16.667ns units
    int stop;         // in 16.667ns units
    SignalSource sigSrc;
    int satelliteID;
    float beam1Sig;
    float beam2Sig;
    float beam3Sig;
    float beam4Sig;
    float beam5Sig;
    float beam6Sig;
    float beam7Sig;
    float beam8Sig;
    float beam9Sig;
    float beam10Sig;
}SigInjectCmdDataType;

// /** SLB Mode Command message data  */

// typedef struct
// {
//    SLBModeOption type;
// } SLBModeCmdDataType;

/* Range Window Commmand message data */
typedef enum
{
    Off,
    Center,
    All,
} DataReportingMode;

typedef struct
{
   DataReportingMode mode;
} RangeWindowCmdDataType;

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
