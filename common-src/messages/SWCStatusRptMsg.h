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

#endif
