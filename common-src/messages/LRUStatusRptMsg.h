#ifndef LRUStatusRptMsg_H
#define LRUStatusRptMsg_H

#include <string>

using namespace std;

#include "ReportMessage.h"
#include "SWCMsgTypes.h"

class LRUStatusRptMsg : public ReportMessage
{
public:

    /**
    * Constructor
    *
    */
    explicit LRUStatusRptMsg(MessageId msgId);

    /**
    * Destructor
    *
    */
    virtual ~LRUStatusRptMsg(){};

    /**
    * Method buildMsg builds the message given the current member 
    * variable settings. 
    *
    * @return Status of operation
    */
    virtual STATUS buildMsg();

    /**
    * Method setLRUStatus sets the status field for the particular
    * LRU
    *
    * @param health Overall health
    */
    inline void setLRUStatus(HealthState health);

private:

    /** Status Report data */

    LRUStatusRptDataType _data;

};

void LRUStatusRptMsg::setLRUStatus(HealthState health)
{
    _data.lruOption = health;
}

#endif
