#ifndef DUDevice_H
#define DUDevice_H

#include "Device.h"

typedef struct
{
    int fwVer;
    int brdStatus;
    int brdCtrl;
    int armKSine[NUM_RFCC_CH];
    int atbKSine[NUM_RFCC_CH];
    int slResult;
    int sysConfigStatus;
    int swcStatusToTwgs;
    int cableDelayComp;
    int armInitStatus;
    int spare1[15];
    int fpgaDieTemp;
    int vccIntVoltage;
    int vccAuxVoltage;
    int vbramVoltage;
    int diagInfo;
    int dcuStatus[NUM_DCU-1];
    int dcuSCLKDelay[NUM_DCU-1];
}DURegType;

typedef enum
{
    DU_HW_OVERALL_STATUS_MASK   = 0x8000003F,    /* Bits 0..5, and 31 */
    DU_READY_STATUS_MASK        = 0x00000001,    /* Bit 0 */
    DU_HIGH_TEMP_ALARM_MASK     = 0x00000002,    /* Bit 1 */
    DU_VCC_INT_ALARM_MASK       = 0x00000004,    /* Bit 2 */
    DU_VCC_AUX_ALARM_MASK       = 0x00000008,    /* Bit 3 */
    DU_VBRAM_ALARM_MASK         = 0x00000010,    /* Bit 4 */
    DU_OVER_TEMP_ALARM_MASK     = 0x00000020,    /* Bit 5 */
    DU_BIT_RESULT_MASK          = 0x80000000,    /* Bit 31 */
}DU_OVERALL_STATUS_ENUM;

typedef enum
{
    DU_BRD_CTRL_MASK                        = 0x8000077F,    /* Bits 0..6, 8..10 and 31 */
    DU_UNIT_TYPE_MASK                       = 0x00000001,    /* Bit 0 */
    DU_TEST_MODE_STEERING_WORD_SRC_MASK     = 0x00000002,    /* Bit 1 */
    DU_TEST_MODE_SW_TRIGGER_MASK            = 0x00000004,    /* Bit 2 */
    DU_FORCE_TEST_MODE_MASK                 = 0x00000008,    /* Bit 3 */
    DU_TEST_MODE_STEERING_WORD_VALID_MASK   = 0x00000010,    /* Bit 4 */
    DU_TEST_MODE_SYSTEM_CONFIG_MASK         = 0x00000060,    /* Bits 5..6 */
    DU_TEST_MODE_DCU_CMD_MASK               = 0x00000700,    /* Bits 8..10 */
    DU_SOFT_RESET_MASK                      = 0x80000000,    /* Bit 31 */
}DU_BOARD_CONTROL_ENUM;

typedef enum
{
    NORMAL  = 0,
    TEST    = 1
}DU_MODE_ENUM;

typedef enum
{
    STEERING_WORD_INVALID  = 0,
    STEERING_WORD_VALID  = 1,
}DU_STEERING_WORD_VALID_FLAG_ENUM;

typedef enum
{
    DCU_CMD_NONE        = 0,
    DCU_CMD_BORESIGHT   = 1,
    DCU_CMD_CALIBRATION = 2,
    DCU_CMD_CRC_RESET   = 2,
    DCU_CMD_DCU_RESET   = 3
}DCU_CMD_ENUM;

typedef enum
{
    DU_KSINE_MASK               = 0x000003FF,    /* Bits 0..9 */
}DU_KSINE_ENUM;

