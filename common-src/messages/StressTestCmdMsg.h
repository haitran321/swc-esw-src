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

    void byteSwapToLocal();

private:

    /** Stress Test Command data */

    StressTestCmdDataType *_data;

};

#endif
