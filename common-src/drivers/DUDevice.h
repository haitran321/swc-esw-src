#ifndef DUice_H
#define DUice_H

#include "Device.h"

typedef struct
{
    int firmwareVersion;
    int boardStatus;
    int boardControl;
    int armKSine[2];
    int atbKSine[2];
    int scanLimitResult;
    int spare1[19];
    int fpgaDieTemp;
    int vccIntVoltage;
    int vccAuxVoltage;
    int vbramVoltage;
    int diagInfo;
}DURegType;

#define DU_CMD_STARTING_ADDR_OFFSET 0x0100

typedef enum
{
    DU_BOARD_CONTROL_MASK               = 0x80000003,    /* Bits 0..1, and 31 */
    DU_TRIGGER_SL_TEST_MASK             = 0x00000001,    /* Bit 0 */
    DU_STEERING_WORD_SRC_MASK           = 0x00000002,    /* Bit 1 */
    DU_SOFT_RESET_MASK                  = 0x80000000,    /* Bit 31 */
}DU_BOARD_CONTROL_ENUM;

typedef enum
{
    DU_ATB              = 0,
    DU_ARM              = 1
}DU_STEERING_WORD_SRC_ENUM;

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
    DU_SL_W_STATUS_MASK         = 0x00000010,    /* Bit 3 */
    DU_SL_EL_STATUS_MASK        = 0x00000020,    /* Bit 4 */
}DU_SL_CHECK_RESULTS_ENUM;

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
