#include "RepollDCUCmdMsg.h"

RepollDCUCmdMsg::RepollDCUCmdMsg(char *buffer, int bufSize) :
    CommandMessage(buffer, bufSize)
{
}

STATUS RepollDCUCmdMsg::validateData()
{
    // Validate header record length against expected record length.

    // Validate data content.

    return (OK);
}

