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
    _data.swcStatus = (HealthState)toNetworkInt(_data.swcStatus);
    // Alpha status
    _data.alphaDUStatus.overallStatus = (HealthState)toNetworkInt(_data.alphaDUStatus.overallStatus);
    _data.alphaDUStatus.readyStatus = (HealthState)toNetworkInt(_data.alphaDUStatus.readyStatus);
    _data.alphaDUStatus.highTempAlarm = (HealthState)toNetworkInt(_data.alphaDUStatus.highTempAlarm);
    _data.alphaDUStatus.vccintAlarm = (HealthState)toNetworkInt(_data.alphaDUStatus.vccintAlarm);
    _data.alphaDUStatus.vccauxAlarm = (HealthState)toNetworkInt(_data.alphaDUStatus.vccauxAlarm);
    _data.alphaDUStatus.vbramAlarm = (HealthState)toNetworkInt(_data.alphaDUStatus.vbramAlarm);
    
    // Beta status
    _data.betaDUStatus.overallStatus = (HealthState)toNetworkInt(_data.betaDUStatus.overallStatus);
    _data.betaDUStatus.readyStatus = (HealthState)toNetworkInt(_data.betaDUStatus.readyStatus);
    _data.betaDUStatus.highTempAlarm = (HealthState)toNetworkInt(_data.betaDUStatus.highTempAlarm);
    _data.betaDUStatus.vccintAlarm = (HealthState)toNetworkInt(_data.betaDUStatus.vccintAlarm);
    _data.betaDUStatus.vccauxAlarm = (HealthState)toNetworkInt(_data.betaDUStatus.vccauxAlarm);
    _data.betaDUStatus.vbramAlarm = (HealthState)toNetworkInt(_data.betaDUStatus.vbramAlarm);

    // TU status
    _data.tuStatus.overallStatus = (HealthState)toNetworkInt(_data.tuStatus.overallStatus);
    _data.tuStatus.readyStatus = (HealthState)toNetworkInt(_data.tuStatus.readyStatus);
    _data.tuStatus.highTempAlarm = (HealthState)toNetworkInt(_data.tuStatus.highTempAlarm);
    _data.tuStatus.vccintAlarm = (HealthState)toNetworkInt(_data.tuStatus.vccintAlarm);
    _data.tuStatus.vccauxAlarm = (HealthState)toNetworkInt(_data.tuStatus.vccauxAlarm);
    _data.tuStatus.vbramAlarm = (HealthState)toNetworkInt(_data.tuStatus.vbramAlarm);

    // PS status
    _data.psStatus.overallStatus = (HealthState)toNetworkInt(_data.psStatus.overallStatus);
    _data.psStatus.psStatus1 = (HealthState)toNetworkInt(_data.psStatus.psStatus1);
    _data.psStatus.psStatus2 = (HealthState)toNetworkInt(_data.psStatus.psStatus2);
    _data.psStatus.psStatus3 = (HealthState)toNetworkInt(_data.psStatus.psStatus3);
    _data.psStatus.psStatus4 = (HealthState)toNetworkInt(_data.psStatus.psStatus4);
    _data.psStatus.psStatus5 = (HealthState)toNetworkInt(_data.psStatus.psStatus5);

    // Temp status
    _data.tempStatus.overallStatus = (HealthState)toNetworkInt(_data.tempStatus.overallStatus);
    _data.tempStatus.tempStatus1 = (HealthState)toNetworkInt(_data.tempStatus.tempStatus1);
    _data.tempStatus.tempStatus2 = (HealthState)toNetworkInt(_data.tempStatus.tempStatus2);
    _data.tempStatus.tempStatus3 = (HealthState)toNetworkInt(_data.tempStatus.tempStatus3);
    _data.tempStatus.tempStatus4 = (HealthState)toNetworkInt(_data.tempStatus.tempStatus4);
    _data.tempStatus.tempStatus5 = (HealthState)toNetworkInt(_data.tempStatus.tempStatus5);


    // Add data to message.
    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
