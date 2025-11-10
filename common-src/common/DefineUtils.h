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

#define NUM_DCU 153

typedef enum
{
    DU_ALPHA    = 0,
    DU_BETA     = 1,
    TU          = 2,
} MODULE_TYPE;

typedef enum
{
    ALPHA = 0,
    BETA = 1,
    NUM_RFCC_CH = 2
} RFCC_CH;

//typedef enum
//{
//    CONFIG_ERR  = 0,
//    SWC         = 1,
//    JOINT       = 2,
//    SWCR        = 3
//} SWC_CONFIG;

typedef enum
{
    CONFIG_ERR  = 0,
    SWC         = 2,
    JOINT       = 1,
    SWCR        = 3
} SWC_CONFIG;

typedef enum
{
    ONLINE = 0,
    TEST_ENABLE = 1,
} SWC_MODE;

typedef enum
{
    NO_GO = 0,
    GO    = 1
} HealthState;

//typedef enum
//{
//    DCU_ROLLED_UP_ERROR    = 0,
//    DCU_ROLLED_UP_RED      = 1,
//    DCU_ROLLED_UP_YELLOW   = 2,
//    DCU_ROLLED_UP_GREEN    = 3
//} DCURolledUpStatus;

typedef enum
{
    DCU_ROLLED_UP_ERROR    = 0,
    DCU_ROLLED_UP_RED      = 2,
    DCU_ROLLED_UP_YELLOW   = 1,
    DCU_ROLLED_UP_GREEN    = 3
} DCURolledUpStatus;

#endif
