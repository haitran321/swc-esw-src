#ifndef StatusSentToTWGSRptMsg_H
#define StatusSentToTWGSRptMsg_H

#include <string>

using namespace std;

#include "ReportMessage.h"
#include "SWCMsgTypes.h"

class StatusSentToTWGSRptMsg : public ReportMessage
{
public:

    /**
    * Constructor
    *
    */
    StatusSentToTWGSRptMsg();

    /**
    * Destructor
    *
    */
    virtual ~StatusSentToTWGSRptMsg(){};

    /**
    * Method buildMsg builds the message given the current member 
    * variable settings. 
    *
    * @return Status of operation
    */
    virtual STATUS buildMsg();

    inline void setSWCStatus(int status);

    inline void setSWCRStatus(int status);

private:

    /** Status Report data */

    StatusSentToTWGSDataType _data;

};

void StatusSentToTWGSRptMsg::setSWCStatus(int status)
{
    _data.swcStatus = status;
}

void StatusSentToTWGSRptMsg::setSWCRStatus(int status)
{
    _data.swcrStatus = status;
}


#endif
