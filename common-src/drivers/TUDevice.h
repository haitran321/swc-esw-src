#ifndef TUDevice_H
#define TUDevice_H

#include "Device.h"

typedef struct
{
    int fwVer;
    int brdStatus;
    int brdCtrl;
    int armKSine[NUM_RFCC_CH];
    int spare1[2];
    int slResult;
    int sysConfigStatus;
    int spare2[2];
    int armInitStatus;
    int swcStatus;
    int swcrStatus;
    int rltdPulseDur;
    int rlcpPulseDur;
    int rlscPulseDur;
    int steeringWordPulseDur;
    int rltdPeriod;             // /RLTD period
    int numRLTDPerCycle;       // /RLTD Runs
    int rltdPreTime;
    int numInc;
    int alphaInc;
    int betaInc;
    int cycleResetTime;        // cycleRepeatTime
    int numCycle;
    int spare3[1];
    int fpgaDieTemp;
    int vccIntVoltage;
    int vccAuxVoltage;
    int vbramVoltage;
    int diagInfo;
    int dcuStatus[NUM_DCU-1];
    int dcuSCLKDelay[NUM_DCU-1];
}TURegType;

typedef enum
{
    TU_OVERALL_STATUS_MASK      = 0x8000003F,    /* Bits 0..5, and 31 */
    TU_READY_STATUS_MASK        = 0x00000001,    /* Bit 0 */
    TU_HIGH_TEMP_ALARM_MASK     = 0x00000002,    /* Bit 1 */
    TU_VCC_INT_ALARM_MASK       = 0x00000004,    /* Bit 2 */
    TU_VCC_AUX_ALARM_MASK       = 0x00000008,    /* Bit 3 */
    TU_VBRAM_ALARM_MASK         = 0x00000010,    /* Bit 4 */
    TU_OVER_TEMP_ALARM_MASK     = 0x00000020,    /* Bit 5 */
    TU_BIT_RESULT_MASK          = 0x80000000,    /* Bit 31 */
}TU_OVERALL_STATUS_ENUM;

typedef enum
{
    TU_BRD_CTRL_MASK                        = 0x80031364,    /* Bits 2, 5..6, 8..9, 10, 16..17 and 31 */
    TU_SW_TRIGGER_MASK                      = 0x00000004,    /* Bit 2 */
    TU_SYSTEM_MODE_MASK                     = 0x00000010,    /* Bits 4 */
    TU_SYSTEM_CONFIG_MASK                   = 0x00000060,    /* Bits 5..6 */
    TU_SYSTEM_OLTE_MASK                     = 0x00000080,    /* Bits 7 */
    TU_RLCP_CMD_MASK                        = 0x00000100,    /* Bits 8 */
    TU_RLSC_CMD_MASK                        = 0x00000200,    /* Bits 9 */
    TU_SHUTDOWN_CMD_MASK                    = 0x00001000,    /* Bit 12 */
    TU_SL_FREQ_SEL_MASK                     = 0x00030000,    /* Bits 16..17 */
    TU_SOFT_RESET_MASK                      = 0x80000000,    /* Bit 31 */
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

typedef enum
{
    TU_ARM_INIT_STATUS_MASK         = 0x80000003,    /* Bits 0..1 */
    TU_OS_INIT_STATUS_MASK          = 0x00000001,    /* Bit 0 */
    TU_APP_INIT_STATUS_MASK         = 0x00000002,    /* Bit 1 */
}TU_ARM_INIT_STATUS_ENUM;

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

    int getSysConfigStatusReg();

    void setARMInitStatusReg(int val);
    int getARMInitStatusReg();

    int getSWCStatusReg();

    int getSWCRStatusReg();

    void setRLTDPulseDurReg(int val);
    int getRLTDPulseDurReg();

    void setRLCPPulseDurReg(int val);
    int getRLCPPulseDurReg();

    void setRLSCPulseDurReg(int val);
    int getRLSCPulseDurReg();

    void setSteeringWordPulseDurReg(int val);
    int getSteeringWordPulseDurReg();

    void setRLTDPeriodReg(int val);
    int getRLTDPeriodReg();

    void setNumRLTDPerCycleReg(int val);
    int getNumRLTDPerCycleReg();

    void setRLTDPreTimeReg(int val);
    int getRLTDPreTimeReg();

    void setNumIncReg(int val);
    int getNumIncReg();

    void setAlphaIncReg(int val);
    int getAlphaIncReg();

    void setBetaIncReg(int val);
    int getBetaIncReg();

    void setCycleResetTimeReg(int val);
    int getCycleResetTimeReg();

    void setNumCycleReg(int val);
    int getNumCycleReg();

    int getFPGADieTempReg();

    void setDiagInfoReg(int val);
    int getDiagInfoReg();

    void getRegs(int startReg, int endReg);

private:

    unsigned int _offset;

    void *_apbBusAddr;

};


#endif // TUDevice_H
