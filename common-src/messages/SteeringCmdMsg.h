/**
*
*/
#ifndef SteeringCmdMsg_H
#define SteeringCmdMsg_H

#include <string>

using namespace std;

#include "SWCMsgTypes.h"
#include "CommandMessage.h"

class SteeringCmdMsg : public CommandMessage
{
public:

    /**
    * Constructor
    *
    * @param buffer Raw message buffer
    * @param bufSize Raw message buffer size
    */
    SteeringCmdMsg(char *buffer, int bufSize);

    /**
    * Destructor
    *
    */
    virtual ~SteeringCmdMsg() { };

    /**
    * Method validateData validates data from the data section of 
    * the raw message to the member variables. 
    *
    * @return Status of operation
    */
    virtual STATUS validateData();

    /**
    * Method getTestSource returns the test source.
    *
    * @return testSource value
    */
    inline TestSource getTestSource();

    /**
    * Method getAlpha returns the KSineAlpha.
    *
    * @return KSineAlpha value
    */
    inline int getAlpha();

    /**
    * Method getBeta returns the KSineBeta.
    *
    * @return KSineBeta value
    */
    inline int getBeta();

    /**
    * Method getRLCPCmd returns the RLCP.
    *
    * @return RPCP value
    */
    inline CmdOnOff getRLCPCmd();

    /**
    * Method getRLSCCmd returns the RLSC.
    *
    * @return RPSC value
    */
    inline CmdOnOff getRLSCCmd();

    void byteSwapToLocal();

private:

    /** Steering Command data */

    SteeringCmdDataType *_data;

};

TestSource SteeringCmdMsg::getTestSource()
{
    return (_data->testSource);
}

int SteeringCmdMsg::getAlpha()
{
    return (_data->alpha);
}

int SteeringCmdMsg::getBeta()
{
    return (_data->beta);
}

CmdOnOff SteeringCmdMsg::getRLCPCmd()
{
    return (_data->RLCP);
}

CmdOnOff SteeringCmdMsg::getRLSCCmd()
{
    return (_data->RLSC);
}

#endif
