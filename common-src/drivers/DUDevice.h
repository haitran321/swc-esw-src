#ifndef DUice_H
#define DUice_H

#include "Device.h"

typedef struct
{
    int amplitude;
    int freq;
    int spare1;
    int spare2;
}DUCWSignalType;

typedef struct
{
    DU_CHANNEL channelId;
    DUCWSignalType signal;
}DUCWCmdType;

typedef struct
{
    int amplitude;
    unsigned int phaseOffset;
    unsigned int freqHz;
    unsigned int freqMHz;
    unsigned int lfmRamp;
    int phaseCode;
    int reserved;
}DUSignalType;

typedef struct
{
    unsigned int startTime;
    unsigned int stopTime;
    DUSignalType signal;
}DUActionType;

// TO DO: Use the DUSignalType after matching FW interface
typedef struct
{
    int amplitude;
    unsigned int phaseOffset;
    unsigned int freq;
    unsigned int lfmRamp;
    int phaseCode;
    int reserved;
}DUFWSignalType;

// TO DO: Use the DUActionType after matching FW interface
typedef struct
{
    unsigned int startTime;
    unsigned int stopTime;
    DUFWSignalType signal;
}DUFWActionType;

typedef struct
{
    int recvId;
    DU_CHANNEL channelId;
    int sid;
    DUActionType action;
}DUCmdType;

typedef struct
{
    int firmwareVersion;
    int boardStatus;
    int boardControl;
    int armKSineAlpha;
    int armKSineBeta;
    int atbKSineAlpha;
    int atbKSineBeta;
    int spare1[20];
    int fpgaDieTemp;
    int vccIntVoltage;
    int vccAuxVoltage;
    int vbramVoltage;
    int diagInfo;
}DURegType;

#define DU_CH0_STARTING_ADDR_OFFSET 0x0100
#define DU_CH1_STARTING_ADDR_OFFSET 0x0740
#define DU_CH2_STARTING_ADDR_OFFSET 0x0D80
#define DU_CH3_STARTING_ADDR_OFFSET 0x13C0
#define DU_CH4_STARTING_ADDR_OFFSET 0x1A00
#define DU_CH5_STARTING_ADDR_OFFSET 0x2040
#define DU_CH6_STARTING_ADDR_OFFSET 0x2680
#define DU_CH7_STARTING_ADDR_OFFSET 0x2CC0
#define DU_CH8_STARTING_ADDR_OFFSET 0x3300
#define DU_CH9_STARTING_ADDR_OFFSET 0x3940

typedef enum
{
    DU_BOARD_CONTROL_MASK                = 0xC7FFFFFF,    /* Bits 0..26, and 30..31 */
    DU_DAC0_ENABLE_MASK                  = 0x00000001,    /* Bit 0 */
    DU_DAC1_ENABLE_MASK                  = 0x00000002,    /* Bit 1 */
    DU_DAC2_ENABLE_MASK                  = 0x00000004,    /* Bit 2 */
    DU_DAC3_ENABLE_MASK                  = 0x00000008,    /* Bit 3 */
    DU_DAC0_PBP_MODE_MASK                = 0x00000010,    /* Bit 4 */
    DU_DAC1_PBP_MODE_MASK                = 0x00000020,    /* Bit 5 */
    DU_DAC2_PBP_MODE_MASK                = 0x00000040,    /* Bit 6 */
    DU_DAC3_PBP_MODE_MASK                = 0x00000080,    /* Bit 7 */
    DU_DAC0_BPSK_MODE_MASK               = 0x00000100,    /* Bit 8 */
    DU_DAC1_BPSK_MODE_MASK               = 0x00000200,    /* Bit 9 */
    DU_DAC2_BPSK_MODE_MASK               = 0x00000400,    /* Bit 10 */
    DU_DAC3_BPSK_MODE_MASK               = 0x00000800,    /* Bit 11 */
    DU_DAC0_LFM_RAMP_MASK                = 0x00001000,    /* Bit 12 */
    DU_DAC1_LFM_RAMP_MASK                = 0x00002000,    /* Bit 13 */
    DU_DAC2_LFM_RAMP_MASK                = 0x00004000,    /* Bit 14 */
    DU_DAC3_LFM_RAMP_MASK                = 0x00008000,    /* Bit 15 */
    DU_DAC0_TRIGGER_TIMEOUT_MASK         = 0x00010000,    /* Bit 16 */
    DU_DAC1_TRIGGER_TIMEOUT_MASK         = 0x00020000,    /* Bit 17 */
    DU_DAC2_TRIGGER_TIMEOUT_MASK         = 0x00040000,    /* Bit 18 */
    DU_DAC3_TRIGGER_TIMEOUT_MASK         = 0x00080000,    /* Bit 19 */
    DU_USE_INIT_REGS_IN_PBP_MODE_MASK    = 0x00100000,    /* Bit 20 */
    DU_TRIGGER_MODE_MASK                 = 0x00600000,    /* Bit 21..22 */
    DU_FORCE_TRIGGER_MASK                = 0x00800000,    /* Bit 23 */
    DU_LOAD_ACTION_CMD_MASK              = 0x01000000,    /* Bit 24 */
    DU_COMBINER_ENABLE_MASK              = 0x02000000,    /* Bit 25 */
    DU_60MHZ_INPUT_MASK                  = 0x04000000,    /* Bit 26 */
}DU_BOARD_CONTROL_ENUM;

