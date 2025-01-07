#ifndef TUDevice_H
#define TUDevice_H

#include "Device.h"

typedef struct
{
    int amplitude;
    int freq;
    int spare1;
    int spare2;
}TUCWSignalType;

typedef struct
{
    TU_CHANNEL channelId;
    TUCWSignalType signal;
}TUCWCmdType;

typedef struct
{
    int amplitude;
    unsigned int phaseOffset;
    unsigned int freqHz;
    unsigned int freqMHz;
    unsigned int lfmRamp;
    int phaseCode;
    int reserved;
}TUSignalType;

typedef struct
{
    unsigned int startTime;
    unsigned int stopTime;
    TUSignalType signal;
}TUActionType;

// TO DO: Use the TUSignalType after matching FW interface
typedef struct
{
    int amplitude;
    unsigned int phaseOffset;
    unsigned int freq;
    unsigned int lfmRamp;
    int phaseCode;
    int reserved;
}TUFWSignalType;

// TO DO: Use the TUActionType after matching FW interface
typedef struct
{
    unsigned int startTime;
    unsigned int stopTime;
    TUFWSignalType signal;
}TUFWActionType;

typedef struct
{
    int recvId;
    TU_CHANNEL channelId;
    int sid;
    TUActionType action;
}TUCmdType;

typedef struct
{
    int firmwareVersion;
    int boardStatus;
    int boardControl;
    TUCWSignalType cwSignals[NUM_TU_CHANNELS];
    // For TX FW
    // int spare1[29];
    // End For TX FW
    // For WG1 FW
    int spare1[9];
    int controlMux;
    int fpgaDieTemp;
    int ddsStatus;
    int lmkReadWrite;
    // End For WG1 FW
//  int gatedCWEmOnDur;
    int spare2;
    int gatedCWEmPeriod;
    int triggerTimeout;
    int diagInfo;
    int ch0NumActions;
    int ch1NumActions;
    int ch2NumActions;
    int ch3NumActions;
}TURegType;

#define TU_CH0_STARTING_ADDR_OFFSET 0x0100
#define TU_CH1_STARTING_ADDR_OFFSET 0x0740
#define TU_CH2_STARTING_ADDR_OFFSET 0x0D80
#define TU_CH3_STARTING_ADDR_OFFSET 0x13C0
#define TU_CH4_STARTING_ADDR_OFFSET 0x1A00
#define TU_CH5_STARTING_ADDR_OFFSET 0x2040
#define TU_CH6_STARTING_ADDR_OFFSET 0x2680
#define TU_CH7_STARTING_ADDR_OFFSET 0x2CC0
#define TU_CH8_STARTING_ADDR_OFFSET 0x3300
#define TU_CH9_STARTING_ADDR_OFFSET 0x3940

