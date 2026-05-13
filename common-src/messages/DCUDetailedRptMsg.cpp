#include "DCUDetailedStatusRptMsg.h"

DCUDetailedStatusRptMsg::DCUDetailedStatusRptMsg() :
ReportMessage(DCU_DETAILED_STATUS_RPT_MSG_ID)
{
    memset(&_data, 0, sizeof(_data));
}

STATUS DCUDetailedStatusRptMsg::buildMsg()
{
    STATUS rc = OK;
    // Build header.

    if (ReportMessage::buildMsg() == ERROR)
    {
        return (ERROR);
    }

    // Byte swap from Local to Network
    _data.group = (RFCC_CH)toNetworkInt(_data.group);
    _data.loc = (int)toNetworkInt(_data.loc);
    _data.fwStatusReg = (int)toNetworkInt(_data.fwStatusReg);
    _data.locStatus = (HealthState)toNetworkInt(_data.locStatus);
    _data.dcuFWMajorRev = (int)toNetworkInt(_data.dcuFWMajorRev);
    _data.dcuFWMinorRev = (int)toNetworkInt(_data.dcuFWMinorRev);
    _data.bypassStatus = (int)toNetworkInt(_data.bypassStatus);
    _data.modeStatus = (int)toNetworkInt(_data.modeStatus);
    _data.overallStatus = (DCUHealthState)toNetworkInt(_data.overallStatus);
    _data.clockStatus = (HealthState)toNetworkInt(_data.clockStatus);
    _data.spiCommStatus = (HealthState)toNetworkInt(_data.spiCommStatus);
    _data.crcStatus = (HealthState)toNetworkInt(_data.crcStatus);
    _data.steeringWordCompare = (HealthState)toNetworkInt(_data.steeringWordCompare);

    // Add data to message.
    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
