#ifndef PSStatusRptMsg_H
#define PSStatusRptMsg_H

#include <string>

using namespace std;

#include "ReportMessage.h"
#include "SWCMsgTypes.h"

class PSStatusRptMsg : public ReportMessage
{
public:

    /**
    * Constructor
    *
    */
    PSStatusRptMsg();

    /**
    * Destructor
    *
    */
    virtual ~PSStatusRptMsg(){};

    /**
    * Method buildMsg builds the message given the current member 
    * variable settings. 
    *
    * @return Status of operation
    */
    virtual STATUS buildMsg();

    inline void setPSStatus(PSStatusParamsType status);

private:

    /** Status Report data */

    PSStatusParamsType _data;

};

void PSStatusRptMsg::setPSStatus(PSStatusParamsType status)
{
    _data = status;
}

#endif
