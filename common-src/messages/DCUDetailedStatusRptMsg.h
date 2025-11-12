#ifndef DCUDetailedStatusRptMsg_H
#define DCUDetailedStatusRptMsg_H

#include <string>

using namespace std;

#include "ReportMessage.h"
#include "SWCMsgTypes.h"

class DCUDetailedStatusRptMsg : public ReportMessage
{
public:

    /**
    * Constructor
    *
    */
    DCUDetailedStatusRptMsg();

    /**
    * Destructor
    *
    */
    virtual ~DCUDetailedStatusRptMsg(){};

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

void DCUDetailedStatusRptMsg::setDCUStatus(DCUStatusParamsType status)
{
    _data = status;
}

#endif
