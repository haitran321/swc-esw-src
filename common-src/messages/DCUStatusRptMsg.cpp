#include "DCUStatusRptMsg.h"

DCUStatusRptMsg::DCUStatusRptMsg() :
ReportMessage(DCU_STATUS_RPT_MSG_ID)
{
    memset(&_data, 0, sizeof(_data));
}

STATUS DCUStatusRptMsg::buildMsg()
{
    STATUS rc = OK;
    // Build header.

    if (ReportMessage::buildMsg() == ERROR)
    {
        return (ERROR);
    }

    // Byte swap from Local to Network
    _data.group = (RFCC_CH)toNetworkInt(_data.group);
    _data.number = (int)toNetworkInt(_data.number);
    _data.fwStatusReg = (int)toNetworkInt(_data.fwStatusReg);
    _data.dcuStatus.bypassStatus = (int)toNetworkInt(_data.dcuStatus.bypassStatus);
    _data.dcuStatus.modeStatus = (int)toNetworkInt(_data.dcuStatus.modeStatus);
    _data.dcuStatus.overallStatus = (HealthState)toNetworkInt(_data.dcuStatus.overallStatus);
    _data.dcuStatus.clockStatus = (HealthState)toNetworkInt(_data.dcuStatus.clockStatus);
    _data.dcuStatus.locValid = (HealthState)toNetworkInt(_data.dcuStatus.locValid);
    _data.dcuStatus.spiCommStatus = (HealthState)toNetworkInt(_data.dcuStatus.spiCommStatus);
    _data.dcuStatus.steeringWordCompare = (HealthState)toNetworkInt(_data.dcuStatus.steeringWordCompare);
    _data.dcuStatus.fwLoc = (HealthState)toNetworkInt(_data.dcuStatus.fwLoc);
    _data.dcuStatus.crcStatus = (HealthState)toNetworkInt(_data.dcuStatus.crcStatus);

    // Add data to message.
    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
