/**
* $Id: SigInjectCmdMsg.cpp 5101 2009-12-04 20:55:24Z nei18232 $
*/
#include "SigInjectCmdMsg.h"

SigInjectCmdMsg::SigInjectCmdMsg(char *buffer, int bufSize) :
RIMSCommandMessage(buffer, bufSize),
_data(reinterpret_cast<SigInjectCmdDataType *>(getDataBufPos()))
{
}

STATUS SigInjectCmdMsg::validateData()
{
   // Validate header record length against expected record length.

   if (_header->recLen != sizeof(SigInjectCmdDataType))
   {
      printf("ERROR::InvalidRecordLength, Signal Injection Command (recLen = %d record size = %ld)",
                _header->recLen, sizeof(SigInjectCmdDataType));
      return(ERROR);
   }

   return(OK);
}

void SigInjectCmdMsg::byteSwapToLocal()
{
   int numActions = getDataSize()/sizeof(SigInjectCmdDataType);

   for (int i = 0; i < numActions; i++)
   {
      _data[i].rcvID = fromNetworkInt(_data[i].rcvID);
      _data[i].start = fromNetworkInt(_data[i].start);
      _data[i].stop = fromNetworkInt(_data[i].stop);
      _data[i].sigSrc = (SignalSource)fromNetworkInt(_data[i].sigSrc);
      _data[i].satelliteID = fromNetworkInt(_data[i].satelliteID);
      _data[i].beam1Sig = fromNetworkFloat(_data[i].beam1Sig);
      _data[i].beam2Sig = fromNetworkFloat(_data[i].beam2Sig);
      _data[i].beam3Sig = fromNetworkFloat(_data[i].beam3Sig);
      _data[i].beam4Sig = fromNetworkFloat(_data[i].beam4Sig);
      _data[i].beam5Sig = fromNetworkFloat(_data[i].beam5Sig);
      _data[i].beam6Sig = fromNetworkFloat(_data[i].beam6Sig);
      _data[i].beam7Sig = fromNetworkFloat(_data[i].beam7Sig);
      _data[i].beam8Sig = fromNetworkFloat(_data[i].beam8Sig);
      _data[i].beam9Sig = fromNetworkFloat(_data[i].beam9Sig);
      _data[i].beam10Sig = fromNetworkFloat(_data[i].beam10Sig);
   }
}
