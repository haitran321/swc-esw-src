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

    inline void setAlphaDCUStatus(int dcu, HealthState health);

    inline void setBetaDCUStatus(int dcu, HealthState health);

    inline void setLastAlpha(int alpha);

    inline void setLastBeta(int beta);

private:

    /** Status Report data */

    SWCStatusRptDataType _data;

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
