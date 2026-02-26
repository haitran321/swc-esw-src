#include "SWCProcessedSteeringWordRptMsg.h"

SWCProcessedSteeringWordRptMsg::SWCProcessedSteeringWordRptMsg() :
ReportMessage(SWC_PROCESSED_SW_RPT_MSG_ID)
{
    memset(&_data, 0, sizeof(_data));
}

STATUS SWCProcessedSteeringWordRptMsg::buildMsg()
{
    STATUS rc = OK;
    // Build header.

    if (ReportMessage::buildMsg() == ERROR)
    {
        return (ERROR);
    }

    // Byte swap from Local to Network
    _data.moduleType = (MODULE_TYPE)toNetworkInt(_data.moduleType);
    _data.processedAlpha = (int)toNetworkInt(_data.processedAlpha);
    _data.processedBeta = (int)toNetworkInt(_data.processedBeta);

    // Add data to message.
    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
