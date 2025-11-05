#ifndef TempStatusRptMsg_H
#define TempStatusRptMsg_H

#include <string>

using namespace std;

#include "ReportMessage.h"
#include "SWCMsgTypes.h"

class TempStatusRptMsg : public ReportMessage
{
public:

    /**
    * Constructor
    *
    */
    TempStatusRptMsg();

    /**
    * Destructor
    *
    */
    virtual ~TempStatusRptMsg(){};

    /**
    * Method buildMsg builds the message given the current member 
    * variable settings. 
    *
    * @return Status of operation
    */
    virtual STATUS buildMsg();

    inline void setTempStatus(TempStatusParamsType status);

private:

    /** Status Report data */

    TempStatusParamsType _data;

};

void TempStatusRptMsg::setTempStatus(TempStatusParamsType status)
{
    _data = status;
}

#endif