typedef enum
{
    STATUS_DATA_TYPE_ERROR      = 0,
    DATA_TYPE_CONFIG_STATUS     = 1,
    DATA_TYPE_CUSTOM_STATUS     = 2,
    DATA_TYPE_IO_MODULE_STATUS  = 3
}SWC_STATUS_DATA_TYPE;

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
    DCU_STATUS_MASK                 = 0x00FFFFFF,    /* Bits 0..23 */
    DCU_CRC_STATUS_MASK             = 0x00000001,    /* Bit 0 */
    DCU_TYPE_STATUS_MASK            = 0x00000002,    /* Bit 1 */
    DCU_FW_MINOR_REV_MASK           = 0x0000003C,    /* Bit 2..5 */
    DCU_FW_MAJOR_REV_MASK           = 0x000000C0,    /* Bit 6..7 */
    DCU_LOCATION_STATUS_MASK        = 0x0000FF00,    /* Bit 8..15 */
    DCU_BIT_TBD_STATUS_MASK         = 0x00010000,    /* Bit 16 */
    DCU_BIT_COMPARE_STATUS_MASK     = 0x00020000,    /* Bit 17 */
    DCU_BIT_SPI_STATUS_MASK         = 0x00040000,    /* Bit 18 */
    DCU_BIT_LOC_VALID_STATUS_MASK   = 0x00080000,    /* Bit 19 */
    DCU_BIT_CLK_STATUS_MASK         = 0x00100000,    /* Bit 20 */
    DCU_BIT_OVERALL_STATUS_MASK     = 0x00200000,    /* Bit 21 */
    DCU_MODE_STATUS_MASK            = 0x00400000,    /* Bit 22 */
    DCU_BYPASS_STATUS_MASK          = 0x00800000,    /* Bit 23 */
}DCU_STATUS_ENUM;

typedef enum
{
    DU_SYSTEM_STATUS_MASK            = 0x8000003F,    /* Bits 0..5 */
    DU_SYSTEM_CONFIG_MASK            = 0x00000003,    /* Bit 0..1 */
    DU_MODE_MASK                     = 0x00000004,    /* Bit 2 */
    DU_OFFLINE_TEST_ENABLED_MASK     = 0x00000008,    /* Bit 3 */
    DU_BORESIGHT_CMD_MASK            = 0x00000010,    /* Bit 4 */
    DU_CAL_CMD_MASK                  = 0x00000020,    /* Bit 5 */
    DU_SPI_HEALTH_MASK               = 0x00000100,    /* Bit 8 */
}DU_SYSTEM_STATUS_ENUM;

typedef enum
{
    WITH_CRC_ERROR      = 0,
    WITHOUT_CRC_ERROR   = 1
}LAST_ACTION_CRC_STATUS;

//typedef enum
//{
//    DU_STATUS_TO_TWGS_MASK              = 0x7FFFFFFF,    /* Bits 0..30 */
//    DU_OVERALL_STATUS_MASK              = 0x00000001,    /* Bit 0 */
//    DU_DATA_TYPE_MASK                   = 0x00000006,    /* Bit 1..2 */
//    // Data Type = Config Status
//    DU_CONFIG_STATUS_MASK               = 0x00000018,    /* Bit 3..4 */
//    DU_MODE_STATUS_MASK                 = 0x00000020,    /* Bit 5 */
//    // Data Type = Custom Status
//    DU_ALPHA_OVERALL_STATUS_MASK        = 0x00000200,    /* Bit 9 */
//    DU_BETA_OVERALL_STATUS_MASK         = 0x00000400,    /* Bit 10 */
//    DU_ALPHA_DCU_ROLLED_UP_STATUS_MASK  = 0x00001800,    /* Bit 11..12 */
//    DU_BETA_DCU_ROLLED_UP_STATUS_MASK   = 0x00006000,    /* Bit 13..14 */
//    // Data Type = COTS Status
//    DU_TEMP_STATUS_MASK                 = 0x00008000,    /* Bit 15 */
//    DU_12V_PWR_STATUS_MASK              = 0x00010000,    /* Bit 16 */
//    DU_24V_PWR_STATUS_MASK              = 0x00020000,    /* Bit 17 */
//    DU_ATB_STATUS_MASK                  = 0x00040000,    /* Bit 18 */
//    // DCU Status
//    DU_DCU_GROUP_STATUS_MASK            = 0x00200000,    /* Bit 21 */
//    DU_DCU_HEALTH_STATUS_MASK           = 0x00400000,    /* Bit 22 */
//    DU_DCU_NUMBER_STATUS_MASK           = 0x7F800000,    /* Bit 11..18 */  /* Bit 23..30 */
//}SWC_STATUS_TO_TWGS_ENUM;

