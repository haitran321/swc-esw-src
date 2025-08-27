#ifndef StatusRptMsg_H
#define StatusRptMsg_H

#include <string>

using namespace std;

#include "ReportMessage.h"

typedef struct
{
    HealthState overallStatus;
    HealthState lmkPLL1Status;
    HealthState lmkPLL2Status;
    HealthState memTestStatus;
    HealthState bitTestStatus;
    HealthState triggerTimeoutError;
    HealthState adcLoopbackStatus;
    HealthState spare;
} StatusRptDataType;

class StatusRptMsg : public ReportMessage
{
public:

    /**
    * Constructor
    *
    */
    explicit StatusRptMsg(MessageId msgId);

    /**
    * Destructor
    *
    */
    virtual ~StatusRptMsg(){};

    /**
    * Method buildMsg builds the message given the current member 
    * variable settings. 
    *
    * @return Status of operation
    */
    virtual STATUS buildMsg();

    /**
    * Method setOverallStatus sets the position field.
    *
    * @param health Overall health
    */
    inline void setOverallStatus(HealthState health);

    /**
    * Method setLMKPLL1Status sets the position field.
    *
    * @param health Overall health
    */
    inline void setLMKPLL1Status(HealthState health);

    /**
    * Method setLMKPLL2Status sets the position field.
    *
    * @param health Overall health
    */
    inline void setLMKPLL2Status(HealthState health);

    /**
    * Method setMemTestStatus sets the position field.
    *
    * @param health Overall health
    */
    inline void setMemTestStatus(HealthState health);

    /**
    * Method setBitTestStatus sets the position field.
    *
    * @param health Overall health
    */
    inline void setBitTestStatus(HealthState health);

    /**
    * Method setTriggerTimeoutError sets the position field.
    *
    * @param health Overall health
    */
    inline void setTriggerTimeoutError(HealthState health);

    /**
    * Method setACDLoopbackStatus sets the position field.
    *
    * @param health Overall health
    */
    inline void setACDLoopbackStatus(HealthState health);

    /**
    * Method setAllGreenStatus sets the position field.
    *
    */
    inline void setAllGreenStatus();


private:

    /** status report data */

    StatusRptDataType _data;

};

void StatusRptMsg::setOverallStatus(HealthState health)
{
    _data.overallStatus = health;
}

void StatusRptMsg::setLMKPLL1Status(HealthState health)
{
    _data.lmkPLL1Status = health;
}

void StatusRptMsg::setLMKPLL2Status(HealthState health)
{
    _data.lmkPLL2Status = health;
}

void StatusRptMsg::setMemTestStatus(HealthState health)
{
    _data.memTestStatus = health;
}

void StatusRptMsg::setBitTestStatus(HealthState health)
{
    _data.bitTestStatus = health;
}

void StatusRptMsg::setTriggerTimeoutError(HealthState health)
{
    _data.triggerTimeoutError = health;
}

void StatusRptMsg::setACDLoopbackStatus(HealthState health)
{
    _data.adcLoopbackStatus = health;
}

void StatusRptMsg::setAllGreenStatus()
{
    setOverallStatus(Green);
    setLMKPLL1Status(Green);
    setLMKPLL2Status(Green);
    setMemTestStatus(Green);
    setBitTestStatus(Green);
    setTriggerTimeoutError(Green);
    setACDLoopbackStatus(Green);
}

#endif
