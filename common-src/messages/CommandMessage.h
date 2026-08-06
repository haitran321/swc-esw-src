#ifndef CommandMessage_H
#define CommandMessage_H

#include "SWCMsgTypes.h"
#include "Message.h"

class CommandMessage : public Message
{
public:

   /**
   * Constructor
   *
   */
   CommandMessage();

   /**
   * Constructor
   *
   * @param buffer Data buffer to initialize RIMS Command message 
   *               with
   * @param bufSize Data buffer size
   */
   CommandMessage(char *buffer, int bufSize);

   /**
   * Destructor
   *
   */
   virtual ~CommandMessage(){};

   /**
   * Method validateData is a virtual function that is to be 
   * implemented by a derived command message class to validate 
   * data in a RIMS Command message data section. 
   *
   * @return Status of operation
   */
   virtual STATUS validateData();

};

#endif
