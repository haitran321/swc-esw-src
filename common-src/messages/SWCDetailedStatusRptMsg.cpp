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
    _data.alphaDUStatus.overallStatus = (RolledUpStatus)toNetworkInt(_data.alphaDUStatus.overallStatus);
    _data.alphaDUStatus.readyStatus = (HealthState)toNetworkInt(_data.alphaDUStatus.readyStatus);
    _data.alphaDUStatus.highTempAlarm = (HealthState)toNetworkInt(_data.alphaDUStatus.highTempAlarm);
    _data.alphaDUStatus.overTempAlarm = (HealthState)toNetworkInt(_data.alphaDUStatus.overTempAlarm);
    _data.alphaDUStatus.vccintAlarm = (HealthState)toNetworkInt(_data.alphaDUStatus.vccintAlarm);
    _data.alphaDUStatus.vccauxAlarm = (HealthState)toNetworkInt(_data.alphaDUStatus.vccauxAlarm);
    _data.alphaDUStatus.vbramAlarm = (HealthState)toNetworkInt(_data.alphaDUStatus.vbramAlarm);
    _data.alphaDUStatus.dieTemp = (HealthState)toNetworkInt(_data.alphaDUStatus.dieTemp);
    
    // Beta status
    _data.betaDUStatus.overallStatus = (RolledUpStatus)toNetworkInt(_data.betaDUStatus.overallStatus);
    _data.betaDUStatus.readyStatus = (HealthState)toNetworkInt(_data.betaDUStatus.readyStatus);
    _data.betaDUStatus.highTempAlarm = (HealthState)toNetworkInt(_data.betaDUStatus.highTempAlarm);
    _data.betaDUStatus.overTempAlarm = (HealthState)toNetworkInt(_data.betaDUStatus.overTempAlarm);
    _data.betaDUStatus.vccintAlarm = (HealthState)toNetworkInt(_data.betaDUStatus.vccintAlarm);
    _data.betaDUStatus.vccauxAlarm = (HealthState)toNetworkInt(_data.betaDUStatus.vccauxAlarm);
    _data.betaDUStatus.vbramAlarm = (HealthState)toNetworkInt(_data.betaDUStatus.vbramAlarm);
    _data.betaDUStatus.dieTemp = (HealthState)toNetworkInt(_data.betaDUStatus.dieTemp);

    // TU status
    _data.tuStatus.overallStatus = (RolledUpStatus)toNetworkInt(_data.tuStatus.overallStatus);
    _data.tuStatus.readyStatus = (HealthState)toNetworkInt(_data.tuStatus.readyStatus);
    _data.tuStatus.highTempAlarm = (HealthState)toNetworkInt(_data.tuStatus.highTempAlarm);
    _data.tuStatus.overTempAlarm = (HealthState)toNetworkInt(_data.tuStatus.overTempAlarm);
    _data.tuStatus.vccintAlarm = (HealthState)toNetworkInt(_data.tuStatus.vccintAlarm);
    _data.tuStatus.vccauxAlarm = (HealthState)toNetworkInt(_data.tuStatus.vccauxAlarm);
    _data.tuStatus.vbramAlarm = (HealthState)toNetworkInt(_data.tuStatus.vbramAlarm);
    _data.tuStatus.dieTemp = (HealthState)toNetworkInt(_data.tuStatus.dieTemp);

    // Add data to message.
    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
