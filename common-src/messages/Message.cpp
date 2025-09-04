#include <iomanip>

#include "Message.h"

Message::Message() : _header(reinterpret_cast<MsgHeaderType *>(_buffer)), _curDataPos(0), _totalMsgSize(0)
{
    memset(_buffer, 0, sizeof(_buffer));
}

Message::Message(char *buffer, int bufSize) : _header(reinterpret_cast<MsgHeaderType *>(_buffer)), _curDataPos(sizeof(MsgHeaderType)),
_totalMsgSize(bufSize)
{
    // Validate input buffer size.
    if (bufSize > MAX_MSG_SIZE)
    {
        throw std::string("Invalid size for new Message");
    }

    // Copy input buffer.
    memset(_buffer, 0, sizeof(_buffer));
    memcpy(_buffer, buffer, bufSize);
}

Message::Message(MessageId msgId) : _header(reinterpret_cast<MsgHeaderType *>(_buffer)), _curDataPos(0),
_totalMsgSize(0)
{
    memset(_buffer, 0, sizeof(_buffer));
    _header->msgId = msgId;
    _header->msgLen = 0;
}

bool Message::isHeaderValid()
{
    // Validate total bytes read to header message length.
    if (_header->msgLen != _totalMsgSize)
    {
        printf("isHeaderValid: header and msgLen mismatch (msgLen = %d _totalMsgSize = %d)", 
                  _header->msgLen, _totalMsgSize);
        return(false);
    }

    // Validate header message length against header record
    // information.
    if (_header->msgLen != (static_cast<int>(sizeof(MsgHeaderType)) +
                           (_header->recLen * _header->recNum)))
    {
        printf("isHeaderValid: message size and msgLen = %d _totalMsgSize = %d", 
                  _header->msgLen, _totalMsgSize);
        return(false);
    }

    return(true);
}

void Message::byteSwapHeaderToLocal()
{
   _header->msgId = (MessageId)fromNetworkInt(_header->msgId);
   _header->msgLen = (int)fromNetworkInt(_header->msgLen);
   _header->recNum = (int)fromNetworkInt(_header->recNum);
   _header->recLen = (int)fromNetworkInt(_header->recLen);
}


void Message::dump(FILE* stream)
{
    char* ptr = getBuf();
    unsigned size = getBufSize();
    char* end = ptr + size;
    unsigned written = 0;

    for (; ptr != end; ptr++)
    {
        fprintf(stream, " %02X", static_cast<unsigned char>(*ptr));

        if ((++written % 16) == 0)
        {
            // Newline after every 16th byte
            fprintf(stream, "\n");
        }
    }

    // Terminate dump with a newline
    fprintf(stream, "\n");
}