typedef enum
{
    TU_BOARD_CONTROL_MASK                = 0xC7FFFFFF,    /* Bits 0..26, and 30..31 */
    TU_DAC0_ENABLE_MASK                  = 0x00000001,    /* Bit 0 */
    TU_DAC1_ENABLE_MASK                  = 0x00000002,    /* Bit 1 */
    TU_DAC2_ENABLE_MASK                  = 0x00000004,    /* Bit 2 */
    TU_DAC3_ENABLE_MASK                  = 0x00000008,    /* Bit 3 */
    TU_DAC0_PBP_MODE_MASK                = 0x00000010,    /* Bit 4 */
    TU_DAC1_PBP_MODE_MASK                = 0x00000020,    /* Bit 5 */
    TU_DAC2_PBP_MODE_MASK                = 0x00000040,    /* Bit 6 */
    TU_DAC3_PBP_MODE_MASK                = 0x00000080,    /* Bit 7 */
    TU_DAC0_BPSK_MODE_MASK               = 0x00000100,    /* Bit 8 */
    TU_DAC1_BPSK_MODE_MASK               = 0x00000200,    /* Bit 9 */
    TU_DAC2_BPSK_MODE_MASK               = 0x00000400,    /* Bit 10 */
    TU_DAC3_BPSK_MODE_MASK               = 0x00000800,    /* Bit 11 */
    TU_DAC0_LFM_RAMP_MASK                = 0x00001000,    /* Bit 12 */
    TU_DAC1_LFM_RAMP_MASK                = 0x00002000,    /* Bit 13 */
    TU_DAC2_LFM_RAMP_MASK                = 0x00004000,    /* Bit 14 */
    TU_DAC3_LFM_RAMP_MASK                = 0x00008000,    /* Bit 15 */
    TU_DAC0_TRIGGER_TIMEOUT_MASK         = 0x00010000,    /* Bit 16 */
    TU_DAC1_TRIGGER_TIMEOUT_MASK         = 0x00020000,    /* Bit 17 */
    TU_DAC2_TRIGGER_TIMEOUT_MASK         = 0x00040000,    /* Bit 18 */
    TU_DAC3_TRIGGER_TIMEOUT_MASK         = 0x00080000,    /* Bit 19 */
    TU_USE_INIT_REGS_IN_PBP_MODE_MASK    = 0x00100000,    /* Bit 20 */
    TU_TRIGGER_MODE_MASK                 = 0x00600000,    /* Bit 21..22 */
    TU_FORCE_TRIGGER_MASK                = 0x00800000,    /* Bit 23 */
    TU_LOAD_ACTION_CMD_MASK              = 0x01000000,    /* Bit 24 */
    TU_COMBINER_ENABLE_MASK              = 0x02000000,    /* Bit 25 */
    TU_60MHZ_INPUT_MASK                  = 0x04000000,    /* Bit 26 */
}TU_BOARD_CONTROL_ENUM;

typedef enum
{
    TU_DISABLE                 = 0,
    TU_ENABLE                  = 1
}TU_ENABLE_ENUM;

typedef enum
{
    TU_CW_MODE            = 0,
    TU_PBP_MODE           = 1
}TU_PBP_MODE_ENUM;

typedef enum
{
    TU_NON_BPSK_MODE       = 0,
    TU_BPSK_MODE           = 1
}TU_PBSK_MODE_ENUM;

typedef enum
{
    TU_LFM_DOWN_CHIRP  = 0,
    TU_LFM_UP_CHIRP    = 1
}TU_LFM_RAMP_ENUM;

typedef enum
{
    TU_NO_TIMEOUT      = 0,
    TU_USE_TIMEOUT     = 1
}TU_TRIGGER_TIMEOUT_ENUM;

typedef enum
{
    TU_TRIGGER_EXTERNAL             = 0,
    TU_TRIGGER_INTERNAL_GATED_CW    = 1,
    TU_TRIGGER_INTERNAL             = 2
}TU_TRIGGER_ENUM;

typedef enum
{
    TU_COMBINER_DISABLE            = 0,
    TU_COMBINER_ENABLE             = 1
}TU_COMBINER_ENABLE_ENUM;

typedef enum
{
    TU_60MHZ_SIL            = 0,
    TU_60MHZ_BSL            = 1
}TU_60MHZ_INPUT_ENUM;

class TUDevice : public Device
{
public:
    TUDevice(unsigned int offset);

    STATUS mmap();

    TUFWActionType* getCh0StartingAddress();
    TUFWActionType* getCh1StartingAddress();
    TUFWActionType* getCh2StartingAddress();
    TUFWActionType* getCh3StartingAddress();
    TUFWActionType* getCh4StartingAddress();
    TUFWActionType* getCh5StartingAddress();
    TUFWActionType* getCh6StartingAddress();
    TUFWActionType* getCh7StartingAddress();
    TUFWActionType* getCh8StartingAddress();
    TUFWActionType* getCh9StartingAddress();

    int readReg(int offset);

    void writeReg(int offset, int data);

    int getFirmwareVersionReg();

    int getBoardStatusReg();

    STATUS setBoardControlReg(int val);

    int getBoardControlReg();

    STATUS setCWRegs(TU_CHANNEL channel, TUCWSignalType signal);

    TUCWSignalType getCWRegs(TU_CHANNEL channel);

    STATUS setNumActionsReg(TU_CHANNEL channel, int val);

    void getRegs(int startReg, int endReg);

    void setGatedCWEmulatorRegs(int period);

private:

    unsigned int _offset;

    void *_apbBusAddr;

};


#endif // TUDevice_H
