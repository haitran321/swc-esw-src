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
      printf("ERROR::InvalidRecordLength, Status Request Command (recLen = %d record size = %d)",
                _header->recLen, static_cast<int>(sizeof(StatusRequestCmdDataType)));
      return(ERROR);
   }

   // Validate data content.

   if (_data->requestType < SWCOverallStatus || _data->requestType > StopSendingProcessedSteeringWord)
   {
      printf("ERROR::InvalidCommandData, Request Type = %d)",
                _data->requestType);
      return(ERROR);
   }

   if ((_data->requestType == AlphaDCUDetailedStatus ||
        _data->requestType == BetaDCUDetailedStatus) &&
       (_data->dcuNum <= 0 || _data->dcuNum >= NUM_DCU))
   {
      printf("ERROR::InvalidCommandData, DCU Number = %d)",
                _data->dcuNum);
      return(ERROR);
   }

   return(OK);
}

void StatusRequestCmdMsg::byteSwapToLocal()
{
   _data->requestType = (StatusRequestType)fromNetworkInt(_data->requestType);
   _data->dcuNum = (int)fromNetworkInt(_data->dcuNum);
}
