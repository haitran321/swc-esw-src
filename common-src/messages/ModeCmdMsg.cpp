/**
* $Id: ModeCmdMsg.cpp 5101 2009-12-04 20:55:24Z nei18232 $
*/
#include "ModeCmdMsg.h"
#include "CSPUMsgTypes.h"

ModeCmdMsg::ModeCmdMsg(char *buffer, int bufSize) :
RIMSCommandMessage(buffer, bufSize),
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

   if (_data->lissState < LISS_Legacy || _data->lissState > LISS_CSPU)
   {
      printf("ERROR::InvalidCommandData, Mode Command (lissState = %d)", 
                _data->lissState);
      return(ERROR);
   }

   if (_data->opState < Live || _data->opState > Simulation)
   {
      printf("ERROR::InvalidCommandData, Mode Command (opState = %d)", 
                _data->opState);
      return(ERROR);
   }

   if (_data->simInterference < SimIntOff || _data->simInterference > SimIntOn)
   {
      printf("ERROR::InvalidCommandData, Mode Command (simInterference = %d)", 
                _data->simInterference);
      return(ERROR);
   }

   return(OK);
}

void ModeCmdMsg::byteSwapToLocal()
{
   _data->mode = (Mode)fromNetworkInt(_data->mode);
   _data->lissState = (LISSState)fromNetworkInt(_data->lissState);
   _data->opState = (OpState)fromNetworkInt(_data->opState);
   _data->simInterference = (SimInterferenceState)fromNetworkInt(_data->simInterference);
}
