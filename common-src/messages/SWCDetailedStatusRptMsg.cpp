#include "SWCDetailedStatusRptMsg.h"

SWCDetailedStatusRptMsg::SWCDetailedStatusRptMsg() :
ReportMessage(SWC_DETAILED_STATUS_RPT_MSG_ID)
{
    memset(&_data, 0, sizeof(_data));
}

STATUS SWCDetailedStatusRptMsg::buildMsg()
{
    STATUS rc = OK;
    // Build header.

    if (ReportMessage::buildMsg() == ERROR)
    {
        return (ERROR);
    }

    // Byte swap from Local to Network
    // Alpha status
    _data.alphaOverall = (HealthState)toNetworkInt(_data.alphaOverall);
    _data.alphaStatus1 = (HealthState)toNetworkInt(_data.alphaStatus1);
    _data.alphaStatus2 = (HealthState)toNetworkInt(_data.alphaStatus2);
    _data.alphaStatus3 = (HealthState)toNetworkInt(_data.alphaStatus3);
    _data.alphaStatus4 = (HealthState)toNetworkInt(_data.alphaStatus4);
    _data.alphaStatus5 = (HealthState)toNetworkInt(_data.alphaStatus5);
    
    // Beta status
    _data.betaOverall = (HealthState)toNetworkInt(_data.betaOverall);
    _data.betaStatus1 = (HealthState)toNetworkInt(_data.betaStatus1);
    _data.betaStatus2 = (HealthState)toNetworkInt(_data.betaStatus2);
    _data.betaStatus3 = (HealthState)toNetworkInt(_data.betaStatus3);
    _data.betaStatus4 = (HealthState)toNetworkInt(_data.betaStatus4);
    _data.betaStatus5 = (HealthState)toNetworkInt(_data.betaStatus5);

    // PS status
    _data.psOverall = (HealthState)toNetworkInt(_data.psOverall);
    _data.psStatus1 = (HealthState)toNetworkInt(_data.psStatus1);
    _data.psStatus2 = (HealthState)toNetworkInt(_data.psStatus2);
    _data.psStatus3 = (HealthState)toNetworkInt(_data.psStatus3);
    _data.psStatus4 = (HealthState)toNetworkInt(_data.psStatus4);
    _data.psStatus5 = (HealthState)toNetworkInt(_data.psStatus5);

    // Temp status
    _data.tempOverall = (HealthState)toNetworkInt(_data.tempOverall);
    _data.tempStatus1 = (HealthState)toNetworkInt(_data.tempStatus1);
    _data.tempStatus2 = (HealthState)toNetworkInt(_data.tempStatus2);
    _data.tempStatus3 = (HealthState)toNetworkInt(_data.tempStatus3);
    _data.tempStatus4 = (HealthState)toNetworkInt(_data.tempStatus4);
    _data.tempStatus5 = (HealthState)toNetworkInt(_data.tempStatus5);


    // Add data to message.
    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
