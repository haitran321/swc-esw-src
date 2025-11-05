#ifndef DUAStatusRptMsg_H
#define DUAStatusRptMsg_H

#include <string>

using namespace std;

#include "ReportMessage.h"
#include "SWCMsgTypes.h"

class DUAStatusRptMsg : public ReportMessage
{
public:

    /**
    * Constructor
    *
    */
    DUAStatusRptMsg();

    /**
    * Destructor
    *
    */
    virtual ~DUAStatusRptMsg(){};

    /**
    * Method buildMsg builds the message given the current member 
    * variable settings. 
    *
    * @return Status of operation
    */
    virtual STATUS buildMsg();

    inline void setDUAStatus(DUAStatusParamsType status);

private:

    /** Status Report data */

    DUAStatusParamsType _data;

};

void DUAStatusRptMsg::setDUAStatus(DUAStatusParamsType status)
{
    _data = status;
}

#endif
