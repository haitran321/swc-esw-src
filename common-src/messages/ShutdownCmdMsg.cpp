/**
* $Id: ShutdownCmdMsg.cpp 5101 2009-12-04 20:55:24Z nei18232 $
*/
#include "ShutdownCmdMsg.h"
#include "SWCMsgTypes.h"

ShutdownCmdMsg::ShutdownCmdMsg(char *buffer, int bufSize) :
CommandMessage(buffer, bufSize),
_data(reinterpret_cast<ShutdownCmdDataType *>(getDataBufPos()))
{
}

STATUS ShutdownCmdMsg::validateData()
{
   // Validate header record length against expected record length.

   if (_header->recLen != sizeof(ShutdownCmdDataType))
   {
      printf("ERROR::InvalidRecordLength, Shutdown Command (recLen = %d record size = %ld)",
                _header->recLen, sizeof(ShutdownCmdDataType));
      return(ERROR);
   }

   // Validate data content.

   if (_data->type < Reboot || _data->type > PowerOff)
   {
      printf("ERROR::InvalidCommandData, Shutdown Command (type = %d)",
                _data->type);
      return(ERROR);
   }

   return(OK);
}

void ShutdownCmdMsg::byteSwapToLocal()
{
   _data->type = (ShutdownOption)fromNetworkInt(_data->type);
}
