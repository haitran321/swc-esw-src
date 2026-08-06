#include "CommandMessage.h"

CommandMessage::CommandMessage()
{
}

CommandMessage::CommandMessage(char *buffer, int bufSize) :
Message(buffer, bufSize)
{
}

STATUS CommandMessage::validateData()
{
   return(OK);
}

