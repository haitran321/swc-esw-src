/**
* $Id: StatusRequestCmdMsg.h 5101 2009-12-04 20:55:24Z nei18232
* $
*
* Class v represents a Shutdown Command message 
* sent to RIMS and is derived from class RIMSCommandMessage. It 
* provides methods to manipulate the body of a Shutdown Command
* message. 
*
*/
#ifndef StatusRequestCmdMsg_H
#define StatusRequestCmdMsg_H

#include <string>

using namespace std;

#include "SWCMsgTypes.h"
#include "CommandMessage.h"

class StatusRequestCmdMsg : public CommandMessage
{
public:

   /**
   * Constructor
   *
   * @param buffer Raw message buffer
   * @param bufSize Raw message buffer size
   */
   StatusRequestCmdMsg(char *buffer, int bufSize);

   /**
   * Destructor
   *
   */
   virtual ~StatusRequestCmdMsg(){};

   /**
   * Method validateData validates data from the data section of 
   * the raw message to the member variables. 
   *
   * @return Status of operation
   */
   virtual STATUS validateData();

   /**
   * Method getStatusRequestType returns the status request type
   * field.
   *
   * @return Status request type value
   */
   inline StatusRequestType getStatusRequestType();

   /**
   * Method getDCUNum returns the DCU Number type field.
   *
   * @return DCU Number value
   */
   inline int getDCUNum();

   void byteSwapToLocal();

private:

   /** Status Request command data */

   StatusRequestCmdDataType *_data;

};

StatusRequestType StatusRequestCmdMsg::getStatusRequestType()
{
    return (_data->requestType);
}

int StatusRequestCmdMsg::getDCUNum()
{
    return (_data->dcuNum);
}


#endif
