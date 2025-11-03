#ifndef DCUStatusRptMsg_H
#define DCUStatusRptMsg_H

#include <string>

using namespace std;

#include "ReportMessage.h"
#include "SWCMsgTypes.h"

class DCUStatusRptMsg : public ReportMessage
{
public:

    /**
    * Constructor
    *
    */
    DCUStatusRptMsg();

    /**
    * Destructor
    *
    */
    virtual ~DCUStatusRptMsg(){};

    /**
    * Method buildMsg builds the message given the current member 
    * variable settings. 
    *
    * @return Status of operation
    */
    virtual STATUS buildMsg();

    inline void setDCUStatus(DCUStatusParamsType status);

private:

    /** Status Report data */

    DCUStatusParamsType _data;

};

void DCUStatusRptMsg::setDCUStatus(DCUStatusParamsType status)
{
    _data = status;
}

#endif
