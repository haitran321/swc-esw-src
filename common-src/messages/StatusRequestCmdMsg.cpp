/**
* $Id: StatusRequestCmdMsg.cpp 5101 2009-12-04 20:55:24Z
* nei18232 $
*/
#include "StatusRequestCmdMsg.h"

StatusRequestCmdMsg::StatusRequestCmdMsg(char *buffer, int bufSize) :
CommandMessage(buffer, bufSize),
_data(reinterpret_cast<StatusRequestCmdDataType *>(getDataBufPos()))
{
}

STATUS StatusRequestCmdMsg::validateData()
{
   // Validate header record length against expected record length.

   if (_header->recLen != sizeof(StatusRequestCmdDataType))
   {
      printf("ERROR::InvalidRecordLength, Status Request Command (recLen = %d record size = %ld)",
                _header->recLen, sizeof(StatusRequestCmdDataType));
      return(ERROR);
   }

   // Validate data content.

   if (_data->requestType < SWCRDetailedStatus || _data->requestType > BetaDCUDetailedStatus)
   {
      printf("ERROR::InvalidCommandData, Request Type = %d)",
                _data->requestType);
      return(ERROR);
   }

   return(OK);
}

void StatusRequestCmdMsg::byteSwapToLocal()
{
   _data->requestType = (StatusRequestType)fromNetworkInt(_data->requestType);
   _data->dcuNum = (int)fromNetworkInt(_data->dcuNum);
}
