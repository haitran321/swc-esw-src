/**
* $Id: RIMSCommandMessage.cpp 5101 2009-12-04 20:55:24Z nei18232 $
*/
#include "RIMSCommandMessage.h"

RIMSCommandMessage::RIMSCommandMessage()
{
}

RIMSCommandMessage::RIMSCommandMessage(char *buffer, int bufSize) :
RIMSMessage(buffer, bufSize)
{
}

STATUS RIMSCommandMessage::validateData()
{
   return(OK);
}

