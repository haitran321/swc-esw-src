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

    inline void setProcessedAlpha(int alpha);

    inline void setProcessedBeta(int beta);

private:

    /** Status Report data */

    SWCProcessedSteerWordDataType _data;

};

void SWCProcessedSteeringWordRptMsg::setProcessedAlpha(int alpha)
{
    _data.processedAlpha = alpha;
    printf("lastProcessedAlpha = %d\n", _data.processedAlpha);
}

void SWCProcessedSteeringWordRptMsg::setProcessedBeta(int beta)
{
    _data.processedBeta = beta;
    printf("lastProcessedBeta = %d\n", _data.processedBeta);
}

#endif
