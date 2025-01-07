/**
* $Id: StatusRptMsg.cpp 1317 2008-07-28 16:15:59Z alv70669 $
*/
#include "StatusRptMsg.h"

StatusRptMsg::StatusRptMsg(RIMSMessageId msgId) :
RIMSReportMessage(msgId)
{
    memset(&_data, 0, sizeof(_data));
}

STATUS StatusRptMsg::buildMsg()
{
    STATUS rc = OK;
    // Build header.

    if (RIMSReportMessage::buildMsg() == ERROR)
    {
        return (ERROR);
    }

    // Add data to RIMS message.

    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
