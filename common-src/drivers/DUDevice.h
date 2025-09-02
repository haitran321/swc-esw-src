#ifndef DUDevice_H
#define DUDevice_H

#include "Device.h"

#define NUM_DCU 152

typedef struct
{
    int fwVer;
    int brdStatus;
    int brdCtrl;
    int armKSine[NUM_RFCC_CH];
    int atbKSine[NUM_RFCC_CH];
    int slResult;
    int sysConfigStatus;
    int swcrStatusToTwgs;
    int spare1[17];
    int fpgaDieTemp;
    int vccIntVoltage;
    int vccAuxVoltage;
    int vbramVoltage;
    int diagInfo;
    int dcuStatus[NUM_DCU];
    int dcuSCLKDelay[NUM_DCU];
}DURegType;

//#define DU_CMD_STARTING_ADDR_OFFSET 0x0100

typedef enum
{
    DU_OVERALL_STATUS_MASK      = 0x8000001F,    /* Bits 0..4, and 31 */
    DU_READY_STATUS_MASK        = 0x80000001,    /* Bit 0 */
    DU_HIGH_TEMP_ALARM_MASK     = 0x00000002,    /* Bit 1 */
    DU_VCC_INT_ALARM_MASK       = 0x00000004,    /* Bit 2 */
    DU_VCC_AUX_ALARM_MASK       = 0x80000008,    /* Bit 3 */
    DU_VBRAM_ALARM_MASK         = 0x80000010,    /* Bit 4 */
    DU_BIT_RESULT_MASK          = 0x80000000,    /* Bit 31 */
}DU_OVERALL_STATUS_ENUM;

typedef enum
{
    DU_BRD_CTRL_MASK            = 0x8000001F,    /* Bits 0..4, and 31 */
    DU_UNIT_TYPE_MASK           = 0x00000001,    /* Bit 0 */
    DU_STEERING_WORD_SRC_MASK   = 0x00000002,    /* Bit 1 */
    DU_NEW_STEERING_WORD_MASK   = 0x00000004,    /* Bit 2 */
    DU_FORCE_TEST_MODE_MASK     = 0x00000008,    /* Bit 3 */
    DU_RESET_CRC_MASK           = 0x00000010,    /* Bit 4 */
    DU_SOFT_RESET_MASK          = 0x80000000,    /* Bit 31 */
}DU_BOARD_CONTROL_ENUM;

typedef enum
{
    ATB     = 0,
    ARM     = 1
}DU_STEERING_WORD_SRC_ENUM;

typedef enum
{
    NORMAL  = 0,
    TEST    = 1
}DU_MODE_ENUM;

typedef enum
{
    DU_KSINE_MASK               = 0x000003FF,    /* Bits 0..9 */
}DU_KSINE_ENUM;

typedef enum
{
    DU_SL_CHECK_RESULTS_MASK    = 0x0000001F,    /* Bits 0..4 */
    DU_SL_OVERALL_STATUS_MASK   = 0x00000001,    /* Bit 0 */
    DU_SL_U_STATUS_MASK         = 0x00000002,    /* Bit 1 */
    DU_SL_V_STATUS_MASK         = 0x00000004,    /* Bit 2 */
    DU_SL_W_STATUS_MASK         = 0x00000008,    /* Bit 3 */
    DU_SL_EL_STATUS_MASK        = 0x00000010,    /* Bit 4 */
}DU_SL_CHECK_RESULTS_ENUM;

typedef enum
{
    DCU_SL_CHECK_RESULTS_MASK   = 0x00FFFF01,    /* Bits 0, 8..23 */
    DCU_CRC_STATUS_MASK         = 0x00000001,    /* Bit 0 */
    DCU_LOCATION_STATUS_MASK    = 0x00000FF0,    /* Bit 8..15 */
    DCU_BIT_STATUS_MASK         = 0x0003F000,    /* Bit 16..21 */
    DCU_MODE_STATUS_MASK        = 0x00040000,    /* Bit 22 */
    DCU_BYPASS_STATUS_MASK      = 0x00080000,    /* Bit 23 */
}DCU_STATUS_ENUM;

typedef enum
{
    DU_SYSTEM_CONFIG_MASK            = 0x8000003F,    /* Bits 0..5 */
    DU_CONFIG_MASK                   = 0x00000003,    /* Bit 0..1 */
    DU_MODE_MASK                     = 0x00000004,    /* Bit 2 */
    DU_OFFLINE_TEST_ENABLED_MASK     = 0x00000008,    /* Bit 3 */
    DU_BORESIGHT_CMD_MASK            = 0x00000010,    /* Bit 4 */
    DU_CAL_CMD_MASK                  = 0x00000020,    /* Bit 5 */
}DU_SYSTEM_CONFIG_ENUM;

typedef enum
{
    DU_SYSTEM_CONFIG_STATUS_MASK    = 0x0003FFFF,    /* Bits 0..17 */
    DU_SWCR_OVERALL_STATUS_MASK     = 0x00000001,    /* Bit 0 */
    DU_SWCR_CONFIG_MASK             = 0x00000006,    /* Bit 1..2 */
    DU_SWCR_MODE_MASK               = 0x00000008,    /* Bit 3 */
    DU_ALPHA_OVERALL_STATUS_MASK    = 0x00000010,    /* Bit 4 */
    DU_ALPHA_PS_STATUS_MASK         = 0x00000020,    /* Bit 5 */
    DU_BETA_OVERALL_STATUS_MASK     = 0x00000040,    /* Bit 6 */
    DU_BETA_PS_STATUS_MASK          = 0x00000080,    /* Bit 7 */
    DU_DCU_STATUS_GROUP_MASK        = 0x00000100,    /* Bit 8 */
    DU_DCU_STATUS_NUMBER_MASK       = 0x0001FE00,    /* Bit 9..16 */
    DU_DCU_STATUS_HEALTH_MASK       = 0x00020000,    /* Bit 17 */
}SWCR_STATUS_TO_TWGS_ENUM;

class DUDevice : public Device
{
public:
    DUDevice(unsigned int offset);

    STATUS mmap();

//  DUFWActionType* getCmdStartingAddress();

    int readReg(int offset);

    void writeReg(int offset, int data);

    int getFWVerReg();

    int getBrdStatusReg();

    void setBrdCtrlReg(int val);
    int getBrdCtrlReg();

    void setArmKSineReg(RFCC_CH ch, int val);
    int getArmKSineReg(RFCC_CH ch);

    void setAtbKSineReg(RFCC_CH ch, int val);
    int getAtbKSineReg(RFCC_CH ch);

    int getSLStatusReg();

    void setDiagInfoReg(int val);
    int getDiagInfoReg();

    int getDCUStatusReg(int dcuNum);
    void setDCUSCLKReg(int dcuNum, int val);

    void getRegs(int startReg, int endReg);

private:

    unsigned int _offset;

    void *_apbBusAddr;

};


#endif // DUDevice_H
