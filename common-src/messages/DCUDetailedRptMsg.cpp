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
    _data.duplicate = (HealthState)toNetworkInt(_data.duplicate);
    _data.fwStatusReg = (int)toNetworkInt(_data.fwStatusReg);
    _data.dcuFWStatus.bypassStatus = (int)toNetworkInt(_data.dcuFWStatus.bypassStatus);
    _data.dcuFWStatus.modeStatus = (int)toNetworkInt(_data.dcuFWStatus.modeStatus);
    _data.dcuFWStatus.overallStatus = (HealthState)toNetworkInt(_data.dcuFWStatus.overallStatus);
    _data.dcuFWStatus.clockStatus = (HealthState)toNetworkInt(_data.dcuFWStatus.clockStatus);
    _data.dcuFWStatus.locValid = (HealthState)toNetworkInt(_data.dcuFWStatus.locValid);
    _data.dcuFWStatus.spiCommStatus = (HealthState)toNetworkInt(_data.dcuFWStatus.spiCommStatus);
    _data.dcuFWStatus.steeringWordCompare = (HealthState)toNetworkInt(_data.dcuFWStatus.steeringWordCompare);
    _data.dcuFWStatus.dcuFWMajorRev = (int)toNetworkInt(_data.dcuFWStatus.dcuFWMajorRev);
    _data.dcuFWStatus.dcuFWMinorRev = (int)toNetworkInt(_data.dcuFWStatus.dcuFWMinorRev);
    _data.dcuFWStatus.dcuType = (RFCC_CH)toNetworkInt(_data.dcuFWStatus.dcuType);
    _data.dcuFWStatus.crcStatus = (HealthState)toNetworkInt(_data.dcuFWStatus.crcStatus);

    // Add data to message.
    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
