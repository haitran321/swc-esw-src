#ifndef TUDevice_H
#define TUDevice_H

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
    int spare1[15];
    int atbEmulator;
    int dcuEmulator;
    int fpgaDieTemp;
    int vccIntVoltage;
    int vccAuxVoltage;
    int vbramVoltage;
    int diagInfo;
    int dcuStatus[NUM_DCU];
}TURegType;

//#define TU_CMD_STARTING_ADDR_OFFSET 0x0100

typedef enum
{
    TU_OVERALL_STATUS_MASK      = 0x8000001F,    /* Bits 0..4 */
    TU_READY_STATUS_MASK        = 0x80000001,    /* Bit 0 */
    TU_HIGH_TEMP_ALARM_MASK     = 0x00000002,    /* Bit 1 */
    TU_VCC_INT_ALARM_MASK       = 0x00000004,    /* Bit 2 */
    TU_VCC_AUX_ALARM_MASK       = 0x80000008,    /* Bit 3 */
    TU_VBRAM_ALARM_MASK         = 0x80000010,    /* Bit 4 */
}TU_OVERALL_STATUS_ENUM;

typedef enum
{
    TU_BRD_CTRL_MASK            = 0x8000001F,    /* Bits 0..4, and 31 */
    TU_UNIT_TYPE_MASK           = 0x00000001,    /* Bit 0 */
    TU_STEERING_WORD_SRC_MASK   = 0x00000002,    /* Bit 1 */
    TU_NEW_STEERING_WORD_MASK   = 0x00000004,    /* Bit 2 */
    TU_MODE_MASK                = 0x00000008,    /* Bit 3 */
    TU_RESET_CRC_MASK           = 0x00000010,    /* Bit 4 */
    TU_SOFT_RESET_MASK          = 0x80000000,    /* Bit 31 */
}TU_BOARD_CONTROL_ENUM;

typedef enum
{
    TU_KSINE_MASK               = 0x000003FF,    /* Bits 0..9 */
}TU_KSINE_ENUM;

typedef enum
{
    TU_SL_CHECK_RESULTS_MASK    = 0x0000001F,    /* Bits 0..4 */
    TU_SL_OVERALL_STATUS_MASK   = 0x00000001,    /* Bit 0 */
    TU_SL_U_STATUS_MASK         = 0x00000002,    /* Bit 1 */
    TU_SL_V_STATUS_MASK         = 0x00000004,    /* Bit 2 */
    TU_SL_W_STATUS_MASK         = 0x00000008,    /* Bit 3 */
    TU_SL_EL_STATUS_MASK        = 0x00000010,    /* Bit 4 */
}TU_SL_CHECK_RESULTS_ENUM;

//typedef enum
//{
//    DCU_SL_CHECK_RESULTS_MASK   = 0x00FFFF01,    /* Bits 0, 8..23 */
//    DCU_CRC_STATUS_MASK         = 0x00000001,    /* Bit 0 */
//    DCU_LOCATION_STATUS_MASK    = 0x00000FF0,    /* Bit 8..15 */
//    DCU_BIT_STATUS_MASK         = 0x0003F000,    /* Bit 16..21 */
//    DCU_MODE_STATUS_MASK        = 0x00040000,    /* Bit 22 */
//    DCU_BYPASS_STATUS_MASK      = 0x00080000,    /* Bit 23 */
//}DCU_STATUS_ENUM;

typedef enum
{
    TU_SYSTEM_CONFIG_STATUS_MASK    = 0x8000000F,    /* Bits 0..3 */
    TU_CONFIG_STATUS_MASK           = 0x80000003,    /* Bit 0..1 */
    TU_MODE_STATUS_MASK             = 0x00000004,    /* Bit 2 */
    TU_TEST_ENABLED_STATUS_MASK     = 0x00000008,    /* Bit 3 */
}TU_SYSTEM_CONFIG_STATUS_ENUM;

class TUDevice : public Device
{
public:
    TUDevice(unsigned int offset);

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

    void getRegs(int startReg, int endReg);

private:

    unsigned int _offset;

    void *_apbBusAddr;

};


#endif // TUDevice_H
