/**
* $Id: RIMSMessage.cpp 5758 2010-05-13 15:45:12Z alv70669 $
*/
#include <iomanip>

#include "RIMSMessage.h"

RIMSMessage::RIMSMessage() : _header(reinterpret_cast<RIMSHeaderType *>(_buffer)), _curDataPos(0), _totalMsgSize(0)
{
    memset(_buffer, 0, sizeof(_buffer));
}

RIMSMessage::RIMSMessage(char *buffer, int bufSize) : _header(reinterpret_cast<RIMSHeaderType *>(_buffer)), _curDataPos(sizeof(RIMSHeaderType)),
_totalMsgSize(bufSize)
{
    // Validate input buffer size.
    if (bufSize > MAX_RIMS_MSG_SIZE)
    {
        throw std::string("Invalid size for new RIMSMessage");
    }

    // Copy input buffer.
    memset(_buffer, 0, sizeof(_buffer));
    memcpy(_buffer, buffer, bufSize);
}

RIMSMessage::RIMSMessage(RIMSMessageId msgId) : _header(reinterpret_cast<RIMSHeaderType *>(_buffer)), _curDataPos(0),
_totalMsgSize(0)
{
    memset(_buffer, 0, sizeof(_buffer));
    _header->msgId = msgId;
    _header->msgLen = 0;
}

bool RIMSMessage::isHeaderValid()
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
    if (_header->msgLen != (static_cast<int>(sizeof(RIMSHeaderType)) +
                           (_header->recLen * _header->recNum)))
    {
        printf("isHeaderValid: message size and msgLen = %d _totalMsgSize = %d", 
                  _header->msgLen, _totalMsgSize);
        return(false);
    }

    return(true);
}

void RIMSMessage::byteSwapHeaderToLocal()
{
   _header->msgId = (RIMSMessageId)fromNetworkInt(_header->msgId);
   _header->pbpId = (int)fromNetworkInt(_header->pbpId);
   _header->msgLen = (int)fromNetworkInt(_header->msgLen);
   _header->recNum = (int)fromNetworkInt(_header->recNum);
   _header->recLen = (int)fromNetworkInt(_header->recLen);
   _header->recvId = (int)fromNetworkInt(_header->recvId);
}


void RIMSMessage::dump(FILE* stream)
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
