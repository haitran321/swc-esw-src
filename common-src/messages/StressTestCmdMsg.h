/**
*
*/
#ifndef StressTestCmdMsg_H
#define StressTestCmdMsg_H

#include <string>

using namespace std;

#include "SWCMsgTypes.h"
#include "CommandMessage.h"

class StressTestCmdMsg : public CommandMessage
{
public:

    /**
    * Constructor
    *
    * @param buffer Raw message buffer
    * @param bufSize Raw message buffer size
    */
    StressTestCmdMsg(char *buffer, int bufSize);

    /**
    * Destructor
    *
    */
    virtual ~StressTestCmdMsg() { };

    /**
    * Method validateData validates data from the data section of 
    * the raw message to the member variables. 
    *
    * @return Status of operation
    */
    virtual STATUS validateData();

    /**
    * Method getNumPBP returns the number of PBP.
    *
    * @return numPBP value
    */
    inline int getNumPBP();

    /**
    * Method getNumAction returns the number of action.
    *
    * @return numActionvalue
    */
    inline int getNumAction();

    /**
    * Method actionSpacingUsec returns the action spacing in usec.
    *
    * @return actionSpacingUsec
    */
    inline int getActionSpacingUsec();

    /**
    * Method getAlphaStartValue returns the KSineAlpha start value.
    *
    * @return kSineAlphaStartValue value
    */
    inline int getAlphaStartValue();

    /**
    * Method getAlphaIncrement returns the KSineAlpha increment.
    *
    * @return kSineAlphaIncrement value
    */
    inline int getAlphaIncrement();

    /**
    * Method getBetaStartValue returns the KSineBeta start value.
    *
    * @return kSineBetaStartValue value
    */
    inline int getBetaStartValue();

    /**
    * Method getBetaIncrement returns the KSineBeta increment.
    *
    * @return kSineBetaIncrement value
    */
    inline int getBetaIncrement();

    void byteSwapToLocal();

private:

    /** Stress Test Command data */

    StressTestCmdDataType *_data;

};

//int StressTestCmdMsg::getNumPBP()
//{
//    return (_data->numPBP);
//}
//
//int StressTestCmdMsg::getNumAction()
//{
//    return (_data->numAction);
//}
//
//int StressTestCmdMsg::getActionSpacingUsec()
//{
//    return (_data->actionSpacingUsec);
//}
//
//int StressTestCmdMsg::getAlphaStartValue()
//{
//    return (_data->kSineAlphaStartValue);
//}
//
//int StressTestCmdMsg::getAlphaIncrement()
//{
//    return (_data->kSineAlphaIncrement);
//}
//
//int StressTestCmdMsg::getBetaStartValue()
//{
//    return (_data->kSineBetaStartValue);
//}
//
//int StressTestCmdMsg::getBetaIncrement()
//{
//    return (_data->kSineBetaIncrement);
//}

#endif
