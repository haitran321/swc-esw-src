#ifndef ReportMessage_H
#define ReportMessage_H

#include "SWCMsgTypes.h"
#include "Message.h"

class ReportMessage : public Message
{
public:

   /**
   * Constructor
   *
   */
   ReportMessage();

   /**
   * Constructor
   *
   * @param msgId Message id for Report message
   */
   explicit ReportMessage(MessageId msgId);

   /**
   * Destructor
   *
   */
   virtual ~ReportMessage(){};

   /**
   * Method buildMsg is a virtual method that is implemented by a 
   * derived report message class. This method is called by the 
   * derived class to build the header of the RIMS Report message, 
   * and the derived class builds the data portion. 
   *
   * @return Status of operation
   */
   virtual STATUS buildMsg();

   void headerByteSwapToNetwork();

protected:

   /**
   * Method addData copies data from the input data buffer to the 
   * data portion of the RIMS Report message at the current data 
   * pointer. 
   *
   * @param data Data buffer to copy data from
   * @param size Number of bytes to copy
   * @return Status of operation
   */
   STATUS addData(const char *data, int size);

};

#endif
