#include "DUBStatusRptMsg.h"

DUBStatusRptMsg::DUBStatusRptMsg() :
ReportMessage(DUB_STATUS_RPT_MSG_ID)
{
    memset(&_data, 0, sizeof(_data));
}

STATUS DUBStatusRptMsg::buildMsg()
{
    STATUS rc = OK;
    // Build header.

    if (ReportMessage::buildMsg() == ERROR)
    {
        return (ERROR);
    }

    // Byte swap from Local to Network
    _data.overall = (HealthState)toNetworkInt(_data.overall);
    _data.status1 = (HealthState)toNetworkInt(_data.status1);
    _data.status2 = (HealthState)toNetworkInt(_data.status2);
    _data.status3 = (HealthState)toNetworkInt(_data.status3);
    _data.status4 = (HealthState)toNetworkInt(_data.status4);
    _data.status5 = (HealthState)toNetworkInt(_data.status5);


    // Add data to message.
    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
