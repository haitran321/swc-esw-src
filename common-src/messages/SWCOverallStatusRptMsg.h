#ifndef SWCOverallStatusRptMsg_H
#define SWCOverallStatusRptMsg_H

#include <string>

using namespace std;

#include "ReportMessage.h"
#include "SWCMsgTypes.h"

class SWCOverallStatusRptMsg : public ReportMessage
{
public:

    /**
    * Constructor
    *
    */
    SWCOverallStatusRptMsg();

    /**
    * Destructor
    *
    */
    virtual ~SWCOverallStatusRptMsg(){};

    /**
    * Method buildMsg builds the message given the current member 
    * variable settings. 
    *
    * @return Status of operation
    */
    virtual STATUS buildMsg();

    inline void setSWCStatus(HealthState health);

    inline void setSWCConfig(SWC_CONFIG config);

    inline void setSWCMode(SWC_MODE mode);

    inline void setAlphaDUStatus(HealthState health);

    inline void setBetaDUStatus(HealthState health);

    inline void setAlphaDCURolledUpStatus(DCURolledUpStatus status);

    inline void setBetaDCURolledUpStatus(DCURolledUpStatus status);

    inline void setTempStatus(HealthState health);

    inline void set12VPwrStatus(HealthState health);

    inline void set24VPwrStatus(HealthState health);

    inline void setATBStatus(HealthState health);

    inline void setTUHWStatus(HealthState health);

    inline void setAlphaDCUStatus(int dcu, HealthState health);

    inline void setBetaDCUStatus(int dcu, HealthState health);

    inline void setLastARMAlpha(int alpha);

    inline void setLastARMBeta(int beta);

    inline void setLastATBAlpha(int alpha);

    inline void setLastATBBeta(int beta);

private:

    /** Status Report data */

    SWCOverallStatusDataType _data;

};

void SWCOverallStatusRptMsg::setSWCStatus(HealthState health)
{
    _data.swcStatus = health;
}

void SWCOverallStatusRptMsg::setSWCConfig(SWC_CONFIG config)
{
    _data.swcConfig = config;
}

void SWCOverallStatusRptMsg::setSWCMode(SWC_MODE mode)
{
    _data.swcMode = mode;
}

void SWCOverallStatusRptMsg::setAlphaDUStatus(HealthState health)
{
    _data.swcAlphaDUStatus = health;
}

void SWCOverallStatusRptMsg::setBetaDUStatus(HealthState health)
{
    _data.swcBetaDUStatus = health;
}

void SWCOverallStatusRptMsg::setAlphaDCURolledUpStatus(DCURolledUpStatus status)
{
    _data.swcAlphaDCURolledUpStatus = status;
}

void SWCOverallStatusRptMsg::setBetaDCURolledUpStatus(DCURolledUpStatus status)
{
    _data.swcBetaDCURolledUpStatus = status;
}

void SWCOverallStatusRptMsg::setTempStatus(HealthState health)
{
    _data.swcTempStatus = health;
}

void SWCOverallStatusRptMsg::set12VPwrStatus(HealthState health)
{
    _data.swc12VPwrStatus = health;
}

void SWCOverallStatusRptMsg::set24VPwrStatus(HealthState health)
{
    _data.swc24VPwrStatus = health;
}

void SWCOverallStatusRptMsg::setATBStatus(HealthState health)
{
    _data.swcATBStatus = health;
}

void SWCOverallStatusRptMsg::setTUHWStatus(HealthState health)
{
    _data.testUnitHWStatus = health;
}

void SWCOverallStatusRptMsg::setAlphaDCUStatus(int dcu, HealthState health)
{
    _data.alphaDCU[dcu] = health;
}

void SWCOverallStatusRptMsg::setBetaDCUStatus(int dcu, HealthState health)
{
    _data.betaDCU[dcu] = health;
}

void SWCOverallStatusRptMsg::setLastARMAlpha(int alpha)
{
    _data.lastARMAlpha = alpha;
    printf("lastARMAlpha = %d\n", _data.lastARMAlpha);
}

void SWCOverallStatusRptMsg::setLastARMBeta(int beta)
{
    _data.lastARMBeta = beta;
    printf("lastARMBeta = %d\n", _data.lastARMBeta);
}

void SWCOverallStatusRptMsg::setLastATBAlpha(int alpha)
{
    _data.lastATBAlpha = alpha;
    printf("lastATBAlpha = %d\n", _data.lastATBAlpha);
}

void SWCOverallStatusRptMsg::setLastATBBeta(int beta)
{
    _data.lastATBBeta = beta;
    printf("lastATBBeta = %d\n", _data.lastATBBeta);
}

#endif
