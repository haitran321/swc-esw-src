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

private:

    /** Status Report data */

    SWCDetailedStatusDataType _data;

};

void SWCDetailedStatusRptMsg::setSWCDetailedStatus(SWCDetailedStatusDataType status)
{
    _data = status;
}

#endif
