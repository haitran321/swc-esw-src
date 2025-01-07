/**
* $Id: SPSummaryRptMsg.cpp 1317 2008-07-28 16:15:59Z alv70669 $
*/
#include "SPSummaryRptMsg.h"

SPSummaryRptMsg::SPSummaryRptMsg() :
RIMSReportMessage(SP_SUMMARY_PRT_MSG_ID)
{
   memset(&_data, 0, sizeof(_data));
}

SPSummaryRptMsg::~SPSummaryRptMsg()
{
}

void SPSummaryRptMsg::addSPSummaryRpt(SPSummaryRptDataType &data)
{
    // Only 1 record per message
   memcpy((SPSummaryRptDataType *)&_data, (SPSummaryRptDataType *)&data, sizeof(SPSummaryRptDataType));
}

STATUS SPSummaryRptMsg::buildMsg()
{
    STATUS rc = OK;
    // Build header.

    if (RIMSReportMessage::buildMsg() == ERROR)
    {
        return (ERROR);
    }

    // Byte swap from Local to Network
    // char doesn't require endian conversion
    _data.startRangeBin = toNetworkInt(_data.startRangeBin);
    _data.stopRangeBin = toNetworkInt(_data.stopRangeBin);
    _data.numberOfThresholdDetectionsExceeded = toNetworkInt(_data.numberOfThresholdDetectionsExceeded);
    _data.numberOfThresholdDetections = toNetworkInt(_data.numberOfThresholdDetections);
    _data.mean = toNetworkFloat(_data.mean);
    _data.PDT = toNetworkFloat(_data.PDT);
    _data.numberOfSLBDetection = toNetworkInt(_data.numberOfSLBDetection);
    _data.satelliteID = toNetworkInt(_data.satelliteID);
    _data.agcSelect = toNetworkInt(_data.agcSelect);
    _data.numberOfMonopulseDetections = toNetworkInt(_data.numberOfMonopulseDetections);
    _data.numberOfNoiseDetections = toNetworkInt(_data.numberOfNoiseDetections);
    _data.noiseCalculationMethod = toNetworkInt(_data.noiseCalculationMethod);
    _data.smoothedPDTMeanNoise = toNetworkFloat(_data.smoothedPDTMeanNoise);
    _data.numberOfRWNoiseSamples = toNetworkInt(_data.numberOfRWNoiseSamples);
    _data.rangeWindowNoise = toNetworkFloat(_data.rangeWindowNoise);

    // Add data to RIMS message.

    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
