#include "SWCAckRptMsg.h"

SWCAckRptMsg::SWCAckRptMsg() :
ReportMessage(SWC_ACK_RPT_MSG_ID)
{
    memset(&_data, 0, sizeof(_data));
}

STATUS SWCAckRptMsg::buildMsg()
{
    STATUS rc = OK;
    // Build header.

    if (ReportMessage::buildMsg() == ERROR)
    {
        return (ERROR);
    }

    // Byte swap from Local to Network
    _data.modType = (int)toNetworkInt(_data.modType);
    _data.ackType = (SWCAckType)toNetworkInt(_data.ackType);

    // Add data to message.
    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
