/**
* $Id: WriteRegCmdMsg.h 5101 2009-12-04 20:55:24Z nei18232 $
*
* Class WriteRegCmdMsg represents a Shutdown Command message 
* sent to RIMS and is derived from class RIMSCommandMessage. It 
* provides methods to manipulate the body of a Shutdown Command
* message. 
*
*/
#ifndef WriteRegCmdMsg_H
#define WriteRegCmdMsg_H

#include <string>

using namespace std;

#include "RIMSCommandMessage.h"
#include "InternalMsgTypes.h"

class WriteRegCmdMsg : public RIMSCommandMessage
{
public:

   /**
   * Constructor
   *
   * @param buffer Raw message buffer
   * @param bufSize Raw message buffer size
   */
   WriteRegCmdMsg(char *buffer, int bufSize);

   /**
   * Destructor
   *
   */
   virtual ~WriteRegCmdMsg(){};

   /**
   * Method validateData validates data from the data section of 
   * the raw message to the member variables. 
   *
   * @return Status of operation
   */
   virtual STATUS validateData();

   /**
   * Method getReg returns register number
   *
   * @return register number 
   */
   inline int getReg();

   /**
   * Method getVal returns value of the register 
   *
   * @return value
   */
   inline int getVal();

private:

   /** Shutodown command data */

   RegCmdDataType *_data;

};

int WriteRegCmdMsg::getReg()
{
   return(_data->reg);
}

int WriteRegCmdMsg::getVal()
{
   return(_data->val);
}


#endif
