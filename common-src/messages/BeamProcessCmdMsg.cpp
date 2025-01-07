/**
* $Id: BeamProcessCmdMsg.cpp 5101 2009-12-04 20:55:24Z nei18232 $
*/
#include "BeamProcessCmdMsg.h"

BeamProcessCmdMsg::BeamProcessCmdMsg(char *buffer, int bufSize) :
RIMSCommandMessage(buffer, bufSize),
_data(reinterpret_cast<BeamProcessCmdDataType *>(getDataBufPos()))
{
}

STATUS BeamProcessCmdMsg::validateData()
{
   // Validate header record length against expected record length.

   if (_header->recLen != sizeof(BeamProcessCmdDataType))
   {
      printf("ERROR::InvalidRecordLength, Beam Process Command (recLen = %d record size = %ld)",
                _header->recLen, sizeof(BeamProcessCmdDataType));
      return(ERROR);
   }

   return(OK);
}

void BeamProcessCmdMsg::byteSwapToLocal()
{
   int numActions = getDataSize()/sizeof(BeamProcessCmdDataType);

   for (int i = 0; i < numActions; i++)
   {
      _data[i].rcvID = fromNetworkInt(_data[i].rcvID);
      _data[i].satelliteID = fromNetworkInt(_data[i].satelliteID);
      _data[i].start = fromNetworkInt(_data[i].start);
      _data[i].stop = fromNetworkInt(_data[i].stop);
      _data[i].tbrsRptType = (TBRSRptType)fromNetworkInt(_data[i].tbrsRptType);
      _data[i].pulseType = (PulseType)fromNetworkInt(_data[i].pulseType);
      _data[i].freq = fromNetworkInt(_data[i].freq);
      _data[i].agc = (AGCLevel)fromNetworkInt(_data[i].agc);
      _data[i].powerStart = fromNetworkFloat(_data[i].powerStart);
      _data[i].rangeStart = fromNetworkFloat(_data[i].rangeStart);
      _data[i].phaseCorr = fromNetworkFloat(_data[i].phaseCorr);
      _data[i].lohStartPhase = fromNetworkFloat(_data[i].lohStartPhase);
      _data[i].lovStartPhase = fromNetworkFloat(_data[i].lovStartPhase);
      _data[i].dspsRptType = (DSPSRptType)fromNetworkInt(_data[i].dspsRptType);
      _data[i].ampCorr = fromNetworkFloat(_data[i].ampCorr);
      _data[i].cfar = fromNetworkFloat(_data[i].cfar);
      _data[i].pdtBiasFactor = fromNetworkFloat(_data[i].pdtBiasFactor);
      _data[i].sequenceNum = fromNetworkInt(_data[i].sequenceNum);
      _data[i].multipulseTemplate = (MultipulseTemplate)fromNetworkInt(_data[i].multipulseTemplate);
      _data[i].isSoiPulse = fromNetworkInt(_data[i].isSoiPulse);  // Need to verify sizeof(bool)
      _data[i].misapMajorFunc = fromNetworkInt(_data[i].misapMajorFunc);
      _data[i].misapMinorFunc = fromNetworkInt(_data[i].misapMinorFunc);
   }
}
