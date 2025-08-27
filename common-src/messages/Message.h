#ifndef Message_H
#define Message_H

#include <cstdio>
#include <string.h>

#include "SWCMsgTypes.h"
#include "EndianUtils.h"


/** Maximum size of a RIMS message, in bytes. */
const int MAX_MSG_SIZE = 4000;

/**
 * Class Message is the base class that represents messages 
 * received from/sent to SWCR. It provides methods common to all
 * messages. 
 */
class Message
{
public:

    /**
     * Constructor
     */
    Message();

    /**
     * Constructor
     *
     * @param buffer Data buffer to initialize RIMS message with
     * @param bufSize Data buffer size
     */
    Message(char *buffer, int bufSize);

    /**
     * Constructor
     *
     * @param msgId RIMS message id
     */
    explicit Message(MessageId msgId);

    /**
     * Method setTotalMsgSize sets the total number of bytes received
     * for this RIMS messages.
     *
     * @param size Message size
     */
    inline void setTotalMsgSize(int size);

    /**
     * Method getBuf returns a pointer to the message.
     *
     * @return Pointer to message
     */
    inline char *getBuf();

    /**
     * Method getBufSize returns the message size as reported in the
     * header.
     *
     * @return message size
     */
    inline unsigned int getBufSize();

    /**
     * Method getDataSize returns the number of bytes in the data
     * section following the header.
     *
     * @return message data size
     */
    inline unsigned int getDataSize();

    /**
     * Method getHeaderSize returns the size of the message header.
     *
     * @return message header size
     */
    inline unsigned int getHeaderSize() const;

    /**
     * Method getMsgId returns the message identifier as reported in
     * the header.
     *
     * @return message identifier
     */
    inline MessageId getMsgId();

    /**
     * Method getRecNum returns the number of data records as
     * reported in the message header.
     *
     * @return Number of data records
     */
    inline int getRecNum();
    
    /**
     * Method getHeaderTime returns the timestamps on the message as reported in 
     * the message header. 
     * 
     * @return UTCTimeType Message timestamp.
     */
    inline UTCTimeType getHeaderTime();
    
    /**
     * Method getDataBufPos returns a pointer to the data portion of
     * the messgae.
     *
     * @return Pointer to message
     */
    inline char *getDataBufPos();

    /**
     * Returns a pointer to the data portion of the RIMS message.
     * @return Pointer to data
     */
    inline const char* getDataBufPos() const;

    /**
     * Method isHeaderValid determines if the message header is
     * valid.
     *
     * @return Flag indicating if message header is valid
     */
    bool isHeaderValid();

    /**
     * Method getPBPId returns the PBP id as reported in the message
     * header.
     *
     * @return PBP id
     */
    inline int getPBPId();

    void byteSwapHeaderToLocal();
    
    /**
     * Writes a hex dump of the message contents to @p stream.
     * @param stream The output stream to dump the message to
     */
    void dump(FILE* stream);

protected:

    /** Message header */
    MsgHeaderType *_header;

    /** Message buffer */
    char _buffer[MAX_MSG_SIZE];

    /** Current message data position */
    int _curDataPos;

    /** Total message size */
    int _totalMsgSize;
};

void Message::setTotalMsgSize(int size)
{
    _totalMsgSize = size;
}

char *Message::getBuf()
{
    return(static_cast<char *>(_buffer));
}

unsigned int Message::getBufSize()
{
    return(_header->msgLen);
}

MessageId Message::getMsgId()
{
    return(_header->msgId);
}

unsigned int Message::getHeaderSize() const
{
    return(sizeof(MsgHeaderType));
}

char *Message::getDataBufPos()
{
    return(&_buffer[getHeaderSize()]);
}

const char* Message::getDataBufPos() const
{
    return &_buffer[getHeaderSize()];
}

unsigned int Message::getDataSize()
{
    return(_header->msgLen - getHeaderSize());
}

inline int Message::getPBPId()
{
    return(_header->pbpId);
}

inline int Message::getRecNum()
{
    return(_header->recNum);
}

inline UTCTimeType Message::getHeaderTime()
{
    return(_header->time);
}

#endif
