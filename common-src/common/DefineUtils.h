#ifndef DefineUtils_H
#define DefineUtils_H

#include <stdbool.h>

#define OK 0
#define ERROR -1
#define STATUS int

#define PASSED 1
#define FAILED 0

#define UINT unsigned int
#define UINT32 unsigned int
#define ULONG unsigned long
#define BOOL bool
#define FALSE false
#define TRUE true
typedef int (*FUNCPTR)(...);

#define LOGGER_MAX_MSG_SIZE 512

// SoC Address
#define APB_BUS_OFFSET 0xA0000000
#define AXI_DDR_OFFSET 0x500000000LL
#define AXI_INT_121_OFFSET 0xA0010000 
#define AXI_INT_122_OFFSET 0xA0011000 
#define MAP_SIZE 2200
#define DU_MAP_SIZE 2200
#define TU_MAP_SIZE 2200

#define MAX_ACTION 50

#define NUM_DCU 152

typedef enum
{
    DU    = 0,
    TU    = 1,
} MODULE_TYPE;

typedef enum
{
    ALPHA = 0,
    BETA = 1,
    NUM_RFCC_CH = 2
} RFCC_CH;

typedef enum
{
    CONFIG_ERR = 0,
    SWCR  = 1,
    SWC   = 2,
    JOINT = 3
} SWC_CONFIG;

typedef enum
{
    ONLINE = 0,
    TEST_ENABLE = 1,
} SWC_MODE;

typedef struct
{
    int statusReg;
    int overallStatus;
    int group;
    int location;
    int bitResults;
    int ySwitchLoc;
    int crcCount;
} DCUStatusParamsType;

#endif