typedef enum
{
    DU_STATUS_TO_TWGS_MASK              = 0x7FFFFFFF,    /* Bits 0..30 */
    // Data Type = Config Status
    DU_MODE_STATUS_MASK                 = 0x00000008,    /* Bit 3 */
    DU_CONFIG_STATUS_MASK               = 0x00000030,    /* Bit 4..5 */
    // Define Data Type
    DU_DATA_TYPE_MASK                   = 0x000000C0,    /* Bit 6..7 */
    // Overall Status
    DU_OVERALL_STATUS_MASK              = 0x00000100,    /* Bit 8 */
    // Data Type = COTS Status
    DU_ATB_STATUS_MASK                  = 0x00000800,    /* Bit 11 */
    DU_24V_PWR_STATUS_MASK              = 0x00001000,    /* Bit 12 */
    DU_12V_PWR_STATUS_MASK              = 0x00002000,    /* Bit 13 */
    DU_TEMP_STATUS_MASK                 = 0x00004000,    /* Bit 14 */
    // Data Type = Custom Status
    DU_BETA_DCU_ROLLED_UP_STATUS_MASK   = 0x00018000,    /* Bit 15..16 */
    DU_ALPHA_DCU_ROLLED_UP_STATUS_MASK  = 0x00060000,    /* Bit 17..18 */
    DU_BETA_OVERALL_STATUS_MASK         = 0x00080000,    /* Bit 19 */
    DU_ALPHA_OVERALL_STATUS_MASK        = 0x00100000,    /* Bit 20 */
    // DCU Status
    DU_DCU_GROUP_STATUS_MASK            = 0x00200000,    /* Bit 21 */
    DU_DCU_HEALTH_STATUS_MASK           = 0x00400000,    /* Bit 22 */
    DU_DCU_NUMBER_STATUS_MASK           = 0x7F800000,    /* Bit 11..18 */  /* Bit 23..30 */
}SWC_STATUS_TO_TWGS_ENUM;

typedef enum
{
    DU_ARM_INIT_STATUS_MASK         = 0x80000003,    /* Bits 0..1 */
    DU_OS_INIT_STATUS_MASK          = 0x00000001,    /* Bit 0 */
    DU_APP_INIT_STATUS_MASK         = 0x00000002,    /* Bit 1 */
}DU_ARM_INIT_STATUS_ENUM;

class DUDevice : public Device
{
public:
    DUDevice(unsigned int offset);

    STATUS mmap();

    int readReg(int offset);

    void writeReg(int offset, int data);

    int getFWVerReg();

    int getBrdStatusReg();

    void setBrdCtrlReg(int val);
    int getBrdCtrlReg();

    int getArmKSineReg(RFCC_CH ch);
    void setArmKSineReg(RFCC_CH ch, int val);

    int getAtbKSineReg(RFCC_CH ch);
    void setAtbKSineReg(RFCC_CH ch, int val);

    int getSLStatusReg();

    int getSysConfigStatusReg();

    int getSwcStatusToTwgsReg();
    void setSwcStatusToTwgsReg(int val);

    int getCableDelayCompReg();
    void setCableDelayCompReg(int val);

    int getARMInitStatusReg();
    void setARMInitStatusReg(int val);

    void setDiagInfoReg(int val);
    int getDiagInfoReg();

    int getDCUStatusReg(int regNum);
    void setDCUSCLKReg(int dcuNum, int val);

    void getRegs(int startReg, int endReg);

private:

    unsigned int _offset;

    void *_apbBusAddr;

};


#endif // DUDevice_H
