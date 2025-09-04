#include "SWCStatusRptMsg.h"

SWCStatusRptMsg::SWCStatusRptMsg() :
ReportMessage(LRU_STATUS_RPT_MSG_ID)
{
    memset(&_data, 0, sizeof(_data));
}

STATUS SWCStatusRptMsg::buildMsg()
{
    STATUS rc = OK;
    // Build header.

    if (ReportMessage::buildMsg() == ERROR)
    {
        return (ERROR);
    }

    // Byte swap from Local to Network
    _data.swcStatus = (HealthState)toNetworkInt(_data.swcStatus);
    _data.swcConfig = (SWC_CONFIG)toNetworkInt(_data.swcConfig);
    _data.swcMode = (SWC_MODE)toNetworkInt(_data.swcMode);
    _data.swcAlphaDUStatus = (HealthState)toNetworkInt(_data.swcAlphaDUStatus);
    _data.swcAlphaPSStatus = (HealthState)toNetworkInt(_data.swcAlphaPSStatus);
    _data.swcBetaDUStatus = (HealthState)toNetworkInt(_data.swcBetaDUStatus);
    _data.swcBetaPSStatus = (HealthState)toNetworkInt(_data.swcBetaPSStatus);
    for (int i = 0; i < NUM_DCU; i++)
    {
        _data.alphaDCU[i] = (HealthState)toNetworkInt(_data.alphaDCU[i]);
        _data.alphaDCU[i] = (HealthState)toNetworkInt(_data.alphaDCU[i]);
    }

    // Add data to message.
    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
