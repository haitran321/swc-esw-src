#ifndef SWCDetailedStatusRptMsg_H
#define SWCDetailedStatusRptMsg_H

#include <string>

using namespace std;

#include "ReportMessage.h"
#include "SWCMsgTypes.h"

class SWCDetailedStatusRptMsg : public ReportMessage
{
public:

    /**
    * Constructor
    *
    */
    SWCDetailedStatusRptMsg();

    /**
    * Destructor
    *
    */
    virtual ~SWCDetailedStatusRptMsg(){};

    /**
    * Method buildMsg builds the message given the current member 
    * variable settings. 
    *
    * @return Status of operation
    */
    virtual STATUS buildMsg();

    inline void setSWCDetailedStatus(SWCDetailedStatusDataType status);
    inline void setSWCOverallStatus(HealthState status);
    inline void setAlphaDUDetailedStatus(DUTUStatusType status);
    inline void setBetaDUDetailedStatus(DUTUStatusType status);
    inline void setTUDetailedStatus(DUTUStatusType status);
    inline void setPSDetailedStatus(PSStatusType status);
    inline void setTempDetailedStatus(TempStatusType status);

private:

    /** Status Report data */

    SWCDetailedStatusDataType _data;

};

void SWCDetailedStatusRptMsg::setSWCDetailedStatus(SWCDetailedStatusDataType status)
{
    _data = status;
}

void SWCDetailedStatusRptMsg::setSWCOverallStatus(HealthState status)
{
    _data.swcStatus = status;
}

void SWCDetailedStatusRptMsg::setAlphaDUDetailedStatus(DUTUStatusType status)
{
    _data.alphaDUStatus = status;
}

void SWCDetailedStatusRptMsg::setBetaDUDetailedStatus(DUTUStatusType status)
{
    _data.betaDUStatus = status;
}

void SWCDetailedStatusRptMsg::setTUDetailedStatus(DUTUStatusType status)
{
    _data.tuStatus = status;
}

void SWCDetailedStatusRptMsg::setPSDetailedStatus(PSStatusType status)
{
    _data.psStatus = status;
}

void SWCDetailedStatusRptMsg::setTempDetailedStatus(TempStatusType status)
{
    _data.tempStatus = status;
}

#endif
