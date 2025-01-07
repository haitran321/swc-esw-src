/**
* $Id: RIMSMessage.h 6264 2010-09-21 18:57:59Z tra18693 $
* @file
*/
#ifndef RIMSMessage_H
#define RIMSMessage_H

#include <cstdio>
#include <string.h>

#include "CSPUMsgTypes.h"
#include "EndianUtils.h"


/** Maximum size of a RIMS message, in bytes. */
const int MAX_RIMS_MSG_SIZE = 4000;

/**
 * Class RIMSMessage is the base class that represents messages 
 * received from/sent to RIMS. It provides methods common to all 
 * RIMS messages. 
 */
class RIMSMessage
{
public:

    /**
     * Constructor
     */
    RIMSMessage();

    /**
     * Constructor
     *
     * @param buffer Data buffer to initialize RIMS message with
     * @param bufSize Data buffer size
     */
    RIMSMessage(char *buffer, int bufSize);

    /**
     * Constructor
     *
     * @param msgId RIMS message id
     */
    explicit RIMSMessage(RIMSMessageId msgId);

    /**
     * Method setTotalMsgSize sets the total number of bytes received
     * for this RIMS messages.
     *
     * @param size Message size
     */
    inline void setTotalMsgSize(int size);

    /**
     * Method getBuf returns a pointer to the RIMS message.
     *
     * @return Pointer to RIMS message
     */
    inline char *getBuf();

    /**
     * Method getBufSize returns the RIMS message size as reported in
     * the header.
     *
     * @return RIMS message size
     */
    inline unsigned int getBufSize();

    /**
     * Method getDataSize returns the number of bytes in the data
     * section following the header.
     *
     * @return RIMS message data size
     */
    inline unsigned int getDataSize();

    /**
     * Method getHeaderSize returns the size of the RIMS message
     * header.
     *
     * @return RIMS message header size
     */
    inline unsigned int getHeaderSize() const;

    /**
     * Method getMsgId returns the RIMS message identifier as
     * reported in the header.
     *
     * @return RIMS message identifier
     */
    inline RIMSMessageId getMsgId();

    /**
     * Method getRecNum returns the number of data records as
     * reported in the RIMS message header.
     *
     * @return Number of data records
     */
    inline int getRecNum();
    
    /**
     * Method getHeaderTime returns the timestamps on the message as reported in 
     * the RIMS message header. 
     * 
     * @return UTCTimeType Message timestamp.
     */
    inline UTCTimeType getHeaderTime();
    
    /**
     * Method getDataBufPos returns a pointer to the data portion of
     * the RIMS messgae.
     *
     * @return Pointer to RIMS message
     */
    inline char *getDataBufPos();

    /**
     * Returns a pointer to the data portion of the RIMS message.
     * @return Pointer to data
     */
    inline const char* getDataBufPos() const;

    /**
     * Method isHeaderValid determines if the RIMS message header is
     * valid.
     *
     * @return Flag indicating if RIMS message header is valid
     */
    bool isHeaderValid();

    /**
     * Method getPBPId returns the PBP id as reported in the RIMS
     * message header.
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

    /** RIMS message header */
    RIMSHeaderType *_header;

    /** RIMS message buffer */
    char _buffer[MAX_RIMS_MSG_SIZE];

    /** Current RIMS message data position */
    int _curDataPos;

    /** Total RIMS message size */
    int _totalMsgSize;
};

void RIMSMessage::setTotalMsgSize(int size)
{
    _totalMsgSize = size;
}

char *RIMSMessage::getBuf()
{
    return(static_cast<char *>(_buffer));
}

unsigned int RIMSMessage::getBufSize()
{
    return(_header->msgLen);
}

RIMSMessageId RIMSMessage::getMsgId()
{
    return(_header->msgId);
}

unsigned int RIMSMessage::getHeaderSize() const
{
    return(sizeof(RIMSHeaderType));
}

char *RIMSMessage::getDataBufPos()
{
    return(&_buffer[getHeaderSize()]);
}

const char* RIMSMessage::getDataBufPos() const
{
    return &_buffer[getHeaderSize()];
}

unsigned int RIMSMessage::getDataSize()
{
    return(_header->msgLen - getHeaderSize());
}

inline int RIMSMessage::getPBPId()
{
    return(_header->pbpId);
}

inline int RIMSMessage::getRecNum()
{
    return(_header->recNum);
}

inline UTCTimeType RIMSMessage::getHeaderTime()
{
    return(_header->time);
}

#endif
