/**
* $Id: WriteRegCmdMsg.cpp 5101 2009-12-04 20:55:24Z nei18232 $
*/
#include "WriteRegCmdMsg.h"

WriteRegCmdMsg::WriteRegCmdMsg(char *buffer, int bufSize) :
RIMSCommandMessage(buffer, bufSize),
_data(reinterpret_cast<RegCmdDataType *>(getDataBufPos()))
{
}

STATUS WriteRegCmdMsg::validateData()
{
   // Validate header record length against expected record length.

   if (_header->recLen != sizeof(RegCmdDataType))
   {
      printf("ERROR::InvalidRecordLength, 0, Write Reg Command (recLen = %d record size = %ld)",
                _header->recLen, sizeof(RegCmdDataType));
      return(ERROR);
   }

   return(OK);
}
