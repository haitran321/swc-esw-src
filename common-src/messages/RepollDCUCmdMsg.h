/**
*
*/
#ifndef RepollDCUCmdMsg_H
#define RepollDCUCmdMsg_H

#include <string>

using namespace std;

#include "SWCMsgTypes.h"
#include "CommandMessage.h"

class RepollDCUCmdMsg : public CommandMessage
{
public:

    /**
    * Constructor
    *
    * @param buffer Raw message buffer
    * @param bufSize Raw message buffer size
    */
    RepollDCUCmdMsg(char *buffer, int bufSize);

    /**
    * Destructor
    *
    */
    virtual ~RepollDCUCmdMsg() { };

    /**
    * Method validateData validates data from the data section of 
    * the raw message to the member variables. 
    *
    * @return Status of operation
    */
    virtual STATUS validateData();

private:

};

#endif
