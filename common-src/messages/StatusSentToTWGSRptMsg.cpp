#include "StatusSentToTWGSRptMsg.h"

StatusSentToTWGSRptMsg::StatusSentToTWGSRptMsg() :
ReportMessage(SYSTEM_STATUS_TO_TWGS_RPT_MSG_ID)
{
    memset(&_data, 0, sizeof(_data));
}

STATUS StatusSentToTWGSRptMsg::buildMsg()
{
    STATUS rc = OK;
    // Build header.

    if (ReportMessage::buildMsg() == ERROR)
    {
        return (ERROR);
    }

    // Byte swap from Local to Network
    _data.swcStatus = (int)toNetworkInt(_data.swcStatus);
    _data.swcrStatus = (int)toNetworkInt(_data.swcrStatus);

    // Add data to message.
    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
