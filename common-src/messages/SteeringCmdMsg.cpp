/**
* $Id: SteeringCmdMsg.cpp 5101 2009-12-04 20:55:24Z nei18232 $
*/
#include "SteeringCmdMsg.h"
#include "SWCMsgTypes.h"

SteeringCmdMsg::SteeringCmdMsg(char *buffer, int bufSize) :
CommandMessage(buffer, bufSize),
_data(reinterpret_cast<SteeringCmdDataType *>(getDataBufPos()))
{
}

STATUS SteeringCmdMsg::validateData()
{
   // Validate header record length against expected record length.

   if (_header->recLen != sizeof(SteeringCmdMsg))
   {
      printf("ERROR::InvalidRecordLength, Steering Command (recLen = %d record size = %d)",
             _header->recLen, sizeof(SteeringCmdDataType));
      return(ERROR);
   }

   // Validate data content.


   return(OK);
}

void SteeringCmdMsg::byteSwapToLocal()
{
    _data->alpha = (int)fromNetworkInt(_data->alpha);
    _data->beta = (int)fromNetworkInt(_data->beta);
}