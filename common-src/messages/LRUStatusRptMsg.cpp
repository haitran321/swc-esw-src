#include "LRUStatusRptMsg.h"

LRUStatusRptMsg::LRUStatusRptMsg(MessageId msgId) :
ReportMessage(LRU_STATUS_RPT_MSG_ID)
{
    memset(&_data, 0, sizeof(_data));
}

STATUS LRUStatusRptMsg::buildMsg()
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
