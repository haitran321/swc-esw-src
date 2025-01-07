/**
* $Id: RIMSCommandMessage.h 5101 2009-12-04 20:55:24Z nei18232 $
*
* Class RIMSCommandMessage is the base class that represents 
* input messages received from RIMS. It provides methods common 
* to all input RIMS messages. It is derived from the base class 
* RIMSMessage. 
*
*/
#ifndef RIMSCommandMessage_H
#define RIMSCommandMessage_H

#include "CSPUMsgTypes.h"
#include "RIMSMessage.h"

class RIMSCommandMessage : public RIMSMessage
{
public:

   /**
   * Constructor
   *
   */
   RIMSCommandMessage();

   /**
   * Constructor
   *
   * @param buffer Data buffer to initialize RIMS Command message 
   *               with
   * @param bufSize Data buffer size
   */
   RIMSCommandMessage(char *buffer, int bufSize);

   /**
   * Destructor
   *
   */
   virtual ~RIMSCommandMessage(){};

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
