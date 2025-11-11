#ifndef TUDevice_H
#define TUDevice_H

#include "Device.h"

typedef struct
{
    int fwVer;
    int brdStatus;
    int brdCtrl;
    int armKSine[NUM_RFCC_CH];
    int swcrStatus;
    int swcStatus;
    int slResult;
    int spare[19];
    int fpgaDieTemp;
    int vccIntVoltage;
    int vccAuxVoltage;
    int vbramVoltage;
    int diagInfo;
}TURegType;

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
    TU_BRD_CTRL_MASK            = 0x80000007,    /* Bits 0..2, and 31 */
    TU_SET_RLTD_SIGNAL_MASK     = 0x00000001,    /* Bit 0 */
    TU_SET_RLCP_SIGNAL_MASK     = 0x00000002,    /* Bit 1 */
    TU_SET_RLSC_SIGNAL_MASK     = 0x00000004,    /* Bit 2 */
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

class TUDevice : public Device
{
public:
    TUDevice(unsigned int offset);

    STATUS mmap();

    int readReg(int offset);

    void writeReg(int offset, int data);

    int getFWVerReg();

    int getBrdStatusReg();

    void setBrdCtrlReg(int val);
    int getBrdCtrlReg();

    void setArmKSineReg(RFCC_CH ch, int val);
    int getArmKSineReg(RFCC_CH ch);

    int getSLStatusReg();

    void setDiagInfoReg(int val);
    int getDiagInfoReg();

    void getRegs(int startReg, int endReg);

private:

    unsigned int _offset;

    void *_apbBusAddr;

};


#endif // TUDevice_H
