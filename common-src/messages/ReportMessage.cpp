#include "ReportMessage.h"

ReportMessage::ReportMessage()
{
}

ReportMessage::ReportMessage(MessageId msgId) :
Message(msgId)
{
}

STATUS ReportMessage::buildMsg()
{
   // Update message size.

   _header->msgLen = sizeof(MsgHeaderType);

   // Update header fields.

   // IRIGDevice::getTime(_header->time);
   _header->recNum = 0;
   _header->recLen = 0;
   _header->recvId = 0;

   return(OK);
}

STATUS ReportMessage::addData(const char *data, int size)
{
   // Validate that data can fit into RIMS message, and
   // that msgLen has been initialized.

   if (_header->msgLen + size > MAX_MSG_SIZE ||
       _header->msgLen < static_cast<int>(sizeof(MsgHeaderType)))
   {
      return(ERROR);
   }

   // Validate a different size record is not being written to the
   // data area. Variable length records are not allowed.

   if (_header->recLen != 0 && _header->recLen != size)
   {
      return(ERROR);
   }
   else
   {
      _header->recLen = size;
   }

   // Copy data after header in RIMS message.

   memcpy(&_buffer[_header->msgLen], data, size);

   // Update header fields.

   _header->msgLen += size;
   _header->recNum++;
   return(OK);
}

void ReportMessage::headerByteSwapToNetwork()
{
    _header->msgId = (MessageId)toNetworkInt(_header->msgId);
    _header->pbpId = toNetworkInt(_header->pbpId);
    // WARNING:  This will make the message incorrect
    _header->msgLen = toNetworkInt(_header->msgLen);    
    _header->recNum = toNetworkInt(_header->recNum);
    _header->recLen = toNetworkInt(_header->recLen);
    _header->recvId = toNetworkInt(_header->recvId);

    // What about time, does it need to be swapped?
}

