#include "ModeCmdMsg.h"
#include "SWCMsgTypes.h"

ModeCmdMsg::ModeCmdMsg(char *buffer, int bufSize) :
CommandMessage(buffer, bufSize),
_data(reinterpret_cast<ModeCmdDataType *>(getDataBufPos()))

{
}

STATUS ModeCmdMsg::validateData()
{
   // Validate header record length against expected record length.

   if (_header->recLen != sizeof(ModeCmdDataType)) 
   {
      printf("ERROR::InvalidRecordLength, Mode Command (recLen = %d record size = %ld)",
                _header->recLen, sizeof(ModeCmdDataType));
      return(ERROR);
   }

   // Validate data content.

   if (_data->mode < Offline || _data->mode > TestAssist)
   {
      printf("ERROR::InvalidCommandData, Mode Command (mode = %d)", 
                _data->mode);
      return(ERROR);
   }

   if (_data->opState < Live || _data->opState > Simulation)
   {
      printf("ERROR::InvalidCommandData, Mode Command (opState = %d)", 
                _data->opState);
      return(ERROR);
   }

   return(OK);
}

void ModeCmdMsg::byteSwapToLocal()
{
   _data->mode = (Mode)fromNetworkInt(_data->mode);
   _data->opState = (OpState)fromNetworkInt(_data->opState);
}
