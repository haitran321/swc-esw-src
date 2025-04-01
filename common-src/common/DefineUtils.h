#ifndef DefineUtils_H
#define DefineUtils_H

#include <stdbool.h>

#define OK 0
#define ERROR -1
#define STATUS int

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
//#define AXI_DDR_OFFSET 0x02000000
//#define AXI_DDR_OFFSET 0x502000000LL  // Use this for the push button interrupt FW version
#define AXI_DDR_OFFSET 0x500000000LL
#define AXI_INT_OFFSET 0xA0010000   // 0xA0001000 used to be this, if see issue change back to 1000
 //#define APB_BUS_OFFSET XPAR_APB_BUS_INTERFACE_0_BASEADDR
#define MAP_SIZE 2200
#define CS_MAP_SIZE 2200
#define DU_MAP_SIZE 2200
#define TU_MAP_SIZE 2200
#define RFGEN_MAP_SIZE 17000
#define DR_MAP_SIZE 0x10000000

// DR base address offsets
#define DDR4_CH_A_OFFSET 0x500000000LL
#define DDR4_CH_B_OFFSET 0x4800000000LL

// DR IQ offset
#define CH0_HI_OFFSET 0x0
#define CH0_HQ_OFFSET 0x10000000
#define CH0_VI_OFFSET 0x20000000
#define CH0_VQ_OFFSET 0x30000000
#define CH1_HI_OFFSET 0x40000000
#define CH1_HQ_OFFSET 0x50000000
#define CH1_VI_OFFSET 0x60000000
#define CH1_VQ_OFFSET 0x70000000

#define APB_REG_1 0x00       
#define APB_REG_2 0x04
#define APB_REG_3 0x08
#define APB_REG_4 0x0C
#define APB_REG_5 0x10

#define MAX_ACTION 50

#define REAL 0
#define IMAG 1
#define NON_COMPLEX 2

typedef enum
{
    CS  = 0,
    TX  = 1,
    WG1 = 2,
    WG2 = 3,
    DRC = 4,
    DR  = 5 
} MODULE_TYPE;

typedef enum
{
    CS_CHANNEL_1 = 0,
    CS_CHANNEL_2 = 1,
    NUM_CS_CHANNELS = 2
} CS_CHANNEL;

typedef enum
{
    RFG_DAC_0 = 0,
    RFG_DAC_1 = 1,
    RFG_DAC_2 = 2,
    RFG_DAC_3 = 3,
    NUM_RFG_DACS = 4
} RFG_DAC;

typedef enum
{
    RFG_CHANNEL_0 = 0,
    RFG_CHANNEL_1 = 1,
    RFG_CHANNEL_2 = 2,
    RFG_CHANNEL_3 = 3,
    RFG_CHANNEL_4 = 4,
    RFG_CHANNEL_5 = 5,
    RFG_CHANNEL_6 = 6,
    RFG_CHANNEL_7 = 7,
    RFG_CHANNEL_8 = 8,
    RFG_CHANNEL_9 = 9,
    NUM_RFG_CHANNELS = 10
} RFG_CHANNEL;

typedef enum
{
    TU_CHANNEL_0 = 0,
    TU_CHANNEL_1 = 1,
    TU_CHANNEL_2 = 2,
    TU_CHANNEL_3 = 3,
    TU_CHANNEL_4 = 4,
    TU_CHANNEL_5 = 5,
    TU_CHANNEL_6 = 6,
    TU_CHANNEL_7 = 7,
    TU_CHANNEL_8 = 8,
    TU_CHANNEL_9 = 9,
    NUM_TU_CHANNELS = 10
} TU_CHANNEL;

typedef enum
{
    DR_IF_CH_0 = 0,
    DR_IF_CH_1 = 1,
    NUM_DR_IF_CH = 2
} DR_IF_CH;

typedef enum
{
    DR_DDR_CH_0 = 0,
    DR_DDR_CH_1 = 1,
    NUM_DDR_CH = 2
} DR_DDR_CH;

typedef enum
{
    DR_CH1_HI = 0,
    DR_CH1_HQ = 1,
    DR_CH1_VI = 2,
    DR_CH1_VQ = 3,
    DR_CH2_HI = 4,
    DR_CH2_HQ = 5,
    DR_CH2_VI = 6,
    DR_CH2_VQ = 7,
    DR_NUM_SECTION = 8
} DR_DDR_SECTION;

// Channel Sim Test defines
// #define CS_ONE_SHOT_TESTING
// #define CS_SW_TRIGGER
// #define CS_INPUT_SRC

#endif
