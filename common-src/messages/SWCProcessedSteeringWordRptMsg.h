#ifndef SWCProcessedSteeringWordRptMsg_H
#define SWCProcessedSteeringWordRptMsg_H

#include <string>

using namespace std;

#include "ReportMessage.h"
#include "SWCMsgTypes.h"

class SWCProcessedSteeringWordRptMsg : public ReportMessage
{
public:

    /**
    * Constructor
    *
    */
    SWCProcessedSteeringWordRptMsg();

    /**
    * Destructor
    *
    */
    virtual ~SWCProcessedSteeringWordRptMsg(){};

    /**
    * Method buildMsg builds the message given the current member 
    * variable settings. 
    *
    * @return Status of operation
    */
    virtual STATUS buildMsg();

    inline void setModuleType(MODULE_TYPE mt);

    inline void setProcessedKSine(int kSine);

private:

    /** Status Report data */

    SWCProcessedSteerWordDataType _data;

};

void SWCProcessedSteeringWordRptMsg::setModuleType(MODULE_TYPE mt)
{
    _data.moduleType = mt;
}

void SWCProcessedSteeringWordRptMsg::setProcessedKSine(int kSine)
{
    _data.processedKSine = kSine;
}

#endif
