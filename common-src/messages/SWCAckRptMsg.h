#ifndef SWCAckRptMsg_H
#define SWCAckRptMsg_H

#include <string>

using namespace std;

#include "ReportMessage.h"
#include "SWCMsgTypes.h"

class SWCAckRptMsg : public ReportMessage
{
public:

    /**
    * Constructor
    *
    */
    SWCAckRptMsg();

    /**
    * Destructor
    *
    */
    virtual ~SWCAckRptMsg(){};

    /**
    * Method buildMsg builds the message given the current member 
    * variable settings. 
    *
    * @return Status of operation
    */
    virtual STATUS buildMsg();

    inline void setModuleType(int mod);

    inline void setAckType(SWCAckType ack);

private:

    /** Ack Report data */

    SWCAckDataType _data;

};

void SWCAckRptMsg::setModuleType(int mod)
{
    _data.modType = mod;
}

void SWCAckRptMsg::setAckType(SWCAckType ack)
{
    _data.ackType = ack;
}

#endif
