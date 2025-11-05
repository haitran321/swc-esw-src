#include "SWCStatusRptMsg.h"

SWCStatusRptMsg::SWCStatusRptMsg() :
ReportMessage(SWC_STATUS_RPT_MSG_ID)
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
    _data.swcBetaDUStatus = (HealthState)toNetworkInt(_data.swcBetaDUStatus);
    _data.swcAlphaDCURolledUpStatus = (DCURolledUpStatus)toNetworkInt(_data.swcAlphaDCURolledUpStatus);
    _data.swcBetaDCURolledUpStatus = (DCURolledUpStatus)toNetworkInt(_data.swcBetaDCURolledUpStatus);
    _data.swcTempStatus = (HealthState)toNetworkInt(_data.swcTempStatus);
    _data.swcPwrSuppliesStatus = (HealthState)toNetworkInt(_data.swcPwrSuppliesStatus);
    _data.swcATBStatus = (HealthState)toNetworkInt(_data.swcATBStatus);
    _data.testUnitHWStatus = (HealthState)toNetworkInt(_data.testUnitHWStatus);
    _data.lastAlpha = (int)toNetworkInt(_data.lastAlpha);
    _data.lastBeta = (int)toNetworkInt(_data.lastBeta);
    for (int i = 0; i < NUM_DCU; i++)
    {
        _data.alphaDCU[i] = (HealthState)toNetworkInt(_data.alphaDCU[i]);
        _data.betaDCU[i] = (HealthState)toNetworkInt(_data.betaDCU[i]);
    }

    // Add data to message.
    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
