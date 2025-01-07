/**
* $Id: ShutdownCmdMsg.h 5101 2009-12-04 20:55:24Z nei18232 $
*
* Class ShutdownCmdMsg represents a Shutdown Command message 
* sent to RIMS and is derived from class RIMSCommandMessage. It 
* provides methods to manipulate the body of a Shutdown Command
* message. 
*
*/
#ifndef ShutdownCmdMsg_H
#define ShutdownCmdMsg_H

#include <string>

using namespace std;

#include "CSPUMsgTypes.h"
#include "RIMSCommandMessage.h"

class ShutdownCmdMsg : public RIMSCommandMessage
{
public:

   /**
   * Constructor
   *
   * @param buffer Raw message buffer
   * @param bufSize Raw message buffer size
   */
   ShutdownCmdMsg(char *buffer, int bufSize);

   /**
   * Destructor
   *
   */
   virtual ~ShutdownCmdMsg(){};

   /**
   * Method validateData validates data from the data section of 
   * the raw message to the member variables. 
   *
   * @return Status of operation
   */
   virtual STATUS validateData();

   /**
   * Method getType returns the shutdown type field.
   *
   * @return Shutdown type value
   */
   inline ShutdownOption getType();

   void byteSwapToLocal();

private:

   /** Shutodown command data */

   ShutdownCmdDataType *_data;

};

ShutdownOption ShutdownCmdMsg::getType()
{
   return(_data->type);
}


#endif
