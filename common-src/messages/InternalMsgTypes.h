#ifndef InternalMsgTypes_H
#define InternalMsgTypes_H

#include "DefineUtils.h"

#define MAX_PULSE_TYPES 12

enum DRNumber
{
    DR1  = 0,
    DR2  = 1,
    DR3  = 2,
    NUM_DR_BRD = 3
};

enum CHNumber
{
    CH1 = 1,
    CH2 = 2,
    CH3 = 3,
    CH4 = 4,
    CH5 = 5,
    CH6 = 6,
    CH7 = 7,
    CH8 = 8,
    CH9 = 9,
    CH10 = 10,
    NUM_CH = 11
};

enum InternalMsgID
{
    TIMING_TRIGGER =    1,
    STATUS_REQUEST =    2,
    PULSE_INFO = 3,
    MEAN_INFO = 4,
    PDT_INFO = 5,
    DETECTION_INFO = 6,
    DETECTION_INFO_FOR_SLB = 7,
    DETECTION_INFO_AFTER_SLB = 8,
    DETECTION_INFO_FOR_IPM = 9, // Interpolated Peak Magnitude
    DETECTION_INFO_AFTER_IPM = 10,
    STATUS_RPT = 20,
    INTERRUPT = 30      // For testing only, to be removed

};

typedef struct
{
    int reg;
    int val;
} RegCmdDataType;

enum TimingTriggers
{
    DELTA_P = 0,
    RHO_P
};

struct TimingTriggersMsg
{
    InternalMsgID msgID;
    int pbpID;
    TimingTriggers trigger;
};

#endif ///InternalMsgTypes_H
