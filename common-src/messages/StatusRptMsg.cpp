#include "StatusRptMsg.h"

StatusRptMsg::StatusRptMsg(MessageId msgId) :
ReportMessage(msgId)
{
    memset(&_data, 0, sizeof(_data));
}

STATUS StatusRptMsg::buildMsg()
{
    STATUS rc = OK;
    // Build header.

    if (ReportMessage::buildMsg() == ERROR)
    {
        return (ERROR);
    }

    // Add data to RIMS message.

    rc = addData(reinterpret_cast<char *>(&_data), sizeof(_data));

    return rc;
}
