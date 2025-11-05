#ifndef DUBStatusRptMsg_H
#define DUBStatusRptMsg_H

#include <string>

using namespace std;

#include "ReportMessage.h"
#include "SWCMsgTypes.h"

class DUBStatusRptMsg : public ReportMessage
{
public:

    /**
    * Constructor
    *
    */
    DUBStatusRptMsg();

    /**
    * Destructor
    *
    */
    virtual ~DUBStatusRptMsg(){};

    /**
    * Method buildMsg builds the message given the current member 
    * variable settings. 
    *
    * @return Status of operation
    */
    virtual STATUS buildMsg();

    inline void setDUBStatus(DUBStatusParamsType status);

private:

    /** Status Report data */

    DUBStatusParamsType _data;

};

void DUBStatusRptMsg::setDUBStatus(DUBStatusParamsType status)
{
    _data = status;
}

#endif
