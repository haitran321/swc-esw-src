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

    inline void setOLTEMode(SWC_MODE mode);

    inline void setAlphaDUStatus(RolledUpStatus status);

    inline void setBetaDUStatus(RolledUpStatus status);

    inline void setAlphaDCURolledUpStatus(RolledUpStatus status);

    inline void setBetaDCURolledUpStatus(RolledUpStatus status);

    inline void setTempStatus(HealthState health);

    inline void set12VPwrStatus(HealthState health);

    inline void set24VPwrStatus(HealthState health);

    inline void setATBStatus(HealthState health);

    inline void setTUHWStatus(RolledUpStatus status);

    inline void setAlphaDCUStatus(int dcu, DCUHealthState health);

    inline void setBetaDCUStatus(int dcu, DCUHealthState health);

    inline void setLastProcessedAlpha(int alpha);

    inline void setLastProcessedBeta(int beta);

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

void SWCOverallStatusRptMsg::setOLTEMode(SWC_MODE mode)
{
    _data.olteMode = mode;
}

void SWCOverallStatusRptMsg::setAlphaDUStatus(RolledUpStatus status)
{
    _data.swcAlphaDUStatus = status;
}

void SWCOverallStatusRptMsg::setBetaDUStatus(RolledUpStatus status)
{
    _data.swcBetaDUStatus = status;
}

void SWCOverallStatusRptMsg::setAlphaDCURolledUpStatus(RolledUpStatus status)
{
    _data.swcAlphaDCURolledUpStatus = status;
}

void SWCOverallStatusRptMsg::setBetaDCURolledUpStatus(RolledUpStatus status)
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

void SWCOverallStatusRptMsg::setTUHWStatus(RolledUpStatus status)
{
    _data.testUnitHWStatus = status;
}

void SWCOverallStatusRptMsg::setAlphaDCUStatus(int dcu, DCUHealthState health)
{
    _data.alphaDCU[dcu] = health;
}

void SWCOverallStatusRptMsg::setBetaDCUStatus(int dcu, DCUHealthState health)
{
    _data.betaDCU[dcu] = health;
}

void SWCOverallStatusRptMsg::setLastProcessedAlpha(int alpha)
{
    _data.lastProcessedAlpha = alpha;
}

void SWCOverallStatusRptMsg::setLastProcessedBeta(int beta)
{
    _data.lastProcessedBeta = beta;
}

#endif
