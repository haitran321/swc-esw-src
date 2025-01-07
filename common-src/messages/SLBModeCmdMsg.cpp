/**
* $Id: SLBModeCmdMsg.cpp 5101 2009-12-04 20:55:24Z nei18232 $
*/
#include "SLBModeCmdMsg.h"
#include "CSPUMsgTypes.h"

SLBModeCmdMsg::SLBModeCmdMsg(char *buffer, int bufSize) :
RIMSCommandMessage(buffer, bufSize),
_data(reinterpret_cast<SLBModeCmdDataType *>(getDataBufPos()))
{
}

STATUS SLBModeCmdMsg::validateData()
{
   // Validate header record length against expected record length.

   if (_header->recLen != sizeof(SLBModeCmdDataType))
   {
      printf("ERROR::InvalidRecordLength, SLB Mode Command (recLen = %d record size = %d)",
             _header->recLen, sizeof(SLBModeCmdDataType));
      return(ERROR);
   }

   // Validate data content.

   if (_data->type < SLB_Off || _data->type > SLB_OnExtended)
   {
      printf("ERROR::InvalidCommandData, SLB Mode Command (type = %d)",
             _data->type);
      return(ERROR);
   }

   return(OK);
}

void SLBModeCmdMsg::byteSwapToLocal()
{
    _data->type = (SLBModeOption)fromNetworkInt(_data->type);
}