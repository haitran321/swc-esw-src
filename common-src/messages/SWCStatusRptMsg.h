#ifndef SWCStatusRptMsg_H
#define SWCStatusRptMsg_H

#include <string>

using namespace std;

#include "ReportMessage.h"
#include "SWCMsgTypes.h"

class SWCStatusRptMsg : public ReportMessage
{
public:

    /**
    * Constructor
    *
    */
    SWCStatusRptMsg();

    /**
    * Destructor
    *
    */
    virtual ~SWCStatusRptMsg(){};

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

    inline void setPwrSuppliesStatus(HealthState health);

    inline void setTUHWStatus(HealthState health);

    inline void setAlphaDCUStatus(int dcu, HealthState health);

    inline void setBetaDCUStatus(int dcu, HealthState health);

    inline void setLastAlpha(int alpha);

    inline void setLastBeta(int beta);

private:

    /** Status Report data */

    SWCStatusDataType _data;

};

void SWCStatusRptMsg::setSWCStatus(HealthState health)
{
    _data.swcStatus = health;
}

void SWCStatusRptMsg::setSWCConfig(SWC_CONFIG config)
{
    _data.swcConfig = config;
}

void SWCStatusRptMsg::setSWCMode(SWC_MODE mode)
{
    _data.swcMode = mode;
}

void SWCStatusRptMsg::setAlphaDUStatus(HealthState health)
{
    _data.swcAlphaDUStatus = health;
}

void SWCStatusRptMsg::setBetaDUStatus(HealthState health)
{
    _data.swcBetaDUStatus = health;
}

void SWCStatusRptMsg::setAlphaDCURolledUpStatus(DCURolledUpStatus status)
{
    _data.swcAlphaDCURolledUpStatus = status;
}

void SWCStatusRptMsg::setBetaDCURolledUpStatus(DCURolledUpStatus status)
{
    _data.swcBetaDCURolledUpStatus = status;
}

void SWCStatusRptMsg::setTempStatus(HealthState health)
{
    _data.swcTempStatus = health;
}

void SWCStatusRptMsg::setPwrSuppliesStatus(HealthState health)
{
    _data.swcPwrSuppliesStatus = health;
}

void SWCStatusRptMsg::setTUHWStatus(HealthState health)
{
    _data.testUnitHWStatus = health;
}

void SWCStatusRptMsg::setAlphaDCUStatus(int dcu, HealthState health)
{
    _data.alphaDCU[dcu] = health;
}

void SWCStatusRptMsg::setBetaDCUStatus(int dcu, HealthState health)
{
    _data.betaDCU[dcu] = health;
}

void SWCStatusRptMsg::setLastAlpha(int alpha)
{
    _data.lastAlpha = alpha;
    printf("lastAlpha = %d\n", _data.lastAlpha);
}

void SWCStatusRptMsg::setLastBeta(int beta)
{
    _data.lastBeta = beta;
    printf("lastBeta = %d\n", _data.lastBeta);
}

#endif
