#ifndef DUice_H
#define DUice_H

#include "Device.h"

#define NUM_DCUs 152

typedef struct
{
    int firmwareVersion;
    int boardStatus;
    int boardControl;
    int armKSine[2];
    int atbKSine[2];
    int scanLimitResult;
    int systemConfigStatus;
    int swcrStatusToTwgs;
    int spare1[15];
    int atbEmulator;
    int dcuEmulator;
    int fpgaDieTemp;
    int vccIntVoltage;
    int vccAuxVoltage;
    int vbramVoltage;
    int diagInfo;
    int dcuStatus[NUM_DCUs];
}DURegType;

//#define DU_CMD_STARTING_ADDR_OFFSET 0x0100

typedef enum
{
    DU_OVERALL_STATUS_MASK      = 0x8000001F,    /* Bits 0..4 */
    DU_READY_STATUS_MASK        = 0x80000001,    /* Bit 0 */
    DU_HIGH_TEMP_ALARM_MASK     = 0x00000002,    /* Bit 1 */
    DU_VCC_INT_ALARM_MASK       = 0x00000004,    /* Bit 2 */
    DU_VCC_AUX_ALARM_MASK       = 0x80000008,    /* Bit 3 */
    DU_VBRAM_ALARM_MASK         = 0x80000010,    /* Bit 4 */
}DU_OVERALL_STATUS_ENUM;

typedef enum
{
    DU_BOARD_CONTROL_MASK       = 0x8000001F,    /* Bits 0..4, and 31 */
    DU_UNIT_TYPE_MASK           = 0x00000001,    /* Bit 0 */
    DU_STEERING_WORD_SRC_MASK   = 0x00000002,    /* Bit 1 */
    DU_NEW_STEERING_WORD_MASK   = 0x00000004,    /* Bit 2 */
    DU_MODE_MASK                = 0x00000008,    /* Bit 3 */
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
}DU_TEST_MODE_ENUM;


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
    DU_SYSTEM_CONFIG_STATUS_MASK    = 0x8000000F,    /* Bits 0..3 */
    DU_CONFIG_STATUS_MASK           = 0x80000003,    /* Bit 0..1 */
    DU_MODE_STATUS_MASK             = 0x00000004,    /* Bit 2 */
    DU_TEST_ENABLED_STATUS_MASK     = 0x00000008,    /* Bit 3 */
}DU_SYSTEM_CONFIG_STATUS_ENUM;

class DUDevice : public Device
{
public:
    DUDevice(unsigned int offset);

    STATUS mmap();

//  DUFWActionType* getCmdStartingAddress();

    int readReg(int offset);

    void writeReg(int offset, int data);

    int getFirmwareVersionReg();

    int getBoardStatusReg();

    STATUS setBoardControlReg(int val);
    int getBoardControlReg();

    STATUS setArmKSineReg(RFCC_CH ch, int val);
    int getArmKSineReg(RFCC_CH ch);

    STATUS setAtbKSineReg(RFCC_CH ch, int val);
    int getAtbKSineReg(RFCC_CH ch);

    int getSLStatusReg();

    STATUS setDiagInfoReg(int val);
    int getDiagInfoReg();

    void getRegs(int startReg, int endReg);

private:

    unsigned int _offset;

    void *_apbBusAddr;

};


#endif // DUice_H
