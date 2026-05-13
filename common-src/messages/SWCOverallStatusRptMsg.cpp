#include "SWCOverallStatusRptMsg.h"

SWCOverallStatusRptMsg::SWCOverallStatusRptMsg() :
ReportMessage(SWC_OVERALL_STATUS_RPT_MSG_ID)
{
    memset(&_data, 0, sizeof(_data));
}

STATUS SWCOverallStatusRptMsg::buildMsg()
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
    _data.olteMode = (SWC_MODE)toNetworkInt(_data.olteMode);
    _data.swcAlphaDUStatus = (HealthState)toNetworkInt(_data.swcAlphaDUStatus);
    _data.swcBetaDUStatus = (HealthState)toNetworkInt(_data.swcBetaDUStatus);
    _data.swcAlphaDCURolledUpStatus = (DCURolledUpStatus)toNetworkInt(_data.swcAlphaDCURolledUpStatus);
    _data.swcBetaDCURolledUpStatus = (DCURolledUpStatus)toNetworkInt(_data.swcBetaDCURolledUpStatus);
    _data.swcTempStatus = (HealthState)toNetworkInt(_data.swcTempStatus);
    _data.swc12VPwrStatus = (HealthState)toNetworkInt(_data.swc12VPwrStatus);
    _data.swc24VPwrStatus = (HealthState)toNetworkInt(_data.swc24VPwrStatus);
    _data.swcATBStatus = (HealthState)toNetworkInt(_data.swcATBStatus);
    _data.testUnitHWStatus = (HealthState)toNetworkInt(_data.testUnitHWStatus);
    _data.lastProcessedAlpha = (int)toNetworkInt(_data.lastProcessedAlpha);
    _data.lastProcessedBeta = (int)toNetworkInt(_data.lastProcessedBeta);
    for (int i = 0; i < NUM_DCU; i++)
    {
        _data.alphaDCU[i] = (DCUHealthState)toNetworkInt(_data.alphaDCU[i]);
        _data.betaDCU[i] = (DCUHealthState)toNetworkInt(_data.betaDCU[i]);
    }

    // Add data to message.
    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