typedef enum
{
    DU_DISABLE                 = 0,
    DU_ENABLE                  = 1
}DU_ENABLE_ENUM;

typedef enum
{
    DU_CW_MODE            = 0,
    DU_PBP_MODE           = 1
}DU_PBP_MODE_ENUM;

typedef enum
{
    DU_NON_BPSK_MODE       = 0,
    DU_BPSK_MODE           = 1
}DU_PBSK_MODE_ENUM;

typedef enum
{
    DU_LFM_DOWN_CHIRP  = 0,
    DU_LFM_UP_CHIRP    = 1
}DU_LFM_RAMP_ENUM;

typedef enum
{
    DU_NO_TIMEOUT      = 0,
    DU_USE_TIMEOUT     = 1
}DU_TRIGGER_TIMEOUT_ENUM;

typedef enum
{
    DU_TRIGGER_EXTERNAL             = 0,
    DU_TRIGGER_INTERNAL_GATED_CW    = 1,
    DU_TRIGGER_INTERNAL             = 2
}DU_TRIGGER_ENUM;

typedef enum
{
    DU_COMBINER_DISABLE            = 0,
    DU_COMBINER_ENABLE             = 1
}DU_COMBINER_ENABLE_ENUM;

typedef enum
{
    DU_60MHZ_SIL            = 0,
    DU_60MHZ_BSL            = 1
}DU_60MHZ_INPUT_ENUM;

class DUDevice : public Device
{
public:
    DUDevice(unsigned int offset);

    STATUS mmap();

    DUFWActionType* getCh0StartingAddress();
    DUFWActionType* getCh1StartingAddress();
    DUFWActionType* getCh2StartingAddress();
    DUFWActionType* getCh3StartingAddress();
    DUFWActionType* getCh4StartingAddress();
    DUFWActionType* getCh5StartingAddress();
    DUFWActionType* getCh6StartingAddress();
    DUFWActionType* getCh7StartingAddress();
    DUFWActionType* getCh8StartingAddress();
    DUFWActionType* getCh9StartingAddress();

    int readReg(int offset);

    void writeReg(int offset, int data);

    int getFirmwareVersionReg();

    int getBoardStatusReg();

    STATUS setBoardControlReg(int val);

    int getBoardControlReg();

    STATUS setDiagInfoReg(int val);

    int getDiagInfoReg();

    STATUS setNumActionsReg(DU_CHANNEL channel, int val);

    void getRegs(int startReg, int endReg);

private:

    unsigned int _offset;

    void *_apbBusAddr;

};


#endif // DUice_H
